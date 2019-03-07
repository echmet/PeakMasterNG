#include "ioniccompositionmodel.h"

#include <cassert>

IonicCompositionModel::IonicCompositionModel(QObject *parent) : QAbstractTableModel{parent},
  m_lowestCharge{0},
  m_chargeSpan{0}
{
}

QVariant IonicCompositionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return {};

  if (section < 0)
    return {};

  if (orientation == Qt::Horizontal) {
    if (section < m_chargeSpan) {
      const int charge = section + m_lowestCharge;
      if (charge < 0)
        return charge;
      else if (charge == 0)
        return "0";
      else
        return QString{"+%1"}.arg(charge);
    }

    if (section < m_complexForms.size() + m_chargeSpan)
      return m_complexForms.at(section - m_chargeSpan);

    return {};
  } else if (orientation == Qt::Vertical) {
    if (section >= m_constituents.size())
      return {};

    return m_constituents.at(section);
  }

  return {};
}

int IonicCompositionModel::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);

  return m_constituents.size();
}

int IonicCompositionModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);

  if (m_constituents.empty())
    return 0;

  return m_chargeSpan + m_complexForms.size();
}

QVariant IonicCompositionModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return {};

  if (role != Qt::DisplayRole)
    return {};

  const int row = index.row();
  const int col = index.column();

  if (row < 0 || row > m_constituents.size())
    return {};

  const auto &ctuentC = m_concentrations.at(row);
  const int charge = m_lowestCharge + col;
  if (charge < ctuentC.lowestCharge)
    return {};
  if (charge <= ctuentC.highestCharge)
    return ctuentC.plain.find(charge).value();

  /* We are in the complex forms section */
  int cpxIdx = col - m_chargeSpan;
  if (cpxIdx >= m_complexForms.size() || cpxIdx < 0)
    return {};
  const auto &name = m_complexForms.at(cpxIdx);
  const auto it = ctuentC.complex.find(name);
  if (it != ctuentC.complex.cend())
    return it.value();

  return {};
}

void IonicCompositionModel::refreshData(const int lowestCharge, const int highestCharge,
                                        QVector<QString> &&constituents,
                                        QVector<QString> &&complexForms,
                                        QVector<ConstituentConcentrations> &&concentrations) noexcept
{
  assert(lowestCharge <= highestCharge);

  beginResetModel();

  m_constituents = std::move(constituents);
  m_complexForms = std::move(complexForms);
  m_concentrations = std::move(concentrations);

  m_lowestCharge = lowestCharge;
  m_chargeSpan = highestCharge - lowestCharge + 1;

  endResetModel();
}
