#ifndef PERSISTENCEEXCEPT_H
#define PERSISTENCEEXCEPT_H

#include <stdexcept>

namespace persistence {

    class BadFileFormat : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

} // namespace persistence

#endif // PERSISTENCEEXCEPT_H
