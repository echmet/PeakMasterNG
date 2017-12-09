#ifndef RESULTSDATA_H
#define RESULTSDATA_H

#include "backgroundpropertiesmodel.h"
#include "eigenzonedetailsmodel.h"
#include "ioniccompositionmodel.h"
#include "systemeigenzonestablemodel.h"

#include <QVector>

class ResultsContainer;

class ResultsData
{
public:
  ResultsData() = delete;
  ResultsData(ResultsContainer &container);

  void backgroundCompositionRefresh(const int lowestCharge, const int highestCharge,
                                    QVector<QString> &&constituents,
                                    QVector<QString> &&complex,
                                    QVector<IonicCompositionModel::ConstituentConcentrations> &&concentrations) noexcept;

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
