#ifndef ANALYTESDISSOCIATIONMODEL_H
#define ANALYTESDISSOCIATIONMODEL_H

#include <QAbstractTableModel>
#include <map>
#include <string>
#include <vector>

class AnalytesDissociationModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  class DissociationRatio {
  public:
    DissociationRatio(std::string &&name, const double fraction) :
      name(std::move(name)),
      fraction{fraction}
    {}

    const std::string name;
    const double fraction;
  };

  class DissociatedAnalyte {
  public:
    DissociatedAnalyte(const double effectiveMobility, std::vector<DissociationRatio> &&ratios) :
      effectiveMobility{effectiveMobility},
      ratios(std::move(ratios))
    {}

    const double effectiveMobility;
    const std::vector<DissociationRatio> ratios;
  };

  explicit AnalytesDissociationModel(QObject *parent = nullptr);

  const std::vector<std::string> & analytes() const;
  void clear();
  int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  double effectiveMobility() const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

  void refreshData(std::map<std::string, DissociatedAnalyte> &&analytes);
  void selectAnalyte(const std::string &analyte);

private:
  std::map<std::string, DissociatedAnalyte> m_analytes;
  std::vector<std::string> m_analyteNames;
  std::string m_selectedAnalyte;

signals:
  void analytesChanged();
};

#endif // ANALYTESDISSOCIATIONMODEL_H
