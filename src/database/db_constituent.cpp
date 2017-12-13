#include "db_constituent.h"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace database {

Constituent::State::State() :
  mobility{0.0},
  pKa{0.0}
{
}

Constituent::State::State(const State &other) :
  mobility{other.mobility},
  pKa{other.pKa}
{
}

Constituent::State::State(const double mobility, const double pKa) :
  mobility{mobility},
  pKa{pKa}
{
}

Constituent::State &Constituent::State::operator=(const State &other)
{
  const_cast<double&>(mobility) = other.mobility;
  const_cast<double&>(pKa) = other.pKa;

  return *this;
}

Constituent::Constituent() :
  m_id{0},
  m_n{0},
  m_p{0}
{
}

Constituent::Constituent(const Constituent &other) :
  m_id{other.m_id},
  m_n{other.m_n},
  m_p{other.m_p},
  m_name{other.m_name},
  m_states(other.m_states)
{
}

Constituent::Constituent(Constituent &&other) noexcept :
 m_id{other.m_id},
 m_n{other.m_n},
 m_p{other.m_p},
 m_name{std::move(other.m_name)},
 m_states{std::move(other.m_states)}
{
}

Constituent::Constituent(const int64_t id, const std::string &name, const int n, const int p) :
  m_id(id),
  m_n(n),
  m_p(p),
  m_name(name)
{
  int chargeRange = p - n;
  if (chargeRange > std::numeric_limits<int>::max())
    throw std::out_of_range("Vector of states is too long");
  if (n > 0 || p < 0)
    throw invalid_charge_range();

  m_states.resize(chargeRange + 1);
}

Constituent &Constituent::operator=(const Constituent &other)
{
  const_cast<int64_t&>(m_id) = other.m_id;
  m_n = other.m_n;
  m_p = other.m_p;
  m_name = other.m_name;

  for (const State &s : other.m_states)
    m_states.push_back(s);

  return *this;
}

bool Constituent::chargeInBounds(const int charge) const
{
  return (charge >= m_n && charge <= m_p);
}

void Constituent::checkChargesSet() const
{
  if (m_p - m_n <= 0)
    throw data_uninitialized{};
}

bool Constituent::addState(const int charge, const double pKa, const double mobility)
{
  checkChargesSet();

  if (!chargeInBounds(charge))
    return false;

  if (charge == 0)
    m_states[-m_n] = std::move(State{mobility, std::nan("")});
  else
    m_states[charge - m_n] = std::move(State{mobility, pKa});

  return true;
}

int64_t Constituent::id() const
{
  return m_id;
}

double Constituent::mobility(const int charge) const
{
  checkChargesSet();

  if (!chargeInBounds(charge))
    throw std::out_of_range{"Charge out of bounds"};

  return m_states.at(charge - m_n).mobility;
}

int Constituent::n() const
{
  return m_n;
}

std::string Constituent::name() const
{
  return m_name;
}

int Constituent::p() const
{
  return m_p;
}

double Constituent::pKa(const int charge) const
{
  checkChargesSet();

  if (!chargeInBounds(charge))
    throw std::out_of_range("Charge out of bounds");

  return m_states.at(charge - m_n).pKa;
}

void Constituent::setConstituent(const std::string &name, const int n, const int p)
{
  if (m_n != 0 || m_p != 0)
    throw charge_already_set();

  if (n > 0 || p < 0 || p - n <= 0)
    throw invalid_charge_range();

  m_name = name;
  m_n = n;
  m_p = p;

  m_states.clear();
  m_states.resize(m_p - m_n + 1);

  m_states[-m_n] = State{0.0, std::nan("")};
}

} // namespace database
