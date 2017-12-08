#include "samplegdmproxy.h"

#include "../gdm/core/gdm.h"
#include "gdmproxy_p.h"

#include <cassert>

SampleGDMProxy::SampleGDMProxy(gdm::GDM &sampleGDM) :
  h_sampleGDM{sampleGDM}
{
}

SampleGDMProxy::~SampleGDMProxy()
{
}

std::vector<double> SampleGDMProxy::concentrations(const std::string &name) const noexcept
{
  assert(h_sampleGDM.find(name) != h_sampleGDM.cend());

  auto sampleIt = h_sampleGDM.find(name);

  auto sampleConc = h_sampleGDM.concentrations(sampleIt);
  assert(sampleConc.size() == 1);

  return {sampleConc.at(0)};
}

bool SampleGDMProxy::complexes(const std::string &name) const noexcept
{
  const auto it = h_sampleGDM.find(name);

  assert(it != h_sampleGDM.cend());

  const auto found = gdm::findComplexations(h_sampleGDM.composition(), it);
  return (found.size() > 0);
}

bool SampleGDMProxy::contains(const std::string &name) const noexcept
{
  const auto sampleIt = h_sampleGDM.find(name);

  return sampleIt != h_sampleGDM.cend();
}

void SampleGDMProxy::erase(const std::string &name) noexcept
{
  const auto sampleIt = h_sampleGDM.find(name);

  GDMProxy_p::eraseComplexations(sampleIt, h_sampleGDM);

  if (sampleIt != h_sampleGDM.cend())
    h_sampleGDM.erase(sampleIt);
}

gdm::Constituent SampleGDMProxy::get(const std::string &name)
{
  auto it = h_sampleGDM.find(name);
  if (it == h_sampleGDM.cend())
    throw GDMProxyException{"Constituent not found"};

  return *it;
}

bool SampleGDMProxy::insert(const gdm::Constituent &ctuent) noexcept
{
  try {
    h_sampleGDM.insert(ctuent);
  } catch (std::bad_alloc &) {
    erase(ctuent.name());
    return false;
  }

  setConcentrations(ctuent.name(), { 0 } );

  return true;
}

bool SampleGDMProxy::isNucleus(const std::string &name) const noexcept
{
  const auto it = h_sampleGDM.find(name);
  assert(it != h_sampleGDM.cend());

  return it->type() == gdm::ConstituentType::Nucleus;
}

void SampleGDMProxy::setConcentrations(const std::string &name, const std::vector<double> &concentrations) noexcept
{
  assert(h_sampleGDM.find(name) != h_sampleGDM.cend());
  assert(concentrations.size() == 1);

  auto sampleIt = h_sampleGDM.find(name);

  h_sampleGDM.setConcentrations(sampleIt, concentrations);
}

bool SampleGDMProxy::update(const std::string &name, const gdm::Constituent &ctuent)
{
  auto it = h_sampleGDM.find(name);
  assert(it != h_sampleGDM.cend());

  if (!GDMProxy_p::typeChangeOk(ctuent, *it, h_sampleGDM))
    throw GDMProxyException{"Type changes are not allowed for complexing constituents"};

  auto ret = h_sampleGDM.update(it, ctuent);

  return std::get<1>(ret);
}
