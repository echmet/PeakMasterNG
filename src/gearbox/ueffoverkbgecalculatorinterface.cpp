// vim: sw=2 ts=2 sts=2 noexcept

#include "ueffoverkbgecalculatorinterface.h"

#include "backgroundgdmproxy.h"
#include "../gdm/conversion/conversion.h"
#include "../gdm/core/gdm.h"

#include <echmetsyscomp.h>
#include <echmetcaes.h>
#include <echmetionprops.h>
#include <lemng.h>

#include <cassert>
#include <cstring>
#include <functional>

using AcVec = std::unique_ptr<ECHMET::RealVec, void(*)(const ECHMET::RealVec*)>;

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
  CalculationContext(const gdm::GDM &GDM,
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
    auto backgroundIcVec = conversion::makeECHMETInConstituentVec(GDM);

    auto tRet = ECHMET::SysComp::makeComposition(*chemSystem, *calcProps, backgroundIcVec.get());
    if (tRet != ECHMET::RetCode::OK)
      throw uEffOverkBGECalculatorInterface::Exception{ECHMET::errorToString(tRet)};

    ctx = makeSolverContext(chemSystem);
    if (ctx == nullptr)
      throw uEffOverkBGECalculatorInterface::Exception{"Cannot create solver context"};

    solver = std::unique_ptr<ECHMET::CAES::Solver, decltype(&releaseSolver)>{
      ECHMET::CAES::createSolver(ctx.get(), ECHMET::CAES::Solver::DISABLE_THREAD_SAFETY, corrs),
      releaseSolver};
    if (solver == nullptr)
      throw uEffOverkBGECalculatorInterface::Exception{"Cannot create solver"};

    ionPropsCtx = std::unique_ptr<ECHMET::IonProps::ComputationContext, decltype(&releaseIonPropsContext)>{
      ECHMET::IonProps::makeComputationContext(*chemSystem, ECHMET::IonProps::ComputationContext::DISABLE_THREAD_SAFETY),
      releaseIonPropsContext};
  }

  size_t indexOfConstituent(const std::string &name)
  {
    const auto &constituents = chemSystem->constituents;
    for (size_t idx = 0; idx < constituents->size(); idx++) {
      const auto ctuent = constituents->at(idx);
      if (std::strcmp(name.c_str(), ctuent->name->c_str()) == 0)
        return idx;
    }

    throw uEffOverkBGECalculatorInterface::Exception{"Cannot find index of constituent " + name};
  }

  ChemicalSystemPtr chemSystem;
  CalculatedPropertiesPtr calcProps;
  SolverContextPtr ctx;
  SolverPtr solver;
  IonPropsCompPtr ionPropsCtx;
  ECHMET::NonidealityCorrections corrs;
};

inline
void solveSystem(const AcVec &acVec, CalculationContext *ctx)
{
  auto tRet = ctx->solver->estimateDistributionSafe(acVec.get(), *ctx->calcProps);
  if (tRet != ECHMET::RetCode::OK)
    throw uEffOverkBGECalculatorInterface::Exception{"Estimator failure: " + std::string{ECHMET::errorToString(tRet)}};

  tRet = ctx->solver->solve(acVec.get(), *ctx->calcProps, 1000);
  if (tRet != ECHMET::RetCode::OK)
    throw uEffOverkBGECalculatorInterface::Exception{"Solver failure: " + std::string{ECHMET::errorToString(tRet)}};

  tRet = ECHMET::IonProps::correctMobilities(ctx->ionPropsCtx.get(), ctx->corrs , acVec.get(), *ctx->calcProps);
  if (tRet != ECHMET::RetCode::OK)
    throw uEffOverkBGECalculatorInterface::Exception{"Cannot correct ionic mobilities: " + std::string{ECHMET::errorToString(tRet)}};

  tRet = ECHMET::IonProps::calculateEffectiveMobilities(ctx->ionPropsCtx.get(), acVec.get(), *ctx->calcProps);
  if (tRet != ECHMET::RetCode::OK)
    throw uEffOverkBGECalculatorInterface::Exception{"Cannot calculate effective mobilities: " + std::string{ECHMET::errorToString(tRet)}};
}

inline
double calculateuEffOverkBGE(const AcVec &acVec, CalculationContext *ctx, const size_t ctuentIdx)
{
  solveSystem(acVec, ctx);

  const double kBGE = ECHMET::IonProps::calculateConductivity(ctx->ionPropsCtx.get(), *ctx->calcProps);
	if (kBGE <= 0.0)
		throw uEffOverkBGECalculatorInterface::Exception{"Failed to calculate conductivity"};

  const double uEff = ctx->calcProps->effectiveMobilities->elem(ctuentIdx);

  return uEff/kBGE;
}


uEffOverkBGECalculatorInterface::uEffOverkBGECalculatorInterface(const BackgroundGDMProxy &GDMProxy,
                                                         const bool debyeHuckel, const bool onsagerFuoss) :
  h_GDMProxy{GDMProxy}
{
  ECHMET::NonidealityCorrections corrs = ECHMET::defaultNonidealityCorrections();

  if (debyeHuckel)
    ECHMET::nonidealityCorrectionSet(corrs, ECHMET::NonidealityCorrectionsItems::CORR_DEBYE_HUCKEL);
  if (onsagerFuoss)
    ECHMET::nonidealityCorrectionSet(corrs, ECHMET::NonidealityCorrectionsItems::CORR_ONSAGER_FUOSS);

  m_corrections = static_cast<int>(corrs);

  m_ctx = new CalculationContext{h_GDMProxy.gdmBackground(), corrs};
}

uEffOverkBGECalculatorInterface::~uEffOverkBGECalculatorInterface()
{
  delete m_ctx;
}

std::map<std::string, double> uEffOverkBGECalculatorInterface::currentuEkBs()
{
	static const double ANALYTE_C{ECHMET::LEMNG::minimumSafeConcentration()};

  auto backgroundAcVec = conversion::makeECHMETAnalyticalConcentrationsVec(h_GDMProxy.gdmBackground(), 0, m_ctx->chemSystem->constituents);
  solveSystem(backgroundAcVec, m_ctx);

  const double kBGE = ECHMET::IonProps::calculateConductivity(m_ctx->ionPropsCtx.get(), *m_ctx->calcProps);
	if (kBGE <= 0.0)
		throw uEffOverkBGECalculatorInterface::Exception{"Failed to calculate conductivity"};

	std::map<std::string, double> uEkBs;
	{
		const auto &constituents = m_ctx->chemSystem->constituents;
		for (size_t idx = 0; idx < constituents->size(); idx++) {
			const auto ctuent = constituents->elem(idx);
			const double uEkB = m_ctx->calcProps->effectiveMobilities->elem(idx) / kBGE;

			uEkBs[ctuent->name->c_str()] = uEkB;
		}
	}

	// We need to make a new ad-hoc GDM that will contain analytes as "fake" background components.
	// We need to do this to calculate uEff/kBGE for analytes.
	gdm::GDM adhocGDM = h_GDMProxy.gdmBackground();
	for (const auto &name : h_GDMProxy.allAnalyteNames()) {
		const auto &analyte = h_GDMProxy.get(name);
		auto inserted = adhocGDM.insert(analyte);
		adhocGDM.setConcentrations(inserted.first, { ANALYTE_C });
	}

	CalculationContext adhocCtx{adhocGDM, static_cast<ECHMET::NonidealityCorrections>(m_corrections)};
	auto adhocAcVec = conversion::makeECHMETAnalyticalConcentrationsVec(adhocGDM, 0, adhocCtx.chemSystem->constituents);
	solveSystem(adhocAcVec, &adhocCtx);

	{
		const auto &constituents = adhocCtx.chemSystem->constituents;
		for (size_t idx = 0; idx < constituents->size(); idx++) {
			const auto ctuent = constituents->elem(idx);
			std::string name = ctuent->name->c_str();

			// Dumb way how to check that we are not trying to add a background component again
			if (uEkBs.find(name) != uEkBs.end())
				continue;

			const double uEkB = adhocCtx.calcProps->effectiveMobilities->elem(idx) / kBGE;
			uEkBs[std::move(name)] = uEkB;
		}
	}

	return uEkBs;
}

double uEffOverkBGECalculatorInterface::findConcentration(const std::string &constituentName, const double targetuEkB)
{
  static const size_t MAX_ITERS{300};
  static const double NR_PREC{1.0e-6};
  static const double NR_DELTA{1.0e-7};
  static const double NR_MINIMUM_C{10.0 * ECHMET::LEMNG::minimumSafeConcentration()};

  const size_t ctuentIdx = m_ctx->indexOfConstituent(constituentName);

  auto backgroundAcVec = conversion::makeECHMETAnalyticalConcentrationsVec(h_GDMProxy.gdmBackground(), 0, m_ctx->chemSystem->constituents);

  size_t iters{0};
  double cActual = h_GDMProxy.concentrations(constituentName).front();
  while (iters < MAX_ITERS) {
    (*backgroundAcVec)[ctuentIdx] = cActual;
    double uEkB = calculateuEffOverkBGE(backgroundAcVec, m_ctx, ctuentIdx);
    if (std::abs(uEkB - targetuEkB) <= NR_PREC)
      return cActual;

    double cLow = cActual > (2.0 * NR_DELTA) ? cActual - NR_DELTA : cActual;
    (*backgroundAcVec)[ctuentIdx] = cLow;
    double uEkBLow = calculateuEffOverkBGE(backgroundAcVec, m_ctx, ctuentIdx);

    double cHigh = cLow + 2.0 * NR_DELTA;
    (*backgroundAcVec)[ctuentIdx] = cHigh;
    double uEkBHigh = calculateuEffOverkBGE(backgroundAcVec, m_ctx, ctuentIdx);

    double duEkBdC = (uEkBHigh - uEkBLow) / (cHigh - cLow);
    double cNew = cActual - (uEkB - targetuEkB) / duEkBdC;
    cActual = cNew < NR_MINIMUM_C ? NR_MINIMUM_C : cNew;

    iters++;
  }

  throw Exception{"Maximum number of iterations exceeded"};
}
