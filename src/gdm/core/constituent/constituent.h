#ifndef GDM_CONSTITUENT_H
#define GDM_CONSTITUENT_H

#include "../common/constituenttype.h"
#include "physicalproperties.h"

#include <string>

namespace gdm {

class Constituent
{
public:
    /*implicit*/ Constituent(ConstituentType type, std::string name, PhysicalProperties physicalProperties = {});

    ConstituentType type() const noexcept;
    const std::string& name() const noexcept;
    const PhysicalProperties& physicalProperties() const noexcept;

    void setType(ConstituentType type) noexcept;
    void setName(const std::string& name); //name must not empty
    void setPhysicalProperties(const PhysicalProperties& physicalProperties);

private:
    ConstituentType _type;
    std::string _name;
    PhysicalProperties _physicalProperties;
};

bool operator==(const Constituent& a, const Constituent& b) noexcept;
bool operator!=(const Constituent& a, const Constituent& b) noexcept;

} // namespace gdm


#endif // GDM_CONSTITUENT_H
