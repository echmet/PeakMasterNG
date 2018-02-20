#ifndef PICKCONSTITUENTFROMDBDIALOG_H
#define PICKCONSTITUENTFROMDBDIALOG_H

#include <QDialog>

namespace Ui {
  class PickConstituentFromDBDialog;
}

class DatabaseProxy;
class DatabaseConstituentsPhysPropsTableModel;

class PickConstituentFromDBDialog : public QDialog
{
  Q_OBJECT

public:
  explicit PickConstituentFromDBDialog(DatabaseConstituentsPhysPropsTableModel &model, DatabaseProxy &dbProxy, QWidget *parent = nullptr);
  ~PickConstituentFromDBDialog();
  int selectedIndex() const;

private:
  Ui::PickConstituentFromDBDialog *ui;

  DatabaseProxy &h_dbProxy;
  DatabaseConstituentsPhysPropsTableModel &m_model;

  int m_selectedIndex;

private slots:
  void onAccepted();
  void onAllCompounds();
  void onConstituentNameChanged(const QString &name);
  void onItemSelected(const int row);
  void onRejected();
};

#endif // PICKCONSTITUENTFROMDBDIALOG_H
