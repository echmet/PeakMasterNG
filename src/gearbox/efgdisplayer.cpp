#include "efgdisplayer.h"

EFGDisplayer::EFGDisplayer(const Executor &executor) :
  m_executor{executor}
{
}

void EFGDisplayer::operator()(const QVector<QPointF> &data, std::vector<CalculatorInterface::SpatialZoneInformation> &&szi, const CalculatorInterface::Signal &signal) const
{
  m_executor(data, std::move(szi), signal);
}
