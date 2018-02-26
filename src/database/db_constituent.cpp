#include "db_constituent.h"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace database {

Constituent::Constituent() :
  m_id{-1},
  m_chargeLow{0},
  m_chargeHigh{0}
{}

Constituent::Constituent(const Constituent &other) :
  m_id{other.m_id},
  m_chargeLow{other.m_chargeLow},
  m_chargeHigh{other.m_chargeHigh},
  m_name{other.m_name},
  m_mobilities(other.m_mobilities),
  m_pKas(other.m_pKas)
{
}

Constituent::Constituent(Constituent &&other) noexcept :
  m_id{other.m_id},
  m_chargeLow{other.m_chargeLow},
  m_chargeHigh{other.m_chargeHigh},
  m_name{std::move(other.m_name)},
  m_mobilities(std::move(other.m_mobilities)),
  m_pKas(std::move(other.m_pKas))
{
}

Constituent::Constituent(const int64_t id, const std::string &name, const int chargeLow, const int chargeHigh) :
  m_id(id),
  m_chargeLow(chargeLow),
  m_chargeHigh(chargeHigh),
  m_name(name)
{
  int chargeRange = chargeHigh - chargeLow;
  if (chargeRange > std::numeric_limits<int>::max())
    throw std::out_of_range("Vector of states is too long");
  if (chargeLow > chargeHigh)
    throw InvalidChargeRangeException{};
}

Constituent & Constituent::operator=(const Constituent &other)
{
  const_cast<int64_t&>(m_id) = other.m_id;
  const_cast<int&>(m_chargeLow) = other.m_chargeLow;
  const_cast<int&>(m_chargeHigh) = other.m_chargeHigh;
  const_cast<std::string&>(m_name) = other.m_name;
  m_mobilities = other.m_mobilities;
  m_pKas = other.m_pKas;

  return *this;
}

Constituent & Constituent::operator=(Constituent &&other) noexcept
{
  const_cast<int64_t&>(m_id) = other.m_id;
  const_cast<int&>(m_chargeLow) = other.m_chargeLow;
  const_cast<int&>(m_chargeHigh) = other.m_chargeHigh;
  const_cast<std::string&>(m_name) = std::move(other.m_name);
  m_mobilities = std::move(other.m_mobilities);
  m_pKas = std::move(other.m_pKas);

  return *this;
}

void Constituent::chargeInBounds(const int charge) const
{
  if (!(charge >= m_chargeLow && charge <= m_chargeHigh))
    throw ChargeOutOfBoundsException{};
}

void Constituent::addMobility(const int charge, const double mobility)
{
  chargeInBounds(charge);

  if (m_mobilities.find(charge) != m_mobilities.cend())
    throw PropertyAlreadySetException{};

  m_mobilities.emplace(charge, mobility);
}

void Constituent::addpKa(const int charge, const double pKa)
{
  chargeInBounds(charge);

  if (m_pKas.find(charge) != m_pKas.cend())
    throw PropertyAlreadySetException{};

  m_pKas.emplace(charge, pKa);
}

int64_t Constituent::id() const
{
  return m_id;
}

double Constituent::mobility(const int charge) const
{
 chargeInBounds(charge);

  const auto it = m_mobilities.find(charge);
  if (it == m_mobilities.cend())
    throw ChargeOutOfBoundsException{};

  return it->second;
}

const std::map<int, double> & Constituent::mobilities() const
{
  return m_mobilities;
}

int Constituent::chargeLow() const
{
  return m_chargeLow;
}

std::string Constituent::name() const
{
  return m_name;
}

int Constituent::chargeHigh() const
{
  return m_chargeHigh;
}

double Constituent::pKa(const int charge) const
{
  chargeInBounds(charge);

  const auto it = m_pKas.find(charge);
  if (it == m_pKas.cend())
    throw ChargeOutOfBoundsException{};

  return it->second;
}

const std::map<int, double> & Constituent::pKas() const
{
  return m_pKas;
}

} // namespace database
