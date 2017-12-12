#include "backgroundgdmproxy.h"

#include "gdmproxy_p.h"
#include "../gdm/core/gdm.h"

#include <cassert>

BackgroundGDMProxy::BackgroundGDMProxy(gdm::GDM &backgroundGDM, gdm::GDM &sampleGDM, const double minimumConcentration) :
  GDMProxy{minimumConcentration},
  h_backgroundGDM{backgroundGDM},
  h_sampleGDM{sampleGDM}
{
}

BackgroundGDMProxy::~BackgroundGDMProxy()
{
}

bool BackgroundGDMProxy::complexes(const std::string &name) const noexcept
{
  const auto it = h_sampleGDM.find(name);

  assert(it != h_sampleGDM.cend());

  const auto found = gdm::findComplexations(h_sampleGDM.composition(), it);
  return (found.size() > 0);
}

std::vector<double> BackgroundGDMProxy::concentrations(const std::string &name) const noexcept
{
  assert(h_backgroundGDM.find(name) != h_backgroundGDM.cend());
  assert(h_sampleGDM.find(name) != h_sampleGDM.cend());

  auto backgroundIt = h_backgroundGDM.find(name);
  auto sampleIt = h_sampleGDM.find(name);

  auto backgroundConc = h_backgroundGDM.concentrations(backgroundIt);
  assert(backgroundConc.size() == 1);

  auto sampleConc = h_sampleGDM.concentrations(sampleIt);
  assert(sampleConc.size() == 1);

  return {backgroundConc.at(0), sampleConc.at(0)};
}

bool BackgroundGDMProxy::contains(const std::string &name) const noexcept
{
  const auto sampleIt = h_sampleGDM.find(name);

  return sampleIt != h_sampleGDM.cend();
}
void BackgroundGDMProxy::erase(const std::string &name) noexcept
{
  const auto backgroundIt = h_backgroundGDM.find(name);
  const auto sampleIt = h_sampleGDM.find(name);

  assert(h_backgroundGDM.find(name) != h_backgroundGDM.cend());
  assert(h_sampleGDM.find(name) != h_sampleGDM.cend());

  if (backgroundIt != h_backgroundGDM.cend()) {
    GDMProxy_p::eraseComplexations(backgroundIt, h_backgroundGDM);
    h_backgroundGDM.erase(backgroundIt);
  }
  if (sampleIt != h_sampleGDM.cend()) {
    GDMProxy_p::eraseComplexations(sampleIt, h_sampleGDM);
    h_sampleGDM.erase(sampleIt);
  }
}

gdm::Constituent BackgroundGDMProxy::get(const std::string &name)
{
  auto it = h_sampleGDM.find(name);
  if (it == h_sampleGDM.cend())
    throw GDMProxyException{"Constituent not found"};

  return *it;
}

bool BackgroundGDMProxy::insert(const gdm::Constituent &ctuent) noexcept
{
  try {
    h_backgroundGDM.insert(ctuent);
    h_sampleGDM.insert(ctuent);
  } catch (std::bad_alloc &) {
    erase(ctuent.name());
    return false;
  }

  setConcentrations(ctuent.name(), { 0, 0 } );

  return true;
}

bool BackgroundGDMProxy::isNucleus(const std::string &name) const noexcept
{
  const auto it = h_sampleGDM.find(name);
  assert(it != h_sampleGDM.cend());

  return it->type() == gdm::ConstituentType::Nucleus;
}

void BackgroundGDMProxy::setConcentrations(const std::string &name, const std::vector<double> &concentrations) noexcept
{
  const auto C = [this](const double d) {
    if (d >= m_minimumConcentration)
      return d;
    return m_minimumConcentration;
  };

  assert(h_backgroundGDM.find(name) != h_backgroundGDM.cend());
  assert(h_sampleGDM.find(name) != h_sampleGDM.cend());
  assert(concentrations.size() == 2);

  auto backgroundIt = h_backgroundGDM.find(name);
  auto sampleIt = h_sampleGDM.find(name);

  h_backgroundGDM.setConcentrations(backgroundIt, {C(concentrations.at(0))});
  h_sampleGDM.setConcentrations(sampleIt, {C(concentrations.at(1))});
}

bool BackgroundGDMProxy::update(const std::string &name, const gdm::Constituent &ctuent)
{
  auto backgroundIt = h_backgroundGDM.find(name);
  auto sampleIt = h_sampleGDM.find(name);

  assert(backgroundIt != h_backgroundGDM.cend());
  assert(sampleIt != h_sampleGDM.cend());

  if (!GDMProxy_p::typeChangeOk(ctuent, *sampleIt, h_sampleGDM))
    throw GDMProxyException{"Type changes are not allowed for complexing constituents"};

  /* Update the sample composition first as all constituents are present there */
  auto samRet = h_sampleGDM.update(sampleIt, ctuent);
  if (!std::get<1>(samRet))
    return false;

  auto bgRet = h_backgroundGDM.update(backgroundIt, ctuent);
  if (!std::get<1>(bgRet))
    throw GDMProxyException{"Inconsistent composition representation. Please restart the program are report a bug. We are sorry about this..."};

  return true;
}
