#ifndef PICKCONSTITUENTFROMDBDIALOG_H
#define PICKCONSTITUENTFROMDBDIALOG_H

#include <QDialog>

namespace Ui {
  class PickConstituentFromDBDialog;
}

class DatabaseProxy;

class PickConstituentFromDBDialog : public QDialog
{
  Q_OBJECT

public:
  explicit PickConstituentFromDBDialog(DatabaseProxy &dbProxy, QWidget *parent = nullptr);
  ~PickConstituentFromDBDialog();

private:
  Ui::PickConstituentFromDBDialog *ui;

  DatabaseProxy &h_dbProxy;

private slots:
  void onConstituentNameChanged(const QString &name);
};

#endif // PICKCONSTITUENTFROMDBDIALOG_H
