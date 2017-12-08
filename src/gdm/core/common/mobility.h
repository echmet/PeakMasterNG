#ifndef GDM_MOBILITY_H
#define GDM_MOBILITY_H

namespace gdm {

using Mobility = double; //Units: 10e-9*m^2/(V*s)

Mobility fromSI(double mobilityInSI) noexcept;
double toSI(Mobility mobility) noexcept;

} // namespace gdm

#endif // GDM_MOBILITY_H
