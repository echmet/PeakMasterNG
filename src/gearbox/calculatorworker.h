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

  CalculatorWorker(CalculatorInterface &calcIface, const bool correctForDebyeHuckel, const bool correctForOnsagerFuoss, const bool correctForViscosity,
                   const std::vector<CalculatorInterface::TracepointState> &tracepointStates,
                   const std::string &traceOutputFile);
  CalculationResult calcStatus() const;
  const QString &errorMsg() const;
  bool traceWrittenOk() const;
public slots:
  void process();

private:
  CalculatorInterface &m_calcIface;
  const bool m_correctForDebyeHuckel;
  const bool m_correctForOnsagerFuoss;
  const bool m_correctForViscosity;
  const std::vector<CalculatorInterface::TracepointState> m_tracepointStates;
  const std::string m_traceOutputFile;
  CalculationResult m_calcStatus;
  bool m_traceWrittenOk;
  QString m_errorMsg;

signals:
  void finished(CalculatorWorker *me);
};

#endif // CALCULATORWORKER_H
