#include "deserializer.h"

#include "../gdm/core/gdm.h"
#include "../gdm/core/common/gdmexcept.h"
#include "../gdm/core/complexation/complexform.h"
#include "persistence.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTextStream>
#include <cassert>
#include <map>

namespace persistence {

static
void checkIfContains(const QString &str, const QJsonObject &obj, const QJsonValue::Type type)
{
  if (!obj.contains(str))
    throw MalformedJSONException{std::string{"Missing "} + str.toStdString()};

  if (obj[str].type() != type)
    throw MalformedJSONException{std::string{"Unexpected type of field "} + str.toStdString()};
}

static
void checkIfContainsInt(const QString &str, const QJsonObject &obj)
{
  if (!obj.contains(str))
    throw MalformedJSONException{std::string{"Missing "} + str.toStdString()};
}

static
std::map<std::string, gdm::Complexation> deserializeNucleusComplexForms(const QJsonObject &obj, const int nucleusChargeLow, const int nucleusChargeHigh)
{
  std::map<std::string, gdm::Complexation> ret{};

  checkIfContains(Persistence::CPX_COMPLEX_FORMS, obj, QJsonValue::Array);
  const QJsonArray cforms = obj[Persistence::CPX_COMPLEX_FORMS].toArray();

  for (const auto &item : cforms) {
    const QJsonObject cf = item.toObject();
    if (cf.isEmpty())
      throw MalformedJSONException{"Invalid complex form entry"};

    /* Read nucleus charge */
    int charge;
    checkIfContainsInt(Persistence::CPX_NUCLEUS_CHARGE, cf);
    charge = cf[Persistence::CPX_NUCLEUS_CHARGE].toInt();

    if (charge < nucleusChargeLow || charge > nucleusChargeHigh)
      throw MalformedJSONException{"Nucleus charge in complexForm definition is outside nucleus' charge range"};

    /* Read ligand groups */
    checkIfContains(Persistence::CPX_LIGAND_GROUPS, cf, QJsonValue::Array);
    const QJsonArray ligandGroups = cf[Persistence::CPX_LIGAND_GROUPS].toArray();
    for (const auto &lg : ligandGroups) {
      QJsonObject lgObj = lg.toObject();
      checkIfContains(Persistence::CPX_LIGANDS, lgObj, QJsonValue::Array);
      const QJsonArray ligands = lgObj[Persistence::CPX_LIGANDS].toArray();
      if (ligands.size() != 1)
        throw DeserializationException{"Invalid number of ligands in \"ligands\" array"};

      const QJsonObject lig = ligands.at(0).toObject();
      if (lig.isEmpty())
        throw MalformedJSONException{"Invalid ligand entry"};

      /* Read ligand name */
      std::string name{};
      checkIfContains(Persistence::CPX_NAME, lig, QJsonValue::String);
      name = lig[Persistence::CPX_NAME].toString().toStdString();
      if (name.length() < 1)
        throw MalformedJSONException{"Invalid ligand name"};

      /* Read ligand charge */
      int ligandCharge;
      checkIfContainsInt(Persistence::CPX_CHARGE, lig);
      ligandCharge = lig[Persistence::CPX_CHARGE].toInt();

      /* Read maxCount */
      size_t maxCount;
      checkIfContainsInt(Persistence::CPX_MAX_COUNT, lig);
      maxCount = lig[Persistence::CPX_MAX_COUNT].toInt();

      /* Read pBs */
      std::vector<double> pBs{};
      checkIfContains(Persistence::CPX_PBS, lig, QJsonValue::Array);
      QJsonArray inpBs = lig[Persistence::CPX_PBS].toArray();
      if (inpBs.size() != maxCount)
        throw MalformedJSONException{"Invalid size of \"pBs\" array"};
      for (const auto b : inpBs) {
        if (b.type() != QJsonValue::Double)
          throw MalformedJSONException{"Unexpected type of pBs value"};
        pBs.emplace_back(b.toDouble());
      }

      /* Read mobilities */
      std::vector<double> mobilities{};
      checkIfContains(Persistence::CPX_MOBILITIES, lig, QJsonValue::Array);
      QJsonArray inMobilities = lig[Persistence::CPX_MOBILITIES].toArray();
      if (inMobilities.size() != maxCount)
        throw MalformedJSONException{"Invalid size of complex \"mobilities\" array"};
      for (const auto u : inMobilities) {
        if (u.type() != QJsonValue::Double)
          throw MalformedJSONException{"Unexpected type of mobility value"};
        mobilities.emplace_back(u.toDouble());
      }

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
    checkIfContains(Persistence::CTUENT_TYPE, ctuent, QJsonValue::String);

    auto inType = ctuent[Persistence::CTUENT_TYPE];
    if (inType == Persistence::CTUENT_TYPE_NUCLEUS)
      type = gdm::ConstituentType::Nucleus;
    else if (inType == Persistence::CTUENT_TYPE_LIGAND)
      type = gdm::ConstituentType::Ligand;
    else
      throw MalformedJSONException{"Invalid \"type\" value"};

    /* Read name */
    std::string name{};
    checkIfContains(Persistence::CTUENT_NAME, ctuent, QJsonValue::String);
    QString inName = ctuent[Persistence::CTUENT_NAME].toString();
    if (inName.isNull())
      throw MalformedJSONException{"Invalid \"name\""};
    name = inName.toStdString();

    /* Read chargeLow */
    int chargeLow;
    checkIfContainsInt(Persistence::CTUENT_CHARGE_LOW, ctuent);
    chargeLow = ctuent[Persistence::CTUENT_CHARGE_LOW].toInt();

    /* Read chargeHigh */
    int chargeHigh;
    checkIfContainsInt(Persistence::CTUENT_CHARGE_HIGH, ctuent);
    chargeHigh = ctuent[Persistence::CTUENT_CHARGE_HIGH].toInt();

    /* Read viscosityCoefficient */
    double viscosityCoefficient;
    checkIfContains(Persistence::CTUENT_VISCOSITY_COEFFICIENT, ctuent, QJsonValue::Double);
    viscosityCoefficient = ctuent[Persistence::CTUENT_VISCOSITY_COEFFICIENT].toDouble();

    if (chargeLow > chargeHigh)
      throw MalformedJSONException{"Invalid charge values"};

    /* Read pKas */
    std::vector<double> pKas{};
    checkIfContains(Persistence::CTUENT_PKAS, ctuent, QJsonValue::Array);
    QJsonArray inpKas = ctuent[Persistence::CTUENT_PKAS].toArray();
    if (inpKas.size() != chargeHigh - chargeLow)
      throw MalformedJSONException{"Invalid pKa array size"};
    for (const auto d : inpKas) {
      if (d.type() != QJsonValue::Double)
        throw MalformedJSONException{"Unexpected type of pKa value"};
      pKas.emplace_back(d.toDouble());
    }

    /* Read mobilities */
    std::vector<double> mobilities{};
    checkIfContains(Persistence::CTUENT_MOBILITIES, ctuent, QJsonValue::Array);
    QJsonArray inMobilities = ctuent[Persistence::CTUENT_MOBILITIES].toArray();
    if (inMobilities.size() != chargeHigh - chargeLow + 1)
      throw MalformedJSONException{"Invalid mobilities array size"};
    for (const auto u : inMobilities) {
      if (u.type() != QJsonValue::Double)
        throw MalformedJSONException{"Unexpected type of mobility value"};
      mobilities.emplace_back(u.toDouble());
    }

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
  checkIfContains(Persistence::CTUENT_CTUENTS, obj, QJsonValue::Array);
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
    checkIfContains(name, obj, QJsonValue::Array);

    QJsonArray arr = obj[name].toArray();
    if (arr.size() != 1)
      throw MalformedJSONException{"Invalid concentrations array size"};

    if (arr.first().type() != QJsonValue::Double)
      throw MalformedJSONException{"Unexpected type of concentration value"};

    gdm.setConcentrations(ctuentIt, { arr.first().toDouble() });
  }

}

static
void deserializeSystem(System &system, const QJsonObject &obj)
{
  const auto readDouble = [&](const QString &name, double &v, bool check = true) {
    checkIfContains(name, obj, QJsonValue::Double);
    v = obj[name].toDouble();
    if (v <= 0 && check)
      throw MalformedJSONException{QString{"Invalid %1 value"}.arg(name).toStdString()};
  };

  readDouble(Persistence::SYS_TOTAL_LENGTH, system.totalLength);
  readDouble(Persistence::SYS_DETECTOR_POSITION, system.detectorPosition);
  readDouble(Persistence::SYS_DRIVING_VOLTAGE, system.drivingVoltage);
  readDouble(Persistence::SYS_EOF_VALUE, system.eofValue, false);

  /* Read EOF type */
  checkIfContains(Persistence::SYS_EOF_TYPE, obj, QJsonValue::String);
  QString t = obj[Persistence::SYS_EOF_TYPE].toString();
  if (!(t == Persistence::SYS_EOF_TYPE_NONE ||
        t == Persistence::SYS_EOF_TYPE_MOBILITY ||
        t == Persistence::SYS_EOF_TYPE_TIME))
    throw MalformedJSONException{"Invalid \"eofType\" value"};
  system.eofType = t;

  /* Read positive voltage */
  checkIfContains(Persistence::SYS_POSITIVE_VOLTAGE, obj, QJsonValue::Bool);
  system.positiveVoltage = obj[Persistence::SYS_POSITIVE_VOLTAGE].toBool();

  /* Read nonideality corrections */
  checkIfContains(Persistence::SYS_CORRECT_FOR_DEBYE_HUCKEL, obj, QJsonValue::Bool);
  system.correctForDebyeHuckel = obj[Persistence::SYS_CORRECT_FOR_DEBYE_HUCKEL].toBool();
  checkIfContains(Persistence::SYS_CORRECT_FOR_ONSAGER_FUOSS, obj, QJsonValue::Bool);
  system.correctForOnsagerFuoss = obj[Persistence::SYS_CORRECT_FOR_ONSAGER_FUOSS].toBool();
  checkIfContains(Persistence::SYS_CORRECT_FOR_VISCOSITY, obj, QJsonValue::Bool);
  system.correctForViscosity = obj[Persistence::SYS_CORRECT_FOR_VISCOSITY].toBool();

  try {
    checkIfContains(Persistence::SYS_INJECTION_ZONE_LENGTH, obj, QJsonValue::Double);
    system.injectionZoneLength = obj[Persistence::SYS_INJECTION_ZONE_LENGTH].toDouble();
  } catch (const MalformedJSONException &) {
    system.injectionZoneLength = 1.0;
  }
  if (system.injectionZoneLength <= 0.0)
    throw MalformedJSONException{"Invalid value of \"injectionZoneLength\""};
}

void Deserializer::deserialize(const Target &target, gdm::GDM &gdmBGE, gdm::GDM &gdmSample,
                               System &system)
{
  auto doc = [&]() {
    QJsonParseError parseError{};

    if (target.type == Target::TT_FILE) {
      QFile input{target.path};

      if (!input.open(QIODevice::ReadOnly | QIODevice::Text))
        throw DeserializationException{"Cannot open input file"};

      QTextStream stm{&input};
      stm.setCodec("UTF-8");

      auto doc = QJsonDocument::fromJson(stm.readAll().toUtf8(), &parseError);
      if (doc.isNull())
        throw DeserializationException{parseError.errorString().toStdString()};

      return doc;
    } else {
      auto clipboard = QGuiApplication::clipboard();
      auto str = clipboard->text();

      auto doc = QJsonDocument::fromJson(str.toUtf8(), &parseError);
      if (doc.isNull())
        throw DeserializationException{parseError.errorString().toStdString()};

      return doc;
    }
  }();

  QJsonObject obj = doc.object();
  if (obj.isEmpty())
    throw MalformedJSONException{"Bad root object"};

  checkIfContains(Persistence::ROOT_COMPOSITION_BGE, obj, QJsonValue::Object);
  deserializeComposition(gdmBGE, obj[Persistence::ROOT_COMPOSITION_BGE].toObject());

  checkIfContains(Persistence::ROOT_COMPOSITION_SAMPLE, obj, QJsonValue::Object);
  deserializeComposition(gdmSample, obj[Persistence::ROOT_COMPOSITION_SAMPLE].toObject());

  checkIfContains(Persistence::ROOT_CONCENTRATIONS_BGE, obj, QJsonValue::Object);
  deserializeConcentrations(gdmBGE, obj[Persistence::ROOT_CONCENTRATIONS_BGE].toObject());

  checkIfContains(Persistence::ROOT_CONCENTRATIONS_SAMPLE, obj, QJsonValue::Object);
  deserializeConcentrations(gdmSample, obj[Persistence::ROOT_CONCENTRATIONS_SAMPLE].toObject());

  checkIfContains(Persistence::ROOT_SYSTEM, obj, QJsonValue::Object);
  deserializeSystem(system, obj[Persistence::ROOT_SYSTEM].toObject());
}

} // namespace persistence
