#ifndef IONICCOMPOSITIONDIALOG_H
#define IONICCOMPOSITIONDIALOG_H

#include <QDialog>

namespace Ui {
  class IonicCompositionDialog;
}

class QAbstractTableModel;

class IonicCompositionDialog : public QDialog
{
  Q_OBJECT

public:
  explicit IonicCompositionDialog(QAbstractTableModel *model, QWidget *parent = nullptr);
  ~IonicCompositionDialog();

private:
  Ui::IonicCompositionDialog *ui;
};

#endif // IONICCOMPOSITIONDIALOG_H
