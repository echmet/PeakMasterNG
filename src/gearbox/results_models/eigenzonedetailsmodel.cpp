#include "eigenzonedetailsmodel.h"

#include <cassert>
#include <QPalette>

const QString EigenzoneDetailsModel::s_typeStr{tr("Type")};
const QString EigenzoneDetailsModel::s_mobilityStr{tr("Mobility (\xe2\x8b\x85 1e-9)")};
const QString EigenzoneDetailsModel::s_timeStr{tr("Time (min)")};
const QString EigenzoneDetailsModel::s_uEMDStr{tr("\xce\xbc EMD (\xe2\x8b\x85 1e-9)")};
const QString EigenzoneDetailsModel::s_conductivityStr{tr("Conductivity (S/m)")};
const QString EigenzoneDetailsModel::s_pHStr{"pH"};

EigenzoneDetailsModel::EigenzoneDetailsModel(QObject *parent)
  : QAbstractTableModel(parent)
{
}

double EigenzoneDetailsModel::constituentsConcentrations(const EigenzoneProps &zone, const int idx) const
{
  if (idx >= zone.concentrations.size())
    return {};
  if (m_displayDeltas)
    return zone.cDeltas.at(idx);
  return zone.concentrations.at(idx);
}

QString EigenzoneDetailsModel::constituentsHeader(const int idx) const
{
  if (idx >= m_constituents.size())
    return {};
  return m_constituents.at(idx);
}

void EigenzoneDetailsModel::displayDeltas(const bool status)
{
  m_displayDeltas = status;

  if (m_eigenzones.size() > 0)
    emit dataChanged(index(6, 0), index(rowCount(), columnCount()));
}

bool EigenzoneDetailsModel::displayDeltasState() const
{
  return m_displayDeltas;
}

const EigenzoneDetailsModel::EigenzoneProps & EigenzoneDetailsModel::eigenzonePropsAt(const int idx) const noexcept
{
  assert(idx >= 0 && idx < rowCount());

  return m_eigenzones.at(idx);
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
      return s_typeStr;
    case 1:
      return s_mobilityStr;
    case 2:
      return s_timeStr;
    case 3:
      return s_uEMDStr;
    case 4:
      return s_conductivityStr;
    case 5:
      return s_pHStr;
    default:
      return constituentsHeader(section - 6);
    }
  }

  return {};
}

int EigenzoneDetailsModel::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);

  return m_constituents.size() + 6;
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
    return zone.mobility;
  case 2:
    return (zone.time > 0.0) ? QVariant{zone.time} : QVariant{"-"};
  case 3:
    return zone.uEMD;
  case 4:
    return m_displayDeltas ? zone.conductivityDelta : zone.conductivity;
  case 5:
    return m_displayDeltas ? zone.pHDelta : zone.pH;
  default:
    return constituentsConcentrations(zone, row - 6);
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
