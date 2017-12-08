#ifndef BACKGROUNDGDMPROXY_H
#define BACKGROUNDGDMPROXY_H

#include "gdmproxy.h"

namespace gdm {
  class GDM;
}

class BackgroundGDMProxy : public GDMProxy
{
public:
  BackgroundGDMProxy(gdm::GDM &backgroudGDM, gdm::GDM &sampleGDM);
  virtual ~BackgroundGDMProxy() override;
  virtual bool complexes(const std::string &name) const noexcept override;
  virtual std::vector<double> concentrations(const std::string &name) const noexcept override;
  virtual bool contains(const std::string &name) const noexcept override;
  virtual void erase(const std::string &name) noexcept override;
  virtual gdm::Constituent get(const std::string &name) override;
  virtual bool insert(const gdm::Constituent &ctuent) noexcept override;
  virtual bool isNucleus(const std::string &name) const noexcept override;
  virtual void setConcentrations(const std::string &name, const std::vector<double> &concentrations) noexcept override;
  virtual bool update(const std::string &name, const gdm::Constituent &ctuent) override;

private:
  gdm::GDM &h_backgroundGDM;
  gdm::GDM &h_sampleGDM;
};

#endif // BACKGROUNDGDMPROXY_H
