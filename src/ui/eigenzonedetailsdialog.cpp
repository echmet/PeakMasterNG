#include "eigenzonedetailsdialog.h"
#include "ui_eigenzonedetailsdialog.h"

#include <QAbstractTableModel>

EigenzoneDetailsDialog::EigenzoneDetailsDialog(QAbstractTableModel *model, const bool displayDeltas, QWidget *parent) :
  QDialog{parent},
  ui{new Ui::EigenzoneDetailsDialog}
{
  ui->setupUi(this);

  ui->qtbv_ezProps->setModel(model);
  ui->qtbv_ezProps->resizeColumnsToContents();
  ui->qcbox_dispalyCDeltas->setChecked(displayDeltas);

  connect(ui->qcbox_dispalyCDeltas, &QCheckBox::stateChanged, this, &EigenzoneDetailsDialog::onDisplayDeltasChanged);
}

EigenzoneDetailsDialog::~EigenzoneDetailsDialog()
{
  delete ui;
}

void EigenzoneDetailsDialog::onDisplayDeltasChanged(const int state)
{
  emit displayDeltasChanged(state == Qt::Checked);
}
