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
    PARTIAL_BGE,
    PARTIAL_EIGENZONES,
    OK,
  };

  CalculatorWorker(CalculatorInterface &calcIface, const bool correctForDebyeHuckel, const bool correctForOnsagerFuoss, const bool correctForViscosity);
  CalculationResult calcResult() const;
  const QString & errorMsg() const;

public slots:
  void process();

private:
  CalculatorInterface &m_calcIface;
  const bool m_correctForDebyeHuckel;
  const bool m_correctForOnsagerFuoss;
  const bool m_correctForViscosity;
  CalculationResult m_calcResult;
  QString m_errorMsg;

signals:
  void finished(CalculatorWorker *me);
};

#endif // CALCULATORWORKER_H
