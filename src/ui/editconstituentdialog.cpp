#include "editconstituentdialog.h"
#include "ui_editconstituentdialog.h"

#include "elementaries/modifyconstituentchargepushbutton.h"
#include "../gdm/core/constituent/physicalproperties.h"
#include "../gearbox/floatingvaluedelegate.h"
#include "../gearbox/databaseproxy.h"
#include "pickconstituentfromdbdialog.h"
#include "internal_models/databaseconstituentsphyspropstablemodel.h"

EditConstituentDialog::EditConstituentDialog(DatabaseProxy &dbProxy, QWidget *parent) :
  QDialog{parent},
  ui{new Ui::EditConstituentDialog},
  h_dbProxy{dbProxy},
  m_fltDelegate{new FloatingValueDelegate{this}}
{
  setupWidget();
  updateChargeModifiers();
  ui->qtblView_charges->setItemDelegate(m_fltDelegate);
}

EditConstituentDialog::EditConstituentDialog(DatabaseProxy &dbProxy,
                                             const QString &name, const EditConstituentDialog::ConstituentType type, const gdm::PhysicalProperties &props,
                                             const bool allowTypeChange, QWidget *parent) :
  QDialog{parent},
  ui{new Ui::EditConstituentDialog},
  h_dbProxy{dbProxy},
  m_fltDelegate{new FloatingValueDelegate{this}}
{
  setupWidget();

  ui->qle_name->setText(name);

  if (props.charges().high() == props.charges().low() && props.charges().low() != 0) {
    m_chargesModel.insertRow(0);
    m_chargesModel.setHeaderData(0, Qt::Vertical, props.charges().low());
    m_chargesModel.setData(m_chargesModel.index(0, ConstituentChargesModel::MOBILITY), props.mobility(props.charges().low()));
    m_chargesModel.removeRow(m_chargesModel.rowCount() - 1);
  } else {
    for (int charge = -1; charge >= props.charges().low(); charge--) {
      m_chargesModel.insertRow(0);
      m_chargesModel.setData(m_chargesModel.index(0, ConstituentChargesModel::MOBILITY), props.mobility(charge));
      m_chargesModel.setData(m_chargesModel.index(0, ConstituentChargesModel::PKA), props.pKa(charge));
    }

    for (int charge = 1; charge <= props.charges().high(); charge++) {
      m_chargesModel.insertRow(m_chargesModel.rowCount());
      m_chargesModel.setData(m_chargesModel.index(m_chargesModel.rowCount() - 1, ConstituentChargesModel::MOBILITY), props.mobility(charge));
      m_chargesModel.setData(m_chargesModel.index(m_chargesModel.rowCount() - 1, ConstituentChargesModel::PKA), props.pKa(charge));
    }
  }

  setConstituentType(type);

  ui->qcbox_type->setEnabled(allowTypeChange);

  updateChargeModifiers();
  ui->qtblView_charges->setItemDelegate(m_fltDelegate);
}

EditConstituentDialog::~EditConstituentDialog()
{
  delete ui;
}

int EditConstituentDialog::chargeHigh() const
{
  return m_chargesModel.headerData(m_chargesModel.rowCount() - 1 , Qt::Vertical).toInt();
}

int EditConstituentDialog::chargeLow() const
{
  return m_chargesModel.headerData(0, Qt::Vertical).toInt();
}

std::vector<double> EditConstituentDialog::mobilities() const
{
  std::vector<double> mobilities{};
  mobilities.reserve(static_cast<size_t>(m_chargesModel.rowCount()));

  for (int row = 0; row < m_chargesModel.rowCount(); row++)
    mobilities.emplace_back(m_chargesModel.data(m_chargesModel.index(row, ConstituentChargesModel::MOBILITY)).toReal());

  return mobilities;
}

QString EditConstituentDialog::name() const
{
  return ui->qle_name->text();
}

void EditConstituentDialog::onAccepted()
{
  bool ok;

  emit validateInput(this, &ok);

  if (ok)
    accept();
}

void EditConstituentDialog::onAddChargeLow()
{
  if (m_chargesModel.insertRows(0, 1))
    updateChargeLow();
}

void EditConstituentDialog::onAddChargeHigh()
{
  if (m_chargesModel.insertRows(m_chargesModel.rowCount(), 1))
    updateChargeHigh();
}

void EditConstituentDialog::onPickFromDatabase()
{
  DatabaseConstituentsPhysPropsTableModel model{};
  PickConstituentFromDBDialog dlg{model, h_dbProxy};

  if (dlg.exec() != QDialog::Accepted)
    return;

  const int idx = dlg.selectedIndex();
  if (idx < 0)
    return;

  try {
    const auto &ctuent = model.constituentAt(idx);

    ui->qle_name->setText(ctuent.name);
    m_chargesModel.refreshData(ctuent.pKas, ctuent.mobilities, ctuent.chargeLow, ctuent.chargeHigh);
    updateChargeModifiers();
  } catch (const std::out_of_range &) {
    return; /* Nothing to actually handle */
  }
}

void EditConstituentDialog::onRejected()
{
  reject();
}

void EditConstituentDialog::onRemoveChargeHigh()
{
  if (m_chargesModel.removeRows(m_chargesModel.rowCount() - 1, 1))
    updateChargeHigh();
}

void EditConstituentDialog::onRemoveChargeLow()
{
  if (m_chargesModel.removeRows(0, 1))
    updateChargeLow();
}

std::vector<double> EditConstituentDialog::pKas() const
{
  std::vector<double> pKas{};
  pKas.reserve(static_cast<size_t>(m_chargesModel.rowCount() - 1));

  if (m_chargesModel.rowCount() == 1)
    return pKas;

  for (int row = 0; row < m_chargesModel.rowCount(); row++) {
    const int charge = m_chargesModel.headerData(row, Qt::Vertical).toInt();
    if (charge == 0)
      continue;

    pKas.emplace_back(m_chargesModel.data(m_chargesModel.index(row, ConstituentChargesModel::PKA)).toReal());
  }

  return pKas;
}

void EditConstituentDialog::setConstituentType(const ConstituentType type)
{
  for (int idx = 0; idx < ui->qcbox_type->count(); idx++) {
    if (ui->qcbox_type->itemData(idx).value<ConstituentType>() == type) {
      ui->qcbox_type->setCurrentIndex(idx);
      return;
    }
  }
}

void EditConstituentDialog::setupWidget()
{
  ui->setupUi(this);
  ui->qtblView_charges->setModel(&m_chargesModel);

  m_qpb_addLow = new ModifyConstituentChargePushButton{ModifyConstituentChargePushButton::ChargeOperation::ADD, this};
  m_qpb_removeLow = new ModifyConstituentChargePushButton{ModifyConstituentChargePushButton::ChargeOperation::REMOVE, this};
  m_qpb_addHigh = new ModifyConstituentChargePushButton{ModifyConstituentChargePushButton::ChargeOperation::ADD, this};
  m_qpb_removeHigh = new ModifyConstituentChargePushButton{ModifyConstituentChargePushButton::ChargeOperation::REMOVE, this};

  connect(m_qpb_addLow, &QPushButton::clicked, this, &EditConstituentDialog::onAddChargeLow);
  connect(m_qpb_removeLow, &QPushButton::clicked, this, &EditConstituentDialog::onRemoveChargeLow);
  connect(m_qpb_addHigh, &QPushButton::clicked, this, &EditConstituentDialog::onAddChargeHigh);
  connect(m_qpb_removeHigh, &QPushButton::clicked, this, &EditConstituentDialog::onRemoveChargeHigh);

  ui->qlay_lowChargeButtons->layout()->addWidget(m_qpb_addLow);
  ui->qlay_lowChargeButtons->layout()->addWidget(m_qpb_removeLow);
  ui->qlay_highChargeButtons->layout()->addWidget(m_qpb_addHigh);
  ui->qlay_highChargeButtons->layout()->addWidget(m_qpb_removeHigh);

  ui->qcbox_type->addItem("Nucleus", QVariant::fromValue<ConstituentType>(ConstituentType::NUCLEUS));
  ui->qcbox_type->addItem("Ligand", QVariant::fromValue<ConstituentType>(ConstituentType::LIGAND));

  if (h_dbProxy.isAvailable()) {
    connect(ui->qpb_pickFromDB, &QPushButton::clicked, this, &EditConstituentDialog::onPickFromDatabase);
    ui->qpb_pickFromDB->setEnabled(true);
  } else
    ui->qpb_pickFromDB->setEnabled(false);
}

EditConstituentDialog::ConstituentType EditConstituentDialog::type() const
{
  return ui->qcbox_type->currentData().value<ConstituentType>();
}

void EditConstituentDialog::updateChargeHigh()
{
  const int chargeHigh = m_chargesModel.headerData(m_chargesModel.rowCount() - 1, Qt::Orientation::Vertical).toInt();

  m_qpb_addHigh->setText(chargeHigh + 1);
  m_qpb_removeHigh->setText(chargeHigh);
}

void EditConstituentDialog::updateChargeLow()
{
  const int chargeLow = m_chargesModel.headerData(0, Qt::Orientation::Vertical).toInt();

  m_qpb_addLow->setText(chargeLow - 1);
  m_qpb_removeLow->setText(chargeLow);
}

void EditConstituentDialog::updateChargeModifiers()
{
  updateChargeHigh();
  updateChargeLow();
}
