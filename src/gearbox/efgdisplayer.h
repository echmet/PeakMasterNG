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
  typedef std::function<void (const QVector<QPointF> &, std::vector<CalculatorInterface::SpatialZoneInformation> &&, const CalculatorInterface::Signal &)> Executor;

  explicit EFGDisplayer(const Executor &executor);

  void operator()(const QVector<QPointF> &data, std::vector<CalculatorInterface::SpatialZoneInformation> &&szi, const CalculatorInterface::Signal &sig) const;

private:
  const Executor m_executor;
};

#endif // EFGDISPLAYER_H
