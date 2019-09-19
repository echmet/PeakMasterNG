#ifndef ADJUSTPHTABLEMODEL_H
#define ADJUSTPHTABLEMODEL_H

#include <QAbstractTableModel>

class AdjustpHTableModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit AdjustpHTableModel(QObject *parent = nullptr);

  // Header:
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  // Basic functionality:
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
};

#endif // ADJUSTPHTABLEMODEL_H
