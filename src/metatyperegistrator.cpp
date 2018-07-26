#include "gearbox/calculatorinterface.h"
#include "softwareupdateresult.h"

#include <QMetaType>

void registerMetatypes()
{
  qRegisterMetaType<CalculatorInterface::Signal>("Signal");
  qRegisterMetaType<SoftwareUpdateResult>("SoftwareUpdateResult");
}
