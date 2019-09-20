#ifndef PHADJUSTERINTERFACE_H
#define PHADJUSTERINTERFACE_H

#include <echmetelems.h>

#include <stdexcept>
#include <string>

class BackgroundGDMProxy;

class pHAdjusterInterface {
public:
  class Exception : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
  };

  pHAdjusterInterface(std::string constituentName, BackgroundGDMProxy &GDMProxy,
                      const bool debyeHuckel, const bool onsagerFuoss);

  void adjustpH(const double targetpH);

private:
  double calculatepH(const ECHMET::NonidealityCorrections corrs);

  const std::string m_constituentName;
  BackgroundGDMProxy &h_GDMProxy;
  const bool m_debyeHuckel;
  const bool m_onsagerFuoss;
};

#endif // PHADJUSTERINTERFACE_H
