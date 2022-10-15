#include "adjustphtablemodel.h"

#include "../../gearbox/gdmproxy.h"

AdjustpHTableModel::AdjustpHTableModel(std::vector<std::string> names, GDMProxy &GDMProxy, QObject *parent)
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

Qt::ItemFlags AdjustpHTableModel::flags(const QModelIndex &index) const
{
  static const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

  if (!index.isValid())
    return Qt::NoItemFlags;

  const int col = index.column();
  const int row = index.row();

  if (row < 0 || row >= rowCount())
    return Qt::NoItemFlags;

  return col == 1 ? defaultFlags | Qt::ItemIsEditable : defaultFlags;
}

bool AdjustpHTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid() || !value.isValid())
    return false;

  if (role != Qt::EditRole)
    return false;

  const int row = index.row();
  const int col = index.column();

  if (row < 0 || col >= rowCount())
    return false;
  if (col != 1)
    return false;

  bool ok;
  double v = value.toDouble(&ok);
  if (!ok)
    return false;

  const auto &name = m_names.at(row);
  auto concs = h_GDMProxy.concentrations(name);
  concs[0] = v;
  h_GDMProxy.setConcentrations(name, concs);

  emit dataChanged(index, index, { role });
  return true;
}

void AdjustpHTableModel::updateConcentration(const QString &name)
{
  for (int row = 0; row < rowCount(); row++) {
    if (data(index(row, 0)) == name)
      emit dataChanged(index(row, 1), index(row, 1));
  }
}
