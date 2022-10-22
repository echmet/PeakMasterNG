// vim: sw=2 ts=2 sts=2 expandtab

#include "adjustueffoverkbgetablemodel.h"
#include "../../gearbox/doubletostringconvertor.h"
#include "../../globals.h"

QString uEffOverkBGEText()
{
  return Globals::isZombieOS() ? QString::fromUtf8("uEff/kBGE") : QString::fromUtf8("\xCE\xBC""Eff/\xCE\xBA""BGE");
}

AdjustuEffOverkBGETableModel::AdjustuEffOverkBGETableModel(QObject *parent) :
  QAbstractTableModel{parent},
  m_floatingPrecision{5}
{
}

int AdjustuEffOverkBGETableModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return 0;

  return 4;
}

QVariant AdjustuEffOverkBGETableModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return {};

  const int col = index.column();
  const int row = index.row();

  if (col < 0 || col >= columnCount() || row < 0 || row >= rowCount())
    return {};

  const auto &item = m_data[row];

  if (role == Qt::DisplayRole) {
    switch (col) {
      case 0:
        return item.isAnalyte ? "A" : "B";
      case 1:
        return item.constituentName;
      case 2:
        return item.concentration;
      case 3:
        return item.uEffOverkBGE;
    }
  } else if (role == Qt::EditRole) {
    switch (col) {
      case 3:
        return item.uEffOverkBGE;
    }
  } else if (role == Qt::UserRole + 1) {
    // Precision value for the FloatingValueDelegate
    return m_floatingPrecision;
  }

  return {};
}

Qt::ItemFlags AdjustuEffOverkBGETableModel::flags(const QModelIndex &index) const
{
  static const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

  if (!index.isValid())
    return Qt::NoItemFlags;

  const int col = index.column();
  const int row = index.row();

  if (col < 0 || col >= columnCount() || row < 0 || row >= rowCount())
    return Qt::NoItemFlags;

  // Only concentration and uEkB columns are editable
  if (!(col == 2 || col == 3))
    return defaultFlags;

  return m_data[row].isAnalyte ? defaultFlags : defaultFlags | Qt::ItemIsEditable;
}

QVariant AdjustuEffOverkBGETableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  static const QString uekbCaption = uEffOverkBGEText();

  if (role != Qt::DisplayRole)
    return {};

  if (orientation == Qt::Horizontal) {
    switch (section) {
      case 0:
        return tr("Type");
      case 1:
        return tr("Name");
      case 2:
        return tr("Concentration (mM)");
      case 3:
        return uekbCaption;
    }
  }

  return {};
}

int AdjustuEffOverkBGETableModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return 0;

  return m_data.size();
}

bool AdjustuEffOverkBGETableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid())
    return false;

  const int col = index.column();
  const int row = index.row();

  if (col < 0 || col >= columnCount() || row < 0 || row >= rowCount())
    return false;

  if (role == Qt::EditRole) {
    const auto &item = m_data[row];
    if (item.isAnalyte)
      return false;

    if (col == 2) {
      bool ok;
      double conc = value.toDouble(&ok);
      if (!ok)
        return false;

      const double oldConc = item.concentration;

      emit concentrationChanged(item.constituentName, conc, oldConc);
      return false; // We did not actually change anything, concentrationChanged() signal must be handled
    } else if (col == 3) {
      bool ok;
      double uEkB = value.toDouble(&ok);
      if (!ok)
        return false;

      const double olduKeB = item.uEffOverkBGE;

      emit uEffOverkBGEChanged(item.constituentName, uEkB, olduKeB);
      return false; // We did not actually change anything, uEffOverkBGEChanged() signal must be handled
    }
  } else if (role == Qt::UserRole + 1) {
    bool ok;
    int prec = value.toInt(&ok);
    if (!ok || prec < 0)
      return false;

    m_floatingPrecision = prec;

    emit dataChanged(index, index, { role });
    return true;
  }

  return false;
}

void AdjustuEffOverkBGETableModel::setUnderlyingData(std::vector<Item> data) noexcept
{
  beginResetModel();

  m_data = std::move(data);

  endResetModel();
}
