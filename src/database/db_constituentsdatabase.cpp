#include "db_constituentsdatabase.h"

#include <sqlite3.h>
#include <cstring>
#include <tuple>
#include <cassert>
#include <QtGlobal>

#if defined (Q_OS_WIN32)
int vasprintf(char **PTR, const char *TEMPLATE, va_list AP)
{
  int res;
  char *buf = new char[16384];
  res = vsnprintf(buf, 16000, TEMPLATE, AP);
  if (res > 0) {
    *PTR = (char*)malloc(res + 1);
    res = vsnprintf(*PTR, res + 1, TEMPLATE, AP);
  }
  delete[] buf;
  return res;
}

int asprintf(char **PTR, const char *TEMPLATE, ...)
{
  int res;
  va_list AP;
  va_start(AP, TEMPLATE);
  res = vasprintf(PTR, TEMPLATE, AP);
  va_end(AP);
  return res;
}
#elif defined(Q_OS_UNIX)
#include <unistd.h>
#else
#error "Unknown platform"
#endif // Q_OS_

#define _STRINGIFY(input) #input
#define ERROR_CODE_CASE(erCase) case RetCode::erCase: return _STRINGIFY(erCase)

namespace database {

const char *ConstituentsDatabase::CONSTITUENTS_TABLE_NAME = "constituents";
const char *ConstituentsDatabase::PKAS_TABLE_NAME = "constituent_pkas";
const char *ConstituentsDatabase::MOBILITIES_TABLE_NAME = "constituent_mobilities";

const char *ConstituentsDatabase::TBC_COL_ID = "id";
const char *ConstituentsDatabase::TBC_COL_ID_TYPE = "INTEGER";
const char *ConstituentsDatabase::TBC_COL_NAME = "name";
const char *ConstituentsDatabase::TBC_COL_NAME_TYPE = "VARCHAR(256)";
const char *ConstituentsDatabase::TBC_COL_CHARGE_LOW = "charge_low";
const char *ConstituentsDatabase::TBC_COL_CHARGE_LOW_TYPE = "INTEGER";
const char *ConstituentsDatabase::TBC_COL_CHARGE_HIGH = "charge_high";
const char *ConstituentsDatabase::TBC_COL_CHARGE_HIGH_TYPE = "INTEGER";

const char *ConstituentsDatabase::TBP_COL_ID = "id";
const char *ConstituentsDatabase::TBP_COL_ID_TYPE = "INTEGER";
const char *ConstituentsDatabase::TBP_COL_CID = "constituent_id";
const char *ConstituentsDatabase::TBP_COL_CID_TYPE = "INTEGER";
const char *ConstituentsDatabase::TBP_COL_CHARGE = "charge";
const char *ConstituentsDatabase::TBP_COL_CHARGE_TYPE = "INTEGER";
const char *ConstituentsDatabase::TBP_COL_TRANSITION = "transition";
const char *ConstituentsDatabase::TBP_COL_TRANSITION_TYPE = "INTEGER";
const char *ConstituentsDatabase::TBP_COL_PKA = "pka";
const char *ConstituentsDatabase::TBP_COL_PKA_TYPE = "DOUBLE";
const char *ConstituentsDatabase::TBP_COL_MOBILITY = "mobility";
const char *ConstituentsDatabase::TBP_COL_MOBILITY_TYPE = "DOUBLE";

void sqlite3_dbh_deleter(sqlite3 *dbh) noexcept
{
  if (dbh != nullptr)
    sqlite3_close_v2(dbh);
}

void sqlite3_stmt_deleter(sqlite3_stmt *stmt) noexcept
{
  if (stmt != nullptr)
    sqlite3_finalize(stmt);
}

template <typename... Args>
ConstituentsDatabase::SQLiteStmtPtr ConstituentsDatabase::makeStatement(Args ...args)
{
  int ret;
  char *stmt;
  sqlite3_stmt *sqliteStmt;

  ret = asprintf(&stmt, args...);
  if (ret < 0)
    throw std::runtime_error{"Cannot prepare SQL statement"};

  ret = sqlite3_prepare_v2(this->m_dbh(), stmt, -1, &sqliteStmt, nullptr);
  free(stmt);
  if (ret != SQLITE_OK)
    throw std::runtime_error{sqlite3_errmsg(this->m_dbh())};

  return SQLiteStmtPtr{sqliteStmt};
}


ConstituentsDatabase::ConstituentsDatabase(const char *path)
{
  if (open(path) != RetCode::OK)
    throw std::runtime_error{"Cannot connect to database"};
}

ConstituentsDatabase::RetCode ConstituentsDatabase::addConstituent(const char *name, const int chargeLow, const int chargeHigh, const std::vector<std::tuple<int, double, double>> &properties)
{
  int ret;
  char *errmsg;
  int64_t constituentId;
  RetCode tRet;

  if (properties.size() != chargeHigh - chargeLow + 1)
    return RetCode::E_DB_INV_ARG;

  if (m_dbh == nullptr)
    return RetCode::E_DB_NOT_OPEN;

  /* Bind statements */
  ret = sqlite3_bind_text(m_constituentExists(), 1, name, -1, SQLITE_STATIC);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));
    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  ret = sqlite3_bind_text(m_insertConstituent(), 1, name, -1, SQLITE_STATIC);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));
    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  ret = sqlite3_bind_int(m_insertConstituent(), 2, chargeLow);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));
    tRet =  RetCode::E_DB_QUERY;
    goto out;
  }

  ret = sqlite3_bind_int(m_insertConstituent(), 3, chargeHigh);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));
    tRet =  RetCode::E_DB_QUERY;
    goto out;
  }

  /* Run queries */
  ret = sqlite3_step(m_constituentExists());
  switch (ret) {
  case SQLITE_ROW:
    tRet = RetCode::E_DB_RECORD_EXISTS;
    goto out;
    break;
  case SQLITE_DONE:
    break;
  default:
    tRet = RetCode::E_DB_QUERY;
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));
    goto out;
    break;
  }

  ret = sqlite3_exec(m_dbh(), "BEGIN TRANSACTION", nullptr, nullptr, &errmsg);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(errmsg);
    sqlite3_free(errmsg);

    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  ret = sqlite3_step(m_insertConstituent());
  if (ret != SQLITE_DONE) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));

    goto rollback;
  }

  constituentId = sqlite3_last_insert_rowid(m_dbh());
  tRet = insertConstituentProperties(constituentId, properties);
  if (tRet != RetCode::OK)
    goto rollback;

  ret = sqlite3_exec(m_dbh(), "END TRANSACTION", nullptr, nullptr, &errmsg);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(errmsg);
    sqlite3_free(errmsg);

    goto rollback;
  }
  /* Everthing went fine */
  tRet = RetCode::OK;
  goto out;

rollback:
  ret = sqlite3_exec(m_dbh(), "ROLLBACK", nullptr, nullptr, &errmsg);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(errmsg);
    sqlite3_free(errmsg);

    tRet = RetCode::E_DB_ROLLBACK;
  } else
    tRet = RetCode::E_DB_QUERY;

out:
  sqlite3_reset(m_constituentExists());
  sqlite3_reset(m_insertConstituent());

  return tRet;
}

/*! @brief Checks if the database is in the expected format.
 *
 * Checks if the database is in the expected format.
 *
 * @return RetCode::OK if the database format looks OK, appropriate error code otherwise.
 */
ConstituentsDatabase::RetCode ConstituentsDatabase::checkDatabase()
{
  RetCode tRet;
  int ret;
  char *errmsg;
  bool table_ok;
  char *stmt;

  ret = asprintf(&stmt, "SELECT tbl_name FROM sqlite_master WHERE tbl_name='%s';", CONSTITUENTS_TABLE_NAME);
  if (ret < 0)
    return RetCode::E_NOMEM;

  /* Check that the dabatase contains our table */
  ret = sqlite3_exec(m_dbh(), stmt,
                     &checkDatabase_table_callback,
                     &table_ok, &errmsg);
  free(stmt);

  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(errmsg);
    sqlite3_free(errmsg);

    return RetCode::E_DB_QUERY;
  }
  sqlite3_free(errmsg);

  if (!table_ok) {
    m_lastDBError = "The database does not contain table " + std::string(CONSTITUENTS_TABLE_NAME) + ".";
    return RetCode::E_DB_FORMAT;
  }

  tRet = checkTableLayout(CONSTITUENTS_TABLE_NAME, 4, &checkTableLayout_constituents_callback);
  if (tRet != RetCode::OK)
    return tRet;
  tRet = checkTableLayout(PKAS_TABLE_NAME, 4, &checkTableLayout_pkas_callback);
  if (tRet != RetCode::OK)
    return tRet;
  tRet = checkTableLayout(MOBILITIES_TABLE_NAME, 4, &checkTableLayout_mobilities_callback);
  if (tRet != RetCode::OK)
    return tRet;


  return RetCode::OK;
}

int ConstituentsDatabase::checkDatabase_table_callback(void *table_ok, int columns, char **text, char **names)
{
  bool *table_ok_b = static_cast<bool *>(table_ok);

  for (int idx = 0; idx < columns; idx++) {
    if (strcmp(names[idx], "tbl_name") == 0) {
      const char *value = text[idx];

      if (value == nullptr)
        continue;
      if (strcmp(value, CONSTITUENTS_TABLE_NAME) == 0) {
        *table_ok_b = true;
        return SQLITE_OK;
      }
    }
  }

  *table_ok_b = false;
  return SQLITE_OK;
}

/*! @brief Checks if the given table contains all the expected columns.
 *
 * Checks if the given table contains all the expected columns.
 *
 * @return true of the table looks OK, false otherwise.
 */
ConstituentsDatabase::RetCode ConstituentsDatabase::checkTableLayout(const char *table, const int columns, SQLite3Callback callback)
{
  int ret;
  char *errmsg;
  int table_cols = 0;
  char *stmt;

  ret = asprintf(&stmt, "PRAGMA table_info('%s');", table);
  if (ret < 0)
    return RetCode::E_NOMEM;

  ret = sqlite3_exec(m_dbh(), stmt,
                     callback,
                     &table_cols, &errmsg);
  free(stmt);

  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(errmsg);
    sqlite3_free(errmsg);

    return RetCode::E_DB_QUERY;
  }
  sqlite3_free(errmsg);

  if (table_cols != columns) {
    m_lastDBError = "Table \"" + std::string(table) + "\" has unexpected layout.";
    return RetCode::E_DB_FORMAT;
  }

  return RetCode::OK;
}

int ConstituentsDatabase::checkTableLayout_constituents_callback(void *table_cols, int columns, char **text, char **names)
{
  (void)names;

  int *table_cols_i = static_cast<int *>(table_cols);
  const char *colName = nullptr;
  const char *dataType = nullptr;

  if (columns < 3)
    return SQLITE_ABORT;

  colName = text[1];
  dataType = text[2];

  if (colName == nullptr || dataType == nullptr)
    return SQLITE_OK;

  /* Check for primary key */
  if (strcmp(colName, TBC_COL_ID) == 0 && strcmp(dataType, TBC_COL_ID_TYPE) == 0) {
    (*table_cols_i)++;
  }
  else if (strcmp(colName, TBC_COL_NAME) == 0 && strcmp(dataType, TBC_COL_NAME_TYPE) == 0) {
    (*table_cols_i)++;
  }
  else if (strcmp(colName, TBC_COL_CHARGE_LOW) == 0 && strcmp(dataType, TBC_COL_CHARGE_LOW_TYPE) == 0) {
    (*table_cols_i)++;
  }
  else if (strcmp(colName, TBC_COL_CHARGE_HIGH) == 0 && strcmp(dataType, TBC_COL_CHARGE_HIGH_TYPE) == 0) {
    (*table_cols_i)++;
  }

  return SQLITE_OK;
}

int ConstituentsDatabase::checkTableLayout_pkas_callback(void *table_cols, int columns, char **text, char **names)
{
  (void)names;

  int *table_cols_i = static_cast<int *>(table_cols);
  const char *colName = nullptr;
  const char *dataType = nullptr;

  if (columns < 3)
    return SQLITE_ABORT;

  colName = text[1];
  dataType = text[2];

  if (colName == nullptr || dataType == nullptr)
    return SQLITE_OK;

  /* Check for primary key */
  if (strcmp(colName, TBP_COL_ID) == 0 && strcmp(dataType, TBP_COL_ID_TYPE) == 0) {
    (*table_cols_i)++;
  }
  else if (strcmp(colName, TBP_COL_CID) == 0 && strcmp(dataType, TBP_COL_CID_TYPE) == 0) {
    (*table_cols_i)++;
  }
  else if (strcmp(colName, TBP_COL_TRANSITION) == 0 && strcmp(dataType, TBP_COL_TRANSITION_TYPE) == 0) {
    (*table_cols_i)++;
  }
  else if (strcmp(colName, TBP_COL_PKA) == 0 && strcmp(dataType, TBP_COL_PKA_TYPE) == 0) {
    (*table_cols_i)++;
  }

  return SQLITE_OK;
}

int ConstituentsDatabase::checkTableLayout_mobilities_callback(void *table_cols, int columns, char **text, char **names)
{
  (void)names;

  int *table_cols_i = static_cast<int *>(table_cols);
  const char *colName = nullptr;
  const char *dataType = nullptr;

  if (columns < 3)
    return SQLITE_ABORT;

  colName = text[1];
  dataType = text[2];

  if (colName == nullptr || dataType == nullptr)
    return SQLITE_OK;

  /* Check for primary key */
  if (strcmp(colName, TBP_COL_ID) == 0 && strcmp(dataType, TBP_COL_ID_TYPE) == 0) {
    (*table_cols_i)++;
  }
  else if (strcmp(colName, TBP_COL_CID) == 0 && strcmp(dataType, TBP_COL_CID_TYPE) == 0) {
    (*table_cols_i)++;
  }
  else if (strcmp(colName, TBP_COL_CHARGE) == 0 && strcmp(dataType, TBP_COL_CHARGE_TYPE) == 0) {
    (*table_cols_i)++;
  }
  else if (strcmp(colName, TBP_COL_MOBILITY) == 0 && strcmp(dataType, TBP_COL_MOBILITY_TYPE) == 0) {
    (*table_cols_i)++;
  }

  return SQLITE_OK;
}

ConstituentsDatabase::RetCode ConstituentsDatabase::deleteConstituent(const int64_t id)
{
  int ret;
  RetCode tRet;
  char *errmsg;

  /* Bind statemets */
  ret = sqlite3_bind_int64(m_constituentExists(), 1, id);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));
    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  ret = sqlite3_bind_int64(m_deleteConstituent(), 1, id);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));
    tRet =  RetCode::E_DB_QUERY;
    goto out;
  }

  ret = sqlite3_bind_int64(m_deleteConstituentPKa(), 1, id);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));
    tRet = RetCode::E_DB_QUERY;
    goto out;
  }
  ret = sqlite3_bind_int64(m_deleteConstituentMobility(), 1, id);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));
    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  /* Run queries */
  ret = sqlite3_step(m_idExists());
  if (ret != SQLITE_DONE) {
    tRet = RetCode::E_DB_NO_RECORD;
    goto out;
  }

  ret = sqlite3_exec(m_dbh(), "BEGIN TRANSACTION", nullptr, nullptr, &errmsg);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(errmsg);
    sqlite3_free(errmsg);

    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  ret = sqlite3_step(m_deleteConstituent());
  if (ret != SQLITE_DONE) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));

    tRet = RetCode::E_DB_QUERY;
    goto rollback;
  }

  ret = sqlite3_step(m_deleteConstituentPKa());
  if (ret != SQLITE_DONE) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));

    tRet = RetCode::E_DB_QUERY;
    goto rollback;
  }

  ret = sqlite3_step(m_deleteConstituentMobility());
  if (ret != SQLITE_DONE) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));

    tRet = RetCode::E_DB_QUERY;
    goto rollback;
  }

  ret = sqlite3_exec(m_dbh(), "END TRANSACTION", nullptr, nullptr, &errmsg);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(errmsg);
    sqlite3_free(errmsg);

    tRet = RetCode::E_DB_QUERY;
    goto rollback;
  }
  /* Everything went fine */
  tRet = RetCode::OK;
  goto out;

rollback:
  ret = sqlite3_exec(m_dbh(), "ROLLBACK", nullptr, nullptr, &errmsg);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(errmsg);
    sqlite3_free(errmsg);
    tRet = RetCode::E_DB_ROLLBACK;
  }

out:
  sqlite3_reset(m_idExists());
  sqlite3_reset(m_deleteConstituent());
  sqlite3_reset(m_deleteConstituentPKa());
  sqlite3_reset(m_deleteConstituentMobility());

  return tRet;
}

ConstituentsDatabase::RetCode ConstituentsDatabase::editConstituent(const int64_t id, const char *name, const int chargeLow, const int chargeHigh,
                                                                    const std::vector<std::tuple<int, double, double>> &properties)
{
  int ret;
  char *errmsg;
  RetCode tRet;

  if (m_dbh == nullptr)
    return RetCode::E_DB_NOT_OPEN;

  /* Bind statements */
  ret = sqlite3_bind_text(m_constituentExists(), 1, name, -1, SQLITE_STATIC);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));
    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  ret = sqlite3_bind_int64(m_deleteConstituentPKa(), 1, id);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));
    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  ret = sqlite3_bind_int64(m_deleteConstituentMobility(), 1, id);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));
    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  ret = sqlite3_bind_text(m_updateConstituent(), 1, name, -1, SQLITE_STATIC);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));
    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  ret = sqlite3_bind_int(m_updateConstituent(), 2, chargeLow);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));
    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  ret = sqlite3_bind_int(m_updateConstituent(), 3, chargeHigh);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));
    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  ret = sqlite3_bind_int64(m_updateConstituent(), 4, id);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));
    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  /* Run queries */
  ret = sqlite3_step(m_constituentExists());
  switch (ret) {
  case SQLITE_ROW:
  {
    int64_t fid = sqlite3_column_int64(m_constituentExists(), 0);
    if (fid != id) {
      tRet = RetCode::E_DB_RECORD_EXISTS;
      goto out;
    }
    break;
  }
  case SQLITE_DONE:
    break;
  default:
    tRet = RetCode::E_DB_QUERY;
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));
    goto out;
    break;
  }

  ret = sqlite3_exec(m_dbh(), "BEGIN TRANSACTION", nullptr, nullptr, &errmsg);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(errmsg);
    sqlite3_free(errmsg);

    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  ret = sqlite3_step(m_updateConstituent());
  if (ret != SQLITE_DONE) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));

    tRet = RetCode::E_DB_QUERY;
    goto rollback;
  }

  ret = sqlite3_step(m_deleteConstituentPKa());
  if (ret != SQLITE_DONE) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));

    tRet = RetCode::E_DB_QUERY;
    goto rollback;
  }

  ret = sqlite3_step(m_deleteConstituentMobility());
  if (ret != SQLITE_DONE) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));

    tRet = RetCode::E_DB_QUERY;
    goto rollback;
  }

  tRet = insertConstituentProperties(id, properties);
  if (tRet != RetCode::OK)
    goto rollback;

  ret = sqlite3_exec(m_dbh(), "END TRANSACTION", nullptr, nullptr, &errmsg);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(errmsg);
    sqlite3_free(errmsg);

    tRet = RetCode::E_DB_QUERY;
    goto rollback;
  }

  /* Everything went fine */
  tRet = RetCode::OK;
  goto out;

rollback:
  ret = sqlite3_exec(m_dbh(), "ROLLBACK", nullptr, nullptr, &errmsg);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(errmsg);
    sqlite3_free(errmsg);
    tRet = RetCode::E_DB_ROLLBACK;
  }

out:
  sqlite3_reset(m_constituentExists());
  sqlite3_reset(m_deleteConstituentPKa());
  sqlite3_reset(m_deleteConstituentMobility());
  sqlite3_reset(m_updateConstituent());
  return tRet;
}

/*! @brief Returns the last error message string returned by the underlying database engine.
 *
 * Returns the last error message string returned by the underlying database engine.
 * The error string is set to a new value only when a function returns RetCode::E_DB_*
 *
 * @return Error message string.
 */
std::string ConstituentsDatabase::lastDBErrorMessage() const
{
  return m_lastDBError;
}

/*! @brief Opens a connection to exisitng database.
 *
 * Opens a connection to existing database file and checks
 * if the database is in the expected format.
 *
 * @param path Path to the dababase file.
 *
 * @return RetCode::OK upon success, appropriate return code upon failure.
 */
ConstituentsDatabase::RetCode ConstituentsDatabase::open(const char *path)
{
  RetCode tRet;

  tRet = openDatabaseHandle(path, false);
  if (tRet != RetCode::OK)
    return tRet;

  tRet = checkDatabase();
  if (tRet != RetCode::OK)
    return tRet;

  try {
    prepareStatements();
  } catch (const std::runtime_error &ex) {
    m_lastDBError = ex.what();
    return RetCode::E_DB_PREP_STMTS;
  }

  return RetCode::OK;
}

ConstituentsDatabase::RetCode ConstituentsDatabase::insertConstituentMobility(const int64_t cid, const int charge, const double mobility)
{
  int ret;
  RetCode tRet;

  ret = sqlite3_bind_int64(m_insertMobility(), 1, cid);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));

    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  ret = sqlite3_bind_int(m_insertMobility(), 2, charge);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));

    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  ret = sqlite3_bind_double(m_insertMobility(), 3, mobility);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));

    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  ret = sqlite3_step(m_insertMobility());
  if (ret != SQLITE_DONE) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));

    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  tRet = RetCode::OK;

out:
  sqlite3_reset(m_insertMobility());
  return tRet;
}

ConstituentsDatabase::RetCode ConstituentsDatabase::insertConstituentPKa(const int64_t cid, const int transition, const double pKa)
{
  int ret;
  RetCode tRet;

  assert(transition != 0);

  ret = sqlite3_bind_int64(m_insertPKa(), 1, cid);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));

    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  ret = sqlite3_bind_int(m_insertPKa(), 2, transition);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));

    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  ret = sqlite3_bind_double(m_insertPKa(), 3, pKa);
  if (ret != SQLITE_OK) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));

    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  ret = sqlite3_step(m_insertPKa());
  if (ret != SQLITE_DONE) {
    m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));

    tRet = RetCode::E_DB_QUERY;
    goto out;
  }

  tRet = RetCode::OK;

out:
  sqlite3_reset(m_insertPKa());
  return tRet;
}

ConstituentsDatabase::RetCode ConstituentsDatabase::insertConstituentProperties(const int64_t id,
                                                                                const std::vector<std::tuple<int, double, double>> &properties)
{
  RetCode tRet = RetCode::E_DB_INV_ARG;

  for (size_t idx = 0; idx < properties.size(); idx++) {
    const auto charge = std::get<0>(properties.at(idx));
    const auto mobility = std::get<1>(properties.at(idx));
    const auto pKa = std::get<2>(properties.at(idx));

    if (charge != 0) {
      tRet = insertConstituentPKa(id, charge, pKa);
      if (tRet != RetCode::OK)
        return tRet;
    }

    tRet = insertConstituentMobility(id, charge, mobility);
    if (tRet != RetCode::OK)
      return tRet;
  }

  return tRet;
}

/*! @brief Opens a handle to database.
 *
 * @param[in] path Path to the database file.
 * @param[in] allowCreate Create a database if it does not exist.
 *
 * @return RetCode::OK upon success, appropriate error code upon failure.
 */
ConstituentsDatabase::RetCode ConstituentsDatabase::openDatabaseHandle(const char *path, const bool allowCreate)
{
  int ret;
  sqlite3 *dbh;
  int flags = SQLITE_OPEN_READWRITE;

  if (allowCreate)
    flags |= SQLITE_OPEN_CREATE;

  ret = sqlite3_open_v2(path, &dbh, flags, nullptr);
  if (ret != SQLITE_OK) {
    if (dbh == nullptr)
      return RetCode::E_NOMEM;

    m_lastDBError = sqlite3_errmsg(dbh);
    return RetCode::E_DB_CANTOPEN;
  }
  m_dbh = SQLiteDBHPtr{dbh};

  return RetCode::OK;
}

void ConstituentsDatabase::prepareStatements()
{
  m_constituentExists = makeStatement("SELECT %s FROM %s WHERE %s LIKE ?;", TBC_COL_ID, CONSTITUENTS_TABLE_NAME, TBC_COL_NAME);

  m_insertConstituent = makeStatement("INSERT INTO %s(%s, %s, %s) VALUES(?, ?, ?);", CONSTITUENTS_TABLE_NAME, TBC_COL_NAME, TBC_COL_CHARGE_LOW, TBC_COL_CHARGE_HIGH);

  m_insertPKa = makeStatement("INSERT INTO %s(%s, %s, %s) VALUES(?, ?, ?)", PKAS_TABLE_NAME, TBP_COL_CID, TBP_COL_TRANSITION, TBP_COL_PKA);

  m_insertMobility = makeStatement("INSERT INTO %s(%s, %s, %s) VALUES(?, ?, ?)", MOBILITIES_TABLE_NAME, TBP_COL_CID, TBP_COL_CHARGE, TBP_COL_MOBILITY);

  m_searchAll = makeStatement("SELECT %s,%s,%s,%s FROM %s",
                              TBC_COL_ID, TBC_COL_CHARGE_LOW, TBC_COL_CHARGE_HIGH, TBC_COL_NAME,
                              CONSTITUENTS_TABLE_NAME);

  m_searchByName = makeStatement("SELECT %s,%s,%s,%s FROM %s WHERE %s LIKE ? ORDER BY %s;",
                                 TBC_COL_ID, TBC_COL_CHARGE_LOW, TBC_COL_CHARGE_HIGH, TBC_COL_NAME,
                                 CONSTITUENTS_TABLE_NAME, TBC_COL_NAME, TBC_COL_NAME);

  m_fetchPKa = makeStatement("SELECT %s,%s FROM %s WHERE %s = ?;", TBP_COL_TRANSITION, TBP_COL_PKA,
                             PKAS_TABLE_NAME, TBP_COL_CID);

  m_fetchMobility = makeStatement("SELECT %s,%s FROM %s WHERE %s = ?;", TBP_COL_CHARGE, TBP_COL_MOBILITY,
                                  MOBILITIES_TABLE_NAME, TBP_COL_CID);

  m_deleteConstituent = makeStatement("DELETE FROM %s WHERE %s = ?;", CONSTITUENTS_TABLE_NAME, TBC_COL_ID);

  m_deleteConstituentPKa = makeStatement("DELETE FROM %s WHERE %s = ?;", PKAS_TABLE_NAME, TBP_COL_CID);

  m_deleteConstituentMobility = makeStatement("DELETE FROM %s WHERE %s = ?;", MOBILITIES_TABLE_NAME, TBP_COL_CID);

  m_idExists = makeStatement("SELECT %s FROM %s WHERE %s = ?;", TBC_COL_ID, CONSTITUENTS_TABLE_NAME, TBC_COL_ID);

  m_updateConstituent = makeStatement("UPDATE %s SET %s = ?, %s = ?, %s = ? WHERE %s = ?;",
                                      CONSTITUENTS_TABLE_NAME,
                                      TBC_COL_NAME, TBC_COL_CHARGE_LOW, TBC_COL_CHARGE_HIGH,
                                      TBC_COL_ID);
}

std::string ConstituentsDatabase::retCodeToString(const RetCode tRet) const
{
  switch (tRet) {
  ERROR_CODE_CASE(OK);
  ERROR_CODE_CASE(E_DB_CANTCREATE);
  ERROR_CODE_CASE(E_DB_CANTOPEN);
  ERROR_CODE_CASE(E_DB_DATA);
  ERROR_CODE_CASE(E_DB_FILE);
  ERROR_CODE_CASE(E_DB_FORMAT);
  ERROR_CODE_CASE(E_DB_INV_ARG);
  ERROR_CODE_CASE(E_DB_NO_RECORD);
  ERROR_CODE_CASE(E_DB_NOT_OPEN);
  ERROR_CODE_CASE(E_DB_QUERY);
  ERROR_CODE_CASE(E_DB_PREP_STMTS);
  ERROR_CODE_CASE(E_DB_RECORD_EXISTS);
  ERROR_CODE_CASE(E_DB_ROLLBACK);
  ERROR_CODE_CASE(E_DB_UNSPECIFIED);
  ERROR_CODE_CASE(E_NOMEM);
  }

  return "Unknown";
}

ConstituentsDatabase::RetCode ConstituentsDatabase::searchByName(const char *name, const MatchType match, SearchResults &results)
{
  int ret = 1;
  RetCode tRet;
  char *pattern;

  if (m_dbh == nullptr)
    return RetCode::E_DB_NOT_OPEN;

  switch (match) {
  case MatchType::BEGINS_WITH:
    ret = asprintf(&pattern, "%s%%", name);
    break;
  case MatchType::CONTAINS:
    ret = asprintf(&pattern, "%%%s%%", name);
    break;
  case MatchType::ENDS_WITH:
    ret = asprintf(&pattern, "%%%s", name);
    break;
  case MatchType::EXACT:
    ret = asprintf(&pattern, "%s", name);
    break;
  case MatchType::ENTIRE_DB:
    break; /* This is handled differently */
  default:
    return RetCode::E_DB_INV_ARG;
  }

  if (ret < 1)
    return RetCode::E_NOMEM;

  sqlite3_stmt *searchStmt;
  if (match == MatchType::ENTIRE_DB) {
    searchStmt = m_searchAll();
  } else {
    ret = sqlite3_bind_text(m_searchByName(), 1, pattern, -1, &sqliteBoundStringDestructor);
    if (ret != SQLITE_OK) {
      m_lastDBError = std::string(sqlite3_errmsg(m_dbh()));

      tRet = RetCode::E_DB_QUERY;
      goto out_2;
    }
    searchStmt = m_searchByName();
  }

  while ((ret = sqlite3_step(searchStmt)) == SQLITE_ROW) {
    int iret;
    int64_t id;
    int chargeLow;
    int chargeHigh;
    Constituent c;
    std::vector<std::pair<int, double>> pKas{};
    std::vector<std::pair<int, double>> mobilities{};

    sqlite3_reset(m_fetchPKa());
    sqlite3_reset(m_fetchMobility());

    try {
      const char *pName = (const char *)sqlite3_column_text(searchStmt, 3);
      if (pName == nullptr)
        continue;

      id = sqlite3_column_int64(searchStmt, 0);
      chargeLow = sqlite3_column_int(searchStmt, 1);
      chargeHigh = sqlite3_column_int(searchStmt, 2);

      c = Constituent{id, pName, chargeLow, chargeHigh};
    } catch (...) {
      tRet =  RetCode::E_DB_DATA;
      goto out;
    }

    iret = sqlite3_bind_int64(m_fetchPKa(), 1, id);
    if (iret != SQLITE_OK)
      continue;
    iret = sqlite3_bind_int64(m_fetchMobility(), 1, id);
    if (iret != SQLITE_OK)
      continue;


    while ((iret = sqlite3_step(m_fetchPKa())) == SQLITE_ROW) {
      const int transition = sqlite3_column_int(m_fetchPKa(), 0);
      const double pKa = sqlite3_column_double(m_fetchPKa(), 1);

      pKas.emplace_back(transition, pKa);
    }

    while ((iret = sqlite3_step(m_fetchMobility())) == SQLITE_ROW) {
      const int charge = sqlite3_column_int(m_fetchMobility(), 0);
      const double mobility = sqlite3_column_double(m_fetchMobility(), 1);

      mobilities.emplace_back(charge, mobility);
    }

    if (pKas.size() +1 != mobilities.size()) {
      tRet = RetCode::E_DB_DATA;
      goto out;
    }

    for (int charge = chargeLow; charge <= chargeHigh; charge++) {
      const auto &mobility = mobilities.at(charge - chargeLow);
      if (charge == 0)
        c.addState(0, std::get<1>(mobility), 0); /* pKa value is ignored for zero charge */
      else {
        const auto &pKa = pKas.at(charge - chargeLow - (charge > 0));
        if (std::get<0>(pKa) != std::get<0>(mobility)) {
          tRet = RetCode::E_DB_DATA;
          goto out;
        }

        if (!c.addState(charge, std::get<1>(pKa), std::get<1>(mobility))) {
          tRet = RetCode::E_DB_DATA;
          goto out;
        }
      }
    }

    results.emplace_back(std::move(c));
  }
  tRet = RetCode::OK;

out:
  sqlite3_reset(searchStmt);
out_2:
  sqlite3_reset(m_fetchPKa());
  sqlite3_reset(m_fetchMobility());

  return tRet;
}

void ConstituentsDatabase::sqliteBoundStringDestructor(void *str)
{
  delete static_cast<char *>(str);
}

} // namespace database

