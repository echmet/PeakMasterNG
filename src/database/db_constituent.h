#ifndef DATABASE_CONSTITUENT_H
#define DATABASE_CONSTITUENT_H

#include <string>
#include <vector>

namespace database {

class Constituent
{
public:
  class invalid_charge_range : public std::exception {
    virtual const char *what() const noexcept
    {
      return "Charge range is invalid";
    }
  };
  class data_uninitialized : public std::exception {
    virtual const char *what() const noexcept
    {
      return "Constituent data have not been set";
    }
  };
  class charge_already_set : public std::exception {
    virtual const char *what() const noexcept
    {
      return "Charges were already set for this constituent";
    }
  };

  struct State {
    explicit State();
    explicit State(const State &other);
    explicit State(const double mobility, const double pKa);

    const double mobility;
    const double pKa;

    State & operator=(const State &other);
  };

  typedef std::vector<State> States;

  explicit Constituent();
  explicit Constituent(const Constituent &other);
  explicit Constituent(Constituent &&other) noexcept;
  explicit Constituent(const int64_t id, const std::string &name, const int n, const int p);
  bool addState(const int charge, const double pKa, const double mobility);
  int64_t id() const;
  double mobility(const int charge) const;
  int n() const;
  std::string name() const;
  int p() const;
  double pKa(const int charge) const;
  void setConstituent(const std::string &name, const int n, const int p);

  Constituent &operator=(const Constituent &other);

private:
  void checkChargesSet() const;
  bool chargeInBounds(const int charge) const;

  const int64_t m_id;
  int m_n;
  int m_p;
  std::string m_name;
  States m_states;

};

} // namespace database

#endif // DATABASE_CONSTITUENT_H
