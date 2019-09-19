#include "adjustphtablemodel.h"

#include "../../gearbox/gdmproxy.h"

AdjustpHTableModel::AdjustpHTableModel(std::vector<std::string> names, const GDMProxy &GDMProxy, QObject *parent)
  : QAbstractTableModel{parent},
    m_names{std::move(names)},
    h_GDMProxy{GDMProxy}
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

  return m_names.size();
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
    return {};

  if (role != Qt::DisplayRole)
    return {};

  const int row = index.row();
  const int col = index.column();

  if (row < 0 || row >= rowCount())
    return {};
  if (col < 0 || col >= columnCount())
    return {};

  const auto &name = m_names.at(row);

  if (col == 0)
    return QString::fromStdString(name);
  else if (col == 1)
    return h_GDMProxy.concentrations(name).at(0);

  return {};
}
