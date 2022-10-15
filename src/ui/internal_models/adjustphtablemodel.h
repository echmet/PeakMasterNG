#ifndef ADJUSTPHTABLEMODEL_H
#define ADJUSTPHTABLEMODEL_H

#include <QAbstractTableModel>
#include <string>
#include <vector>

class GDMProxy;

class AdjustpHTableModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit AdjustpHTableModel(std::vector<std::string> names, GDMProxy &GDMProxy, QObject *parent = nullptr);

  // Header:
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  // Basic functionality:
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

  void updateConcentration(const QString &name);

private:
  std::vector<std::string> m_names;

  GDMProxy &h_GDMProxy;
};

#endif // ADJUSTPHTABLEMODEL_H
