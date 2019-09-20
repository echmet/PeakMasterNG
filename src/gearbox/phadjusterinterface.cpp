#include "phadjusterinterface.h"

#include "backgroundgdmproxy.h"
#include "../gdm/conversion/conversion.h"

#include <echmetsyscomp.h>
#include <echmetcaes.h>
#include <echmetionprops.h>

#include <cassert>
#include <functional>

void releaseChemicalSystem(ECHMET::SysComp::ChemicalSystem *p)
{
  ECHMET::SysComp::releaseChemicalSystem(*p);
  delete p;
}

void releaseCalculateProperties(ECHMET::SysComp::CalculatedProperties *p)
{
  ECHMET::SysComp::releaseCalculatedProperties(*p);
  delete p;
}

void releaseSolverContext(ECHMET::CAES::SolverContext *p)
{
  p->destroy();
}

void releaseSolver(ECHMET::CAES::Solver *p)
{
  p->destroy();
}

void releaseIonPropsContext(ECHMET::IonProps::ComputationContext *p)
{
  p->destroy();
}

pHAdjusterInterface::pHAdjusterInterface(std::string constituentName, BackgroundGDMProxy &GDMProxy,
                                         const bool debyeHuckel, const bool onsagerFuoss) :
  m_constituentName{std::move(constituentName)},
  h_GDMProxy{GDMProxy},
  m_debyeHuckel{debyeHuckel},
  m_onsagerFuoss{onsagerFuoss}
{
}

void pHAdjusterInterface::adjustpH(const double targetpH)
{
  static const size_t MAX_ITERS{300};
  static const double CCORR_PREC{1.0e-6};

  ECHMET::NonidealityCorrections corrs = ECHMET::defaultNonidealityCorrections();

  if (m_debyeHuckel)
    ECHMET::nonidealityCorrectionSet(corrs, ECHMET::NonidealityCorrectionsItems::CORR_DEBYE_HUCKEL);
  if (m_onsagerFuoss)
    ECHMET::nonidealityCorrectionSet(corrs, ECHMET::NonidealityCorrectionsItems::CORR_ONSAGER_FUOSS);

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
    return max > 3000 ? 3000 : max;
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

      const double pHLeft = calculatepH(corrs);

      cVec[0] = cRight;
      h_GDMProxy.setConcentrations(m_constituentName, cVec);

      const double pHRight = calculatepH(corrs);

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
    double pH = calculatepH(corrs);

    while (!pHMatches(pH) && iters < MAX_ITERS) {
      adjustCNow(pH);
      cNow = (cRight - cLeft) / 2.0 + cLeft;
      cVec[0] = cNow;
      h_GDMProxy.setConcentrations(m_constituentName, cVec);

      pH = calculatepH(corrs);

      iters++;
    }
  } catch (Exception &) {
    restoreConc();

    throw;
  }

  if (iters >= MAX_ITERS) {
    restoreConc();

    throw Exception{"Maximum number of iterations exceeded"};
  }
}

double pHAdjusterInterface::calculatepH(const ECHMET::NonidealityCorrections corrs)
{
  auto makeSolverContext = [](auto &chemSystem) {
    ECHMET::CAES::SolverContext *ctx;
    ECHMET::CAES::createSolverContext(ctx, *chemSystem);

    return std::unique_ptr<ECHMET::CAES::SolverContext, decltype(&releaseSolverContext)>{ctx, releaseSolverContext};
  };

  auto chemSystem = std::unique_ptr<ECHMET::SysComp::ChemicalSystem, decltype(&releaseChemicalSystem)>{new ECHMET::SysComp::ChemicalSystem{},
                                                                                                       releaseChemicalSystem};
  auto calcProps = std::unique_ptr<ECHMET::SysComp::CalculatedProperties, decltype(&releaseCalculateProperties)>{new ECHMET::SysComp::CalculatedProperties{},
                                                                                                                 releaseCalculateProperties};
  auto backgroundIcVec = conversion::makeECHMETInConstituentVec(h_GDMProxy.gdmBackground());

  auto tRet = ECHMET::SysComp::makeComposition(*chemSystem, *calcProps, backgroundIcVec.get());
  if (tRet != ECHMET::RetCode::OK)
    throw Exception{ECHMET::errorToString(tRet)};

  auto backgroundAcVec = conversion::makeECHMETAnalyticalConcentrationsVec(h_GDMProxy.gdmBackground(), 0, chemSystem->constituents);

  auto ctx = makeSolverContext(chemSystem);
  if (ctx == nullptr)
    throw Exception{"Cannot create solver context"};

  auto solver = std::unique_ptr<ECHMET::CAES::Solver, decltype(&releaseSolver)>{ECHMET::CAES::createSolver(ctx.get(),
                                                                                                           ECHMET::CAES::Solver::DISABLE_THREAD_SAFETY,
                                                                                                           corrs),
                                                                                releaseSolver};
  if (solver == nullptr)
    throw Exception{"Cannot create solver"};

  tRet = solver->estimateDistributionSafe(backgroundAcVec.get(), *calcProps);
  if (tRet != ECHMET::RetCode::OK)
    throw Exception{"Estimator failure: " + std::string{ECHMET::errorToString(tRet)}};

  tRet = solver->solve(backgroundAcVec.get(), *calcProps, 1000);
  if (tRet != ECHMET::RetCode::OK)
    throw Exception{"Solver failure: " + std::string{ECHMET::errorToString(tRet)}};

  auto ionPropsCtx = std::unique_ptr<ECHMET::IonProps::ComputationContext, decltype(&releaseIonPropsContext)>{ECHMET::IonProps::makeComputationContext(*chemSystem,
                                                                                                                                                       ECHMET::IonProps::ComputationContext::DISABLE_THREAD_SAFETY),
                                                                                                              releaseIonPropsContext};
  return ECHMET::IonProps::calculatepH(ionPropsCtx.get(), corrs, *calcProps);
}
