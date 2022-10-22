#ifndef ADJUSTUEFFOVERKBGETABLEMODEL_H
#define ADJUSTUEFFOVERKBGETABLEMODEL_H

#include <QAbstractTableModel>

#include <vector>

class AdjustuEffOverkBGETableModel : public QAbstractTableModel {
  Q_OBJECT
public:

  class Item {
  public:
    Item() = default;
    Item(QString constituentName, bool isAnalyte, double concentration, double uEffOverkBGE) :
      constituentName{std::move(constituentName)},
      isAnalyte{isAnalyte},
      concentration{concentration},
      uEffOverkBGE{uEffOverkBGE}
    {}

    QString constituentName;
    bool isAnalyte;
    double concentration;
    double uEffOverkBGE;
  };

  explicit AdjustuEffOverkBGETableModel(QObject *parent = nullptr);

  // Header:
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  // Basic functionality:
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

  void setUnderlyingData(std::vector<Item> data) noexcept;

private:
  std::vector<Item> m_data;
  int m_floatingPrecision;

signals:
  void concentrationChanged(const QString &constituentName, const double conc, const double oldConc);
  void uEffOverkBGEChanged(const QString &constituentName, const double uEffOverkBGE, const double olduEffOverkBGE);
};

QString uEffOverkBGEText();

#endif // ADJUSTUEFFOVERKBGETABLEMODEL_H
