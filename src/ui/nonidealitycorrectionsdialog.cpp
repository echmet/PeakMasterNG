#include "nonidealitycorrectionsdialog.h"
#include "ui_nonidealitycorrectionsdialog.h"

#include <QMessageBox>

NonidealityCorrectionsDialog::NonidealityCorrectionsDialog(QWidget *parent) :
  QDialog{parent},
  ui{new Ui::NonidealityCorrectionsDialog}
{
  ui->setupUi(this);

  connect(ui->qcb_viscosity, &QCheckBox::stateChanged, this, &NonidealityCorrectionsDialog::onViscosityCorrectionToggled);
  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &NonidealityCorrectionsDialog::onAccept);
  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &NonidealityCorrectionsDialog::onReject);
}

NonidealityCorrectionsDialog::~NonidealityCorrectionsDialog()
{
  delete ui;
}

int NonidealityCorrectionsDialog::exec()
{
  m_lastState = state();
  return QDialog::exec();
}

void NonidealityCorrectionsDialog::onAccept()
{
  m_lastState = state();
  accept();
}

void NonidealityCorrectionsDialog::onReject()
{
  setState(m_lastState);
  reject();
}

void NonidealityCorrectionsDialog::setState(const bool debyeHuckel, const bool onsagerFuoss, const bool viscosity)
{
  ui->qcb_debyeHuckel->setChecked(debyeHuckel);
  ui->qcb_onsagerFuoss->setChecked(onsagerFuoss);
  ui->qcb_viscosity->setChecked(viscosity);
}

void NonidealityCorrectionsDialog::setState(const State &state)
{
  setState(state.debyeHuckel, state.onsagerFuoss, state.viscosity);
}

NonidealityCorrectionsDialog::State NonidealityCorrectionsDialog::state() const
{
  return {ui->qcb_debyeHuckel->isChecked(), ui->qcb_onsagerFuoss->isChecked(), ui->qcb_viscosity->isChecked()};
}

void NonidealityCorrectionsDialog::onViscosityCorrectionToggled(const int state)
{
  if (state == Qt::Checked) {
    QMessageBox msg{QMessageBox::Warning,
                    tr("Confirm action"),
                    tr("Current implementation of viscosity correction is very approximative and unlikely to yield better results "
                       "unless you are familiar with its specifics and scope of validity.\n\n"
                       "Are you sure you want to enable viscosity corrrection?"),
                    QMessageBox::Yes | QMessageBox::No};

    int ret = msg.exec();
    if (ret != QMessageBox::Yes)
      ui->qcb_viscosity->setCheckState(Qt::Unchecked);
  }
}
