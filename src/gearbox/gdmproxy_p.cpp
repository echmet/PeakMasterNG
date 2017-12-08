#include "gdmproxy_p.h"

#include <cassert>

void GDMProxy_p::eraseComplexations(gdm::GDM::const_iterator ctuentIt, gdm::GDM &gdm)
{
  for (auto it = gdm.cbegin(); it != gdm.cend(); it++) {
    if (it->type() == ctuentIt->type())
        continue;
    if (it == ctuentIt)
      continue;
    if (gdm.haveComplexation(ctuentIt, it))
      gdm.eraseComplexation(ctuentIt, it);
  }
}

bool GDMProxy_p::typeChangeOk(const gdm::Constituent &newCtuent, const gdm::Constituent &oldCtuent, gdm::GDM &_gdm)
{
  const auto it = _gdm.find(oldCtuent);

  assert(it != _gdm.cend());

  const auto found = gdm::findComplexations(_gdm.composition(), it);

  if (found.size() > 0 && (oldCtuent.type() != newCtuent.type()))
    return false;
  return true;
}
