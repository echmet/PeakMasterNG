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

  if (col < 0) return {};
  if (size_t(col) >= it->second.ratios.size()) return {};

  return it->second.ratios.at(col).fraction;
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

  if (section < 0) return {};
  if (size_t(section) >= it->second.ratios.size()) return {};

  return QString::fromStdString(it->second.ratios.at(section).name);
}

bool AnalytesDissociationModel::isMiscalculated() const
{
  const auto it = m_miscalculated.find(m_selectedAnalyte);

  if (it == m_miscalculated.cend())
    return false;

  return it->second;
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

  m_miscalculated = [this]() {
    const double TOL{1.0e-9};
    const double LOW = 1.0 - TOL;
    const double HIGH = 1.0 + TOL;

    std::map<std::string, bool> miscalc;

    for (const auto &it : m_analytes) {
      const auto &drs = it.second;

      double sum = 0.0;
      for (const auto &dr : drs.ratios)
        sum += dr.fraction;

      miscalc.emplace(it.first, (sum < LOW) || (sum > HIGH));
    }

    return miscalc;
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
