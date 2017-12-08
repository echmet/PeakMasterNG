#ifndef GDM_GDM_H
#define GDM_GDM_H

#include "composition.h"

#include <cstddef>
#include <unordered_map>
#include <string>
#include <vector>

namespace gdm {

class GDM : private Composition
{
public:
    using Composition::const_iterator;
    using Composition::iterator;

    explicit GDM(std::size_t concentrationBlockCount = 1);

    //Iterators
    using Composition::begin;
    using Composition::cbegin;
    using Composition::end;
    using Composition::cend;

    //Capacity
    using Composition::empty;
    using Composition::size;

    //Element lookup
    using Composition::count;
    using Composition::find;

    //Modifiers
    void clear() noexcept;
    using Composition::insert;
    iterator erase(const_iterator pos) noexcept;

    //In-place element mutation
    std::pair<iterator, bool> update(const_iterator pos, const Constituent& value);

    //Access complexations
    using Composition::haveComplexation;
    using Composition::complexationBetween;

    //Edit complexations
    using Composition::setComplexation;
    using Composition::eraseComplexation;
    using Composition::clearComplexations;

    //Access concentrations
    std::size_t concentrationBlockCount() const noexcept;
    std::vector<double> concentrations(const_iterator pos) const noexcept;

    //Edit concentrations
    void setConcentrationBlockCount(std::size_t concentrationBlockCount);
    void setConcentrations(const_iterator pos, std::vector<double> concentrations);

    //Access underlying composition
    const Composition& composition() const noexcept;

private:
    std::size_t _concentrationBlockCount;
    std::unordered_map<std::string, std::vector<double>> _concentrations; //mapped vectors always have as many elements as the block count
};

bool operator==(const GDM& a, const GDM& b) noexcept;
bool operator!=(const GDM& a, const GDM& b) noexcept;

std::vector<std::pair<GDM::const_iterator, double>> concentrationBlock(const GDM& obj, std::size_t block);

} // namespace GDM

#endif //GDM_GDM_H
