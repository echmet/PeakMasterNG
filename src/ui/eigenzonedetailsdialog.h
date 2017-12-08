#ifndef EIGENZONEDETAILSDIALOG_H
#define EIGENZONEDETAILSDIALOG_H

#include <QDialog>

namespace Ui {
  class EigenzoneDetailsDialog;
}

class EigenzoneDetailsModel;

class EigenzoneDetailsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit EigenzoneDetailsDialog(EigenzoneDetailsModel *model, QWidget *parent = nullptr);
  ~EigenzoneDetailsDialog();

private:
  Ui::EigenzoneDetailsDialog *ui;
};

#endif // EIGENZONEDETAILSDIALOG_H
