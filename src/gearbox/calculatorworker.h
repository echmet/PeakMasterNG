#ifndef CALCULATORWORKER_H
#define CALCULATORWORKER_H

#include "calculatorinterface.h"
#include "../ui/maincontrolwidget.h"

class CalculatorWorker : public QObject
{
  Q_OBJECT

public:
  CalculatorWorker(CalculatorInterface &calcIface, const bool ionicStrengthCorrection, bool &calcOk, QString &errorMsg, QObject *parent = nullptr);
public slots:
  void process();

private:
  CalculatorInterface &m_calcIface;
  const bool m_ionicStrengthCorrection;
  bool &m_calcOk;
  QString &m_errorMsg;

signals:
  void finished();
};

#endif // CALCULATORWORKER_H
