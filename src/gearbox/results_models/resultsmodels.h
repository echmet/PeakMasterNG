#ifndef RESULTSMODELS_H
#define RESULTSMODELS_H

#include "analytesextrainfomodel.h"
#include "backgroundpropertiesmapping.h"
#include "systemeigenzonestablemodel.h"

class AnalytesDissociationModel;
class BackgroundEffectiveMobilitiesModel;
class ResultsContainer;

class ResultsModels
{
public:
  ResultsModels() = delete;
  ResultsModels(ResultsContainer &container);
  AnalytesDissociationModel * analytesDissociationModel();
  AnalytesExtraInfoModel * analytesExtraInfoModel();
  BackgroundEffectiveMobilitiesModel * backgroundEffectiveMobilitiesModel();
  BackgroundPropertiesMapping::MapperModel * backgroundMapperModel();
  QAbstractTableModel * bgeIonicCompositionModel();
  QAbstractTableModel * eigenzoneDetailsModel();
  SystemEigenzonesTableModel * systemEigenzonesModel();

private:
  ResultsContainer &m_container;
};

#endif // RESULTSMODELS_H
