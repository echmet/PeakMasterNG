#include "calculatorworker.h"

CalculatorWorker::CalculatorWorker(CalculatorInterface &calcIface, const MainControlWidget::RunSetup &rs,
                                   bool &calcOk, QString &errorMsg) :
  QObject{nullptr},
  m_calcIface{calcIface},
  m_rs{rs},
  m_calcOk(calcOk),
  m_errorMsg{errorMsg}
{
}

void CalculatorWorker::process()
{
  try {
    m_calcIface.calculate(m_rs.totalLength, m_rs.detectorPosition,
                          m_rs.drivingVoltage,
                          m_rs.positiveVoltage, m_rs.ionicStrengthCorrection);
    m_calcOk = true;
  } catch (CalculatorInterfaceException &ex) {
    m_calcOk = false;
    m_errorMsg = ex.what();
  }

  emit finished();
}
