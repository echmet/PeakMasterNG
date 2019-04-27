#include "checkforupdatedialog.h"
#include "ui_checkforupdatedialog.h"
#include "softwareupdatewidget.h"
#include "../softwareupdateresult.h"
#include "../persistence/swsettings.h"

CheckForUpdateDialog::CheckForUpdateDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::CheckForUpdateDialog)
{
  ui->setupUi(this);

  ui->ql_inProgress->setVisible(false);

  connect(ui->qcb_checkOnStartup, &QCheckBox::clicked, this, &CheckForUpdateDialog::onCheckOnStartupClicked);
  connect(ui->qpb_check, &QPushButton::clicked, this, &CheckForUpdateDialog::onCheckNowClicked);
  connect(ui->qpb_close, &QPushButton::clicked, this, &CheckForUpdateDialog::onCloseClicked);

  const auto autoUpdateEnabled = persistence::SWSettings::get<int>(persistence::SWSettings::KEY_AUTOUPDATE_ENABLED);
  ui->qcb_checkOnStartup->setChecked(autoUpdateEnabled > 0);
}

CheckForUpdateDialog::~CheckForUpdateDialog()
{
  delete ui;
}

void CheckForUpdateDialog::closeEvent(QCloseEvent *ev)
{
  Q_UNUSED(ev);

  emit closed();
}

void CheckForUpdateDialog::onCheckComplete(const SoftwareUpdateResult &result)
{
  ui->ql_inProgress->setVisible(false);
  ui->qw_result->setDisplay(result);
}

void CheckForUpdateDialog::onCheckNowClicked()
{
  ui->ql_inProgress->setVisible(true);
  emit checkForUpdate();
}

void CheckForUpdateDialog::onCheckOnStartupClicked()
{
  const bool enabled = ui->qcb_checkOnStartup->checkState() == Qt::Checked;

  persistence::SWSettings::set(persistence::SWSettings::KEY_AUTOUPDATE_ENABLED, int(enabled));
}

void CheckForUpdateDialog::onCloseClicked()
{
  close();
}

void CheckForUpdateDialog::setCheckOnStartup(const bool checked)
{
  ui->qcb_checkOnStartup->setChecked(checked);
}
