#include "eigenzonedetailsdialog.h"
#include "ui_eigenzonedetailsdialog.h"

#include <QAbstractTableModel>

EigenzoneDetailsDialog::EigenzoneDetailsDialog(QAbstractTableModel *model, QWidget *parent) :
  QDialog{parent},
  ui{new Ui::EigenzoneDetailsDialog}
{
  ui->setupUi(this);

  ui->qtbv_ezProps->setModel(model);
  ui->qtbv_ezProps->resizeColumnsToContents();
}

EigenzoneDetailsDialog::~EigenzoneDetailsDialog()
{
  delete ui;
}
