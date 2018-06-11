#include "databaseconstituentsphyspropstablemodel.h"

#include <limits>

DatabaseConstituentsPhysPropsTableModel::DatabaseConstituentsPhysPropsTableModel(QObject *parent)
  : QAbstractTableModel(parent)
{
}

static
QString chargeToString(const int charge)
{
  if (charge <= 0)
    return QString::number(charge);
  return
      QString{"+%1"}.arg(charge);
}

QVariant DatabaseConstituentsPhysPropsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return {};

  const size_t usection = static_cast<size_t>(section);

  if (orientation == Qt::Vertical) {
    if (usection >= m_constituents.size())
      return {};

    return m_constituents.at(usection).name;
  }
  else if (orientation == Qt::Horizontal) {
    if (section >= m_span - 1) /* Mobility section */
      return QString{"\xce\xbc (%1)"}.arg(chargeToString(section - m_span + m_minimumCharge + 1));
    else { /* pKa section */
      int charge = section + m_minimumCharge;
      if (charge >= 0)
        charge++;
      return QString{"pKa (%1)"}.arg(chargeToString(charge));
    }
  }

  return {};
}

int DatabaseConstituentsPhysPropsTableModel::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);

  return static_cast<int>(m_constituents.size());
}

int DatabaseConstituentsPhysPropsTableModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);

  if (m_constituents.size() < 1)
    return 0;

  return 2 * m_span - 1;
}

const DatabaseConstituent & DatabaseConstituentsPhysPropsTableModel::constituentAt(const int idx) const
{
  const size_t uidx = static_cast<size_t>(idx);
  if (uidx >= m_constituents.size())
    throw std::out_of_range{"Constituent index out of range"};

  return m_constituents.at(uidx);
}

QVariant DatabaseConstituentsPhysPropsTableModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return {};

  if (role != Qt::DisplayRole)
    return {};

  const int row = index.row();
  const int col = index.column();
  const size_t urow = static_cast<size_t>(row);

  if (urow >= m_constituents.size())
    return  {};

  if (col < 0 || col >= columnCount())
    return {};

  int charge;
  bool getMobility;
  if (col >= m_span - 1) {
    getMobility = true;
    charge = col - m_span + m_minimumCharge + 1;
  } else {
    getMobility = false;
    charge = col + m_minimumCharge;
    if (charge >= 0)
      charge++;
  }

  const auto &ctuent = m_constituents.at(urow);
  if (getMobility) {
    const auto &v = ctuent.mobilities.find(charge);
    if (v == ctuent.mobilities.cend())
      return {};
    return v->second;
  } else {
    const auto &v = ctuent.pKas.find(charge);
    if (v == ctuent.pKas.cend())
      return {};
    return v->second;
  }
}

void DatabaseConstituentsPhysPropsTableModel::refreshData(std::vector<DatabaseConstituent> &&constituents) noexcept
{
  beginResetModel();

  int min = std::numeric_limits<int>::max();
  int max = std::numeric_limits<int>::min();

  for (const auto &item : constituents) {
    if (item.chargeLow < min)
      min = item.chargeLow;
    if (item.chargeHigh > max)
      max = item.chargeHigh;
  }

  m_maximumCharge = max;
  m_minimumCharge = min;
  m_span = m_maximumCharge - m_minimumCharge + 1;

  m_constituents = std::move(constituents);

  endResetModel();
}
