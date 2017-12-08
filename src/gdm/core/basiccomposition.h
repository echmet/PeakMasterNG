#ifndef GDM_BASICCOMPOSITION_H
#define GDM_BASICCOMPOSITION_H

#include "constituent/constituent.h"

#include <set>
#include <string>

namespace gdm {

struct ConstituentNameCompare;

using BasicComposition = std::set<Constituent, ConstituentNameCompare>;

struct ConstituentNameCompare
{
    using is_transparent = void; //Tells std::set to enable comparisons of constituents with strings

    bool operator()(const Constituent& a, const Constituent& b) const noexcept;
    bool operator()(const Constituent& obj, const std::string& constituentName) const noexcept;
    bool operator()(const std::string& constituentName, const Constituent& obj) const noexcept;
};

} // namespace gdm

#endif // GDM_BASICCOMPOSITION_H
