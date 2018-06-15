#ifndef RESULTSMODELS_H
#define RESULTSMODELS_H

#include "analytesextrainfomodel.h"
#include "backgroundpropertiesmodel.h"
#include "systemeigenzonestablemodel.h"

class AnalytesDissociationModel;
class ResultsContainer;

class ResultsModels
{
public:
  ResultsModels() = delete;
  ResultsModels(ResultsContainer &container);
  AnalytesDissociationModel * analytesDissociationModel();
  AnalytesExtraInfoModel * analytesExtraInfoModel();
  BackgroundPropertiesMapping::MapperModel * backgroundMapperModel();
  QAbstractTableModel * bgeIonicCompositionModel();
  QAbstractTableModel * eigenzoneDetailsModel();
  SystemEigenzonesTableModel * systemEigenzonesModel();

private:
  ResultsContainer &m_container;
};

#endif // RESULTSMODELS_H
