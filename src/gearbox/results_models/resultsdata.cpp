#include "resultsdata.h"

#include "resultscontainer.h"

ResultsData::ResultsData(ResultsContainer &container) :
  m_container{container}
{
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
