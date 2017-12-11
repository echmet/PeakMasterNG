#include "persistence.h"

#include "../gdm/core/gdm.h"

#include "deserializer.h"
#include "serializer.h"

namespace persistence {

Persistence::Persistence(gdm::GDM &bgeGDM, gdm::GDM &sampleGDM) :
  m_bgeGDM{bgeGDM},
  m_sampleGDM{sampleGDM}
{
}

void Persistence::deserialize(const QString &filepath, System &system)
{
  gdm::GDM bgeGDM;
  gdm::GDM sampleGDM;

  Deserializer::deserialize(filepath, bgeGDM, sampleGDM, system);

  m_bgeGDM = bgeGDM;
  m_sampleGDM = sampleGDM;

  emit deserialized();
}

void Persistence::serialize(const QString &filepath, const System &system)
{
  Serializer::serialize(filepath, m_bgeGDM, m_sampleGDM, system);
}

} // namespace persistence
