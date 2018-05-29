#ifndef DATABASEPROXY_H
#define DATABASEPROXY_H

#include <map>
#include <stdexcept>
#include <QObject>
#include <QString>
#include <vector>

namespace database {
  class ConstituentsDatabase;
}

typedef std::map<int, double> DBChargePropsMap;

class DatabaseConstituent {
public:
  const int64_t id;
  const QString name;
  const DBChargePropsMap pKas;
  const DBChargePropsMap mobilities;
  const int chargeLow;
  const int chargeHigh;
};

class DatabaseException : public std::runtime_error
{
  using std::runtime_error::runtime_error;
};

class DatabaseProxy {
  Q_GADGET
public:
  enum class MatchType {
    BEGINS_WITH,
    CONTAINS
  };
  Q_ENUM(MatchType)

  DatabaseProxy();
  ~DatabaseProxy() noexcept;
  bool addConstituent(const std::string &name, const std::vector<double> &pKas, const std::vector<double> &mobilities, const int chargeLow, const int chargeHigh);
  bool deleteById(const int64_t id);
  bool editConstituent(const int64_t id, const std::string &name, const std::vector<double> &pKas, const std::vector<double> &mobilities, const int chargeLow, const int chargeHigh);
  std::vector<DatabaseConstituent> fetchAll();
  bool isAvailable() const;
  bool openDatabase(const QString &path);
  std::vector<DatabaseConstituent> search(const std::string &name, const MatchType matchType);

private:
  database::ConstituentsDatabase *m_db;

  static const char *DATABASE_PATH;
};

Q_DECLARE_METATYPE(DatabaseProxy::MatchType)

#endif // DATABASEPROXY_H
