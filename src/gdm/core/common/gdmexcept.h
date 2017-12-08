#ifndef GDM_GDMEXCEPT_H
#define GDM_GDMEXCEPT_H

#include <stdexcept>

namespace gdm {

class LogicError : public std::logic_error
{
public:
    using std::logic_error::logic_error;
};

class InvalidArgument : public LogicError
{
public:
    using LogicError::LogicError;
};

class OutOfRange : public LogicError
{
public:
    using LogicError::LogicError;
};

} // namespace gdm

#endif // GDM_GDMEXCEPT_H
