#ifndef EFGDISPLAYER_H
#define EFGDISPLAYER_H

#include <QPointF>
#include <QVector>
#include <functional>
#include <vector>
#include "calculatorinterface.h"

class EFGDisplayer
{
public:
  typedef std::function<void (const QVector<QPointF> &, const std::vector<CalculatorInterface::TimeDependentZoneInformation> &, const CalculatorInterface::Signal &)> Executor;

  explicit EFGDisplayer(Executor executor);

  void operator()(const QVector<QPointF> &data, const std::vector<CalculatorInterface::TimeDependentZoneInformation> &szi, const CalculatorInterface::Signal &sig) const;

private:
  const Executor m_executor;
};

#endif // EFGDISPLAYER_H
