#include "constituent_ui.h"

Constituent::Constituent(const int lowCharge, const int highCharge, QString name, const Type ctype) :
  lowCharge(lowCharge),
  highCharge(highCharge),
  name(std::move(name)),
  ctype(ctype)
{
}
