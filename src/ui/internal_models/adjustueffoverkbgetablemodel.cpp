// vim: sw=2 ts=2 sts=2 expandtab

#include "adjustueffoverkbgetablemodel.h"
#include "../../globals.h"

QString uEffOverkBGEText()
{
  return Globals::isZombieOS() ? QString::fromUtf8("uEff/kBGE") : QString::fromUtf8("\xCE\xBC""Eff/\xCE\xBA""BGE");
}

AdjustuEffOverkBGETableModel::AdjustuEffOverkBGETableModel(QObject *parent) :
  QAbstractTableModel{parent}
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
  } else if (role == Qt::UserRole + 1) {
    switch (col) {
      case 0:
	return item.isAnalyte;
     }
  }

  return {};
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

void AdjustuEffOverkBGETableModel::setUnderlyingData(std::vector<Item> data) noexcept
{
  beginResetModel();

  m_data = std::move(data);

  endResetModel();
}
