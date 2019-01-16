#include "complexationmapkey.h"

#include <cassert>
#include "../../../msvc_hacks.h"

const std::string& gdm::detail::name(const ComplexationMapKey& obj, ConstituentType type) noexcept
{
    switch(type) {

    case ConstituentType::Nucleus:
        return obj.nucleusName;

    case ConstituentType::Ligand:
        return obj.ligandName;

    }

    IMPOSSIBLE_PATH;
}

void gdm::detail::setName(ComplexationMapKey& obj, ConstituentType type, const std::string& name)
{
    switch(type) {

    case ConstituentType::Nucleus:
        obj.nucleusName = name;
        return;

    case ConstituentType::Ligand:
        obj.ligandName = name;
        return;
    }

    IMPOSSIBLE_PATH;
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



