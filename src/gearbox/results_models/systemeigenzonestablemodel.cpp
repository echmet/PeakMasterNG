#include "systemeigenzonestablemodel.h"

SystemEigenzonesTableModel::SystemEigenzonesTableModel(QObject *parent)
  : QAbstractTableModel(parent)
{
}

QVariant SystemEigenzonesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  const size_t usection = static_cast<size_t>(section);

  if (role != Qt::DisplayRole)
    return QVariant{};

  if (orientation == Qt::Vertical) {
    if (usection >= m_dataVec.size())
      return QVariant{};
    return section + 1;
  }

  switch (section) {
  case 0:
    return tr("Mobility (1e-9)");
  case 1:
    return tr("Time (min)");
  }

  return QVariant{};
}

int SystemEigenzonesTableModel::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);

  return static_cast<int>(m_dataVec.size());
}

int SystemEigenzonesTableModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);

  return 2;
}

QVariant SystemEigenzonesTableModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (role != Qt::DisplayRole)
    return QVariant{};

  const size_t urow = static_cast<size_t>(index.row());
  if (urow >= m_dataVec.size())
    return QVariant{};

  switch (index.column()) {
  case 0:
    return std::get<0>(m_dataVec.at(urow));
  case 1:
  {
    const double t = std::get<1>(m_dataVec.at(urow));
    if (t <= 0)
      return "-";
    return t;
  }
  }

  return QVariant{};
}

void SystemEigenzonesTableModel::refreshData(const DataVector &dataVec)
{
  beginResetModel();
  m_dataVec = dataVec;
  endResetModel();
}

void SystemEigenzonesTableModel::refreshData(DataVector &&dataVec) noexcept
{
  beginResetModel();
  m_dataVec = dataVec;
  endResetModel();
}
