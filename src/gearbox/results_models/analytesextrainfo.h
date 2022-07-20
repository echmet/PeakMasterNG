#ifndef ANALYTESEXTRAINFO_H
#define ANALYTESEXTRAINFO_H

#include "extrainfobase.h"

class AnalytesExtraInfo : public ExtraInfoBase<AnalytesExtraInfo>
{
public:
  enum {
    U_EMD = _CUSTOM_BEGIN,
    TIME,
    C_MAX,
    CND_MAX,
    _CUSTOM_END
  };

  struct Initializer : ExtraInfoBase<AnalytesExtraInfo>::Initializer {
    QVariant time;
    QVariant uEMD;
    QVariant cMax;
    QVariant cndMax;
  };

  AnalytesExtraInfo() = delete;
  AnalytesExtraInfo(const AnalytesExtraInfo &) = default;
  AnalytesExtraInfo(AnalytesExtraInfo &&) = default;

  AnalytesExtraInfo &operator=(AnalytesExtraInfo &) = default;
  AnalytesExtraInfo &operator=(AnalytesExtraInfo &&) = default;

  AnalytesExtraInfo(QString name, Initializer &&data);

  static const QString &customHeader(int index) noexcept;

};

#endif // ANALYTESEXTRAINFO_H
