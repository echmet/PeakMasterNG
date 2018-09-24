#ifndef DATABASETABLEVIEW_H
#define DATABASETABLEVIEW_H

#include <QTableView>

class DatabaseTableView : public QTableView
{
  Q_OBJECT
public:
  explicit DatabaseTableView(QWidget *parent = nullptr);

protected:
  virtual void mouseDoubleClickEvent(QMouseEvent *evt) override;

private slots:
  void onSectionSelected(const int idx);

signals:
  void itemSelected(const QModelIndex &index);
};

#endif // DATABASETABLEVIEW_H
