#include "calculatorworker.h"

CalculatorWorker::CalculatorWorker(CalculatorInterface &calcIface, const bool correctForDeybeHuckel, const bool correctForOnsagerFuoss, const bool correctForViscosity,
                                   const std::vector<CalculatorInterface::TracepointState> &tracepointStates,
                                   const std::string &traceOutputFile) :
  m_calcIface{calcIface},
  m_correctForDebyeHuckel{correctForDeybeHuckel},
  m_correctForOnsagerFuoss{correctForOnsagerFuoss},
  m_correctForViscosity{correctForViscosity},
  m_tracepointStates{tracepointStates},
  m_traceOutputFile{traceOutputFile}
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
    m_calcIface.calculate(m_correctForDebyeHuckel, m_correctForOnsagerFuoss, m_correctForViscosity,
                          m_tracepointStates,
                          m_traceOutputFile,
                          m_traceWrittenOk);
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
    }(ex.state);

    m_errorMsg = ex.what();
  }

  emit finished(this);
}

bool CalculatorWorker::traceWrittenOk() const
{
  return m_traceWrittenOk;
}
