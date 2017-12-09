#ifndef EIGENZONEDETAILSDIALOG_H
#define EIGENZONEDETAILSDIALOG_H

#include <QDialog>

namespace Ui {
  class EigenzoneDetailsDialog;
}

class QAbstractTableModel;

class EigenzoneDetailsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit EigenzoneDetailsDialog(QAbstractTableModel *model, QWidget *parent = nullptr);
  ~EigenzoneDetailsDialog();

private:
  Ui::EigenzoneDetailsDialog *ui;
};

#endif // EIGENZONEDETAILSDIALOG_H
