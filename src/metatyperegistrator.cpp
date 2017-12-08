#include "gearbox/calculatorinterface.h"

#include <QMetaType>

void registerMetatypes()
{
  qRegisterMetaType<CalculatorInterface::Signal>("Signal");
}
