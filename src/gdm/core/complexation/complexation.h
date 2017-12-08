#ifndef GDM_COMPLEXFORMSET_H
#define GDM_COMPLEXFORMSET_H

#include "complexform.h"
#include "../common/chargeinterval.h"

#include <set>

namespace gdm {

struct ChargeCombinationCompare;

using Complexation = std::set<ComplexForm, ChargeCombinationCompare>;

ChargeInterval chargeSpan(const Complexation& obj, ConstituentType type) noexcept; //Returns an interval that covers all charges of the type in the complexation
void clamp(Complexation& obj, ConstituentType type, ChargeInterval interval); //Removes complex forms with charges outside the given interval for the type
bool isClamped(const Complexation& obj, ConstituentType type, ChargeInterval interval) noexcept;


struct ChargeCombinationCompare
{
    using is_transparent = void; //Tells std::set to enable comparisons of complex forms with charge combinations

    bool operator()(const ComplexForm& a, const ComplexForm& b) const noexcept;
    bool operator()(const ComplexForm& obj, ChargeCombination charges) const noexcept;
    bool operator()(ChargeCombination charges, const ComplexForm& obj) const noexcept;

private:
    bool operator()(ChargeCombination chargesA, ChargeCombination chargesB) const noexcept;
};

} // namespace gdm

#endif // GDM_COMPLEXFORMSET_H
