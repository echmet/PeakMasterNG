#ifndef DATABASECONSTITUENTSPHYSPROPSTABLEMODEL_H
#define DATABASECONSTITUENTSPHYSPROPSTABLEMODEL_H

#include "../../gearbox/databaseproxy.h"

#include <QAbstractTableModel>

class DatabaseConstituentsPhysPropsTableModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit DatabaseConstituentsPhysPropsTableModel(QObject *parent = nullptr);

  // Header:
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  // Basic functionality:
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  const DatabaseConstituent & constituentAt(const int idx) const;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  void refreshData(std::vector<DatabaseConstituent> &&constituents) noexcept;

private:
  std::vector<DatabaseConstituent> m_constituents;
  int m_maximumCharge;
  int m_minimumCharge;
  int m_span;

};

#endif // DATABASECONSTITUENTSPHYSPROPSTABLEMODEL_H
