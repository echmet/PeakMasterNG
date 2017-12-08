#include "constituent.h"

#include <functional>
#include <cassert>

#include "../common/gdmexcept.h"

gdm::Constituent::Constituent(ConstituentType type, std::string name, PhysicalProperties physicalProperties)
    : _type{type},
      _name{std::move(name)},
      _physicalProperties{std::move(physicalProperties)}
{
    if(_name.empty()) throw InvalidArgument{"Name of constituent must not be empty"};
}

gdm::ConstituentType gdm::Constituent::type() const noexcept
{
    return _type;
}

const std::string& gdm::Constituent::name() const noexcept
{
    return _name;
}

const gdm::PhysicalProperties& gdm::Constituent::physicalProperties() const noexcept
{
    return _physicalProperties;
}

void gdm::Constituent::setType(ConstituentType type) noexcept
{
    _type = type;
}

void gdm::Constituent::setName(const std::string& name)
{
    if(name.empty()) throw InvalidArgument{"Name of constituent must not be empty"};

    _name = name;
}

void gdm::Constituent::setPhysicalProperties(const PhysicalProperties& physicalProperties)
{
    _physicalProperties = physicalProperties;
}

bool gdm::operator==(const Constituent& a, const Constituent& b) noexcept
{
    return a.type() == b.type()
            && a.name() == b.name()
            && a.physicalProperties() == b.physicalProperties();
}

bool gdm::operator!=(const Constituent& a, const Constituent& b) noexcept
{
    return !(a == b);
}
