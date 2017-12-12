#ifndef GDMPROXY_H
#define GDMPROXY_H

#include <string>
#include <stdexcept>
#include <vector>

namespace gdm {
  class Constituent;
}

class GDMProxyException : public std::runtime_error
{
public:
  using std::runtime_error::runtime_error;
};

class GDMProxy
{
public:
  GDMProxy(const double minimumConcentration);
  virtual ~GDMProxy() = 0;
  virtual std::vector<double> concentrations(const std::string &name) const noexcept = 0;
  virtual bool complexes(const std::string &name) const noexcept = 0;
  virtual bool contains(const std::string &name) const noexcept = 0;
  virtual void erase(const std::string &name) noexcept = 0;
  virtual gdm::Constituent get(const std::string &name) = 0;
  virtual bool insert(const gdm::Constituent &ctuent) noexcept = 0;
  virtual bool isNucleus(const std::string &name) const noexcept = 0;
  virtual void setConcentrations(const std::string &name, const std::vector<double> &concentrations) noexcept = 0;
  virtual bool update(const std::string &name, const gdm::Constituent &ctuent) = 0;

protected:
  const double m_minimumConcentration;
};

#endif // GDMPROXY_H
