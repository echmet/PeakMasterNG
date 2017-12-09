#include "ioniccompositiondialog.h"
#include "ui_ioniccompositiondialog.h"

#include <QAbstractTableModel>

IonicCompositionDialog::IonicCompositionDialog(QAbstractTableModel *model, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::IonicCompositionDialog)
{
  ui->setupUi(this);

  ui->qtvb_ionicComposition->setModel(model);
}

IonicCompositionDialog::~IonicCompositionDialog()
{
  delete ui;
}
