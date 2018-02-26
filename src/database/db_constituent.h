#ifndef DATABASE_CONSTITUENT_H
#define DATABASE_CONSTITUENT_H

#include <stdexcept>
#include <map>
#include <string>

namespace database {

class Constituent
{
public:
  class ConstituentOperationException : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
  };

  class PropertyAlreadySetException : public ConstituentOperationException {
  public:
    PropertyAlreadySetException() : ConstituentOperationException{"Property for this charge has already been set"} {}
  };

  class ChargeOutOfBoundsException : public ConstituentOperationException {
  public:
    ChargeOutOfBoundsException() : ConstituentOperationException{"Charge out of bounds"} {}
  };

  class InvalidChargeRangeException : public ConstituentOperationException {
  public:
    InvalidChargeRangeException() : ConstituentOperationException{"Charge range is invalid"} {}
  };

  explicit Constituent();
  explicit Constituent(const Constituent &other);
  explicit Constituent(Constituent &&other) noexcept;
  explicit Constituent(const int64_t id, const std::string &name, const int chargeLow, const int chargeHigh);
  void addMobility(const int charge, const double mobility);
  void addpKa(const int charge, const double pKa);
  int64_t id() const;
  double mobility(const int charge) const;
  const std::map<int, double> & mobilities() const;
  int chargeLow() const;
  std::string name() const;
  int chargeHigh() const;
  double pKa(const int charge) const;
  const std::map<int, double> & pKas() const;
  void setConstituent(const std::string &name, const int n, const int p);

  Constituent & operator=(const Constituent &other);
  Constituent & operator=(Constituent &&other) noexcept;

private:
  void chargeInBounds(const int charge) const;

  const int64_t m_id;
  const int m_chargeLow;
  const int m_chargeHigh;
  const std::string m_name;

  std::map<int, double> m_mobilities;
  std::map<int, double> m_pKas;

};

} // namespace database

#endif // DATABASE_CONSTITUENT_H
