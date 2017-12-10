#include "gearbox.h"

#include "backgroundgdmproxy.h"
#include "samplegdmproxy.h"
#include "../persistence/persistence.h"

Gearbox::Gearbox() :
  m_complexMgr{m_backgroundGDM, m_sampleGDM},
  m_persistence{m_backgroundGDM, m_sampleGDM},
  m_backgroundGDMProxy{std::make_unique<BackgroundGDMProxy>(m_backgroundGDM, m_sampleGDM)},
  m_sampleGDMProxy{std::make_unique<SampleGDMProxy>(m_sampleGDM)}
{
}

GDMProxy & Gearbox::backgroundGDMProxy()
{
  return *m_backgroundGDMProxy.get();
}

CalculatorInterface Gearbox::calculatorInterface()
{
  return CalculatorInterface{m_backgroundGDM, m_sampleGDM, resultsData()};
}

ComplexationManager & Gearbox::complexationManager()
{
  return m_complexMgr;
}

ResultsData Gearbox::resultsData()
{
  return ResultsData{m_resultsContainer};
}

ResultsModels Gearbox::resultsModels()
{
  return ResultsModels{m_resultsContainer};
}

persistence::Persistence & Gearbox::persistence()
{
  return m_persistence;
}

GDMProxy & Gearbox::sampleGDMProxy()
{
  return *m_sampleGDMProxy.get();
}


