#ifndef DATABASEPROXY_H
#define DATABASEPROXY_H

#include <map>
#include <stdexcept>
#include <vector>

namespace database {
  class ConstituentsDatabase;
};

class DatabaseConstituent {
public:
  const std::string name;
  const std::map<int, double> pKas;
  const std::map<int, double> mobilities;
};

class DatabaseException : public std::runtime_error
{
  using std::runtime_error::runtime_error;
};

class DatabaseProxy {
public:
  DatabaseProxy();
  ~DatabaseProxy() noexcept;
  bool isAvailable() const;
  std::vector<DatabaseConstituent> search(std::string name);

private:
  database::ConstituentsDatabase *m_db;

  static const char *DATABASE_PATH;
};

#endif // DATABASEPROXY_H
