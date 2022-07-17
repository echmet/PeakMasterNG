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

BackgroundEffectiveMobilitiesModel * ResultsModels::backgroundEffectiveMobilitiesModel()
{
  return m_container.m_backgroundEffMobilitiesModel.get();
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

const AnalytesDissociationModel *ResultsModels::analytesDissociationModel() const
{
  return m_container.m_analytesDissociationModel.get();
}

const AnalytesExtraInfoModel * ResultsModels::analytesExtraInfoModel() const
{
  return m_container.m_analytesExtraInfoModel.get();
}

const BackgroundEffectiveMobilitiesModel * ResultsModels::backgroundEffectiveMobilitiesModel() const
{
  return m_container.m_backgroundEffMobilitiesModel.get();
}

const BackgroundPropertiesMapping::MapperModel * ResultsModels::backgroundMapperModel() const
{
  return m_container.m_backgroundPropsModel.get();
}

const QAbstractTableModel * ResultsModels::bgeIonicCompositionModel() const
{
  return m_container.m_bgeIonicComposition.get();
}

const QAbstractTableModel * ResultsModels::eigenzoneDetailsModel() const
{
  return m_container.m_eigenzoneDetailsModel.get();
}

const SystemEigenzonesTableModel * ResultsModels::systemEigenzonesModel() const
{
  return m_container.m_systemEigenzonesTableModel.get();
}
