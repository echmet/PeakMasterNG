#include "databaseproxy.h"

#include "../database/db_constituentsdatabase.h"

#include <algorithm>
#include <cassert>
#include <QString>

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

bool DatabaseProxy::isAvailable() const
{
  return m_db != nullptr;
}

std::vector<DatabaseConstituent> DatabaseProxy::search(std::string name)
{
  database::SearchResults _results{};
  std::vector<DatabaseConstituent> results{};

  std::transform(name.begin(), name.end(), name.begin(), ::toupper);

  database::ConstituentsDatabase::RetCode tRet;
  tRet = m_db->searchByName(name.c_str(), database::ConstituentsDatabase::MatchType::BEGINS_WITH, _results);
  switch (tRet) {
  case database::ConstituentsDatabase::RetCode::OK:
    break; /* Fall through */
  case database::ConstituentsDatabase::RetCode::E_DB_NO_RECORD:
    return {};
  default:
    {
    std::string err{"Database lookup failed: " + m_db->lastDBErrorMessage() + " " + m_db->retCodeToString(tRet)};
    throw DatabaseException{err};
    }
  }

  results.reserve(_results.size());

  for (const auto &c : _results)
    results.emplace_back(makeDatabaseConstituent(c));

  return results;
}
