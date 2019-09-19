#include "adjustphtablemodel.h"

AdjustpHTableModel::AdjustpHTableModel(QObject *parent)
  : QAbstractTableModel{parent}
{
}

QVariant AdjustpHTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation != Qt::Horizontal)
    return {};

  if (role != Qt::DisplayRole)
    return {};

  switch (section) {
  case 0:
    return "Constituent";
  case 1:
    return "Concentration (mM)";
  }

  return {};
}

int AdjustpHTableModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return 0;

  // FIXME: Implement me!
}

int AdjustpHTableModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return 0;

  return 2;
}

QVariant AdjustpHTableModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  // FIXME: Implement me!
  return QVariant();
}
