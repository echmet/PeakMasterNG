#ifndef RESULTSCONTAINER_H
#define RESULTSCONTAINER_H

#include "analytesdissociationmodel.h"
#include "analytesextrainfomodel.h"
#include "backgroundeffectivemobilitiesmodel.h"
#include "backgroundpropertiesmodel.h"
#include "eigenzonedetailsmodel.h"
#include "ioniccompositionmodel.h"
#include "systemeigenzonestablemodel.h"

#include <memory>
#include <QMap>
#include <QVector>

class ResultsContainer
{
public:
  explicit ResultsContainer();

  std::unique_ptr<AnalytesDissociationModel> m_analytesDissociationModel;
  std::unique_ptr<AnalytesExtraInfoModel> m_analytesExtraInfoModel;
  std::unique_ptr<BackgroundEffectiveMobilitiesModel> m_backgroundEffMobilitiesModel;
  std::unique_ptr<BackgroundPropertiesMapping::MapperModel> m_backgroundPropsModel;
  std::unique_ptr<EigenzoneDetailsModel> m_eigenzoneDetailsModel;
  std::unique_ptr<IonicCompositionModel> m_bgeIonicComposition;
  std::unique_ptr<SystemEigenzonesTableModel> m_systemEigenzonesTableModel;

  QVector<double> m_backgroundPropsData;
};

#endif // RESULTSCONTAINER_H
