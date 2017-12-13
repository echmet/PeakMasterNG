#ifndef GEARBOX_H
#define GEARBOX_H

#include "../gdm/core/gdm.h"
#include "calculatorinterface.h"
#include "complexationmanager.h"
#include "gdmproxy.h"
#include "results_models/resultsmodels.h"
#include "results_models/resultsdata.h"
#include "results_models/resultscontainer.h"
#include "../persistence/persistence.h"
#include "databaseproxy.h"

#include <memory>
#include <QObject>

namespace persistence {
  class Persistence;
}

class AbstractConstituentsModelBase;

class Gearbox : public QObject {
  Q_OBJECT

public:
  explicit Gearbox();
  GDMProxy & backgroundGDMProxy();
  CalculatorInterface calculatorInterface();
  ComplexationManager & complexationManager();
  DatabaseProxy & databaseProxy();
  GDMProxy & sampleGDMProxy();
  ResultsData resultsData();
  ResultsModels resultsModels();
  persistence::Persistence & persistence();
  void setUICompositionModels(AbstractConstituentsModelBase *analytesUIModel, AbstractConstituentsModelBase *backgroundUIModel);

public slots:
  void onClearAll();

private:
  gdm::GDM m_backgroundGDM;
  gdm::GDM m_sampleGDM;
  ResultsContainer m_resultsContainer;
  ComplexationManager m_complexMgr;
  persistence::Persistence m_persistence;

  std::unique_ptr<GDMProxy> m_backgroundGDMProxy;
  std::unique_ptr<GDMProxy> m_sampleGDMProxy;
  std::unique_ptr<DatabaseProxy> m_dbProxy;

  AbstractConstituentsModelBase *m_analytesUIModel;
  AbstractConstituentsModelBase *m_backgroundUIModel;

private slots:
  void onDeserialized();
};

#endif // GEARBOX_H
