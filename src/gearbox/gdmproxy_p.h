#ifndef GDMPROXY_P_H
#define GDMPROXY_P_H

#include "../gdm/core/gdm.h"

class GDMProxy_p
{
public:
  static void eraseComplexations(gdm::GDM::const_iterator ctuentIt, gdm::GDM &gdm);
  static bool typeChangeOk(const gdm::Constituent &newCtuent, const gdm::Constituent &oldCtuent, gdm::GDM &gdm);
};

#endif // GDMPROXY_P_H
