#ifndef USERROLES_H
#define USERROLES_H

#include <Qt>

struct UserRoles
{
    static constexpr int ReadRole = Qt::UserRole;
    static constexpr int PrecissionRole = Qt::UserRole + 1;
};

#endif // USERROLES_H
