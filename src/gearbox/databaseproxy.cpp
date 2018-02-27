#include "databaseproxy.h"

#include "../database/db_constituentsdatabase.h"

#include <algorithm>
#include <cassert>
#include <QString>

const char *DatabaseProxy::DATABASE_PATH = "pmng_db.sql";

DatabaseConstituent makeDatabaseConstituent(const database::Constituent &c)
{
  assert(c.chargeLow() <= c.chargeHigh());

  auto pKas = c.pKas();

  const int bChg = database::ConstituentsDatabase::baseCharge(c.chargeLow(), c.chargeHigh());
  assert(pKas.find(bChg) == pKas.cend());
  pKas.emplace(bChg, 0);

  return DatabaseConstituent{c.id(), QString::fromStdString(c.name()), pKas, c.mobilities(), c.chargeLow(), c.chargeHigh()};
}

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

std::vector<DatabaseConstituent> DatabaseProxy::search(const std::string &name)
{
  return doSearch(m_db, database::ConstituentsDatabase::MatchType::BEGINS_WITH, name);
}
