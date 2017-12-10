#include "eigenzonedetailsmodel.h"

#include <cassert>
#include <QPalette>

EigenzoneDetailsModel::EigenzoneDetailsModel(QObject *parent)
  : QAbstractTableModel(parent)
{
}

double EigenzoneDetailsModel::constituentsConcentrations(const EigenzoneProps &zone, const int idx) const
{
  if (idx >= zone.concentrations.size())
    return {};
  return zone.concentrations.at(idx);
}

QString EigenzoneDetailsModel::constituentsHeader(const int idx) const
{
  if (idx >= m_constituents.size())
    return {};
  return m_constituents.at(idx);
}

QVariant EigenzoneDetailsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return {};

  if (orientation == Qt::Horizontal) {
    if (section >= m_constituents.size())
      return {};
    return QString{"Zone %1"}.arg(section + 1);
  }

  if (orientation == Qt::Vertical) {
    switch (section) {
    case 0:
      return "Type";
    case 1:
      return "Time (min)";
    case 2:
      return "\xce\xbc EMD (.1e-9)";
    case 3:
      return "pH";
    default:
      return constituentsHeader(section - 4);
    }
  }

  return {};
}

int EigenzoneDetailsModel::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);

  return m_constituents.size() + 4;
}

int EigenzoneDetailsModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);

  return m_eigenzones.size();
}

QVariant EigenzoneDetailsModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return {};

  const int col = index.column();
  if (col < 0 || col >= m_eigenzones.size())
    return {};

  const auto &zone = m_eigenzones.at(col);

  if (role == Qt::ForegroundRole) {
    const QPalette p{};

    if (zone.tainted)
      return QColor{Qt::red};
    return p.text().color();
  }

  if (role != Qt::DisplayRole)
    return {};

  const int row = index.row();
  if (row < 0 || row >= rowCount())
    return {};

  switch (row) {
  case 0:
    return (zone.isAnalyte) ? "Analyte" : "System";
  case 1:
    return (zone.time > 0.0) ? QVariant{zone.time} : QVariant{"-"};
  case 2:
    return zone.uEMD;
  case 3:
    return zone.pH;
  default:
    return constituentsConcentrations(zone, row - 4);
  }
}

void EigenzoneDetailsModel::refreshData(QVector<QString> &&constituents, QVector<EigenzoneProps> &&eigenzones) noexcept
{
  assert(constituents.size() == eigenzones.size());

  beginResetModel();

  m_constituents = constituents;
  m_eigenzones = eigenzones;

  endResetModel();
}
