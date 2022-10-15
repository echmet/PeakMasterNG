#include "phadjusterinterface.h"

#include "backgroundgdmproxy.h"
#include "../gdm/conversion/conversion.h"

#include <echmetsyscomp.h>
#include <echmetcaes.h>
#include <echmetionprops.h>

#include <cassert>
#include <functional>

inline
void releaseChemicalSystem(ECHMET::SysComp::ChemicalSystem *p)
{
  ECHMET::SysComp::releaseChemicalSystem(*p);
  delete p;
}

inline
void releaseCalculatedProperties(ECHMET::SysComp::CalculatedProperties *p)
{
  ECHMET::SysComp::releaseCalculatedProperties(*p);
  delete p;
}

inline
void releaseSolverContext(ECHMET::CAES::SolverContext *p)
{
  if (p != nullptr)
    p->destroy();
}

inline
void releaseSolver(ECHMET::CAES::Solver *p)
{
  if (p != nullptr)
    p->destroy();
}

inline
void releaseIonPropsContext(ECHMET::IonProps::ComputationContext *p)
{
  if (p != nullptr)
    p->destroy();
}

using ChemicalSystemPtr = std::unique_ptr<ECHMET::SysComp::ChemicalSystem, decltype(&releaseChemicalSystem)>;
using CalculatedPropertiesPtr = std::unique_ptr<ECHMET::SysComp::CalculatedProperties, decltype(&releaseCalculatedProperties)>;
using IonPropsCompPtr = std::unique_ptr<ECHMET::IonProps::ComputationContext, decltype(&releaseIonPropsContext)>;
using SolverContextPtr = std::unique_ptr<ECHMET::CAES::SolverContext, decltype(&releaseSolverContext)>;
using SolverPtr = std::unique_ptr<ECHMET::CAES::Solver, decltype(&releaseSolver)>;

class CalculationContext {
public:
  CalculationContext(const BackgroundGDMProxy &GDMProxy,
                     const ECHMET::NonidealityCorrections _corrs) :
    chemSystem{nullptr, releaseChemicalSystem},
    calcProps{nullptr, releaseCalculatedProperties},
    ctx{nullptr, releaseSolverContext},
    solver{nullptr, releaseSolver},
    ionPropsCtx{nullptr, releaseIonPropsContext},
    corrs{_corrs}
  {
    auto makeSolverContext = [](auto &chemSystem) {
      ECHMET::CAES::SolverContext *_ctx;
      ECHMET::CAES::createSolverContext(_ctx, *chemSystem);

      return std::unique_ptr<ECHMET::CAES::SolverContext, decltype(&releaseSolverContext)>{_ctx, releaseSolverContext};
    };

    chemSystem = std::unique_ptr<ECHMET::SysComp::ChemicalSystem, decltype(&releaseChemicalSystem)>{new ECHMET::SysComp::ChemicalSystem{},
                                                                                                   releaseChemicalSystem};
    calcProps = std::unique_ptr<ECHMET::SysComp::CalculatedProperties, decltype(&releaseCalculatedProperties)>{new ECHMET::SysComp::CalculatedProperties{},
                                                                                                               releaseCalculatedProperties};
    auto backgroundIcVec = conversion::makeECHMETInConstituentVec(GDMProxy.gdmBackground());

    auto tRet = ECHMET::SysComp::makeComposition(*chemSystem, *calcProps, backgroundIcVec.get());
    if (tRet != ECHMET::RetCode::OK)
      throw pHAdjusterInterface::Exception{ECHMET::errorToString(tRet)};

    ctx = makeSolverContext(chemSystem);
    if (ctx == nullptr)
      throw pHAdjusterInterface::Exception{"Cannot create solver context"};

    solver = std::unique_ptr<ECHMET::CAES::Solver, decltype(&releaseSolver)>{ECHMET::CAES::createSolver(ctx.get(),
                                                                                                        ECHMET::CAES::Solver::DISABLE_THREAD_SAFETY,
                                                                                                        corrs),
                                                                             releaseSolver};
    if (solver == nullptr)
      throw pHAdjusterInterface::Exception{"Cannot create solver"};

    ionPropsCtx = std::unique_ptr<ECHMET::IonProps::ComputationContext, decltype(&releaseIonPropsContext)>{ECHMET::IonProps::makeComputationContext(*chemSystem,
                                                                                                                                                     ECHMET::IonProps::ComputationContext::DISABLE_THREAD_SAFETY),
                                                                                                           releaseIonPropsContext};
  }

  ChemicalSystemPtr chemSystem;
  CalculatedPropertiesPtr calcProps;
  SolverContextPtr ctx;
  SolverPtr solver;
  IonPropsCompPtr ionPropsCtx;
  ECHMET::NonidealityCorrections corrs;
};

inline
double calculatepH(const BackgroundGDMProxy &GDMProxy, CalculationContext *ctx)
{
  auto backgroundAcVec = conversion::makeECHMETAnalyticalConcentrationsVec(GDMProxy.gdmBackground(), 0, ctx->chemSystem->constituents);

  auto tRet = ctx->solver->estimateDistributionSafe(backgroundAcVec.get(), *ctx->calcProps);
  if (tRet != ECHMET::RetCode::OK)
    throw pHAdjusterInterface::Exception{"Estimator failure: " + std::string{ECHMET::errorToString(tRet)}};

  tRet = ctx->solver->solve(backgroundAcVec.get(), *ctx->calcProps, 1000);
  if (tRet != ECHMET::RetCode::OK)
    throw pHAdjusterInterface::Exception{"Solver failure: " + std::string{ECHMET::errorToString(tRet)}};

  return ECHMET::IonProps::calculatepH(ctx->ionPropsCtx.get(), ctx->corrs, *ctx->calcProps);
}


pHAdjusterInterface::pHAdjusterInterface(std::string constituentName, BackgroundGDMProxy &GDMProxy,
                                         const bool debyeHuckel, const bool onsagerFuoss) :
  m_constituentName{std::move(constituentName)},
  h_GDMProxy{GDMProxy},
  m_debyeHuckel{debyeHuckel},
  m_onsagerFuoss{onsagerFuoss}
{
  if (!h_GDMProxy.contains(m_constituentName))
    throw Exception{"Constituent " + m_constituentName + "is not present in BGE composition"};

  ECHMET::NonidealityCorrections corrs = ECHMET::defaultNonidealityCorrections();

  if (m_debyeHuckel)
    ECHMET::nonidealityCorrectionSet(corrs, ECHMET::NonidealityCorrectionsItems::CORR_DEBYE_HUCKEL);
  if (m_onsagerFuoss)
    ECHMET::nonidealityCorrectionSet(corrs, ECHMET::NonidealityCorrectionsItems::CORR_ONSAGER_FUOSS);

  m_ctx = new CalculationContext{h_GDMProxy, corrs};
}

pHAdjusterInterface::~pHAdjusterInterface()
{
  delete m_ctx;
}

double pHAdjusterInterface::adjustpH(const double targetpH)
{
  static const size_t MAX_ITERS{300};
  static const double CCORR_PREC{1.0e-6};

  const double cSample = h_GDMProxy.concentrations(m_constituentName).at(1);

  double cLeft{CCORR_PREC};
  double cRight = [&]() {
    auto bgeNames = h_GDMProxy.allBackgroundNames();

    if (bgeNames.size() == 1)
      return 500.0;

    double cMax{0.0};

    for (const auto &name : bgeNames) {
      double c = h_GDMProxy.concentrations(name).at(0);
      if (c > cMax)
        cMax = c;
    }

    double max = 50.0 * cMax;
    return max > 2000 ? 2000 : max;
  }();

  const double cOriginal{h_GDMProxy.concentrations(m_constituentName).front()};

  if (cRight < 2.0 * CCORR_PREC)
    cRight = 2.0 * CCORR_PREC;
  assert(cLeft < cRight);

  double cNow = (cRight - cLeft) / 2.0 + cLeft;

  std::vector<double> cVec{cLeft, cSample};
  auto restoreConc = [&]() {
    cVec[0] = cOriginal;
    h_GDMProxy.setConcentrations(m_constituentName, cVec);
  };

  const bool acidic = [&]() {
    try {
      cVec[0] = cLeft;
      h_GDMProxy.setConcentrations(m_constituentName, cVec);

      const double pHLeft = calculatepH(h_GDMProxy, m_ctx);

      cVec[0] = cRight;
      h_GDMProxy.setConcentrations(m_constituentName, cVec);

      const double pHRight = calculatepH(h_GDMProxy, m_ctx);

      return pHRight < pHLeft;
    } catch (Exception &) {
      restoreConc();

      throw;
    }
  }();

  size_t iters{0};
  cVec[0] = cNow;
  auto adjustCNow = [&]() -> std::function<void(const double)> {
    if (acidic) {
      return [&](const double pH) {
        if (pH > targetpH)
          cLeft = cNow;
        else
          cRight = cNow;
      };
    }
    return [&](const double pH) {
      if (pH > targetpH)
        cRight = cNow;
      else
        cLeft = cNow;
    };
  }();

  auto pHMatches = [&](const double pH) {
    return targetpH + CCORR_PREC > pH && targetpH - CCORR_PREC < pH;
  };

  try {
    h_GDMProxy.setConcentrations(m_constituentName, cVec);
    double pH = calculatepH(h_GDMProxy, m_ctx);

    while (!pHMatches(pH) && iters < MAX_ITERS) {
      adjustCNow(pH);
      cNow = (cRight - cLeft) / 2.0 + cLeft;
      cVec[0] = cNow;
      h_GDMProxy.setConcentrations(m_constituentName, cVec);

      pH = calculatepH(h_GDMProxy, m_ctx);

      iters++;
    }

    if (iters >= MAX_ITERS)
      throw Exception{"Maximum number of iterations exceeded"};

    return pH;
  } catch (Exception &) {
    restoreConc();

    throw;
  }
}
