#include "calculatorworker.h"

CalculatorWorker::CalculatorWorker(CalculatorInterface &calcIface, const bool correctForDeybeHuckel, const bool correctForOnsagerFuoss, const bool correctForViscosity) :
  m_calcIface{calcIface},
  m_correctForDebyeHuckel{correctForDeybeHuckel},
  m_correctForOnsagerFuoss{correctForOnsagerFuoss},
  m_correctForViscosity{correctForViscosity}
{
}

CalculatorWorker::CalculationResult CalculatorWorker::calcResult() const
{
  return m_calcResult;
}

const QString & CalculatorWorker::errorMsg() const
{
  return m_errorMsg;
}

void CalculatorWorker::process()
{
  try {
    m_calcIface.calculate(m_correctForDebyeHuckel, m_correctForOnsagerFuoss, m_correctForViscosity);
    m_calcResult = CalculationResult::OK;
  } catch (CalculatorInterfaceException &ex) {
    m_calcResult = [](const CalculatorInterfaceException::SolutionState state) {
      switch (state) {
      case CalculatorInterfaceException::SolutionState::INVALID:
        return CalculationResult::INVALID;
      case CalculatorInterfaceException::SolutionState::BGE_ONLY:
        return CalculationResult::PARTIAL_BGE;
      case CalculatorInterfaceException::SolutionState::PARTIAL_EIGENZONES:
        return CalculationResult::PARTIAL_EIGENZONES;
      };

      throw std::runtime_error("Unhandled solution state value");
    }(ex.state);

    m_errorMsg = ex.what();
  }

  emit finished(this);
}
