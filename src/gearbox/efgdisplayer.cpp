#include "efgdisplayer.h"

EFGDisplayer::EFGDisplayer(const Executor &executor) :
  m_executor{executor}
{
}

void EFGDisplayer::operator()(const QVector<QPointF> &data, const std::vector<CalculatorInterface::TimeDependentZoneInformation> &tdzi, const CalculatorInterface::Signal &signal) const
{
  m_executor(data, tdzi, signal);
}
