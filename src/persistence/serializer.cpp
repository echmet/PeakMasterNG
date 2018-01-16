#include "serializer.h"

#include "../gdm/core/gdm.h"
#include "../gdm/core/complexation/complexform.h"
#include "persistence.h"

#include <cassert>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

namespace persistence {

QJsonArray generateFromDoubles(const std::vector<double> &src)
{
  QJsonArray ret{};

  std::copy(src.begin(), src.end(), std::back_inserter(ret));

  return ret;
}

QJsonObject generateFromNucleusComplexForm(const std::string &ligandName, gdm::ChargeNumber ligandCharge, const gdm::ComplexForm &complexForm)
{
  QJsonObject ret{};

  ret["name"] = QString::fromStdString(ligandName);
  ret["charge"] = ligandCharge;
  ret["maxCount"] = static_cast<int>(complexForm.pBs().size());
  ret["pBs"] = generateFromDoubles(complexForm.pBs());
  ret["mobilities"] = generateFromDoubles(complexForm.mobilities());

  return ret;
}

QJsonObject serializeComposition(const gdm::GDM gdm)
{
   QJsonArray constituents{};

   for (auto it = gdm.cbegin(); it != gdm.cend(); ++it) {
     QJsonObject constituent{};

     switch (it->type()) {
     case gdm::ConstituentType::Nucleus:
       constituent["type"] = "N";
       break;
     case gdm::ConstituentType::Ligand:
        constituent["type"] = "L";
        break;
     }

     constituent["name"] = QString::fromStdString(it->name());

     constituent["chargeLow"] = it->physicalProperties().charges().low();
     constituent["chargeHigh"] = it->physicalProperties().charges().high();

     constituent["pKas"] = generateFromDoubles(it->physicalProperties().pKas());
     constituent["mobilities"] = generateFromDoubles(it->physicalProperties().mobilities());
     constituent["viscosityCoefficient"] = it->physicalProperties().viscosityCoefficient();

     //complexForms
     if (it->type() == gdm::ConstituentType::Nucleus) {
       QJsonArray complexForms{};

       auto nucleusCharges = it->physicalProperties().charges();
       auto foundComplexations = findComplexations(gdm.composition(), it);

       for (auto nucleusCharge = nucleusCharges.low(); nucleusCharge <= nucleusCharges.high(); ++nucleusCharge) {
         QJsonObject complexForm{};

         complexForm["nucleusCharge"] = nucleusCharge;

         QJsonArray ligandGroups{};

         for (const auto &foundComplexation : foundComplexations) {
           Q_ASSERT(foundComplexation.first->type() == gdm::ConstituentType::Ligand);

           const auto& ligandIt = foundComplexation.first;
           const auto& complexation = foundComplexation.second;

           auto ligandCharges = ligandIt->physicalProperties().charges();
           for (auto ligandCharge = ligandCharges.low(); ligandCharge <= ligandCharges.high(); ++ligandCharge) {
             gdm::ChargeCombination charges{nucleusCharge, ligandCharge};

             auto complexFormIt = complexation.find(charges);

             if (complexFormIt != complexation.end()) {
               auto ligand = generateFromNucleusComplexForm(ligandIt->name(), ligandCharge, *complexFormIt);

               QJsonObject ligandGroup{{"ligands", QJsonArray{ligand}}};

               ligandGroups.push_back(ligandGroup);
             }
           }
         }
         complexForm["ligandGroups"] = ligandGroups;

         complexForms.push_back(complexForm);
       }
       constituent["complexForms"] = complexForms;
    }
    constituents.push_back(constituent);
  }

  return QJsonObject{{"constituents", constituents}};
}

QJsonObject serializeConcentrations(const gdm::GDM &gdm)
{
  QJsonObject concentrations;

  for (auto it = gdm.cbegin(); it != gdm.cend(); it++) {
    auto sampleConcs = gdm.concentrations(it);
    assert(sampleConcs.size() == 1);

    const QString name = QString::fromStdString(it->name());
    QJsonArray _concs;
    for (auto d : sampleConcs)
      _concs.append(d);

    concentrations.insert(name, _concs);
  }

  return concentrations;
}

QJsonObject serializeSystem(const System &system)
{
  if (!(system.eofType == "N" ||
        system.eofType == "U" ||
        system.eofType == "T"))
    throw SerializationException{"Invalid EOF type"};

  QJsonObject sys{
    {"totalLength", system.totalLength},
    {"detectorPosition", system.detectorPosition},
    {"drivingVoltage", system.drivingVoltage},
    {"positiveVoltage", system.positiveVoltage},
    {"eofType", system.eofType},
    {"eofValue", system.eofValue},
    {"correctForDebyeHuckel", system.correctForDebyeHuckel},
    {"correctForOnsagerFuoss", system.correctForOnsagerFuoss},
    {"correctForViscosity", system.correctForViscosity},
  };

  return sys;
}

void Serializer::serialize(const QString &filepath, const gdm::GDM &gdmBGE, const gdm::GDM &gdmSample,
                           const System &system)
{
  QFile out{filepath};

  if (!out.open(QIODevice::WriteOnly | QIODevice::Text)) {
    throw SerializationException{"Cannot open output file"};
    return;
  }

  QJsonObject compositionBGE = serializeComposition(gdmBGE);
  QJsonObject compositionSample = serializeComposition(gdmSample);
  QJsonObject concentrationsBGE = serializeConcentrations(gdmBGE);
  QJsonObject concentrationsSample = serializeConcentrations(gdmSample);
  QJsonObject sys = serializeSystem(system);

  QJsonDocument doc{{{"compositionBGE", compositionBGE},
                     {"compositionSample", compositionSample},
                     {"concentrationsBGE", concentrationsBGE},
                     {"concentrationsSample", concentrationsSample},
                     {"system", sys},
                    }};

  QByteArray str = doc.toJson();

  out.write(str);
}

} // namespace persistence

