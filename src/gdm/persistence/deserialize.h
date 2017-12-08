#ifndef PERSISTENCE_DESERIALIZE_H
#define PERSISTENCE_DESERIALIZE_H

#include "../core/gdm.h"
#include <fstream>

namespace persistence {

gdm::GDM deserialize(std::ifstream file);

} // namespace persistence

#endif // PERSISTENCE_DESERIALIZE_H
