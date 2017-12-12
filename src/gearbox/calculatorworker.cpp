#include "calculatorworker.h"

CalculatorWorker::CalculatorWorker(CalculatorInterface &calcIface, const bool ionicStrengthCorrection) :
  m_calcIface{calcIface},
  m_ionicStrengthCorrection{ionicStrengthCorrection}
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
    m_calcIface.calculate(m_ionicStrengthCorrection);
    m_calcStatus = CalculationResult::OK;
  } catch (CalculatorInterfaceException &ex) {
    m_calcStatus = ex.isBGEValid ? CalculationResult::PARTIAL : CalculationResult::INVALID;
    m_errorMsg = ex.what();
  }

  emit finished(this);
}
