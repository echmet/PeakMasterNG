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
  explicit AdjustpHTableModel(std::vector<std::string> names, const GDMProxy &GDMProxy, QObject *parent = nullptr);

  // Header:
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  // Basic functionality:
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  void updateConcentration(const QString &name);

private:
  std::vector<std::string> m_names;

  const GDMProxy &h_GDMProxy;
};

#endif // ADJUSTPHTABLEMODEL_H
