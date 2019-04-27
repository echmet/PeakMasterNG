#include "databaseproxy.h"

#include "../database/db_constituentsdatabase.h"
#include "../persistence/swsettings.h"

#include <algorithm>
#include <cassert>
#include <QDir>
#include <QString>

#ifdef Q_OS_LINUX
#ifdef PMNG_FLATPAK_BUILD
  const char *DatabaseProxy::DATABASE_PATH = "/app/share/PeakMasterNG/pmng_db.sql";
#else
  const char *DatabaseProxy::DATABASE_PATH = "/etc/ECHMET/PeakMasterNG/pmng_db.sql";
#endif // PMNG_FLATPAK_BUILD
#else
const char *DatabaseProxy::DATABASE_PATH = "pmng_db.sql";
#endif // Q_OS_LINUX

static
DatabaseConstituent makeDatabaseConstituent(const database::Constituent &c)
{
  assert(c.chargeLow() <= c.chargeHigh());

  auto pKas = c.pKas();

  const int bChg = database::ConstituentsDatabase::baseCharge(c.chargeLow(), c.chargeHigh());
  assert(pKas.find(bChg) == pKas.cend());
  pKas.emplace(bChg, 0);

  return DatabaseConstituent{c.id(), QString::fromStdString(c.name()), pKas, c.mobilities(), c.chargeLow(), c.chargeHigh()};
}

static
std::vector<std::tuple<int, double, double>> makeProperties(const std::vector<double> &pKas, const std::vector<double> &mobilities,
                                                            const int chargeLow, const int chargeHigh)
{
  std::vector<std::tuple<int, double, double>> properties{};

  const int bChg = database::ConstituentsDatabase::baseCharge(chargeLow, chargeHigh);
  for (int charge = chargeLow; charge <= chargeHigh; charge++) {
    if (charge == bChg)
      properties.emplace_back(charge, mobilities.at(charge - chargeLow), 0);
    else
      properties.emplace_back(charge, mobilities.at(charge - chargeLow), pKas.at(charge - chargeLow - (charge > bChg)));
  }

  return properties;
}

static
std::vector<DatabaseConstituent> doSearch(database::ConstituentsDatabase *dbh,
                                         const database::ConstituentsDatabase::MatchType match, const std::string &name)
{
  database::SearchResults _results{};
  std::vector<DatabaseConstituent> results{};

  database::ConstituentsDatabase::RetCode tRet;
  tRet = dbh->searchByName(name.c_str(), match, _results);
  switch (tRet) {
  case database::ConstituentsDatabase::RetCode::OK:
    break;
  case database::ConstituentsDatabase::RetCode::E_DB_NO_RECORD:
    return {};
  default:
    {
    std::string err{"Database lookup failed: " + dbh->lastDBErrorMessage() + " " + dbh->retCodeToString(tRet)};
    throw DatabaseException{err};
    }
  }

  results.reserve(_results.size());

  for (const auto &c : _results)
    results.emplace_back(makeDatabaseConstituent(c));

  return results;
}

DatabaseProxy::DatabaseProxy()
{
  QString usableDbPath;

#ifdef Q_OS_LINUX
  const auto userPath = persistence::SWSettings::get<QString>(persistence::SWSettings::KEY_USER_DB_PATH);
  if (userPath.isEmpty()) {
    static const QString locPath(".local/share/ECHMET/PeakMasterNG/");

    QDir homeDir = QDir::home();
    const QString editableDbPath = homeDir.absolutePath() + "/" + locPath + "pmng_db.sql";
    if (QFileInfo::exists(editableDbPath))
      usableDbPath = editableDbPath;
    else {
      if (!QFileInfo::exists(DATABASE_PATH)) {
        m_db = nullptr;
        return;
      }

      if (!homeDir.mkpath(locPath)) {
        m_db = nullptr;
        return;
      }

      if (!QFile::copy(DATABASE_PATH, editableDbPath)) {
        m_db = nullptr;
        return;
      }

      usableDbPath = editableDbPath;
    }
  } else
    usableDbPath = userPath;
#else
  usableDbPath = DATABASE_PATH;
#endif // Q_OS_LINUX

  try {
    m_db = new database::ConstituentsDatabase{usableDbPath.toUtf8()};
  } catch (const std::runtime_error &) {
    m_db = nullptr;
  }
}

DatabaseProxy::~DatabaseProxy() noexcept
{
  delete m_db;
}

bool DatabaseProxy::addConstituent(const std::string &name, const std::vector<double> &pKas, const std::vector<double> &mobilities, const int chargeLow, const int chargeHigh)
{
  const auto properties = makeProperties(pKas, mobilities, chargeLow, chargeHigh);

  const auto tRet = m_db->addConstituent(name.c_str(), chargeLow, chargeHigh, properties);

  return tRet == database::ConstituentsDatabase::RetCode::OK;
}

bool DatabaseProxy::deleteById(const int64_t id)
{
  return m_db->deleteConstituent(id) == database::ConstituentsDatabase::RetCode::OK;
}

bool DatabaseProxy::editConstituent(const int64_t id, const std::string &name, const std::vector<double> &pKas, const std::vector<double> &mobilities,
                                    const int chargeLow, const int chargeHigh)
{
  const auto properties = makeProperties(pKas, mobilities, chargeLow, chargeHigh);

  const auto tRet = m_db->editConstituent(id, name.c_str(), chargeLow, chargeHigh, properties);

  return tRet == database::ConstituentsDatabase::RetCode::OK;
}

std::vector<DatabaseConstituent> DatabaseProxy::fetchAll()
{
  return doSearch(m_db, database::ConstituentsDatabase::MatchType::ENTIRE_DB, std::string{});
}

bool DatabaseProxy::isAvailable() const
{
  return m_db != nullptr;
}

bool DatabaseProxy::openDatabase(const QString &path)
{
  delete m_db;

  try {
    m_db = new database::ConstituentsDatabase{path.toLocal8Bit()};
  } catch (const std::runtime_error &) {
    m_db = nullptr;
    return false;
  }

  return true;
}

std::vector<DatabaseConstituent> DatabaseProxy::search(const std::string &name, const MatchType matchType)
{
  const auto match = [matchType]() {
    switch (matchType) {
      case MatchType::CONTAINS:
        return database::ConstituentsDatabase::MatchType::CONTAINS;
      default:
      return database::ConstituentsDatabase::MatchType::BEGINS_WITH;
    }
  }();

  return doSearch(m_db, match, name);
}
