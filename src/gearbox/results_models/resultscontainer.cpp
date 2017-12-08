#include "resultscontainer.h"

ResultsContainer::ResultsContainer()
{
  m_backgroundPropsModel = std::make_unique<BackgroundPropertiesMapping::MapperModel>(nullptr);
  m_backgroundPropsData.resize(m_backgroundPropsModel->indexFromItem(BackgroundPropertiesMapping::Items::LAST_INDEX));
  m_backgroundPropsModel->setUnderlyingData(&m_backgroundPropsData);

  m_eigenzoneDetailsModel = std::make_unique<EigenzoneDetailsModel>();

  m_systemEigenzonesTableModel = std::make_unique<SystemEigenzonesTableModel>();
}
