#include "resultsmodels.h"

#include "resultscontainer.h"

ResultsModels::ResultsModels(ResultsContainer &container) :
  m_container{container}
{
}

BackgroundPropertiesMapping::MapperModel * ResultsModels::backgroundMapperModel()
{
  return m_container.m_backgroundPropsModel.get();
}

EigenzoneDetailsModel * ResultsModels::eigenzoneDetailsModel()
{
  return m_container.m_eigenzoneDetailsModel.get();
}

SystemEigenzonesTableModel * ResultsModels::systemEigenzonesModel()
{
  return m_container.m_systemEigenzonesTableModel.get();
}
