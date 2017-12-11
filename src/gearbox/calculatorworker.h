#ifndef CALCULATORWORKER_H
#define CALCULATORWORKER_H

#include "calculatorinterface.h"
#include "../ui/maincontrolwidget.h"

class CalculatorWorker : public QObject
{
  Q_OBJECT

public:
  CalculatorWorker(CalculatorInterface &calcIface, const bool ionicStrengthCorrection);
public slots:
  bool calcOk() const;
  const QString &errorMsg() const;
  void process();

private:
  CalculatorInterface &m_calcIface;
  const bool m_ionicStrengthCorrection;
  bool m_calcOk;
  QString m_errorMsg;

signals:
  void finished(CalculatorWorker *me);
};

#endif // CALCULATORWORKER_H
