#include "calculatorworker.h"

CalculatorWorker::CalculatorWorker(CalculatorInterface &calcIface, const bool correctForDeybeHuckel, const bool correctForOnsagerFuoss, const bool correctForViscosity) :
  m_calcIface{calcIface},
  m_correctForDebyeHuckel{correctForDeybeHuckel},
  m_correctForOnsagerFuoss{correctForOnsagerFuoss},
  m_correctForViscosity{correctForViscosity}
{
}

CalculatorWorker::CalculationResult CalculatorWorker::calcStatus() const
{
  return m_calcStatus;
}

const QString & CalculatorWorker::errorMsg() const
{
  return m_errorMsg;
}

void CalculatorWorker::process()
{
  try {
    m_calcIface.calculate(m_correctForDebyeHuckel, m_correctForOnsagerFuoss, m_correctForViscosity);
    m_calcStatus = CalculationResult::OK;
  } catch (CalculatorInterfaceException &ex) {
    m_calcStatus = ex.isBGEValid ? CalculationResult::PARTIAL : CalculationResult::INVALID;
    m_errorMsg = ex.what();
  }

  emit finished(this);
}
