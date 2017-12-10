#ifndef DESERIALIZER_H
#define DESERIALIZER_H

#include "persistence.h"

namespace persistence {

class System;

class Deserializer
{
public:
  Deserializer() = delete;
  static void deserialize(const QString &filepath, gdm::GDM &gdmBGE, gdm::GDM &gdmSample,
                          System &system);
};

} // namespace persistence

#endif // DESERIALIZER_H
