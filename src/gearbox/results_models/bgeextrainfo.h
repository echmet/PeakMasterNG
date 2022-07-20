#ifndef BGEEXTRAINFO_H
#define BGEEXTRAINFO_H

#include "extrainfobase.h"

class BGEExtraInfo : public ExtraInfoBase<BGEExtraInfo>
{
public:
  enum {
    _CUSTOM_END = _CUSTOM_BEGIN
  };

  using ExtraInfoBase::ExtraInfoBase;

  static const QString &customHeader(int index) noexcept
  {
    static const QString dummy;

    return dummy;
  }

};

#endif // BGEEXTRAINFO_H
