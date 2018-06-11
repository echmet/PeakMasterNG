#include "complexationmanager.h"

#include "../gdm/core/gdm.h"
#include "../gdm/core/common/gdmexcept.h"
#include "../ui/complexation_models/complexationrelationshipsmodel.h"
#include "../ui/editcomplexationdialog.h"

#include <cassert>
#include <QMessageBox>

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
EditComplexationDialog * makeComplexationDialog(const gdm::GDM::const_iterator nucleusIt, const gdm::GDM::const_iterator ligandIt, ComplexationRelationshipsModel *model, const gdm::GDM &gdm)
{
  EditComplexationDialog *dlg = new EditComplexationDialog{};
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
        if (complexFormIt != _cpxn.cend()) {
          QVector<double> _mobilities;
          QVector<double> _pBs;

          rel.addComplexForm(nucleusCharge, _ligandCharge, QVector<double>::fromStdVector(complexFormIt->mobilities()), QVector<double>::fromStdVector(complexFormIt->pBs()));
        }
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

        if (mobilities.size() == 0)
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

    if (complexations.size() > 0) {
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
  return found.size() > 0;
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

  ComplexationRelationshipsModel *model = new ComplexationRelationshipsModel{};
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

  if (nucleusIt->type() == ligandIt->type()) {
    QMessageBox mbox{QMessageBox::Information, QObject::tr("Invalid input"), QObject::tr("Complexations are allowed only between ligands and nuclei")};
    mbox.exec();
    return;
  }

  if (nucleusIt->type() != gdm::ConstituentType::Nucleus)
    std::swap(nucleusIt, ligandIt);

  assert(nucleusIt->type() == gdm::ConstituentType::Nucleus && ligandIt->type() == gdm::ConstituentType::Ligand);

  ComplexationRelationshipsModel *model = new ComplexationRelationshipsModel{};
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
    if (found.size() > 0) {
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
