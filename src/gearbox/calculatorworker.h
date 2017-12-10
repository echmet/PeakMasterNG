#ifndef CALCULATORWORKER_H
#define CALCULATORWORKER_H

#include "calculatorinterface.h"
#include "../ui/maincontrolwidget.h"

class CalculatorWorker : public QObject
{
  Q_OBJECT

public:
  CalculatorWorker(CalculatorInterface &calcIface, const MainControlWidget::RunSetup &rs,
                   const double EOFValue, const CalculatorInterface::EOFValueType EOFvt,
                   bool &calcOk, QString &errorMsg);
public slots:
  void process();

private:
  CalculatorInterface &m_calcIface;
  const MainControlWidget::RunSetup &m_rs;
  const double m_EOFValue;
  const CalculatorInterface::EOFValueType m_EOFvt;
  bool &m_calcOk;
  QString &m_errorMsg;

signals:
  void finished();
};

#endif // CALCULATORWORKER_H
