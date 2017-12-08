#ifndef GDM_DETAIL_COMPLEXATIONMAPKEY_H
#define GDM_DETAIL_COMPLEXATIONMAPKEY_H

#include "../common/constituenttype.h"

#include <string>

namespace gdm {
namespace detail {

struct ComplexationMapKey //Constituents involved in a complexation are identified by their names
{
    std::string nucleusName;
    std::string ligandName;
};

bool operator==(const ComplexationMapKey& a, const ComplexationMapKey& b) noexcept;
bool operator!=(const ComplexationMapKey& a, const ComplexationMapKey& b) noexcept;


const std::string& name(const ComplexationMapKey& obj, ConstituentType type) noexcept;
void setName(ComplexationMapKey& obj, ConstituentType type, const std::string& name);

} // namespace detail
} // namespace gdm

namespace std {

template <> struct hash<gdm::detail::ComplexationMapKey>
{
    size_t operator()(const gdm::detail::ComplexationMapKey& obj) const noexcept;
};

} // namespace std

#endif // GDM_DETAIL_COMPLEXATIONMAPKEY_H
