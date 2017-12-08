#ifndef GDM_COMPOSITION_H
#define GDM_COMPOSITION_H

#include "basiccomposition.h"
#include "detail/complexationmap.h"

#include <vector>
#include <utility>

namespace gdm {

class Composition : private BasicComposition
{
public:
    using BasicComposition::const_iterator;
    using BasicComposition::iterator;

    /*implicit*/ Composition() = default;

    //Iterators
    using BasicComposition::begin;
    using BasicComposition::cbegin;
    using BasicComposition::end;
    using BasicComposition::cend;

    //Capacity
    using BasicComposition::empty;
    using BasicComposition::size;

    //Element lookup
    using BasicComposition::count;
    using BasicComposition::find;

    //Modifiers
    void clear() noexcept;
    using BasicComposition::insert;
    iterator erase(const_iterator pos) noexcept;

    //In-place element mutation
    std::pair<iterator,bool> update(const_iterator pos, const Constituent& value); //Keeps complexations if constituent's type does not change (but they are clamped)

    //Access complexations
    bool haveComplexation(const_iterator pos1, const_iterator pos2) const;
    const Complexation& complexationBetween(const_iterator pos1, const_iterator pos2) const;

    //Edit complexations
    void setComplexation(const_iterator pos1, const_iterator pos2, Complexation complexation); //Given complexation is clamped to the nucleus' and ligand's valid charges
    void eraseComplexation(const_iterator pos1, const_iterator pos2);
    void clearComplexations() noexcept;

    //Access underlying constituents
    const BasicComposition& constituents() const noexcept;

    friend bool operator==(const Composition& a, const Composition& b) noexcept;

private:
    detail::ComplexationMap _complexations;
};

bool operator==(const Composition& a, const Composition& b) noexcept;
bool operator!=(const Composition& a, const Composition& b) noexcept;

std::vector<std::pair<Composition::const_iterator, Complexation>> findComplexations(const Composition& obj, Composition::const_iterator pos);

} // namespace gdm

#endif // GDM_COMPOSITION_H
