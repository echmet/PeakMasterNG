#include "mobility.h"

gdm::Mobility gdm::fromSI(double mobilityInSI) noexcept
{
    return mobilityInSI * 10e9;
}

double gdm::toSI(gdm::Mobility mobility) noexcept
{
    return mobility * 10e-9;
}
