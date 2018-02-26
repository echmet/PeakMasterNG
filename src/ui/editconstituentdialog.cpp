#include "editconstituentdialog.h"
#include "ui_editconstituentdialog.h"

#include "elementaries/modifyconstituentchargepushbutton.h"
#include "../gdm/core/constituent/physicalproperties.h"
#include "../gearbox/floatingvaluedelegate.h"
#include "../gearbox/databaseproxy.h"
#include "../gearbox/doubletostringconvertor.h"
#include "pickconstituentfromdbdialog.h"
#include "internal_models/databaseconstituentsphyspropstablemodel.h"

const double EditConstituentDialog::VISCOSITY_COEFF_VERY_SMALL{0.0};
const double EditConstituentDialog::VISCOSITY_COEFF_SMALL{1.0e-4};
const double EditConstituentDialog::VISCOSITY_COEFF_LARGE{3.0e-3};

EditConstituentDialog::EditConstituentDialog(DatabaseProxy &dbProxy, const bool viscosityCorrectionEnabled, QWidget *parent) :
  QDialog{parent},
  ui{new Ui::EditConstituentDialog},
  h_dbProxy{dbProxy},
  m_fltDelegate{new FloatingValueDelegate{this}}
{
  setupWidget();
  updateChargeModifiers();
  ui->qtblView_charges->setItemDelegate(m_fltDelegate);

  ui->qgbox_viscosity->setVisible(viscosityCorrectionEnabled);
}

EditConstituentDialog::EditConstituentDialog(DatabaseProxy &dbProxy,
                                             const QString &name, const EditConstituentDialog::ConstituentType type, const gdm::PhysicalProperties &props,
                                             const bool allowTypeChange,
                                             const bool viscosityCorrectionEnabled,
                                             QWidget *parent) :
  QDialog{parent},
  ui{new Ui::EditConstituentDialog},
  h_dbProxy{dbProxy},
  m_fltDelegate{new FloatingValueDelegate{this}}
{
  setupWidget();

  ui->qle_name->setText(name);

  const int chargeLow = props.charges().low();
  const int chargeHigh = props.charges().high();
  const int bChg = [&]() {
    if (chargeHigh < 0)
      return chargeHigh;
    if (chargeLow > 0)
      return chargeLow;
    return 0;
  }();

  std::map<int, double> pKas;
  std::map<int, double> mobilities;
  for (int charge = chargeLow; charge <= chargeHigh; charge++) {
    mobilities.emplace(charge, props.mobility(charge));
    if (charge != bChg)
      pKas.emplace(charge, props.pKa(charge));
    else
      pKas.emplace(charge, 0);
  }
  m_chargesModel.refreshData(pKas, mobilities, chargeLow, chargeHigh);

  setConstituentType(type);
  setViscosityElements(props.viscosityCoefficient());
  ui->qgbox_viscosity->setVisible(viscosityCorrectionEnabled);

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

void EditConstituentDialog::onAddToDatabase()
{
  emit addToDatabase(this);
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
    ui->bbox_dlgCtrls->setFocus();
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

void EditConstituentDialog::onViscosityCoefficientIndexChanged(const int idx)
{
  const QVariant v = ui->qcbox_viscosityCoefficient->itemData(idx);
  const double k = v.toDouble();

  if (k < 0.0)
    ui->qle_customCoefficient->setReadOnly(false);
  else {
    ui->qle_customCoefficient->setReadOnly(true);
    ui->qle_customCoefficient->setText(DoubleToStringConvertor::convert(k));
  }
}

std::vector<double> EditConstituentDialog::pKas() const
{
  std::vector<double> pKas{};
  pKas.reserve(static_cast<size_t>(m_chargesModel.rowCount() - 1));

  if (m_chargesModel.rowCount() == 1)
    return pKas;

  for (int row = 0; row < m_chargesModel.rowCount(); row++) {
    const int charge = m_chargesModel.headerData(row, Qt::Vertical).toInt();
    if (m_chargesModel.isBaseCharge(charge))
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

void EditConstituentDialog::setViscosityElements(const double viscosityCoefficient)
{
  const int lastIdx = ui->qcbox_viscosityCoefficient->count() - 1;
  for (int idx = 0; idx <= lastIdx; idx++) {
    const QVariant v = ui->qcbox_viscosityCoefficient->itemData(idx);
    const double k = v.toDouble();

    if (k == viscosityCoefficient) {
      ui->qcbox_viscosityCoefficient->setCurrentIndex(idx);
      onViscosityCoefficientIndexChanged(idx);
      return;
    }
  }

  ui->qcbox_viscosityCoefficient->setCurrentIndex(lastIdx);
  onViscosityCoefficientIndexChanged(lastIdx);
  ui->qle_customCoefficient->setText(DoubleToStringConvertor::convert(viscosityCoefficient));
}

void EditConstituentDialog::setupWidget()
{
  ui->setupUi(this);
  ui->qtblView_charges->setModel(&m_chargesModel);

  ui->qcbox_viscosityCoefficient->addItem(QString{"Very small compound (k = %1)"}.arg(VISCOSITY_COEFF_VERY_SMALL), VISCOSITY_COEFF_VERY_SMALL);
  ui->qcbox_viscosityCoefficient->addItem(QString{"Small compound (k = %1)"}.arg(VISCOSITY_COEFF_SMALL), VISCOSITY_COEFF_SMALL);
  ui->qcbox_viscosityCoefficient->addItem(QString{"Large compound (k = %1)"}.arg(VISCOSITY_COEFF_LARGE), VISCOSITY_COEFF_LARGE);
  ui->qcbox_viscosityCoefficient->addItem(QString{"Custom compound"}, -1);
  connect(ui->qcbox_viscosityCoefficient, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &EditConstituentDialog::onViscosityCoefficientIndexChanged);
  onViscosityCoefficientIndexChanged(0);

  m_qpb_addLow = new ModifyConstituentChargePushButton{ModifyConstituentChargePushButton::ChargeOperation::ADD, this};
  m_qpb_removeLow = new ModifyConstituentChargePushButton{ModifyConstituentChargePushButton::ChargeOperation::REMOVE, this};
  m_qpb_addHigh = new ModifyConstituentChargePushButton{ModifyConstituentChargePushButton::ChargeOperation::ADD, this};
  m_qpb_removeHigh = new ModifyConstituentChargePushButton{ModifyConstituentChargePushButton::ChargeOperation::REMOVE, this};

  connect(m_qpb_addLow, &QPushButton::clicked, this, &EditConstituentDialog::onAddChargeLow);
  connect(m_qpb_removeLow, &QPushButton::clicked, this, &EditConstituentDialog::onRemoveChargeLow);
  connect(m_qpb_addHigh, &QPushButton::clicked, this, &EditConstituentDialog::onAddChargeHigh);
  connect(m_qpb_removeHigh, &QPushButton::clicked, this, &EditConstituentDialog::onRemoveChargeHigh);
  connect(ui->qpb_addToDatabase, &QPushButton::clicked, this, &EditConstituentDialog::onAddToDatabase);

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

double EditConstituentDialog::viscosityCoefficient() const
{
  bool ok;
  const double d = DoubleToStringConvertor::back(ui->qle_customCoefficient->text(), &ok);
  if (!ok)
    return -1.0;

  return d;
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
