#include "databaseproxy.h"

#include "../database/db_constituentsdatabase.h"

#include <algorithm>
#include <cassert>
#include <QString>

typedef std::function<database::ConstituentsDatabase::RetCode (database::ConstituentsDatabase *,database::ConstituentsDatabase::MatchType, const std::string &, database::SearchResults)> QueryExecutor;

const char *DatabaseProxy::DATABASE_PATH = "pmng_db.sql";

DatabaseConstituent makeDatabaseConstituent(const database::Constituent &c)
{
  std::map<int, double> pKas;
  std::map<int, double> mobilities;

  assert(c.n() <= c.p());

  for (int charge = c.n(); charge <= c.p(); charge++) {
    pKas.emplace(charge, c.pKa(charge));
    mobilities.emplace(charge, c.mobility(charge));
  }

  return DatabaseConstituent{QString::fromStdString(c.name()), std::move(pKas), std::move(mobilities), c.n(), c.p()};
}

std::vector<DatabaseConstituent> doQuery(database::ConstituentsDatabase *dbh,
                                         const database::ConstituentsDatabase::MatchType match, std::string name)
{

  database::SearchResults _results{};
  std::vector<DatabaseConstituent> results{};

  std::transform(name.begin(), name.end(), name.begin(), ::toupper);

  database::ConstituentsDatabase::RetCode tRet;
  tRet = dbh->searchByName(name.c_str(), match, _results);
  switch (tRet) {
  case database::ConstituentsDatabase::RetCode::OK:
    break; /* Fall through */
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
  try {
    m_db = new database::ConstituentsDatabase{DATABASE_PATH};
  } catch (const std::runtime_error &ex) {
    m_db = nullptr;
  }
}

DatabaseProxy::~DatabaseProxy() noexcept
{
  if (m_db != nullptr)
    delete m_db;
}

std::vector<DatabaseConstituent> DatabaseProxy::fetchAll()
{
  return doQuery(m_db, database::ConstituentsDatabase::MatchType::ENTIRE_DB, std::string{});
}

bool DatabaseProxy::isAvailable() const
{
  return m_db != nullptr;
}

std::vector<DatabaseConstituent> DatabaseProxy::search(const std::string &name)
{
  return doQuery(m_db, database::ConstituentsDatabase::MatchType::BEGINS_WITH, name);
}
