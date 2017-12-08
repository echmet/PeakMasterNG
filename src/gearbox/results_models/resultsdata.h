#ifndef RESULTSDATA_H
#define RESULTSDATA_H

#include "backgroundpropertiesmodel.h"
#include "eigenzonedetailsmodel.h"
#include "systemeigenzonestablemodel.h"

#include <QVector>

class ResultsContainer;

class ResultsData
{
public:
  ResultsData() = delete;
  ResultsData(ResultsContainer &container);

  QVector<double> & backgroundPropsData();
  int backgroundPropsIndex(const BackgroundPropertiesMapping::Items item) const;
  void backgroundPropsRefresh();

  void eigenzoneDetailsRefresh(QVector<QString> &&constituents, QVector<EigenzoneDetailsModel::EigenzoneProps> &&eigenzones) noexcept;

  void systemEigenzonesRefresh(const SystemEigenzonesTableModel::DataVector &dataVec);
  void systemEigenzonesRefresh(SystemEigenzonesTableModel::DataVector &&dataVec) noexcept;

private:
  ResultsContainer &m_container;
};

#endif // RESULTSDATA_H
