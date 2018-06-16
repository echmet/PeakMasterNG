#ifndef OPERATIONINPROGRESSDIALOG_H
#define OPERATIONINPROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
class OperationInProgressDialog;
}

class OperationInProgressDialog : public QDialog
{
  Q_OBJECT

public:
  explicit OperationInProgressDialog(const QString &title, QWidget *parent = nullptr);
  ~OperationInProgressDialog() override;
  int exec() override;
  void reject() override;

public slots:
  void onOperationCompleted();

private:
  Ui::OperationInProgressDialog *ui;

  bool m_isExecable;
};

#endif // OPERATIONINPROGRESSDIALOG_H
