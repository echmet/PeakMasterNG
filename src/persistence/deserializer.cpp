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

void checkIfContains(const QString &str, const QJsonObject &obj)
{
  if (!obj.contains(str))
    throw MalformedJSONException{std::string{"Missing "} + str.toStdString()};
}
std::map<std::string, gdm::Complexation> deserializeNucleusComplexForms(const QJsonObject &obj)
{
  std::map<std::string, gdm::Complexation> ret{};

  checkIfContains("complexForms", obj);
  const QJsonArray cforms = obj["complexForms"].toArray();

  for (const auto &item : cforms) {
    const QJsonObject cf = item.toObject();
    if (cf.isEmpty())
      throw MalformedJSONException{"Invalid complex form entry"};

    /* Read nucleus charge */
    int charge;
    checkIfContains("nucleusCharge", cf);
    charge = cf["nucleusCharge"].toInt();

    /* Read ligand groups */
    checkIfContains("ligandGroups", cf);
    const QJsonArray ligandGroups = cf["ligandGroups"].toArray();
    for (const auto &lg : ligandGroups) {
      QJsonObject lgObj = lg.toObject();
      checkIfContains("ligands", lgObj);
      const QJsonArray ligands = lgObj["ligands"].toArray();
      if (ligands.size() != 1)
        throw DeserializationException{"Invalid number of ligands in \"ligands\" array"};

      const QJsonObject lig = ligands.at(0).toObject();
      if (lig.isEmpty())
        throw MalformedJSONException{"Invalid ligand entry"};

      /* Read ligand name */
      std::string name{};
      checkIfContains("name", lig);
      name = lig["name"].toString().toStdString();
      if (name.length() < 1)
        throw MalformedJSONException{"Invalid ligand name"};

      /* Read ligand charge */
      int ligandCharge;
      checkIfContains("charge", lig);
      ligandCharge = lig["charge"].toInt();

      /* Read maxCount */
      size_t maxCount;
      checkIfContains("maxCount", lig);
      maxCount = lig["maxCount"].toInt();

      /* Read pBs */
      std::vector<double> pBs{};
      checkIfContains("pBs", lig);
      QJsonArray inpBs = lig["pBs"].toArray();
      if (inpBs.size() != maxCount)
        throw MalformedJSONException{"Invalid size of \"pBs\" array"};
      for (const auto &b : inpBs)
        pBs.emplace_back(b.toDouble());

      /* Read mobilities */
      std::vector<double> mobilities{};
      checkIfContains("mobilities", lig);
      QJsonArray inMobilities = lig["mobilities"].toArray();
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
    checkIfContains("type", ctuent);

    auto inType = ctuent["type"];
    if (inType == "N")
      type = gdm::ConstituentType::Nucleus;
    else if (inType == "L")
      type = gdm::ConstituentType::Ligand;
    else
      throw MalformedJSONException{"Invalid \"type\" value"};

    /* Read name */
    std::string name{};
    checkIfContains("name", ctuent);
    QString inName = ctuent["name"].toString();
    if (inName.isNull())
      throw MalformedJSONException{"Invalid \"name\""};
    name = inName.toStdString();

    /* Read chargeLow */
    int chargeLow;
    checkIfContains("chargeLow", ctuent);
    chargeLow = ctuent["chargeLow"].toInt();

    /* Read chargeHigh */
    int chargeHigh;
    checkIfContains("chargeHigh", ctuent);
    chargeHigh = ctuent["chargeHigh"].toInt();

    if (chargeLow > chargeHigh)
      throw MalformedJSONException{"Invalid charge values"};

    /* Read pKas */
    std::vector<double> pKas{};
    checkIfContains("pKas", ctuent);
    QJsonArray inpKas = ctuent["pKas"].toArray();
    if (inpKas.size() != chargeHigh - chargeLow)
      throw MalformedJSONException{"Invalid pKa array size"};
    for (const auto &d : inpKas)
      pKas.emplace_back(d.toDouble());

    /* Read mobilities */
    std::vector<double> mobilities{};
    checkIfContains("mobilities", ctuent);
    QJsonArray inMobilities = ctuent["mobilities"].toArray();
    if (inMobilities.size() != chargeHigh - chargeLow + 1)
      throw MalformedJSONException{"Invalid mobilities array size"};
    for (const auto &u : inMobilities)
      mobilities.emplace_back(u.toDouble());

    gdm::ChargeInterval charges{chargeLow, chargeHigh};
    gdm::PhysicalProperties physProps{charges, pKas, mobilities};

    if (type == gdm::ConstituentType::Nucleus) {
      auto complexForms = deserializeNucleusComplexForms(ctuent);
      ret.emplace_back(std::make_pair<gdm::Constituent, std::map<std::string, gdm::Complexation>>({type, name, physProps}, std::move(complexForms)));
    } else {
      if (ctuent.contains("complexForms"))
        throw MalformedJSONException{"Ligands must not have \"complexForms\""};

      ret.emplace_back(std::make_pair<gdm::Constituent, std::map<std::string, gdm::Complexation>>({type, name, physProps}, {}));
    }
  }

  return ret;
}

void deserializeComposition(gdm::GDM &gdm, const QJsonObject &obj)
{
  checkIfContains("constituents", obj);
  try {
    const auto constituents = deserializeConstituents(obj["constituents"].toArray());

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

void deserializeSystem(System &system, const QJsonObject &obj)
{
  const auto readDouble = [&](const QString &name, double &v, bool check = true) {
    checkIfContains(name, obj);
    v = obj[name].toDouble();
    if (v <= 0 && check)
      throw MalformedJSONException{QString{"Invalid %1 value"}.arg(name).toStdString()};
  };

  readDouble("totalLength", system.totalLength);
  readDouble("detectorPosition", system.detectorPosition);
  readDouble("drivingVoltage", system.drivingVoltage);
  readDouble("eofValue", system.eofValue, false);

  /* Read EOF type */
  checkIfContains("eofType", obj);
  QString t = obj["eofType"].toString();
  if (!(t == "N" || t == "U" || t == "T"))
    throw MalformedJSONException{"Invalid \"eofType\" value"};
  system.eofType = t;

  /* Read positive voltage */
  checkIfContains("positiveVoltage", obj);
  system.positiveVoltage = obj["positiveVoltage"].toBool();

  /* Read ionic strength */
  checkIfContains("ionicStrengthCorrection", obj);
  system.ionicStrengthCorrection = obj["ionicStrengthCorrection"].toBool();
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

  checkIfContains("compositionBGE", obj);
  deserializeComposition(gdmBGE, obj["compositionBGE"].toObject());

  checkIfContains("compositionSample", obj);
  deserializeComposition(gdmSample, obj["compositionSample"].toObject());

  checkIfContains("concentrationsBGE", obj);
  deserializeConcentrations(gdmBGE, obj["concentrationsBGE"].toObject());

  checkIfContains("concentrationsSample", obj);
  deserializeConcentrations(gdmSample, obj["concentrationsSample"].toObject());

  checkIfContains("system", obj);
  deserializeSystem(system, obj["system"].toObject());
}

} // namespace persistence
