#include "complexationmanager.h"

#include "../gdm/core/gdm.h"
#include "../gdm/core/common/gdmexcept.h"
#include "../ui/complexation_models/complexationrelationshipsmodel.h"
#include "../ui/editcomplexationdialog.h"

#include <cassert>
#include <QMessageBox>

enum class Conflict {
  NONE,
  RESOLVED,
  UNRESOLVED
};

static
int calculateHue(const int step, const int lastHue)
{
  auto wrap = [](const int v) {
    if (v < 0)
      return 360 + v;
    return v;
  };
  auto shift = [&wrap](const int hue) {
    int newHue = hue - 180;
    return wrap(newHue);
  };

  if (step % 2 == 0) {
    return shift(lastHue);
  } else {
    const int rot = 180 / (step + 1);
    int newHue = lastHue - rot;
    newHue = wrap(newHue);
    return shift(newHue);
  }
}

static
gdm::ConstituentType otherType(const gdm::ConstituentType t)
{
  if (t == gdm::ConstituentType::Ligand)
    return gdm::ConstituentType::Nucleus;
  return gdm::ConstituentType::Ligand;
}

static
QString makeOptionText(const std::string &name, const gdm::ConstituentType newType)
{
  const auto typeName = [newType]() -> QString {
    if (newType == gdm::ConstituentType::Ligand)
      return "ligand";
    return "nucleus";
  }();

  return QString(QObject::tr("Convert \"%1\" to %2")).arg(name.c_str(), typeName);
}

static
void switchRole(gdm::GDM::iterator ctuent, gdm::GDM &backgroundGDM, gdm::GDM &sampleGDM)
{
  double c = sampleGDM.concentrations(ctuent).at(0);
  auto changed = gdm::Constituent(otherType(ctuent->type()), ctuent->name(), ctuent->physicalProperties());

  sampleGDM.erase(ctuent);
  auto inserted = sampleGDM.insert(changed);
  sampleGDM.setConcentrations(inserted.first, { c });

  auto bgIt = backgroundGDM.find(changed.name());
  if (bgIt != backgroundGDM.end()) {
    c = backgroundGDM.concentrations(bgIt).at(0);
    backgroundGDM.erase(bgIt);
    inserted = backgroundGDM.insert(changed);
    backgroundGDM.setConcentrations(inserted.first, { c });
  }
}

static
Conflict resolveTypeConflict(gdm::GDM::iterator first, gdm::GDM::iterator second, gdm::GDM &backgroundGDM, gdm::GDM &sampleGDM)
{
  if (first->type() != second->type())
    return Conflict::NONE;

  QMessageBox mbox{QMessageBox::Question, QObject::tr("Constituent types conflict"), QObject::tr("Constituents have incompatible types to form a complexation relationship. What do you want to do?")};

  QPushButton *fixFirst = nullptr;
  QPushButton *fixSecond = nullptr;

  auto cpxs = gdm::findComplexations(sampleGDM.composition(), first);
  if (cpxs.empty())
    fixFirst = mbox.addButton(makeOptionText(first->name(), otherType(first->type())), QMessageBox::AcceptRole);

  cpxs = gdm::findComplexations(sampleGDM.composition(), second);
  if (cpxs.empty())
    fixSecond = mbox.addButton(makeOptionText(second->name(), otherType(second->type())), QMessageBox::AcceptRole);

  mbox.addButton(QMessageBox::Cancel);

  mbox.exec();

  if (static_cast<void *>(mbox.clickedButton()) == fixFirst) {
    assert(fixFirst != nullptr);
    switchRole(first, backgroundGDM, sampleGDM);
    return Conflict::RESOLVED;
  } else if (static_cast<void *>(mbox.clickedButton()) == fixSecond) {
    assert(fixSecond != nullptr);
    switchRole(second, backgroundGDM, sampleGDM);
    return Conflict::RESOLVED;
  }

  return Conflict::UNRESOLVED;
}

static
EditComplexationDialog * makeComplexationDialog(const gdm::GDM::const_iterator nucleusIt, const gdm::GDM::const_iterator ligandIt, ComplexationRelationshipsModel *model, const gdm::GDM &gdm)
{
  auto dlg = new EditComplexationDialog{};
  dlg->setComplexationModel(model);

  const auto nucleusCharges = nucleusIt->physicalProperties().charges();

  auto nucleus = std::make_shared<::Constituent>(nucleusCharges.low(), nucleusCharges.high(), QString::fromStdString(nucleusIt->name()), ::Constituent::Type::NUCLEUS);

  const auto complexations = gdm::findComplexations(gdm.composition(), nucleusIt);

  for (const auto &cpxn : complexations) {
    const auto &_ligandIt = cpxn.first;
    const auto &_cpxn = cpxn.second;
    const auto _ligandCharges = _ligandIt->physicalProperties().charges();

    auto ligand = std::make_shared<::Constituent>(_ligandCharges.low(), _ligandCharges.high(), QString::fromStdString(_ligandIt->name()), ::Constituent::Type::LIGAND);
    ComplexationRelationship rel{nucleus, ligand};

    for (auto nucleusCharge = nucleusCharges.low(); nucleusCharge <= nucleusCharges.high(); nucleusCharge++) {
      for (auto _ligandCharge = _ligandCharges.low(); _ligandCharge <= _ligandCharges.high(); _ligandCharge++) {
        gdm::ChargeCombination chargesCombo{nucleusCharge, _ligandCharge};

        auto complexFormIt = _cpxn.find(chargesCombo);
        if (complexFormIt != _cpxn.cend())
          rel.addComplexForm(nucleusCharge, _ligandCharge, QVector<double>::fromStdVector(complexFormIt->mobilities()), QVector<double>::fromStdVector(complexFormIt->pBs()));
      }
    }

    dlg->addRelationship(rel);
  }

  if (ligandIt != gdm.cend()) {
    const auto ligandCharges = ligandIt->physicalProperties().charges();
    auto newLigand = std::make_shared<::Constituent>(ligandCharges.low(), ligandCharges.high(), QString::fromStdString(ligandIt->name()), ::Constituent::Type::LIGAND);
    dlg->addRelationship({nucleus, newLigand});
  }

  return dlg;
}

static
bool processComplexationsForGDM(const std::string &nucleus, gdm::GDM &GDM,
                                const EditComplexationDialog::AllComplexationRelationships &allRels,
                                const EditComplexationDialog::AllComplexationRelationships &oldRels)
{
  std::vector<std::pair<gdm::GDM::const_iterator, gdm::Complexation>> allComplexations{};

  const auto nucleusIt = GDM.find(nucleus);

  if (nucleusIt == GDM.cend())
    return true;

  /* Clear all old complexations */
  for (auto mapIt = oldRels.cbegin(); mapIt != oldRels.cend(); mapIt++) {
    const auto ligandIt = GDM.find(mapIt.key().toStdString());
    if (ligandIt == GDM.cend())
      continue;

    GDM.eraseComplexation(nucleusIt, ligandIt);
  }

  const auto ncCharges = nucleusIt->physicalProperties().charges();

  for (auto mapIt = allRels.cbegin(); mapIt != allRels.cend(); mapIt++) {
    gdm::Complexation complexations{};

    auto ligandIt = GDM.find(mapIt.key().toStdString());

    if (ligandIt == GDM.cend())
      continue;

    const auto &ligandMap = *mapIt;

    const auto lgCharges = ligandIt->physicalProperties().charges();
    for (auto nucleusCharge = ncCharges.low(); nucleusCharge <= ncCharges.high(); nucleusCharge++) {
      if (!ligandMap.contains(nucleusCharge))
        continue;

      const auto &ncBlock = ligandMap[nucleusCharge];
      for (auto ligandCharge = lgCharges.low(); ligandCharge <= lgCharges.high(); ligandCharge++) {
        if (!ncBlock.contains(ligandCharge))
          continue;

        const auto &cpxnProps = ncBlock[ligandCharge];
        const auto mobilities = std::get<0>(cpxnProps).toStdVector();
        const auto pBs = std::get<1>(cpxnProps).toStdVector();

        if (mobilities.size() != pBs.size()) {
          QMessageBox mbox{QMessageBox::Warning, QObject::tr("Invalid complexation parameters"), QObject::tr("Mismatching number of mobilities and pBs")};
          mbox.exec();
          return false;
        }

        if (mobilities.empty())
          continue;

        try {
          gdm::ChargeCombination cc = {nucleusCharge, ligandCharge};
          gdm::ComplexForm cForm{cc, mobilities.size(), pBs, mobilities};
          complexations.insert(std::move(cForm));
        } catch (gdm::InvalidArgument &ex) {
          QMessageBox mbox{QMessageBox::Warning, QObject::tr("Invalid complexation parameters"), ex.what()};
          mbox.exec();
          return false;
        }
      }
    }

    allComplexations.emplace_back(ligandIt, std::move(complexations));
  }

  for (auto it = allComplexations.begin(); it != allComplexations.end(); it++) {
    const auto ligandIt = it->first;
    auto &&complexations = it->second;

    if (!complexations.empty()) {
      try {
        GDM.setComplexation(nucleusIt, ligandIt, std::move(complexations));
      } catch (gdm::InvalidArgument &ex) {
        QMessageBox mbox{QMessageBox::Warning, QObject::tr("Failed to set complexations"), ex.what()};
        return false;
      }
    }
  }

  return true;
}

void ComplexationManager::addNucleus(const std::string &nucleusName)
{
  if (m_complexingNuclei.find(nucleusName) != m_complexingNuclei.cend())
    return;

  int max = 0;
  for (auto it = m_complexingNuclei.cbegin(); it != m_complexingNuclei.cend(); it++) {
    if (it->second > max)
      max = it->second;
  }

  m_complexingNuclei.emplace(nucleusName, max + 1);
}

ComplexationManager::ComplexationManager(gdm::GDM &backgroundGDM, gdm::GDM &sampleGDM, QObject *parent) :
  QObject{parent},
  h_backgroundGDM{backgroundGDM},
  h_sampleGDM{sampleGDM}
{
}

std::vector<int> ComplexationManager::complexationStatus(const std::string &name) const
{
  const auto it = m_complexationStatus.find(name);
  if (it == m_complexationStatus.cend())
    return {};

  return it->second;
}

bool ComplexationManager::complexes(const std::string &name)
{
  const auto it = h_sampleGDM.find(name);

  if (it == h_sampleGDM.cend())
    return false;

  const auto found = gdm::findComplexations(h_sampleGDM.composition(), it);
  return !found.empty();
}

void ComplexationManager::editComplexation(const std::string &name)
{
  auto it = h_sampleGDM.find(name);

  if (it == h_sampleGDM.cend())
    return;

  if (it->type() == gdm::ConstituentType::Ligand) {
    QMessageBox mbox{QMessageBox::Information, QObject::tr("Invalid input"), QObject::tr("Please use nuclei to edit complexations")};
    mbox.exec();
    return;
  }

  auto model = new ComplexationRelationshipsModel{};
  auto dlg = makeComplexationDialog(it, h_sampleGDM.cend(), model, h_sampleGDM);
  handleUserInput(dlg, name, "");

  delete model;
  delete dlg;
}

void ComplexationManager::handleUserInput(EditComplexationDialog *dlg, const std::string &nucleusName, const std::string &ligandName)
{
  if (ligandName.length() > 0)
    dlg->showLigand(QString::fromStdString(ligandName));

  const auto oldRelationships = dlg->allRelationships();

  while (true) {
    const int answer = dlg->exec();
    if (answer != QDialog::Accepted)
      return;

    const auto relationships = dlg->allRelationships();

    if (!processComplexationsForGDM(nucleusName, h_backgroundGDM, relationships, oldRelationships))
      continue;
    if (!processComplexationsForGDM(nucleusName, h_sampleGDM, relationships, oldRelationships))
      continue;

    break;
  }

  updateComplexingNuclei();
  updateComplexationStatus();
}

void ComplexationManager::makeComplexation(const std::string &first, const std::string &second)
{
  auto nucleusIt = h_sampleGDM.find(first);
  auto ligandIt = h_sampleGDM.find(second);

  auto conflict = resolveTypeConflict(nucleusIt, ligandIt, h_backgroundGDM, h_sampleGDM);
  switch (conflict) {
  case Conflict::NONE:
    break;
  case Conflict::UNRESOLVED:
    return;
  case Conflict::RESOLVED:
    /* Iterators may have been invalidated, get them again */
    nucleusIt = h_sampleGDM.find(first);
    ligandIt = h_sampleGDM.find(second);
    break;
  }

  assert(nucleusIt != h_sampleGDM.end() && ligandIt != h_sampleGDM.end());
  assert(nucleusIt->type() != ligandIt->type());

  if (nucleusIt->type() != gdm::ConstituentType::Nucleus)
    std::swap(nucleusIt, ligandIt);

  assert(nucleusIt->type() == gdm::ConstituentType::Nucleus && ligandIt->type() == gdm::ConstituentType::Ligand);

  auto model = new ComplexationRelationshipsModel{};
  auto dlg = makeComplexationDialog(nucleusIt, ligandIt, model, h_sampleGDM);
  handleUserInput(dlg, nucleusIt->name(), ligandIt->name());

  delete model;
  delete dlg;
}

void ComplexationManager::notifyConstituentRemoved()
{
  updateComplexingNuclei();
  updateComplexationStatus();
}

void ComplexationManager::updateComplexingNuclei()
{
  int ctr = 0;
  int hue = 225;
  ComplexingNucleiMap map{};

  for (auto it = h_sampleGDM.cbegin(); it != h_sampleGDM.cend(); it++) {
    if (it->type() != gdm::ConstituentType::Nucleus)
      continue;

    const auto found = gdm::findComplexations(h_sampleGDM.composition(), it);
    if (!found.empty()) {
      map[it->name()] = hue;
      hue = calculateHue(ctr++, hue);
    }
  }

  m_complexingNuclei = std::move(map);
}

void ComplexationManager::updateComplexationStatus()
{
  ComplexationStatusMap map;

  for (auto it = h_sampleGDM.cbegin(); it != h_sampleGDM.cend(); it++) {
    if (it->type() == gdm::ConstituentType::Nucleus) {
      const auto _it = m_complexingNuclei.find(it->name());
      if (_it != m_complexingNuclei.cend())
        map[it->name()] = { _it->second };
    } else {
      std::vector<int> nucleiIDs{};

      const auto found = gdm::findComplexations(h_sampleGDM.composition(), it);
      for (auto _it = found.cbegin(); _it != found.cend(); _it++)
        nucleiIDs.emplace_back(m_complexingNuclei.at(_it->first->name()));

      map[it->name()] = std::move(nucleiIDs);
    }
  }

  m_complexationStatus = std::move(map);
  emit complexationStatusChanged();
}

void ComplexationManager::refreshAll()
{
  m_complexationStatus.clear();
  m_complexingNuclei.clear();

  updateComplexingNuclei();
  updateComplexationStatus();
}
