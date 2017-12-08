#ifndef GDM_CHARGECOMBINATION_H
#define GDM_CHARGECOMBINATION_H

#include "../common/chargenumber.h"
#include "../common/constituenttype.h"

#include <functional>

namespace gdm {

struct ChargeCombination
{
    ChargeNumber nucleusCharge;
    ChargeNumber ligandCharge;
};

bool operator==(ChargeCombination a, ChargeCombination b) noexcept;
bool operator!=(ChargeCombination a, ChargeCombination b) noexcept;

ChargeNumber charge(ChargeCombination obj, ConstituentType type) noexcept;
void setCharge(ChargeCombination& obj, ConstituentType type, ChargeNumber charge) noexcept;

} // namespace gdm

#endif // GDM_CHARGECOMBINATION_H
