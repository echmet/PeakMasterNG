#include "systemeigenzonestablemodel.h"

#include "../../globals.h"

#include <QPalette>

SystemEigenzonesTableModel::SystemEigenzonesTableModel(QObject *parent)
  : QAbstractTableModel(parent)
{
  if (Globals::isZombieOS())
    m_mobilityStr = QString(tr("Mobility (. 1e-9)"));
  else
    m_mobilityStr = QString(tr("Mobility (\xE2\x8B\x85 1e-9)"));
}

QVariant SystemEigenzonesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  const auto usection = static_cast<size_t>(section);

  if (role != Qt::DisplayRole)
    return QVariant{};

  if (orientation == Qt::Vertical) {
    if (usection >= m_dataVec.size())
      return QVariant{};
    return section + 1;
  }

  switch (section) {
  case 0:
    return m_mobilityStr;
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

  const auto urow = static_cast<size_t>(index.row());

  if (role == Qt::ForegroundRole) {
    const QPalette p{};

    if (!std::get<2>(m_dataVec.at(urow)))
      return QColor{Qt::red};
    return p.text().color();
  }

  if (role != Qt::DisplayRole)
    return QVariant{};

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
