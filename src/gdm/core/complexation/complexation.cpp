#include "complexation.h"
#include "complexform.h"

#include <numeric>
#include <cassert>

gdm::ChargeInterval gdm::chargeSpan(const Complexation& obj, ConstituentType type) noexcept
{
    return std::accumulate(obj.begin(), obj.end(),
                           ChargeInterval{},
                           [type](auto accum, const auto& complexForm)
                            {
                                return hull(accum, charge(complexForm.charges(), type));
                            });
}

void gdm::clamp(Complexation& obj, ConstituentType type, ChargeInterval interval)
{
    for(auto it = obj.cbegin(); it != obj.cend(); /*nothing*/) { //If-erase loop
        if(!contains(interval, charge(it->charges(), type))) {
            it = obj.erase(it);
        }
        else ++it;
    }

    assert(isClamped(obj, type, interval));
}

bool gdm::isClamped(const Complexation& obj, ConstituentType type, ChargeInterval interval) noexcept
{
    return contains(interval, chargeSpan(obj, type));
}


bool gdm::ChargeCombinationCompare::operator()(const gdm::ComplexForm& a, const gdm::ComplexForm& b) const noexcept
{
    return operator()(a.charges(), b.charges());
}

bool gdm::ChargeCombinationCompare::operator()(const gdm::ComplexForm& obj, gdm::ChargeCombination charges) const noexcept
{
    return operator()(obj.charges(), charges);
}

bool gdm::ChargeCombinationCompare::operator()(gdm::ChargeCombination charges, const gdm::ComplexForm& obj) const noexcept
{
    return operator()(charges, obj.charges());
}

bool gdm::ChargeCombinationCompare::operator()(gdm::ChargeCombination chargesA, gdm::ChargeCombination chargesB) const noexcept
{
    return chargesA.nucleusCharge < chargesB.nucleusCharge
            || (chargesA.nucleusCharge == chargesB.nucleusCharge && chargesA.ligandCharge < chargesB.ligandCharge);
}
