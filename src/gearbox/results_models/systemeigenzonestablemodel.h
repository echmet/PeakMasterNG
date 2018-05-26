#ifndef SYSTEMEIGENZONESTABLEMODEL_H
#define SYSTEMEIGENZONESTABLEMODEL_H

#include <QAbstractTableModel>
#include <vector>

class SystemEigenzonesTableModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  typedef std::vector<std::tuple<double, double, bool>> DataVector;

  explicit SystemEigenzonesTableModel(QObject *parent = nullptr);

  // Header:
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  // Basic functionality:
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  void refreshData(const DataVector &dataVec);
  void refreshData(DataVector &&dataVec) noexcept;

private:
  DataVector m_dataVec;

};

#endif // SYSTEMEIGENZONESTABLEMODEL_H
