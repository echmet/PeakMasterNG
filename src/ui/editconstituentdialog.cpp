#include "editconstituentdialog.h"
#include "ui_editconstituentdialog.h"

#include "elementaries/modifyconstituentchargepushbutton.h"
#include "../gearbox/databaseproxy.h"
#include "../gearbox/doubletostringconvertor.h"
#include "pickconstituentfromdbdialog.h"
#include "internal_models/databaseconstituentsphyspropstablemodel.h"
#include "editchargeswidget.h"
#include "../gdm/core/constituent/physicalproperties.h"

#include <QVBoxLayout>

const double EditConstituentDialog::VISCOSITY_COEFF_VERY_SMALL{0.0};
const double EditConstituentDialog::VISCOSITY_COEFF_SMALL{1.0e-4};
const double EditConstituentDialog::VISCOSITY_COEFF_LARGE{3.0e-3};

EditConstituentDialog::EditConstituentDialog(DatabaseProxy &dbProxy, const bool viscosityCorrectionEnabled, QWidget *parent) :
  QDialog{parent},
  ui{new Ui::EditConstituentDialog},
  h_dbProxy{dbProxy}
{
  setupWidget();

  m_editChargesWidget = new EditChargesWidget{this};
  static_cast<QVBoxLayout *>(layout())->insertWidget(3, m_editChargesWidget);

  ui->qgbox_viscosity->setVisible(viscosityCorrectionEnabled);
}

EditConstituentDialog::EditConstituentDialog(DatabaseProxy &dbProxy,
                                             const QString &name, const EditConstituentDialog::ConstituentType type, const gdm::PhysicalProperties &props,
                                             const bool allowTypeChange,
                                             const bool viscosityCorrectionEnabled,
                                             QWidget *parent) :
  QDialog{parent},
  ui{new Ui::EditConstituentDialog},
  h_dbProxy{dbProxy}
{
  setupWidget();

  m_editChargesWidget = new EditChargesWidget{props, this};
  static_cast<QVBoxLayout *>(layout())->insertWidget(3, m_editChargesWidget);

  ui->qle_name->setText(name);

  setConstituentType(type);
  setViscosityElements(props.viscosityCoefficient());
  ui->qgbox_viscosity->setVisible(viscosityCorrectionEnabled);

  ui->qcbox_type->setEnabled(allowTypeChange);
}

EditConstituentDialog::~EditConstituentDialog()
{
  delete ui;
}

int EditConstituentDialog::chargeLow() const
{
  return m_editChargesWidget->chargeLow();
}

int EditConstituentDialog::chargeHigh() const
{
  return m_editChargesWidget->chargeHigh();
}

std::vector<double> EditConstituentDialog::mobilities() const
{
  return m_editChargesWidget->mobilities();
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
    m_editChargesWidget->setCharges(ctuent.pKas, ctuent.mobilities, ctuent.chargeLow, ctuent.chargeHigh);
    ui->bbox_dlgCtrls->setFocus();
  } catch (const std::out_of_range &) {
    return; /* Nothing to actually handle */
  }
}

void EditConstituentDialog::onRejected()
{
  reject();
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
  return m_editChargesWidget->pKas();
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

  ui->qcbox_viscosityCoefficient->addItem(QString{"Very small compound (k = %1)"}.arg(VISCOSITY_COEFF_VERY_SMALL), VISCOSITY_COEFF_VERY_SMALL);
  ui->qcbox_viscosityCoefficient->addItem(QString{"Small compound (k = %1)"}.arg(VISCOSITY_COEFF_SMALL), VISCOSITY_COEFF_SMALL);
  ui->qcbox_viscosityCoefficient->addItem(QString{"Large compound (k = %1)"}.arg(VISCOSITY_COEFF_LARGE), VISCOSITY_COEFF_LARGE);
  ui->qcbox_viscosityCoefficient->addItem(QString{"Custom compound"}, -1);
  connect(ui->qcbox_viscosityCoefficient, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &EditConstituentDialog::onViscosityCoefficientIndexChanged);
  onViscosityCoefficientIndexChanged(0);

  ui->qcbox_type->addItem("Nucleus", QVariant::fromValue<ConstituentType>(ConstituentType::NUCLEUS));
  ui->qcbox_type->addItem("Ligand", QVariant::fromValue<ConstituentType>(ConstituentType::LIGAND));

  if (h_dbProxy.isAvailable()) {
    connect(ui->qpb_pickFromDB, &QPushButton::clicked, this, &EditConstituentDialog::onPickFromDatabase);
    connect(ui->qpb_addToDatabase, &QPushButton::clicked, this, &EditConstituentDialog::onAddToDatabase);
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
