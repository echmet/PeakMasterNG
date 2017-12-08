#include "constituent_ui.h"

Constituent::Constituent(const int lowCharge, const int highCharge, const QString &name, const Type ctype) :
  lowCharge(lowCharge),
  highCharge(highCharge),
  name(name),
  ctype(ctype)
{
}
