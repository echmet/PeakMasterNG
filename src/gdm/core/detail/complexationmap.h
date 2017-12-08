#ifndef GDM_DETAIL_COMPLEXATIONMAP_H
#define GDM_DETAIL_COMPLEXATIONMAP_H

#include "../complexation/complexation.h"
#include "complexationmapkey.h"

#include <unordered_map>

namespace gdm {
namespace detail {

using ComplexationMap = std::unordered_map<ComplexationMapKey, Complexation>;

ComplexationMap findAll(const ComplexationMap& obj, ConstituentType constituentType, const std::string& constituentName); //Returns the subset of complexations that involve a constituent
ChargeInterval chargeSpan(const ComplexationMap& obj, ConstituentType constituentType, const std::string& constituentName); //Returns the charge interval that covers all complexations that involve a constituent
void eraseAll(ComplexationMap& obj, ConstituentType constituentType, const std::string& constituentName) noexcept; //Removes all complexations of a constituent
void clampAll(ComplexationMap& obj, ConstituentType constituentType, const std::string& constituentName, ChargeInterval interval) noexcept; //Clamps all complexations that involve a constituent
void rename(ComplexationMap& obj, ConstituentType constituentType, const std::string& constituentName, const std::string& newName); //Renames a constituent across all entries

} // namespace detail
} // namespace gdm

#endif // GDM_DETAIL_COMPLEXATIONMAP_H
