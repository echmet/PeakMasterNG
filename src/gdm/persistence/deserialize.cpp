#include "deserialize.h"

#include "persistenceexcept.h"
#include "../core/common/gdmexcept.h"

#include "json.hpp"

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <iterator>
#include <tuple>
#include <cassert>
#include <stdexcept>

using json = nlohmann::json;

namespace {
std::vector<std::pair<gdm::Constituent, std::map<std::string, gdm::Complexation>>> parseConstituents(const json& src);
std::pair<gdm::Constituent, std::map<std::string, gdm::Complexation>> parseConstituent(const json& src);
gdm::ConstituentType parseConstituentType(const std::string& type);
gdm::PhysicalProperties parsePhysicalProperties(const json& src);
std::map<std::string, gdm::Complexation> parseNucleusComplexForms(const json& src);
} // unnamed namespace

gdm::GDM persistence::deserialize(std::ifstream file)
{
    gdm::GDM ret{};

    try {
        const auto& jsonDocument = json::parse(file);

        auto parsed = parseConstituents(jsonDocument.at("constituents"));

        //First pass: insert constituents
        for(const auto& p : parsed) {
            bool inserted;
            std::tie(std::ignore, inserted) = ret.insert(p.first);
            if(!inserted) throw persistence::BadFileFormat{"Duplicate names are not allowed in constituents"};
        }

        //Second pass: insert complex forms
        for(const auto& p : parsed) {
            auto nucleusIt = ret.find(p.first.name());
            assert(nucleusIt != ret.end());

            for(const auto& c : p.second) {
                auto ligandIt = ret.find(c.first);
                if(ligandIt == ret.end()) throw persistence::BadFileFormat{"Complex form refers to missing ligand"};
                assert(!ret.haveComplexation(nucleusIt, ligandIt));
                ret.setComplexation(nucleusIt, ligandIt, c.second);
            }
        }

    } catch (const gdm::LogicError& e) {
        throw persistence::BadFileFormat{std::string{"Document invariant violation: "} + e.what()};
    } catch (const std::runtime_error& e) {
        throw persistence::BadFileFormat{std::string{"JSON parse error: "} + e.what()};
    }

    return ret;
}

namespace {
std::vector<std::pair<gdm::Constituent, std::map<std::string, gdm::Complexation>>>
parseConstituents(const json& src)
{
    std::vector<std::pair<gdm::Constituent, std::map<std::string, gdm::Complexation>>> ret{};
    ret.reserve(src.size());

    std::transform(src.begin(), src.end(),
                   std::back_inserter(ret),
                   parseConstituent);

    return ret;
}

std::pair<gdm::Constituent, std::map<std::string, gdm::Complexation>>
parseConstituent(const json& src)
{
    std::pair<gdm::Constituent, std::map<std::string, gdm::Complexation>> ret {
        {parseConstituentType(src.at("type")), src.at("name"), parsePhysicalProperties(src)},
        {}
    };

    switch(ret.first.type()) {

    case gdm::ConstituentType::Nucleus:
        ret.second = parseNucleusComplexForms(src.at("complexForms"));
        break;

    case gdm::ConstituentType::Ligand:
        if(src.count("complexForms") != 0) throw persistence::BadFileFormat{"Ligands must not have \"complexForms\""};
        break;

    }

    return ret;
}

gdm::ConstituentType parseConstituentType(const std::string& src)
{
    if(src == "N") return gdm::ConstituentType::Nucleus;
    else if(src == "L") return gdm::ConstituentType::Ligand;
    else throw persistence::BadFileFormat{"Invalid constituent type"};
}

gdm::PhysicalProperties parsePhysicalProperties(const json& src)
{
    gdm::ChargeInterval charges{src.at("chargeLow"), src.at("chargeHigh")};

    const auto& pKas = src.at("pKas");
    const auto& mobilities = src.at("mobilities");

    return gdm::PhysicalProperties{charges, pKas, mobilities};
}

std::map<std::string, gdm::Complexation> parseNucleusComplexForms(const json& src)
{
    std::map<std::string, gdm::Complexation> ret{};

    for(const auto& cf : src) {

        const auto& nucleusCharge = cf.at("nucleusCharge");

        const auto& ligandGroups = cf.at("ligandGroups");

        for(auto lg : ligandGroups) {
            const auto& ligands = lg.at("ligands");
            if(ligands.size() != 1) throw persistence::BadFileFormat{"Must have one ligand per ligand group"};

            const auto& ligand = ligands.front();

            const auto& name = ligand.at("name");
            const auto& charge = ligand.at("charge");
            const auto& maxCount = ligand.at("maxCount");
            const auto& pBs = ligand.at("pBs");
            const auto& mobilities = ligand.at("mobilities");

            gdm::ChargeCombination charges = {nucleusCharge, charge};

            bool inserted;
            std::tie(std::ignore, inserted) = ret[name].insert({charges, maxCount, pBs, mobilities});
            if(!inserted) throw persistence::BadFileFormat{"Duplicate charge combination"};
        }
    }

    return ret;
}


} // unnamed namespace
