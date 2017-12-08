#include "gdm.h"

#include <cassert>
#include <utility>
#include <algorithm>

#include "common/gdmexcept.h"

gdm::GDM::GDM(std::size_t concentrationBlockCount)
    : _concentrationBlockCount{concentrationBlockCount}
{
    if(concentrationBlockCount <= 0) throw InvalidArgument{"concentrationBlockCount must be positive"};
}

void gdm::GDM::clear() noexcept
{
    Composition::clear();
    _concentrations.clear();
}

gdm::GDM::iterator gdm::GDM::erase(const_iterator pos) noexcept
{
    _concentrations.erase(pos->name());
    return Composition::erase(pos);
}

std::pair<gdm::GDM::iterator,bool> gdm::GDM::update(const_iterator pos, const Constituent& value)
{
    auto oldName = pos->name();
    auto ret = Composition::update(pos, value);

    bool updated = ret.second;
    if(updated && value.name() != oldName) {
        //auto concentrations = _concentrations[value.name()]; <- Original code
        auto concentrations = _concentrations.at(oldName);
        _concentrations.erase(oldName);
        _concentrations.insert({value.name(), std::move(concentrations)});
    }

    return ret;
}

std::size_t gdm::GDM::concentrationBlockCount() const noexcept
{
    return _concentrationBlockCount;
}

std::vector<double> gdm::GDM::concentrations(const_iterator pos) const noexcept
{
    auto it = _concentrations.find(pos->name());
    if(it != _concentrations.end()) {
        assert(it->second.size() == _concentrationBlockCount);
        return it->second;
    }
    else return std::vector<double>(_concentrationBlockCount);
}

void gdm::GDM::setConcentrationBlockCount(std::size_t concentrationBlockCount)
{
    if(concentrationBlockCount <= 0) throw InvalidArgument{"concentrationBlockCount must be positive"};

    for(auto& c : _concentrations) {
        c.second.resize(concentrationBlockCount);
    }
}

void gdm::GDM::setConcentrations(const_iterator pos, std::vector<double> concentrations)
{
    concentrations.resize(_concentrationBlockCount);

    if(!std::all_of(concentrations.begin(), concentrations.end(), [](auto c){ return c >= 0.0; })) throw InvalidArgument{"concentration cannot be negative"};

    _concentrations[pos->name()] = std::move(concentrations);
}

const gdm::Composition& gdm::GDM::composition() const noexcept
{
    return *this;
}

bool gdm::operator==(const GDM& a, const GDM& b) noexcept
{
    if(a.composition() != b.composition()) return false;

    for(auto it=a.begin(); it != a.end(); ++it) {
        if(a.concentrations(it) != b.concentrations(b.find(it->name()))) return false;
    }

    return true;
}

std::vector<std::pair<gdm::GDM::const_iterator, double>> gdm::concentrationBlock(const GDM& obj, std::size_t block)
{
    if(block >= obj.concentrationBlockCount()) throw OutOfRange{"block out of range"};

    std::vector<std::pair<GDM::const_iterator, double>> ret{};
    ret.reserve(obj.size());

    for(auto it=obj.begin(); it != obj.end(); ++it) {
        ret.push_back({it, obj.concentrations(it)[block]});
    }

    return ret;
}
