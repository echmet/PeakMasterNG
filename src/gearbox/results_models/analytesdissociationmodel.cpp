#include "analytesdissociationmodel.h"

AnalytesDissociationModel::AnalytesDissociationModel(QObject *parent)
  : QAbstractTableModel(parent)
{
}

const std::vector<std::string> & AnalytesDissociationModel::analytes() const
{
  return m_analyteNames;
}

void AnalytesDissociationModel::clear()
{
  refreshData({});
}

int AnalytesDissociationModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent)

  const auto it = m_analytes.find(m_selectedAnalyte);
  if (it == m_analytes.cend())
    return 0;

  return it->second.ratios.size();
}

QVariant AnalytesDissociationModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant{};

  if (role != Qt::DisplayRole)
    return QVariant{};

  const auto it = m_analytes.find(m_selectedAnalyte);
  if (it == m_analytes.cend())
    return QVariant{};

  const int col = index.column();

  if (col < 0 || col >= it->second.ratios.size())
    return QVariant{};

  return it->second.ratios.at(col).fraction;
}

double AnalytesDissociationModel::effectiveMobility() const
{
  const auto it = m_analytes.find(m_selectedAnalyte);
  if (it == m_analytes.cend())
    return 0;

  return it->second.effectiveMobility;
}

QVariant AnalytesDissociationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation != Qt::Horizontal)
    return QVariant{};

  if (role != Qt::DisplayRole)
    return QVariant{};

  const auto it = m_analytes.find(m_selectedAnalyte);
  if (it == m_analytes.cend())
    return QVariant{};

  if (section < 0 || section >= it->second.ratios.size())
    return QVariant{};

  return QString::fromStdString(it->second.ratios.at(section).name);
}

void AnalytesDissociationModel::refreshData(std::map<std::string, DissociatedAnalyte> &&analytes)
{
  beginResetModel();
  m_analytes = std::move(analytes);
  m_analyteNames = [&]() {
    std::vector<std::string> keys{};
    for (const auto &item : m_analytes)
      keys.emplace_back(item.first);

    return keys;
  }();
  m_selectedAnalyte = "";
  endResetModel();

  emit analytesChanged();
}

int AnalytesDissociationModel::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent)
  const auto it = m_analytes.find(m_selectedAnalyte);
  if (it == m_analytes.cend())
    return 0;

  return 1;
}

void AnalytesDissociationModel::selectAnalyte(const std::string &name)
{
  beginResetModel();
  m_selectedAnalyte = name;
  endResetModel();
}
