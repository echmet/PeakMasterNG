#ifndef DATABASEEDITORDIALOG_H
#define DATABASEEDITORDIALOG_H

#include <QDialog>

namespace Ui {
  class DatabaseEditorDialog;
}

class DatabaseConstituentsPhysPropsTableModel;
class DatabaseProxy;

class DatabaseEditorDialog : public QDialog
{
  Q_OBJECT

public:
  explicit DatabaseEditorDialog(DatabaseProxy &dbProxy, QWidget *parent = nullptr);
  ~DatabaseEditorDialog();

private:
  int getIndex() const;

  Ui::DatabaseEditorDialog *ui;

  DatabaseConstituentsPhysPropsTableModel *m_model;
  DatabaseProxy &h_dbProxy;

private slots:
  void onAddConstituent();
  void onAllCompounds();
  void onConstituentNameChanged(const QString &name);
  void onDeleteConstituent();
  void onEditConstituent();
};

#endif // DATABASEEDITORDIALOG_H
