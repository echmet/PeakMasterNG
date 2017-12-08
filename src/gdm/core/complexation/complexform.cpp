#include "complexform.h"

#include <utility>
#include <cassert>

#include "../common/gdmexcept.h"

gdm::ComplexForm::ComplexForm(ChargeCombination charges, std::size_t maxCount, std::vector<double> pBs, std::vector<Mobility> mobilities)
    : _charges{charges},
      _pBs{std::move(pBs)},
      _mobilities{std::move(mobilities)}
{
    if(maxCount <= 0) throw InvalidArgument{"maxCount must be > 1"};
    if(_pBs.size() != maxCount) throw InvalidArgument{"pBs must be of size maxCount"};
    if(_mobilities.size() != maxCount) throw InvalidArgument{"mobilities must be of size maxCount"};

    for(std::size_t i=0; i < _mobilities.size(); ++i) {
        if(_charges.nucleusCharge + _charges.ligandCharge*static_cast<int>(i+1) == 0) {
            if(_mobilities[i] != 0.0) throw InvalidArgument{"Mobility for total charge zero must be zero"};
        }
        else {
            if(_mobilities[i] <= 0.0) throw InvalidArgument{"Mobilities for non-zero total charges must be positive"};
        }
    }
}

gdm::ChargeCombination gdm::ComplexForm::charges() const noexcept
{
    return _charges;
}

std::size_t gdm::ComplexForm::maxCount() const noexcept
{
    return _pBs.size();
}

const std::vector<double>& gdm::ComplexForm::pBs() const noexcept
{
    return _pBs;
}

const std::vector<gdm::Mobility>& gdm::ComplexForm::mobilities() const noexcept
{
    return _mobilities;
}

void gdm::ComplexForm::assign(ChargeCombination charges, std::size_t maxCount, const std::vector<double>& pBs, const std::vector<Mobility>& mobilities)
{
    *this = {charges, maxCount, pBs, mobilities};
}


void gdm::ComplexForm::assign(std::size_t maxCount, const std::vector<double>& pBs, const std::vector<Mobility>& mobilities)
{
    *this = {_charges, maxCount, pBs, mobilities};
}

bool gdm::operator==(const ComplexForm& a, const ComplexForm& b) noexcept
{
    return a.charges() == b.charges()
            && a.pBs() == b.pBs()
            && a.mobilities() == b.mobilities();
}

bool gdm::operator!=(const ComplexForm& a, const ComplexForm& b) noexcept
{
    return !(a == b);
}
