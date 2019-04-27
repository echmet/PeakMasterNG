#include "swsettings.h"

#include "../globals.h"

namespace persistence {

const QString SWSettings::KEY_AUTOUPDATE_ENABLED{"autoUpdateEnabled"};

SWSettings *SWSettings::s_me{nullptr};

QMap<QString, QVariant> SWSettings::s_defaults = {
  { KEY_AUTOUPDATE_ENABLED, 1 }
};

SWSettings::SWSettings() :
  m_settings{QSettings::IniFormat, QSettings::UserScope, Globals::ORG_NAME, Globals::SOFTWARE_NAME}
{
}

void SWSettings::destroy()
{
  assert(s_me != nullptr);

  delete s_me;

  s_me = nullptr;
}

void SWSettings::initialize()
{
  if (s_me == nullptr)
    s_me = new SWSettings{};
}

} // namespace persitence
