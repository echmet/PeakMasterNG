// vim: sw=4 ts=4 sts=4 expandtab :

#ifndef PMNG_DATA_ROLES_H
#define PMNG_DATA_ROLES_H

#include <Qt>

enum PMNGDataRole {
    ConstituentNameRole = Qt::UserRole + 2,
    ComplexationStatusRole,
    ComplexationParameterTypeRole,
    SignalTypeRole,
    AnalyteNameRole,
    DecimalDigitsRole
};

#endif // PMNG_DATA_ROLES_H
