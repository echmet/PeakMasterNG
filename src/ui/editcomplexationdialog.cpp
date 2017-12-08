#include "editcomplexationdialog.h"
#include "ui_editcomplexationdialog.h"
#include "complexation_models/constituent_ui.h"

#include <QComboBox>
#include <QPushButton>


EditComplexationDialog::EditComplexationDialog(QWidget *parent) :
  QDialog{parent},
  ui{new Ui::EditComplexationDialog}
{
  ui->setupUi(this);

  connect(ui->qcbox_ligands, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &EditComplexationDialog::onLigandChanged);
  connect(ui->qpb_remove, &QPushButton::clicked, this, &EditComplexationDialog::onRemoveLigandClicked);

  connect(ui->qbbox_okCanc, &QDialogButtonBox::accepted, this, &EditComplexationDialog::onAcceptClicked);
  connect(ui->qbbox_okCanc, &QDialogButtonBox::rejected, this, &EditComplexationDialog::onRejectClicked);
}

EditComplexationDialog::~EditComplexationDialog()
{
  delete ui;
}

void EditComplexationDialog::addRelationship(const ComplexationRelationship &relationship)
{
  if (_relationships.contains(relationship.ligand()->name))
    return;

  auto root = _crModel->buildTree(relationship);

  if (root == nullptr)
    return;

  _relationships.insert(relationship.ligand()->name, QPair<std::shared_ptr<ComplexationRelationshipsModel::RootTreeItem>, ComplexationRelationship>(root, relationship));

  ui->qcbox_ligands->addItem(relationship.ligand()->name, relationship.ligand()->name);
  ui->qcbox_ligands->activated(ui->qcbox_ligands->count() - 1);
}

EditComplexationDialog::AllComplexationRelationships EditComplexationDialog::allRelationships() const
{
  AllComplexationRelationships rels{};

  for (auto rootIt = _relationships.cbegin(); rootIt != _relationships.cend(); rootIt++)
    rels.insert(rootIt.key(), relationshipsForTree(rootIt.value().first));

  return rels;
}

void EditComplexationDialog::onAcceptClicked()
{
  accept();
}

void EditComplexationDialog::onLigandChanged(const int idx)
{
  const QString s = ui->qcbox_ligands->itemData(idx).toString();

  const auto &rel = _relationships[s];

  _crModel->setRoot(rel.first);
  ui->qtrv_complexation->expandAll();
}

void EditComplexationDialog::onRejectClicked()
{
  reject();
}

void EditComplexationDialog::onRemoveLigandClicked()
{
  if (ui->qcbox_ligands->count() < 1)
    return;

  const int idx = ui->qcbox_ligands->currentIndex();
  const QString s = ui->qcbox_ligands->itemData(idx).toString();

  ui->qcbox_ligands->removeItem(idx);
  _relationships.remove(s);

  if (ui->qcbox_ligands->count() < 1) {
    _crModel->clear();
    return;
  }

  int newIdx = (idx > 1) ? idx - 1 : 0;
  ui->qcbox_ligands->activated(newIdx);
}

ComplexationRelationship::RelationshipsMap EditComplexationDialog::relationshipsForTree(const std::shared_ptr<ComplexationRelationshipsModel::RootTreeItem> &root) const
{
  ComplexationRelationship::RelationshipsMap relMap{};

  for (int idx = 0; idx < root->childrenCount(); idx++) {
    ComplexationRelationshipsModel::NucleusTreeItem *nti = static_cast<ComplexationRelationshipsModel::NucleusTreeItem *>(root->childAt(idx));

    for (int jdx = 0; jdx < nti->childrenCount(); jdx++) {
      const ComplexationRelationshipsModel::LigandTreeItem *lti = static_cast<const ComplexationRelationshipsModel::LigandTreeItem *>(nti->childAt(jdx));

      relMap[nti->charge][lti->charge] = {lti->mobilities, lti->pBs};
    }
  }

  return relMap;
}

void EditComplexationDialog::setComplexationModel(QAbstractItemModel *model)
{
  ComplexationRelationshipsModel *_model = qobject_cast<ComplexationRelationshipsModel *>(model);
  if (_model == nullptr)
    return;

  _crModel = _model;

  ui->qtrv_complexation->setModel(_crModel);
}

void EditComplexationDialog::showLigand(const QString &name)
{
  for (int idx = 0; idx < ui->qcbox_ligands->count(); idx++) {
    if (ui->qcbox_ligands->itemData(idx) == name) {
      ui->qcbox_ligands->setCurrentIndex(idx);
      ui->qcbox_ligands->activated(idx);
      return;
    }
  }
}
