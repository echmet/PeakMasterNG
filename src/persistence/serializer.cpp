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

static
QJsonArray generateFromDoubles(const std::vector<double> &src)
{
  QJsonArray ret{};

  std::copy(src.begin(), src.end(), std::back_inserter(ret));

  return ret;
}

static
QJsonObject generateFromNucleusComplexForm(const std::string &ligandName, gdm::ChargeNumber ligandCharge, const gdm::ComplexForm &complexForm)
{
  QJsonObject ret{};

  ret[Persistence::CPX_NAME] = QString::fromStdString(ligandName);
  ret[Persistence::CPX_CHARGE] = ligandCharge;
  ret[Persistence::CPX_MAX_COUNT] = static_cast<int>(complexForm.pBs().size());
  ret[Persistence::CPX_PBS] = generateFromDoubles(complexForm.pBs());
  ret[Persistence::CPX_MOBILITIES] = generateFromDoubles(complexForm.mobilities());

  return ret;
}

static
QJsonObject serializeComposition(const gdm::GDM &gdm)
{
   QJsonArray constituents{};

   for (auto it = gdm.cbegin(); it != gdm.cend(); ++it) {
     QJsonObject constituent{};

     switch (it->type()) {
     case gdm::ConstituentType::Nucleus:
       constituent[Persistence::CTUENT_TYPE] = Persistence::CTUENT_TYPE_NUCLEUS;
       break;
     case gdm::ConstituentType::Ligand:
       constituent[Persistence::CTUENT_TYPE] = Persistence::CTUENT_TYPE_LIGAND;
       break;
     }

     constituent[Persistence::CTUENT_NAME] = QString::fromStdString(it->name());

     constituent[Persistence::CTUENT_CHARGE_LOW] = it->physicalProperties().charges().low();
     constituent[Persistence::CTUENT_CHARGE_HIGH] = it->physicalProperties().charges().high();

     constituent[Persistence::CTUENT_PKAS] = generateFromDoubles(it->physicalProperties().pKas());
     constituent[Persistence::CTUENT_MOBILITIES] = generateFromDoubles(it->physicalProperties().mobilities());
     constituent[Persistence::CTUENT_VISCOSITY_COEFFICIENT] = it->physicalProperties().viscosityCoefficient();

     //complexForms
     if (it->type() == gdm::ConstituentType::Nucleus) {
       QJsonArray complexForms{};

       auto nucleusCharges = it->physicalProperties().charges();
       auto foundComplexations = findComplexations(gdm.composition(), it);

       for (auto nucleusCharge = nucleusCharges.low(); nucleusCharge <= nucleusCharges.high(); ++nucleusCharge) {
         QJsonObject complexForm{};

         complexForm[Persistence::CPX_NUCLEUS_CHARGE] = nucleusCharge;

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

               QJsonObject ligandGroup{{Persistence::CPX_LIGANDS, QJsonArray{ligand}}};

               ligandGroups.push_back(ligandGroup);
             }
           }
         }
         complexForm[Persistence::CPX_LIGAND_GROUPS] = ligandGroups;

         complexForms.push_back(complexForm);
       }
       constituent[Persistence::CPX_COMPLEX_FORMS] = complexForms;
    }
    constituents.push_back(constituent);
  }

  return QJsonObject{{Persistence::CTUENT_CTUENTS, constituents}};
}

static
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

static
QJsonObject serializeSystem(const System &system)
{
  if (!(system.eofType == Persistence::SYS_EOF_TYPE_NONE ||
        system.eofType == Persistence::SYS_EOF_TYPE_MOBILITY ||
        system.eofType == Persistence::SYS_EOF_TYPE_TIME))
    throw SerializationException{"Invalid EOF type"};

  QJsonObject sys{
    {Persistence::SYS_TOTAL_LENGTH, system.totalLength},
    {Persistence::SYS_DETECTOR_POSITION, system.detectorPosition},
    {Persistence::SYS_DRIVING_VOLTAGE, system.drivingVoltage},
    {Persistence::SYS_POSITIVE_VOLTAGE, system.positiveVoltage},
    {Persistence::SYS_EOF_TYPE, system.eofType},
    {Persistence::SYS_EOF_VALUE, system.eofValue},
    {Persistence::SYS_CORRECT_FOR_DEBYE_HUCKEL, system.correctForDebyeHuckel},
    {Persistence::SYS_CORRECT_FOR_ONSAGER_FUOSS, system.correctForOnsagerFuoss},
    {Persistence::SYS_CORRECT_FOR_VISCOSITY, system.correctForViscosity},
    {Persistence::SYS_INJECTION_ZONE_LENGTH, system.injectionZoneLength}
  };

  return sys;
}

void Serializer::serialize(QString filepath, const gdm::GDM &gdmBGE, const gdm::GDM &gdmSample,
                           const System &system)
{
  if (!filepath.endsWith(".json", Qt::CaseInsensitive))
    filepath.append(".json");

  QFile out{filepath};

  if (!out.open(QIODevice::WriteOnly | QIODevice::Text))
    throw SerializationException{"Cannot open output file"};

  QJsonObject compositionBGE = serializeComposition(gdmBGE);
  QJsonObject compositionSample = serializeComposition(gdmSample);
  QJsonObject concentrationsBGE = serializeConcentrations(gdmBGE);
  QJsonObject concentrationsSample = serializeConcentrations(gdmSample);
  QJsonObject sys = serializeSystem(system);

  QJsonDocument doc{{{Persistence::ROOT_COMPOSITION_BGE, compositionBGE},
                     {Persistence::ROOT_COMPOSITION_SAMPLE, compositionSample},
                     {Persistence::ROOT_CONCENTRATIONS_BGE, concentrationsBGE},
                     {Persistence::ROOT_CONCENTRATIONS_SAMPLE, concentrationsSample},
                     {Persistence::ROOT_SYSTEM, sys},
                    }};

  QByteArray str = doc.toJson();

  out.write(str);
}

} // namespace persistence

