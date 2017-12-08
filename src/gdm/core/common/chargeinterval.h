#ifndef GDM_CHARGEINTERVAL_H
#define GDM_CHARGEINTERVAL_H

#include "chargenumber.h"

#include <cstddef>

namespace gdm {

class ChargeInterval
{
public:
    /*implicit*/ ChargeInterval() noexcept; //Constructs an empty interval
    /*implicit*/ ChargeInterval(ChargeNumber charge) noexcept; //Constructs a singleton interval containing only charge
    /*implicit*/ ChargeInterval(ChargeNumber low, ChargeNumber high) noexcept; //Constructs the [low, high] interval (empty if high > low)

    bool empty() const noexcept;

    ChargeNumber low() const; //Must be non-empty
    ChargeNumber high() const; //Must be non-empty

    void clear() noexcept; //Makes the interval empty
    void setLow(ChargeNumber charge); //Must be non-empty (otherwise use operator= instead). Will clear the interval if charge > high()
    void setHigh(ChargeNumber charge); //Must be non-empty (otherwise use operator= instead). Will clear the interval if charge < low()

private:
    ChargeNumber _low;
    ChargeNumber _high;
};

bool operator==(ChargeInterval a, ChargeInterval b) noexcept;
bool operator!=(ChargeInterval a, ChargeInterval b) noexcept;

std::size_t size(ChargeInterval obj) noexcept;
std::size_t sizeExcludingZero(ChargeInterval obj) noexcept; //Doesn't count charge zero if present

bool contains(ChargeInterval obj, ChargeInterval other) noexcept;

ChargeInterval intersect(ChargeInterval a, ChargeInterval b) noexcept;
ChargeInterval hull(ChargeInterval a, ChargeInterval b) noexcept;

} // namespace gdm

#endif // GDM_CHARGEINTERVAL_H
