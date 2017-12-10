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

#include <memory>

namespace persistence {
  class Persistence;
}

class Gearbox
{
public:
  Gearbox();
  GDMProxy & backgroundGDMProxy();
  CalculatorInterface calculatorInterface();
  ComplexationManager & complexationManager();
  GDMProxy & sampleGDMProxy();
  ResultsData resultsData();
  ResultsModels resultsModels();
  persistence::Persistence & persistence();

private:
  gdm::GDM m_backgroundGDM;
  gdm::GDM m_sampleGDM;
  ResultsContainer m_resultsContainer;
  ComplexationManager m_complexMgr;
  persistence::Persistence m_persistence;

  std::unique_ptr<GDMProxy> m_backgroundGDMProxy;
  std::unique_ptr<GDMProxy> m_sampleGDMProxy;
};

#endif // GEARBOX_H
