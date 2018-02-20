#include "constituentchargesmodel.h"
#include <cassert>

ConstituentChargesModel::ConstituentChargesModel(QObject *parent)
  : QAbstractTableModel{parent}
{
  m_charges.emplace_back(0, 0.0, 0.0);
}

QVariant ConstituentChargesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant{};

  const size_t usection = static_cast<size_t>(section);

  switch (orientation) {
  case Qt::Horizontal:
    switch (usection) {
    case MOBILITY:
      return QVariant{tr("Mobility")};
    case PKA:
      return QVariant{"pKa"};
    }
    return QVariant{};
  case Qt::Vertical:
  {
    if (m_charges.size() <= usection)
      return QVariant{};

    return std::get<0>(m_charges.at(usection));
  }
  }

  return QVariant{};
}

bool ConstituentChargesModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
  Q_UNUSED(section); Q_UNUSED(orientation); Q_UNUSED(value); Q_UNUSED(role);

  return false;
}


int ConstituentChargesModel::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);

  assert(m_charges.size() != 0);
  return static_cast<int>(m_charges.size());
}

int ConstituentChargesModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);

  return 2;
}

QVariant ConstituentChargesModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant{};

  if (role != Qt::DisplayRole)
    return QVariant{};

  const size_t urow = static_cast<size_t>(index.row());
  if (m_charges.size() <= urow)
    return QVariant{};

  const int col = index.column();
  if (col > 2)
    return QVariant{};

  switch (col) {
  case 0:
    return std::get<1>(m_charges.at(urow));
  case 1:
  {
    if (std::get<0>(m_charges.at(urow)) == 0 || m_charges.size() == 1)
      return "-";

    return std::get<2>(m_charges.at(urow));
  }
  }

  return QVariant{};
}

bool ConstituentChargesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  const size_t urow = static_cast<size_t>(index.row());

  if (data(index, role) != value) {
    if (m_charges.size() <= urow || index.column() > 2)
      return false;

    /* Properties pf zero charge are immutable */
    if (std::get<0>(m_charges.at(urow)) == 0)
      return false;

    bool isReal;
    const qreal realVal = value.toReal(&isReal);
    if (!isReal)
      return false;

    switch (index.column()) {
    case 0:
      std::get<1>(m_charges[urow]) = realVal;
      break;
    case 1:
      std::get<2>(m_charges[urow]) = realVal;
      break;
    default:
      return false;
    }

    emit dataChanged(index, index, { role });
    return true;
  }
  return false;
}

Qt::ItemFlags ConstituentChargesModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags defaultFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  if (!index.isValid())
    return Qt::NoItemFlags;

  const size_t urow = static_cast<size_t>(index.row());

  if (m_charges.size() <= urow)
    return Qt::NoItemFlags;

  /* Properties of zero charge are immutable */
  if (std::get<0>(m_charges.at(urow)) == 0)
    return defaultFlags;
  /* Changing pKa value if these is only one charge makes no sense */
  if (index.column() == 1 && m_charges.size() == 1)
    return defaultFlags;

  return defaultFlags | Qt::ItemIsEditable;
}

bool ConstituentChargesModel::insertRows(int row, int count, const QModelIndex &parent)
{
  const size_t urow = static_cast<size_t>(row);
  if (m_charges.size() < urow)
    return false;

  beginInsertRows(parent, row, row + count - 1);
  const int fromCharge = [urow, this]() {
    if (m_charges.size() == urow)
      return std::get<0>(m_charges.at(urow - 1));
    return std::get<0>(m_charges.at(urow));
  }();
  const int toCharge = [row, count, fromCharge]() {
    if (row == 0)
      return fromCharge - count;
    return fromCharge + count;
  }();

  auto backup = m_charges; // TODO: Figure out something safer!
  try {
    if (fromCharge < toCharge) {
      for (int charge = fromCharge + 1; charge <= toCharge; charge++)
        m_charges.emplace_back(charge, 0.0, 0.0);
    } else {
      for (int charge = fromCharge - 1; charge >= toCharge; charge--)
        m_charges.emplace(m_charges.begin(), charge, 0.0, 0.0);
    }
  } catch (std::bad_alloc &) {
    m_charges = backup;
    endInsertRows();
    return false;
  }

  endInsertRows();
  return true;
}

bool ConstituentChargesModel::insertColumns(int column, int count, const QModelIndex &parent)
{
  Q_UNUSED(column); Q_UNUSED(count); Q_UNUSED(parent);

  return false;
}

bool ConstituentChargesModel::removeRows(int row, int count, const QModelIndex &parent)
{
  assert(row >= 0);
  assert(count > 0);

  const size_t urow = static_cast<size_t>(row);

  if (m_charges.size() <= urow)
    return false;
  if (m_charges.size() == 1)
    return false;

  beginRemoveRows(parent, row, row + count - 1);
  std::vector<ChargeBlock>::const_iterator from = m_charges.cbegin() + row;
  std::vector<ChargeBlock>::const_iterator to = from + count;

  m_charges.erase(from, to);
  endRemoveRows();

  return true;
}

void ConstituentChargesModel::refreshData(const std::map<int, double> &pKas, const std::map<int, double> &mobilities,
                                          const int chargeLow, const int chargeHigh)
{
  assert(pKas.size() == mobilities.size());
  assert(pKas.size() == static_cast<size_t>(chargeHigh - chargeLow + 1));

  beginResetModel();

  m_charges.clear();

  for (int charge = chargeLow; charge <= chargeHigh; charge++) {
    const auto &pKa = pKas.at(charge);
    const auto &mobility = mobilities.at(charge);

    m_charges.emplace_back(charge, mobility, pKa);
  }

  endResetModel();
}

bool ConstituentChargesModel::removeColumns(int column, int count, const QModelIndex &parent)
{
  Q_UNUSED(column); Q_UNUSED(count); Q_UNUSED(parent);

  return false;
}
