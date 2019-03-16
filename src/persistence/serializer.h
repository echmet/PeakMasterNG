#ifndef SERIALIZER_H
#define SERIALIZER_H

namespace gdm {
  class GDM;
}

class QString;

namespace persistence {

class System;
class Target;

class Serializer
{
public:
  Serializer() = delete;
  static void serialize(const Target &target, const gdm::GDM &gdmBGE, const gdm::GDM &gdmSample,
                        const System &system);
};

} // namespace persistence

#endif // SERIALIZER_H
