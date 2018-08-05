#ifndef BACKGROUNDPROPERTIESMAPPING_H
#define BACKGROUNDPROPERTIESMAPPING_H

#include "../../mappers/floatmappermodel.h"

namespace BackgroundPropertiesMapping {
  enum class Items {
    PH = 0,
    IONIC_STRENGTH = 1,
    CONDUCTIVITY = 2,
    BUFFER_CAPACITY = 3,
    RESISTIVITY = 4,
    EOF_MOBILITY = 5,
    EOF_MARKER_TIME = 6,
    LAST_INDEX
  };

  class MapperModel : public FloatMapperModel<Items> {
  public:
    using FloatMapperModel<Items>::FloatMapperModel;
  };
}

#endif // BACKGROUNDPROPERTIESMAPPERMAPPING_H
