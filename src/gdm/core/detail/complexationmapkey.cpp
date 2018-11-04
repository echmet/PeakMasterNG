#include "complexationmapkey.h"

#include <cassert>

const std::string& gdm::detail::name(const ComplexationMapKey& obj, ConstituentType type) noexcept
{
    switch(type) {

    case ConstituentType::Nucleus:
        return obj.nucleusName;

    case ConstituentType::Ligand:
        return obj.ligandName;
    }

    assert(false);
}

void gdm::detail::setName(ComplexationMapKey& obj, ConstituentType type, const std::string& name)
{
    switch(type) {

    case ConstituentType::Nucleus:
        obj.nucleusName = name;
        break;

    case ConstituentType::Ligand:
        obj.ligandName = name;
        break;
    }
}

bool gdm::detail::operator==(const gdm::detail::ComplexationMapKey& a, const gdm::detail::ComplexationMapKey& b) noexcept
{
    return a.nucleusName == b.nucleusName
            && a.ligandName == b.ligandName;
}

bool gdm::detail::operator!=(const gdm::detail::ComplexationMapKey& a, const gdm::detail::ComplexationMapKey& b) noexcept
{
    return !(a == b);
}

size_t std::hash<gdm::detail::ComplexationMapKey>::operator()(const gdm::detail::ComplexationMapKey& obj) const noexcept
{
    static const std::hash<std::string> stringHash{};
    return stringHash(obj.nucleusName) ^ stringHash(obj.ligandName);
}



