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
  explicit EigenzoneDetailsDialog(QAbstractTableModel *model, const bool displayDeltas, QWidget *parent = nullptr);
  ~EigenzoneDetailsDialog();

private:
  Ui::EigenzoneDetailsDialog *ui;

private slots:
  void onDisplayDeltasChanged(const int state);

signals:
  void displayDeltasChanged(const int state);
};

#endif // EIGENZONEDETAILSDIALOG_H
