#include "calculatorworker.h"

CalculatorWorker::CalculatorWorker(CalculatorInterface &calcIface, const bool ionicStrengthCorrection, bool &calcOk, QString &errorMsg,
                                   QObject *parent) :
  QObject{parent},
  m_calcIface{calcIface},
  m_ionicStrengthCorrection{ionicStrengthCorrection},
  m_calcOk(calcOk),
  m_errorMsg{errorMsg}
{
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

  emit finished();
}
