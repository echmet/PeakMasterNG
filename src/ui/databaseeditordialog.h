#ifndef DATABASEEDITORDIALOG_H
#define DATABASEEDITORDIALOG_H

#include <QDialog>
#include <QSortFilterProxyModel>

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
  QModelIndex getIndex() const;
  int getConstituentRow(const QModelIndex &index) const;
  void executeSearch(const QString &name, const QVariant &matchVar);
  void editConstituent(const QModelIndex &index);

  Ui::DatabaseEditorDialog *ui;

  DatabaseConstituentsPhysPropsTableModel *m_model;
  DatabaseProxy &h_dbProxy;
  QSortFilterProxyModel m_proxyModel;

private slots:
  void onAddConstituent();
  void onAllCompounds();
  void onConstituentNameChanged(const QString &name);
  void onDeleteConstituent();
  void onEditConstituent();
  void onMatchTypeActivated(const int);
};

#endif // DATABASEEDITORDIALOG_H
