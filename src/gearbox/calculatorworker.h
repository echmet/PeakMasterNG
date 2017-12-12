#ifndef CALCULATORWORKER_H
#define CALCULATORWORKER_H

#include "calculatorinterface.h"
#include "../ui/maincontrolwidget.h"

class CalculatorWorker : public QObject
{
  Q_OBJECT

public:
  enum class CalculationResult {
    INVALID,
    PARTIAL,
    OK,
  };

  CalculatorWorker(CalculatorInterface &calcIface, const bool ionicStrengthCorrection);
  CalculationResult calcStatus() const;
  const QString &errorMsg() const;
public slots:
  void process();

private:
  CalculatorInterface &m_calcIface;
  const bool m_ionicStrengthCorrection;
  CalculationResult m_calcStatus;
  QString m_errorMsg;

signals:
  void finished(CalculatorWorker *me);
};

#endif // CALCULATORWORKER_H
