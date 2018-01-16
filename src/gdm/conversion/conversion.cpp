#include "conversion.h"

#include <new>
#include <cassert>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <cstddef>

#include "../core/complexation/complexform.h"

/*unnamed*/ namespace {
    std::unique_ptr<ECHMET::RealVec, void(*)(const ECHMET::RealVec*)> makeECHMETRealVec(const std::vector<double>& input = {});
} // unnamed namespace

std::unique_ptr<ECHMET::SysComp::InConstituentVec, void(ECHMET_CC *)(const ECHMET::SysComp::InConstituentVec*)>
    conversion::makeECHMETInConstituentVec(const gdm::GDM& doc)
{   
    auto ptr = ECHMET::SysComp::createInConstituentVec(doc.size());
    if(ptr == nullptr) throw std::bad_alloc{};
    std::unique_ptr<ECHMET::SysComp::InConstituentVec, void(ECHMET_CC *)(const ECHMET::SysComp::InConstituentVec*)>
            ret{ptr, ECHMET::SysComp::releaseInputData};

    for(auto it = doc.begin(); it != doc.end(); ++it) {
        //Push back a zero-initialized element before working on it (so everything will get cleaned up by the deleter if we throw)
        //We'll keep following this pattern throughout this function
        auto retcode = ret->push_back(ECHMET::SysComp::InConstituent{});
        if(retcode == ECHMET::RetCode::E_NO_MEMORY) throw std::bad_alloc{};
        assert(retcode == ECHMET::RetCode::OK);

        //Set the element's values
        auto& back = (*ret)[ret->size()-1];

        switch(it->type()) {
        case gdm::ConstituentType::Nucleus:
            back.ctype = ECHMET::SysComp::ConstituentType::NUCLEUS;
            break;
        case gdm::ConstituentType::Ligand:
            back.ctype = ECHMET::SysComp::ConstituentType::LIGAND;
            break;
        }

        back.name = ECHMET::createFixedString(it->name().c_str());
        if(back.name == nullptr) throw std::bad_alloc{};

        back.chargeLow = it->physicalProperties().charges().low();
        back.chargeHigh = it->physicalProperties().charges().high();
        back.viscosityCoefficient = it->physicalProperties().viscosityCoefficient();

        back.pKas = makeECHMETRealVec(it->physicalProperties().pKas()).release();
        assert(back.pKas->size() == static_cast<std::size_t>(back.chargeHigh - back.chargeLow));
        back.mobilities = makeECHMETRealVec(it->physicalProperties().mobilities()).release();
        assert(back.mobilities->size() == static_cast<std::size_t>(back.chargeHigh - back.chargeLow + 1));

        //Create the complex forms vector (only for nuclei)
        if(back.ctype == ECHMET::SysComp::ConstituentType::NUCLEUS) {
            assert(it->type() == gdm::ConstituentType::Nucleus);

            auto foundComplexations = findComplexations(doc.composition(), it);
            auto nucleusCharges = it->physicalProperties().charges();

            back.complexForms = ECHMET::SysComp::createInCFVec(0);
            if(back.complexForms == nullptr) throw std::bad_alloc{};

            for(auto nucleusCharge = nucleusCharges.low(); nucleusCharge <= nucleusCharges.high(); ++nucleusCharge) { //For each nucleus charge

                for(const auto& foundComplexation : foundComplexations) { //For each ligand associated with a nucleus
                    assert(foundComplexation.first->type() == gdm::ConstituentType::Ligand);

                    const auto& ligandIt = foundComplexation.first;
                    const auto& complexation = foundComplexation.second;

                    auto ligandCharges = ligandIt->physicalProperties().charges();
                    for(auto ligandCharge = ligandCharges.low(); ligandCharge <= ligandCharges.high(); ++ligandCharge) { //For each ligand charge

                        auto complexFormIt = complexation.find(gdm::ChargeCombination{nucleusCharge, ligandCharge});

                        if(complexFormIt != complexation.end()) { //If there is a complex form for this charge combination

                            //If one doesn't exist already, create a complex form entry for this nucleus charge
                            if(back.complexForms->size() == 0 || back.complexForms->back().nucleusCharge < nucleusCharge) {
                                auto retcode = back.complexForms->push_back(ECHMET::SysComp::InComplexForm{});
                                if(retcode == ECHMET::RetCode::E_NO_MEMORY) throw std::bad_alloc{};
                                assert(retcode == ECHMET::RetCode::OK);

                                (*back.complexForms)[back.complexForms->size()-1].nucleusCharge = nucleusCharge;

                                (*back.complexForms)[back.complexForms->size()-1].ligandGroups = ECHMET::SysComp::createInLGVec(1);
                                if((*back.complexForms)[back.complexForms->size()-1].ligandGroups == nullptr) throw std::bad_alloc{};
                            }

                            //The Document class is deliberately unaware of ligand groups. We just create one ligand group per ligand
                            auto& cfBack = (*back.complexForms)[back.complexForms->size()-1];

                            auto retcode = cfBack.ligandGroups->push_back(ECHMET::SysComp::InLigandGroup{});
                            if(retcode == ECHMET::RetCode::E_NO_MEMORY) throw std::bad_alloc{};
                            assert(retcode == ECHMET::RetCode::OK);

                            //Set the new ligand group's value
                            auto& lgBack = (*cfBack.ligandGroups)[cfBack.ligandGroups->size()-1];

                            lgBack.ligands = ECHMET::SysComp::createInLFVec(1);
                            if(lgBack.ligands == nullptr) throw std::bad_alloc{};

                            retcode = lgBack.ligands->push_back(ECHMET::SysComp::InLigandForm{});
                            if(retcode == ECHMET::RetCode::E_NO_MEMORY) throw std::bad_alloc{};
                            assert(retcode == ECHMET::RetCode::OK);

                            //Set the new ligand form's values
                            auto& lfBack = (*lgBack.ligands)[lgBack.ligands->size()-1];

                            lfBack.ligandName = ECHMET::createFixedString(ligandIt->name().c_str());
                            if(lfBack.ligandName == nullptr) throw std::bad_alloc{};

                            lfBack.charge = ligandCharge;

                            lfBack.maxCount = complexFormIt->maxCount();

                            lfBack.pBs = makeECHMETRealVec(complexFormIt->pBs()).release();
                            assert(lfBack.pBs->size() == lfBack.maxCount);

                            lfBack.mobilities = makeECHMETRealVec(complexFormIt->mobilities()).release();
                            assert(lfBack.mobilities->size() == lfBack.maxCount);
                        }
                    }
                }
            }
        }
    }

    //Check postconditions
    ECHMET::SysComp::ChemicalSystem sys;
    ECHMET::SysComp::CalculatedProperties props;
    auto retcode = makeComposition(sys, props, ret.get());
    assert(retcode == ECHMET::RetCode::OK);

    ECHMET::SysComp::releaseChemicalSystem(sys);
    ECHMET::SysComp::releaseCalculatedProperties(props);

    return ret;
}

std::unique_ptr<ECHMET::RealVec, void(*)(const ECHMET::RealVec*)>
    conversion::makeECHMETAnalyticalConcentrationsVec(const gdm::GDM& doc, std::size_t block, const ECHMET::SysComp::ConstituentVec* constituents)
{
    assert(constituents != nullptr);
    assert(constituents->size() == doc.size());
    assert(block < doc.concentrationBlockCount());

    auto ptr = ECHMET::createRealVec(constituents->size());
    if(ptr == nullptr) throw std::bad_alloc{};
    std::unique_ptr<ECHMET::RealVec, void(*)(const ECHMET::RealVec*)>
            ret{ptr, [](const ECHMET::RealVec* obj) noexcept { obj->destroy(); }};

    auto retcode = ret->resize(constituents->size());
    if(retcode == ECHMET::RetCode::E_NO_MEMORY) throw std::bad_alloc{};
    assert(retcode == ECHMET::RetCode::OK);

    for(std::size_t i=0; i < constituents->size(); ++i) {
        auto name = constituents->at(i)->name;
        auto index = constituents->at(i)->analyticalConcentrationIndex;
        assert(index < constituents->size());

        auto found = doc.find(name->c_str());
        assert(found != doc.end());

        (*ret)[index] = doc.concentrations(found)[block];
        static_assert(std::is_same<ECHMET::ECHMETReal, double>::value, "Conversion from double to ECHMETReal may be unwanted");
    }

    return ret;
}

/*unnamed*/ namespace {
std::unique_ptr<ECHMET::RealVec, void(*)(const ECHMET::RealVec*)> makeECHMETRealVec(const std::vector<double>& input)
{
    auto ptr = ECHMET::createRealVec(input.size());
    if(ptr == nullptr) throw std::bad_alloc{};
    std::unique_ptr<ECHMET::RealVec, void(*)(const ECHMET::RealVec*)>
            ret{ptr, [](const ECHMET::RealVec* obj) noexcept { obj->destroy(); }};

    for(auto d : input) {
        auto retcode = ret->push_back(d);
        static_assert(std::is_same<ECHMET::ECHMETReal, double>::value, "Conversion from double to ECHMETReal may be unwanted");
        if(retcode == ECHMET::RetCode::E_NO_MEMORY) throw std::bad_alloc{};
        assert(retcode == ECHMET::RetCode::OK);
    }

    return ret;
}
} //unnamed namespace


