#include "gearbox.h"

#include "backgroundgdmproxy.h"
#include "samplegdmproxy.h"
#include "../persistence/persistence.h"
#include "../ui/internal_models/abstractconstituentsmodelbase.h"

Gearbox::Gearbox() :
  m_complexMgr{m_backgroundGDM, m_sampleGDM},
  m_persistence{m_backgroundGDM, m_sampleGDM},
  m_backgroundGDMProxy{std::make_unique<BackgroundGDMProxy>(m_backgroundGDM, m_sampleGDM, CalculatorInterface::minimumConcentration())},
  m_sampleGDMProxy{std::make_unique<SampleGDMProxy>(m_sampleGDM, CalculatorInterface::minimumConcentration())},
  m_dbProxy{std::make_unique<DatabaseProxy>()}
{
  connect(&m_persistence, &persistence::Persistence::deserialized, this, &Gearbox::onDeserialized);
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

DatabaseProxy & Gearbox::databaseProxy()
{
  return *m_dbProxy.get();
}

void Gearbox::onClearAll()
{
    m_backgroundGDM.clear();
    m_sampleGDM.clear();
    m_backgroundUIModel->refreshAll({});
    m_analytesUIModel->refreshAll({});
    m_complexMgr.refreshAll();
}

void Gearbox::onDeserialized()
{
  /* We need to figure out which constituents are analytes and BGE components */
  QVector<QString> analytes;
  QVector<QString> background;
  for (const auto &item : m_sampleGDM) {
    auto bit = m_backgroundGDM.find(item.name());
    if (bit == m_backgroundGDM.cend())
      analytes.append(QString::fromStdString(item.name()));
    else
      background.append(QString::fromStdString(item.name()));
  }

  m_backgroundUIModel->refreshAll(std::move(background));
  m_analytesUIModel->refreshAll(std::move(analytes));

  m_complexMgr.refreshAll();
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

void Gearbox::setUICompositionModels(AbstractConstituentsModelBase *analytesUIModel, AbstractConstituentsModelBase *backgroundUIModel)
{
  m_analytesUIModel = analytesUIModel;
  m_backgroundUIModel = backgroundUIModel;
}
