#include "adjustphdialog.h"
#include "ui_adjustphdialog.h"

AdjustpHDialog::AdjustpHDialog(QWidget *parent) :
  QDialog{parent},
  ui{new Ui::AdjustpHDialog}
{
  ui->setupUi(this);
}

AdjustpHDialog::~AdjustpHDialog()
{
  delete ui;
}
