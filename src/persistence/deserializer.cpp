#include "deserializer.h"

#include "../gdm/core/gdm.h"
#include "../gdm/core/common/gdmexcept.h"
#include "../gdm/core/complexation/complexform.h"
#include "persistence.h"

#include <cassert>
#include <map>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

namespace persistence {

static
void checkIfContains(const QString &str, const QJsonObject &obj)
{
  if (!obj.contains(str))
    throw MalformedJSONException{std::string{"Missing "} + str.toStdString()};
}

static
std::map<std::string, gdm::Complexation> deserializeNucleusComplexForms(const QJsonObject &obj, const int nucleusChargeLow, const int nucleusChargeHigh)
{
  std::map<std::string, gdm::Complexation> ret{};

  checkIfContains(Persistence::CPX_COMPLEX_FORMS, obj);
  const QJsonArray cforms = obj[Persistence::CPX_COMPLEX_FORMS].toArray();

  for (const auto &item : cforms) {
    const QJsonObject cf = item.toObject();
    if (cf.isEmpty())
      throw MalformedJSONException{"Invalid complex form entry"};

    /* Read nucleus charge */
    int charge;
    checkIfContains(Persistence::CPX_NUCLEUS_CHARGE, cf);
    charge = cf[Persistence::CPX_NUCLEUS_CHARGE].toInt();

    if (charge < nucleusChargeLow || charge > nucleusChargeHigh)
      throw MalformedJSONException{"Nucleus charge in complexForm definition is outside nucleus' charge range"};

    /* Read ligand groups */
    checkIfContains(Persistence::CPX_LIGAND_GROUPS, cf);
    const QJsonArray ligandGroups = cf[Persistence::CPX_LIGAND_GROUPS].toArray();
    for (const auto &lg : ligandGroups) {
      QJsonObject lgObj = lg.toObject();
      checkIfContains(Persistence::CPX_LIGANDS, lgObj);
      const QJsonArray ligands = lgObj[Persistence::CPX_LIGANDS].toArray();
      if (ligands.size() != 1)
        throw DeserializationException{"Invalid number of ligands in \"ligands\" array"};

      const QJsonObject lig = ligands.at(0).toObject();
      if (lig.isEmpty())
        throw MalformedJSONException{"Invalid ligand entry"};

      /* Read ligand name */
      std::string name{};
      checkIfContains(Persistence::CPX_NAME, lig);
      name = lig[Persistence::CPX_NAME].toString().toStdString();
      if (name.length() < 1)
        throw MalformedJSONException{"Invalid ligand name"};

      /* Read ligand charge */
      int ligandCharge;
      checkIfContains(Persistence::CPX_CHARGE, lig);
      ligandCharge = lig[Persistence::CPX_CHARGE].toInt();

      /* Read maxCount */
      size_t maxCount;
      checkIfContains(Persistence::CPX_MAX_COUNT, lig);
      maxCount = lig[Persistence::CPX_MAX_COUNT].toInt();

      /* Read pBs */
      std::vector<double> pBs{};
      checkIfContains(Persistence::CPX_PBS, lig);
      QJsonArray inpBs = lig[Persistence::CPX_PBS].toArray();
      if (inpBs.size() != maxCount)
        throw MalformedJSONException{"Invalid size of \"pBs\" array"};
      for (const auto &b : inpBs)
        pBs.emplace_back(b.toDouble());

      /* Read mobilities */
      std::vector<double> mobilities{};
      checkIfContains(Persistence::CPX_MOBILITIES, lig);
      QJsonArray inMobilities = lig[Persistence::CPX_MOBILITIES].toArray();
      if (inMobilities.size() != maxCount)
        throw MalformedJSONException{"Invalid size of complex \"mobilities\" array"};
      for (const auto &u : inMobilities)
        mobilities.emplace_back(u.toDouble());

      gdm::ChargeCombination charges{charge, ligandCharge};

      bool inserted;
      std::tie(std::ignore, inserted) = ret[name].insert({charges, maxCount, pBs, mobilities});
      if (!inserted)
        throw MalformedJSONException{"Duplicit charge combinations"};
    }
  }

  return ret;
}

static
std::vector<std::pair<gdm::Constituent, std::map<std::string, gdm::Complexation>>> deserializeConstituents(const QJsonArray &arr)
{
  std::vector<std::pair<gdm::Constituent, std::map<std::string, gdm::Complexation>>> ret{};

  ret.reserve(arr.count());

  for (const auto &item : arr) {
    const QJsonObject ctuent = item.toObject();
    if (ctuent.isEmpty())
      throw MalformedJSONException{"Invalid constituent object"};

    /* Read type */
    gdm::ConstituentType type;
    checkIfContains(Persistence::CTUENT_TYPE, ctuent);

    auto inType = ctuent[Persistence::CTUENT_TYPE];
    if (inType == Persistence::CTUENT_TYPE_NUCLEUS)
      type = gdm::ConstituentType::Nucleus;
    else if (inType == Persistence::CTUENT_TYPE_LIGAND)
      type = gdm::ConstituentType::Ligand;
    else
      throw MalformedJSONException{"Invalid \"type\" value"};

    /* Read name */
    std::string name{};
    checkIfContains(Persistence::CTUENT_NAME, ctuent);
    QString inName = ctuent[Persistence::CTUENT_NAME].toString();
    if (inName.isNull())
      throw MalformedJSONException{"Invalid \"name\""};
    name = inName.toStdString();

    /* Read chargeLow */
    int chargeLow;
    checkIfContains(Persistence::CTUENT_CHARGE_LOW, ctuent);
    chargeLow = ctuent[Persistence::CTUENT_CHARGE_LOW].toInt();

    /* Read chargeHigh */
    int chargeHigh;
    checkIfContains(Persistence::CTUENT_CHARGE_HIGH, ctuent);
    chargeHigh = ctuent[Persistence::CTUENT_CHARGE_HIGH].toInt();

    /* Read viscosityCoefficient */
    double viscosityCoefficient;
    checkIfContains(Persistence::CTUENT_VISCOSITY_COEFFICIENT, ctuent);
    viscosityCoefficient = ctuent[Persistence::CTUENT_VISCOSITY_COEFFICIENT].toDouble();

    if (chargeLow > chargeHigh)
      throw MalformedJSONException{"Invalid charge values"};

    /* Read pKas */
    std::vector<double> pKas{};
    checkIfContains(Persistence::CTUENT_PKAS, ctuent);
    QJsonArray inpKas = ctuent[Persistence::CTUENT_PKAS].toArray();
    if (inpKas.size() != chargeHigh - chargeLow)
      throw MalformedJSONException{"Invalid pKa array size"};
    for (const auto &d : inpKas)
      pKas.emplace_back(d.toDouble());

    /* Read mobilities */
    std::vector<double> mobilities{};
    checkIfContains(Persistence::CTUENT_MOBILITIES, ctuent);
    QJsonArray inMobilities = ctuent[Persistence::CTUENT_MOBILITIES].toArray();
    if (inMobilities.size() != chargeHigh - chargeLow + 1)
      throw MalformedJSONException{"Invalid mobilities array size"};
    for (const auto &u : inMobilities)
      mobilities.emplace_back(u.toDouble());

    gdm::ChargeInterval charges{chargeLow, chargeHigh};
    gdm::PhysicalProperties physProps{charges, pKas, mobilities, viscosityCoefficient};

    if (type == gdm::ConstituentType::Nucleus) {
      auto complexForms = deserializeNucleusComplexForms(ctuent, chargeLow, chargeHigh);
      ret.emplace_back(std::make_pair<gdm::Constituent, std::map<std::string, gdm::Complexation>>({type, name, physProps}, std::move(complexForms)));
    } else {
      if (ctuent.contains(Persistence::CPX_COMPLEX_FORMS))
        throw MalformedJSONException{"Ligands must not have \"complexForms\""};

      ret.emplace_back(std::make_pair<gdm::Constituent, std::map<std::string, gdm::Complexation>>({type, name, physProps}, {}));
    }
  }

  return ret;
}

static
void deserializeComposition(gdm::GDM &gdm, const QJsonObject &obj)
{
  checkIfContains(Persistence::CTUENT_CTUENTS, obj);
  try {
    const auto constituents = deserializeConstituents(obj[Persistence::CTUENT_CTUENTS].toArray());

    /* Insert all constituents first*/
    for (const auto &ctuent : constituents) {
      bool inserted;
      std::tie(std::ignore, inserted) = gdm.insert(ctuent.first);
      if (!inserted)
        throw DeserializationException("Duplicit constituents");
    }

    /* Insert all complexations */
    for (const auto & ctuent : constituents) {
      auto nucleusIt = gdm.find(ctuent.first.name());
      if (nucleusIt == gdm.end())
        throw DeserializationException{"Internal deserialization error"};

      for (const auto &cf : ctuent.second) {
        auto ligandIt = gdm.find(cf.first);
        if (ligandIt == gdm.end())
          throw DeserializationException{"ComplexForm refers to missing ligand"};

        if (gdm.haveComplexation(nucleusIt, ligandIt))
          throw DeserializationException{"Internal deserialization error"};

        gdm.setComplexation(nucleusIt, ligandIt, cf.second);
      }
    }
  } catch (gdm::LogicError &ex) {
    throw DeserializationException{std::string{"Failed to parse constituents: "} + std::string{ex.what()}};
  }
}

static
void deserializeConcentrations(gdm::GDM &gdm, const QJsonObject &obj)
{
  for (auto ctuentIt = gdm.begin(); ctuentIt != gdm.end(); ctuentIt++) {
    const QString name = QString::fromStdString(ctuentIt->name());
    checkIfContains(name, obj);

    QJsonArray arr = obj[name].toArray();
    if (arr.size() != 1)
      throw MalformedJSONException{"Invalid concentrations array size"};

    gdm.setConcentrations(ctuentIt, { arr.first().toDouble() });
  }

}

static
void deserializeSystem(System &system, const QJsonObject &obj)
{
  const auto readDouble = [&](const QString &name, double &v, bool check = true) {
    checkIfContains(name, obj);
    v = obj[name].toDouble();
    if (v <= 0 && check)
      throw MalformedJSONException{QString{"Invalid %1 value"}.arg(name).toStdString()};
  };

  readDouble(Persistence::SYS_TOTAL_LENGTH, system.totalLength);
  readDouble(Persistence::SYS_DETECTOR_POSITION, system.detectorPosition);
  readDouble(Persistence::SYS_DRIVING_VOLTAGE, system.drivingVoltage);
  readDouble(Persistence::SYS_EOF_VALUE, system.eofValue, false);

  /* Read EOF type */
  checkIfContains(Persistence::SYS_EOF_TYPE, obj);
  QString t = obj[Persistence::SYS_EOF_TYPE].toString();
  if (!(t == Persistence::SYS_EOF_TYPE_NONE ||
        t == Persistence::SYS_EOF_TYPE_MOBILITY ||
        t == Persistence::SYS_EOF_TYPE_TIME))
    throw MalformedJSONException{"Invalid \"eofType\" value"};
  system.eofType = t;

  /* Read positive voltage */
  checkIfContains(Persistence::SYS_POSITIVE_VOLTAGE, obj);
  system.positiveVoltage = obj[Persistence::SYS_POSITIVE_VOLTAGE].toBool();

  /* Read nonideality corrections */
  checkIfContains(Persistence::SYS_CORRECT_FOR_DEBYE_HUCKEL, obj);
  system.correctForDebyeHuckel = obj[Persistence::SYS_CORRECT_FOR_DEBYE_HUCKEL].toBool();
  checkIfContains(Persistence::SYS_CORRECT_FOR_ONSAGER_FUOSS, obj);
  system.correctForOnsagerFuoss = obj[Persistence::SYS_CORRECT_FOR_ONSAGER_FUOSS].toBool();
  checkIfContains(Persistence::SYS_CORRECT_FOR_VISCOSITY, obj);
  system.correctForViscosity = obj[Persistence::SYS_CORRECT_FOR_VISCOSITY].toBool();

  try {
    checkIfContains(Persistence::SYS_INJECTION_ZONE_LENGTH, obj);
    system.injectionZoneLength = obj[Persistence::SYS_INJECTION_ZONE_LENGTH].toDouble();
  } catch (const MalformedJSONException &) {
    system.injectionZoneLength = 1.0;
  }
}

void Deserializer::deserialize(const QString &filepath, gdm::GDM &gdmBGE, gdm::GDM &gdmSample,
                               System &system)
{
  QFile input{filepath};

  if (!input.open(QIODevice::ReadOnly))
    throw DeserializationException{"Cannot open input file"};

  QJsonParseError parseError{};
  QJsonDocument doc = QJsonDocument::fromJson(input.readAll(), &parseError);
  if (doc.isNull())
    throw DeserializationException{parseError.errorString().toStdString()};

  QJsonObject obj = doc.object();
  if (obj.isEmpty())
    throw MalformedJSONException{"Bad root object"};

  checkIfContains(Persistence::ROOT_COMPOSITION_BGE, obj);
  deserializeComposition(gdmBGE, obj[Persistence::ROOT_COMPOSITION_BGE].toObject());

  checkIfContains(Persistence::ROOT_COMPOSITION_SAMPLE, obj);
  deserializeComposition(gdmSample, obj[Persistence::ROOT_COMPOSITION_SAMPLE].toObject());

  checkIfContains(Persistence::ROOT_CONCENTRATIONS_BGE, obj);
  deserializeConcentrations(gdmBGE, obj[Persistence::ROOT_CONCENTRATIONS_BGE].toObject());

  checkIfContains(Persistence::ROOT_CONCENTRATIONS_SAMPLE, obj);
  deserializeConcentrations(gdmSample, obj[Persistence::ROOT_CONCENTRATIONS_SAMPLE].toObject());

  checkIfContains(Persistence::ROOT_SYSTEM, obj);
  deserializeSystem(system, obj[Persistence::ROOT_SYSTEM].toObject());
}

} // namespace persistence
