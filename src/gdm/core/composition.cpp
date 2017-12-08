#include "composition.h"

#include <cassert>

#include "common/gdmexcept.h"

/*unnamed*/ namespace {
gdm::detail::ComplexationMapKey complexationMapKey(gdm::Composition::const_iterator pos1, gdm::Composition::const_iterator pos2);
} // unnamed namespace


void gdm::Composition::clear() noexcept
{
    BasicComposition::clear();
    clearComplexations();
}

gdm::Composition::iterator gdm::Composition::erase(const_iterator pos) noexcept
{
    eraseAll(_complexations, pos->type(), pos->name());
    return BasicComposition::erase(pos);
}

std::pair<gdm::Composition::iterator,bool> gdm::Composition::update(const_iterator pos, const Constituent& value)
{
    auto oldValue = *pos;

    BasicComposition::erase(pos);

    auto ret = BasicComposition::insert(value);

    bool wasInserted = ret.second;
    if(wasInserted) {
        bool typeChanged = (oldValue.type() != value.type());
        if(typeChanged) {
            eraseAll(_complexations, oldValue.type(), oldValue.name());
        }
        else {
            bool chargesNarrowed = !contains(value.physicalProperties().charges(), oldValue.physicalProperties().charges());
            if(chargesNarrowed) clampAll(_complexations, value.type(), oldValue.name(), value.physicalProperties().charges());

            bool nameChanged = (oldValue.name() != value.name());
            if(nameChanged) rename(_complexations, value.type(), oldValue.name(), value.name());
        }
    }
    else BasicComposition::insert(oldValue);

    return ret;
}

bool gdm::Composition::haveComplexation(const_iterator pos1, const_iterator pos2) const
{
    if(pos1->type() == pos2->type()) throw InvalidArgument{"Complexations are defined between a nucleus and a ligand"};

    return _complexations.count(complexationMapKey(pos1, pos2)) >= 1;
}

const gdm::Complexation& gdm::Composition::complexationBetween(const_iterator pos1, const_iterator pos2) const
{
    if(pos1->type() == pos2->type()) throw InvalidArgument{"Complexations are defined between a nucleus and a ligand"};

    try {
        return _complexations.at(complexationMapKey(pos1, pos2));
    } catch (std::out_of_range&) {
        throw OutOfRange{"No such complexation"};
    }
}

void gdm::Composition::setComplexation(const_iterator pos1, const_iterator pos2, Complexation complexation)
{
    if(pos1->type() == pos2->type()) throw InvalidArgument{"Complexations must be defined between a nucleus and a ligand"};

    clamp(complexation, pos1->type(), pos1->physicalProperties().charges());
    clamp(complexation, pos2->type(), pos2->physicalProperties().charges());

    _complexations[complexationMapKey(pos1, pos2)] = std::move(complexation);

    assert(haveComplexation(pos1, pos2));
}

void gdm::Composition::eraseComplexation(const_iterator pos1, const_iterator pos2)
{
    if(pos1->type() == pos2->type()) throw InvalidArgument{"Complexations must be defined between a nucleus and a ligand"};

    _complexations.erase(complexationMapKey(pos1, pos2));

    assert(!haveComplexation(pos1, pos2));
}

void gdm::Composition::clearComplexations() noexcept
{
    _complexations.clear();
}

const gdm::BasicComposition& gdm::Composition::constituents() const noexcept
{
    return *this;
}

bool gdm::operator==(const gdm::Composition& a, const gdm::Composition& b) noexcept
{
    return (static_cast<BasicComposition>(a) == static_cast<BasicComposition>(b)
            && a._complexations == b._complexations);
}

bool gdm::operator!=(const gdm::Composition& a, const gdm::Composition& b) noexcept
{
    return !(a == b);
}

std::vector<std::pair<gdm::BasicComposition::const_iterator, gdm::Complexation> > gdm::findComplexations(const Composition& obj, Composition::const_iterator pos)
{
    std::vector<std::pair<Composition::const_iterator, Complexation>> ret{};

    for(auto otherPos = obj.cbegin(); otherPos != obj.cend(); ++otherPos) {
        if(pos->type() != otherPos->type() && obj.haveComplexation(pos, otherPos)) {
            ret.push_back({otherPos, obj.complexationBetween(pos, otherPos)});
        }
    }

    return ret;
}

/*unnamed*/ namespace {
gdm::detail::ComplexationMapKey complexationMapKey(gdm::Composition::const_iterator pos1, gdm::Composition::const_iterator pos2)
{
    assert(pos1->type() != pos2->type());

    gdm::detail::ComplexationMapKey ret;

    switch(pos1->type()) {

    case gdm::ConstituentType::Nucleus:
        assert(pos2->type() == gdm::ConstituentType::Ligand);
        ret.nucleusName = pos1->name();
        ret.ligandName = pos2->name();
        break;

    case gdm::ConstituentType::Ligand:
        assert(pos2->type() == gdm::ConstituentType::Nucleus);
        ret.nucleusName = pos2->name();
        ret.ligandName = pos1->name();
        break;
    }

    return ret;
}
} // unnamed namespace

