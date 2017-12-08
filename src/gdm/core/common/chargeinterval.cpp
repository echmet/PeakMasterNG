#include "chargeinterval.h"

#include <algorithm>
#include <cassert>

#include "gdmexcept.h"

gdm::ChargeInterval::ChargeInterval() noexcept
    : _low(0),
      _high(-1)
{
    assert(empty());
}

gdm::ChargeInterval::ChargeInterval(ChargeNumber charge) noexcept
    : _low(charge),
      _high(charge)
{
    assert(!empty());
}

gdm::ChargeInterval::ChargeInterval(ChargeNumber low, ChargeNumber high) noexcept
    : _low(low),
      _high(high)
{}

bool gdm::ChargeInterval::empty() const noexcept
{
    return _high < _low;
}

gdm::ChargeNumber gdm::ChargeInterval::low() const
{
    if(empty()) throw LogicError{"Trying to read bound of an empty interval"};

    return _low;
}

gdm::ChargeNumber gdm::ChargeInterval::high() const
{
    if(empty()) throw LogicError{"Trying to read bound of an empty interval"};

    return _high;
}

void gdm::ChargeInterval::setLow(ChargeNumber charge)
{
    if(empty()) throw LogicError{"Trying to set one bound of an empty interval"};

    _low = charge;
}

void gdm::ChargeInterval::setHigh(ChargeNumber charge)
{
    if(empty()) throw LogicError{"Trying to set one bound of an empty interval"};

    _high = charge;
}

void gdm::ChargeInterval::clear() noexcept
{
    *this = {};

    assert(empty());
}

bool gdm::operator==(ChargeInterval a, ChargeInterval b) noexcept
{
    if(a.empty()) return b.empty();
    else if(b.empty()) return false;
    else return a.low() == b.low()
                && a.high() == b.high();
}

bool gdm::operator!=(ChargeInterval a, ChargeInterval b) noexcept
{
    return !(a == b);
}

std::size_t gdm::size(ChargeInterval obj) noexcept
{
    if(obj.empty()) return 0;
    else return static_cast<std::size_t>(obj.high() - obj.low() + 1);
}

std::size_t gdm::sizeExcludingZero(ChargeInterval obj) noexcept
{
    if(contains(obj, 0)) return size(obj) - 1;
    else return size(obj);
}

bool gdm::contains(ChargeInterval obj, ChargeInterval other) noexcept
{
    if(other.empty()) return true;
    else if(obj.empty()) return false;
    else return obj.low() <= other.low()
                && other.high() <= obj.high();
}

gdm::ChargeInterval gdm::hull(ChargeInterval a, ChargeInterval b) noexcept
{
    if(a.empty()) return b;
    else if(b.empty()) return a;
    else return {std::min(a.low(), b.low()), std::max(a.high(), b.high())};
}

gdm::ChargeInterval gdm::intersect(ChargeInterval a, ChargeInterval b) noexcept
{
    if(a.empty() || b.empty()) return {};
    else return {std::max(a.low(), b.low()), std::min(a.high(), b.high())};
}
