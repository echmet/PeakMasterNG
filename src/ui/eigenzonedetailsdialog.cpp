#include "eigenzonedetailsdialog.h"
#include "ui_eigenzonedetailsdialog.h"

#include "../gearbox/results_models/eigenzonedetailsmodel.h"

EigenzoneDetailsDialog::EigenzoneDetailsDialog(EigenzoneDetailsModel *model, QWidget *parent) :
  QDialog{parent},
  ui{new Ui::EigenzoneDetailsDialog}
{
  ui->setupUi(this);

  ui->qtbv_ezProps->setModel(model);
}

EigenzoneDetailsDialog::~EigenzoneDetailsDialog()
{
  delete ui;
}
