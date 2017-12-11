#include "calculatorworker.h"

CalculatorWorker::CalculatorWorker(CalculatorInterface &calcIface, const bool ionicStrengthCorrection) :
  m_calcIface{calcIface},
  m_ionicStrengthCorrection{ionicStrengthCorrection}
{
}

bool CalculatorWorker::calcOk() const
{
  return m_calcOk;
}

const QString & CalculatorWorker::errorMsg() const
{
  return m_errorMsg;
}

void CalculatorWorker::process()
{
  try {
    m_calcIface.calculate(m_ionicStrengthCorrection);
    m_calcOk = true;
  } catch (CalculatorInterfaceException &ex) {
    m_calcOk = false;
    m_errorMsg = ex.what();
  }

  emit finished(this);
}
