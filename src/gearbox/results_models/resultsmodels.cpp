#include "resultsmodels.h"

#include "resultscontainer.h"

ResultsModels::ResultsModels(ResultsContainer &container) :
  m_container{container}
{
}

AnalytesDissociationModel *ResultsModels::analytesDissociationModel()
{
  return m_container.m_analytesDissociationModel.get();
}

AnalytesExtraInfoModel * ResultsModels::analytesExtraInfoModel()
{
  return m_container.m_analytesExtraInfoModel.get();
}

BackgroundPropertiesMapping::MapperModel * ResultsModels::backgroundMapperModel()
{
  return m_container.m_backgroundPropsModel.get();
}

QAbstractTableModel * ResultsModels::bgeIonicCompositionModel()
{
  return m_container.m_bgeIonicComposition.get();
}

QAbstractTableModel * ResultsModels::eigenzoneDetailsModel()
{
  return m_container.m_eigenzoneDetailsModel.get();
}

SystemEigenzonesTableModel * ResultsModels::systemEigenzonesModel()
{
  return m_container.m_systemEigenzonesTableModel.get();
}
