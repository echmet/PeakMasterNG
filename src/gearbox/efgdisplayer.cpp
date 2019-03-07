#include "efgdisplayer.h"

EFGDisplayer::EFGDisplayer(Executor executor) :
  m_executor{std::move(executor)}
{
}

void EFGDisplayer::operator()(const QVector<QPointF> &data, const std::vector<CalculatorInterface::TimeDependentZoneInformation> &tdzi, const CalculatorInterface::Signal &signal) const
{
  m_executor(data, tdzi, signal);
}
