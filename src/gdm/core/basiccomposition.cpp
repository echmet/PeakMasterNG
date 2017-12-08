#include "basiccomposition.h"

bool gdm::ConstituentNameCompare::operator()(const gdm::Constituent& a, const gdm::Constituent& b) const noexcept
{
    return a.name() < b.name();
}

bool gdm::ConstituentNameCompare::operator()(const gdm::Constituent& obj, const std::string& constituentName) const noexcept
{
    return obj.name() < constituentName;
}

bool gdm::ConstituentNameCompare::operator()(const std::string& constituentName, const gdm::Constituent& obj) const noexcept
{
    return constituentName < obj.name();
}

