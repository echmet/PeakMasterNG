#include "analytesextrainfo.h"

#include "../../globals.h"

AnalytesExtraInfo::AnalytesExtraInfo(QString name, Initializer &&data)
  : ExtraInfoBase(std::move(name), static_cast<ExtraInfoBase::Initializer &&>(data))
{
  set(TIME, std::move(data.time));
  set(U_EMD, std::move(data.uEMD));
  set(C_MAX, std::move(data.cMax));
  set(CND_MAX, std::move(data.cndMax));
}

const QString &AnalytesExtraInfo::customHeader(int index) noexcept
{
  static QVector<QString> strings;
  if (strings.empty())
  {
      strings.resize(base(_CUSTOM_END));
      strings[base(TIME)] = QObject::tr("Time Max (min)");
      strings[base(U_EMD)] = Globals::isZombieOS() ? QObject::tr("u EMD (. 1e-9)") : QObject::tr("\xCE\xBC EMD (\xE2\x8B\x85 1e-9)");
      strings[base(C_MAX)] = QObject::tr("c Max (mM)");
      strings[base(CND_MAX)] = Globals::isZombieOS() ? QObject::tr("K Max (S/m)") : QObject::tr("\xCE\xBA Max (S/m)");
  }

  return strings[base(index)];
}
