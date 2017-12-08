#include "chargecombination.h"

bool gdm::operator==(ChargeCombination a, ChargeCombination b) noexcept
{
    return a.nucleusCharge == b.nucleusCharge
            && a.ligandCharge == b.ligandCharge;
}

bool gdm::operator!=(ChargeCombination a, ChargeCombination b) noexcept
{
    return !(a == b);
}

gdm::ChargeNumber gdm::charge(ChargeCombination obj, ConstituentType type) noexcept
{
    switch(type) {

    case ConstituentType::Nucleus:
        return obj.nucleusCharge;

    case ConstituentType::Ligand:
        return obj.ligandCharge;
    }
}

void gdm::setCharge(ChargeCombination& obj, ConstituentType type, ChargeNumber charge) noexcept
{
    switch(type) {

    case ConstituentType::Nucleus:
        obj.nucleusCharge = charge;
        break;

    case ConstituentType::Ligand:
        obj.ligandCharge = charge;
        break;
    }
}

