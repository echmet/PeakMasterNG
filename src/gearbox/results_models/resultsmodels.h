#ifndef RESULTSMODELS_H
#define RESULTSMODELS_H

#include "backgroundpropertiesmodel.h"
#include "eigenzonedetailsmodel.h"
#include "systemeigenzonestablemodel.h"

class ResultsContainer;

class ResultsModels
{
public:
  ResultsModels() = delete;
  ResultsModels(ResultsContainer &container);
  BackgroundPropertiesMapping::MapperModel * backgroundMapperModel();
  EigenzoneDetailsModel * eigenzoneDetailsModel();
  SystemEigenzonesTableModel * systemEigenzonesModel();

private:
  ResultsContainer &m_container;
};

#endif // RESULTSMODELS_H
