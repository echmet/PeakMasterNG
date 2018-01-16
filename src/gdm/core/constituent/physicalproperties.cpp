#include "physicalproperties.h"

#include <utility>
#include <iterator>
#include <cassert>

#include "../common/gdmexcept.h"

gdm::PhysicalProperties::PhysicalProperties(ChargeInterval charges,
                                            std::vector<double> pKas,
                                            std::vector<Mobility> mobilities,
                                            double viscosityCoefficient)
    : _charges{charges},
      _pKas{std::move(pKas)},
      _mobilities{std::move(mobilities)},
      _viscosityCoefficient{viscosityCoefficient}
{
    if(charges.empty()) throw InvalidArgument{"charges must not be empty"};
    if(_pKas.size() != size(_charges) - 1) throw InvalidArgument{"There must be as many pKas as the number of charges minus one"};
    if(_mobilities.size() != size(_charges)) throw InvalidArgument{"There must be as many mobilities as charges"};
    if(_viscosityCoefficient < 0.0) throw InvalidArgument{"Viscosity coefficient must not be negative"};
    for(auto c = _charges.low(); c <= _charges.high(); ++c) {
        if(c == 0 && _mobilities[mobilityIndex(c)] != 0.0) throw InvalidArgument{"Mobility for charge zero must be zero"};
        if(c != 0 && _mobilities[mobilityIndex(c)] <= 0.0) throw InvalidArgument{"Mobilities for charges other than zero must be positive"};
    }
}

gdm::ChargeInterval gdm::PhysicalProperties::charges() const noexcept
{
    return _charges;
}

double gdm::PhysicalProperties::pKa(ChargeNumber targetCharge) const noexcept
{
    assert(contains(_charges, targetCharge)); assert(targetCharge != 0);
    if(_charges.low() > 0) assert(_charges.low() < targetCharge);
    if(_charges.high() < 0) assert(targetCharge < _charges.high());

    return _pKas[pKaIndex(targetCharge)];
}

gdm::Mobility gdm::PhysicalProperties::mobility(ChargeNumber charge) const noexcept
{
    assert(contains(_charges, charge));

    auto ret = _mobilities[mobilityIndex(charge)];

    if(charge == 0) assert(ret == 0.0);
    else assert(ret > 0.0);

    return ret;
}

gdm::ChargeNumber gdm::PhysicalProperties::pushLowCharge(double pKa, Mobility mobility)
{
    if(_charges.low() - 1 == 0) {
        if(mobility != 0.0) throw InvalidArgument{"Mobility for charge zero must be zero"};
    }
    else {
        if(mobility <= 0.0) throw InvalidArgument{"Mobility for non-zero charge must be positive"};
    }

    //Make a copy in case insertions fail
    auto mobilitiesCopy = _mobilities;
    auto pKasCopy = _pKas;

    mobilitiesCopy.insert(mobilitiesCopy.begin(), mobility);
    pKasCopy.insert(pKasCopy.begin(), pKa);

    //Commit the changes
    _charges.setLow(_charges.low() - 1);
    _mobilities = std::move(mobilitiesCopy);
    _pKas = std::move(pKasCopy);

    assert(!_charges.empty());
    assert(_mobilities.size() == size(_charges));
    assert(_mobilities.size() == _pKas.size() + 1);

    return _charges.low(); //Return the new charge
}

gdm::ChargeNumber gdm::PhysicalProperties::pushHighCharge(double pKa, Mobility mobility)
{
    if(_charges.high() + 1 == 0) {
        if(mobility != 0.0) throw InvalidArgument{"Mobility for charge zero must be zero"};
    }
    else {
        if(mobility <= 0.0) throw InvalidArgument{"Mobility for non-zero charge must be positive"};
    }

    //Make a copy in case insertions fail
    auto mobilitiesCopy = _mobilities;
    auto pKasCopy = _pKas;

    mobilitiesCopy.push_back(mobility);
    pKasCopy.push_back(pKa);

    //Commit the changes
    _charges.setHigh(_charges.high() + 1);
    _mobilities = std::move(mobilitiesCopy);
    _pKas = std::move(pKasCopy);

    assert(!_charges.empty());
    assert(_mobilities.size() == size(_charges));
    assert(_mobilities.size() == _pKas.size() + 1);

    return _charges.high(); //Return the new charge
}

gdm::ChargeNumber gdm::PhysicalProperties::popLowCharge()
{
    if(size(_charges) == 1) throw OutOfRange{"Cannot pop the last remaining charge"};

    _mobilities.erase(_mobilities.begin());
    _pKas.erase(_pKas.begin());
    _charges.setLow(_charges.low() + 1);

    assert(!_charges.empty());
    assert(_mobilities.size() == size(_charges));
    assert(_mobilities.size() == _pKas.size() + 1);

    return _charges.low() - 1;
}

gdm::ChargeNumber gdm::PhysicalProperties::popHighCharge()
{
    if(size(_charges) == 1) throw OutOfRange{"Cannot pop the last remaining charge"};

    _mobilities.pop_back();
    _pKas.pop_back();
    _charges.setHigh(_charges.high() - 1);

    assert(!_charges.empty());
    assert(_mobilities.size() == size(_charges));
    assert(_mobilities.size() == _pKas.size() + 1);

    return _charges.high() + 1;
}

void gdm::PhysicalProperties::setMobility(ChargeNumber charge, Mobility mobility)
{
    if(!contains(_charges, charge)) throw InvalidArgument{"charge is not present"};
    if(charge == 0) {
        if(mobility != 0.0) throw InvalidArgument{"Mobility for charge zero must be zero"};
    }
    else {
        if(mobility <= 0.0) throw InvalidArgument{"Mobility for non-zero charge must be positive"};
    }

    _mobilities[mobilityIndex(charge)] = mobility;
}

void gdm::PhysicalProperties::setPKa(ChargeNumber targetCharge, double pKa)
{
    if(targetCharge == 0
            || !contains(_charges, targetCharge)
            || (_charges.low() > 0 && _charges.low() == targetCharge)
            || (_charges.high() < 0 && targetCharge == _charges.high())) {
        throw InvalidArgument{"Not a valid target charge"};
    }

    _pKas[pKaIndex(targetCharge)] = pKa;
}

const std::vector<gdm::Mobility>& gdm::PhysicalProperties::mobilities() const noexcept
{
    return _mobilities;
}

const std::vector<double>& gdm::PhysicalProperties::pKas() const noexcept
{
    return _pKas;
}

double gdm::PhysicalProperties::viscosityCoefficient() const noexcept
{
  return _viscosityCoefficient;
}

void gdm::PhysicalProperties::assign(ChargeInterval charges, const std::vector<double>& pKas, const std::vector<double>& mobilities, double viscosityCoefficient)
{
    *this = PhysicalProperties{charges, pKas, mobilities, viscosityCoefficient};
}

std::vector<gdm::Mobility>::size_type gdm::PhysicalProperties::mobilityIndex(ChargeNumber charge) const noexcept
{
    assert(contains(_charges, charge));

    auto index = static_cast<std::vector<double>::size_type>(charge - _charges.low());

    assert(index >= 0); assert(index < _mobilities.size());

    return index;
}

std::vector<double>::size_type gdm::PhysicalProperties::pKaIndex(ChargeNumber targetCharge) const noexcept
{
    assert(contains(_charges, targetCharge)); assert(targetCharge != 0);
    if(_charges.low() > 0) assert(_charges.low() < targetCharge);
    if(_charges.high() < 0) assert(targetCharge < _charges.high());

    auto index = mobilityIndex(targetCharge);

    if(targetCharge > 0) --index; //Adjust index iff target is positive

    assert(index >= 0); assert(index < _pKas.size());

    return index;
}

bool gdm::operator==(const PhysicalProperties& a, const PhysicalProperties& b) noexcept
{
   return a.charges() == b.charges()
           && a.pKas() == b.pKas()
           && a.mobilities() == b.mobilities()
           && a.viscosityCoefficient() == b.viscosityCoefficient();
}

bool gdm::operator!=(const PhysicalProperties& a, const PhysicalProperties& b) noexcept
{
    return !(a == b);
}
