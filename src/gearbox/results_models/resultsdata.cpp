#include "resultsdata.h"

#include "resultscontainer.h"

ResultsData::ResultsData(ResultsContainer &container) :
  m_container{container}
{
}

void ResultsData::analytesDissociationRefresh(std::map<std::string, AnalytesDissociationModel::DissociatedAnalyte> &&analytes)
{
  m_container.m_analytesDissociationModel->refreshData(std::move(analytes));
}

void ResultsData::analytesExtraInfoRefresh(const QVector<AnalytesExtraInfoModel::ExtraInfo> &dataVec)
{
  m_container.m_analytesExtraInfoModel->setData(dataVec);
}

void ResultsData::backgroundCompositionRefresh(const int lowestCharge, const int highestCharge,
                                               QVector<QString> &&constituents,
                                               QVector<QString> &&complex,
                                               QVector<IonicCompositionModel::ConstituentConcentrations> &&concentrations) noexcept
{
  return m_container.m_bgeIonicComposition->refreshData(lowestCharge, highestCharge, std::move(constituents), std::move(complex), std::move(concentrations));
}

void ResultsData::backgroundExtraInfoRefresh(const QVector<BGEExtraInfoModel::ExtraInfo> &dataVec)
{
  m_container.m_bgeExtraInfoModel->setData(dataVec);
}

QVector<double> & ResultsData::backgroundPropsData()
{
  return m_container.m_backgroundPropsData;
}

int ResultsData::backgroundPropsIndex(const BackgroundPropertiesMapping::Items item) const
{
  return m_container.m_backgroundPropsModel->indexFromItem(item);
}

void ResultsData::backgroundPropsRefresh()
{
  m_container.m_backgroundPropsModel->notifyAllDataChanged();
}

void ResultsData::eigenzoneDetailsRefresh(QVector<QString> &&constituents, QVector<EigenzoneDetailsModel::EigenzoneProps> &&eigenzones) noexcept
{
  m_container.m_eigenzoneDetailsModel->refreshData(std::move(constituents), std::move(eigenzones));
}

void ResultsData::systemEigenzonesRefresh(const SystemEigenzonesTableModel::DataVector &dataVec)
{
  m_container.m_systemEigenzonesTableModel->refreshData(dataVec);
}

void ResultsData::systemEigenzonesRefresh(SystemEigenzonesTableModel::DataVector &&dataVec) noexcept
{
  m_container.m_systemEigenzonesTableModel->refreshData(dataVec);
}
