#ifndef RESULTSMODELS_H
#define RESULTSMODELS_H

#include "analytesextrainfomodel.h"
#include "bgeextrainfomodel.h"
#include "backgroundpropertiesmapping.h"
#include "systemeigenzonestablemodel.h"

class AnalytesDissociationModel;
class BGEExtraInfoModel;
class ResultsContainer;

class ResultsModels
{
public:
  ResultsModels() = delete;
  ResultsModels(ResultsContainer &container);
  AnalytesDissociationModel * analytesDissociationModel();
  AnalytesExtraInfoModel * analytesExtraInfoModel();
  BGEExtraInfoModel * bgeExtraInfoModel();
  BackgroundPropertiesMapping::MapperModel * backgroundMapperModel();
  QAbstractTableModel * bgeIonicCompositionModel();
  QAbstractTableModel * eigenzoneDetailsModel();
  SystemEigenzonesTableModel * systemEigenzonesModel();

  const AnalytesDissociationModel * analytesDissociationModel() const;
  const AnalytesExtraInfoModel * analytesExtraInfoModel() const;
  const BGEExtraInfoModel * bgeExtraInfoModel() const;
  const BackgroundPropertiesMapping::MapperModel * backgroundMapperModel() const;
  const QAbstractTableModel * bgeIonicCompositionModel() const;
  const QAbstractTableModel * eigenzoneDetailsModel() const;
  const SystemEigenzonesTableModel * systemEigenzonesModel() const;

private:
  ResultsContainer &m_container;
};

#endif // RESULTSMODELS_H
