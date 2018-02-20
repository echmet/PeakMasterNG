#ifndef DATABASEPROXY_H
#define DATABASEPROXY_H

#include <map>
#include <stdexcept>
#include <QString>
#include <vector>

namespace database {
  class ConstituentsDatabase;
};

class DatabaseConstituent {
public:
  const QString name;
  const std::map<int, double> pKas;
  const std::map<int, double> mobilities;
  const int chargeLow;
  const int chargeHigh;
};

class DatabaseException : public std::runtime_error
{
  using std::runtime_error::runtime_error;
};

class DatabaseProxy {
public:
  DatabaseProxy();
  ~DatabaseProxy() noexcept;
  std::vector<DatabaseConstituent> fetchAll();
  bool isAvailable() const;
  std::vector<DatabaseConstituent> search(const std::string &name);

private:
  database::ConstituentsDatabase *m_db;

  static const char *DATABASE_PATH;
};

#endif // DATABASEPROXY_H
