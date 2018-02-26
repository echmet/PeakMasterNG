#ifndef DATABASE_CONSTITUENTSDATABASE_H
#define DATABASE_CONSTITUENTSDATABASE_H

#include "db_constituent.h"

#include <memory>
#include <string>
#include <vector>

struct sqlite3;
struct sqlite3_stmt;

namespace database {

typedef int (*SQLite3Callback)(void *, int, char **, char **);
typedef std::vector<Constituent> SearchResults;

void sqlite3_dbh_deleter(sqlite3 *dbh) noexcept;
void sqlite3_stmt_deleter(sqlite3_stmt *stmt) noexcept;

class ConstituentsDatabase
{

public:

  enum class RetCode {
    OK,
    E_DB_CANTCREATE,
    E_DB_CANTOPEN,
    E_DB_DATA,
    E_DB_FILE,
    E_DB_FORMAT,
    E_DB_INV_ARG,
    E_DB_NO_RECORD,
    E_DB_NOT_OPEN,
    E_DB_QUERY,
    E_DB_PREP_STMTS,
    E_DB_RECORD_EXISTS,
    E_DB_ROLLBACK,
    E_DB_UNSPECIFIED,
    E_NOMEM
  };

  enum class MatchType {
    EXACT,
    BEGINS_WITH,
    ENDS_WITH,
    CONTAINS,
    ENTIRE_DB
  };

  explicit ConstituentsDatabase(const char *path);
  RetCode addConstituent(const char *name, const int chargeLow, const int chargeHigh, const std::vector<std::tuple<int, double, double>> &properties);
  RetCode editConstituent(const int64_t id, const char *name, const int chargeLow, const int chargeHigh, const std::vector<std::tuple<int, double, double>> &properties);
  RetCode deleteConstituent(const int64_t id);
  std::string lastDBErrorMessage() const;
  std::string retCodeToString(const RetCode tRet) const;
  RetCode searchByName(const char *name, const MatchType match, SearchResults &results);

  static int baseCharge(const int chargeLow, const int chargeHigh) noexcept;

private:
  typedef void(* SQLiteDBHDeleter)(sqlite3 *);
  typedef void(* SQLiteStmtDeleter)(sqlite3_stmt *);

  class SQLiteDBHPtr : public std::unique_ptr<sqlite3, SQLiteDBHDeleter> {
  public:
    SQLiteDBHPtr() :
      std::unique_ptr<sqlite3, SQLiteDBHDeleter>{nullptr, sqlite3_dbh_deleter}
    {}
    SQLiteDBHPtr(sqlite3 *dbh) :
      std::unique_ptr<sqlite3, SQLiteDBHDeleter>{dbh, sqlite3_dbh_deleter}
    {}

    sqlite3 * operator()()
    {
      return this->get();
    }
  };

  class SQLiteStmtPtr : public std::unique_ptr<sqlite3_stmt, SQLiteStmtDeleter> {
  public:
    SQLiteStmtPtr() :
      std::unique_ptr<sqlite3_stmt, SQLiteStmtDeleter>{nullptr, sqlite3_stmt_deleter}
    {}
    SQLiteStmtPtr(sqlite3_stmt *stmt) :
      std::unique_ptr<sqlite3_stmt, SQLiteStmtDeleter>{stmt, sqlite3_stmt_deleter}
    {}

    sqlite3_stmt * operator()()
    {
      return this->get();
    }
  };

  std::string m_lastDBError;

  SQLiteStmtPtr m_constituentExists;
  SQLiteStmtPtr m_deleteConstituent;
  SQLiteStmtPtr m_deleteConstituentPKa;
  SQLiteStmtPtr m_deleteConstituentMobility;
  SQLiteStmtPtr m_fetchPKa;
  SQLiteStmtPtr m_fetchMobility;
  SQLiteStmtPtr m_idExists;
  SQLiteStmtPtr m_insertConstituent;
  SQLiteStmtPtr m_insertPKa;
  SQLiteStmtPtr m_insertMobility;
  SQLiteStmtPtr m_searchAll;
  SQLiteStmtPtr m_searchByName;
  SQLiteStmtPtr m_updateConstituent;

  SQLiteDBHPtr m_dbh;

  RetCode checkDatabase();
  RetCode checkTableLayout(const char *table, const int columns, SQLite3Callback callback);
  RetCode insertConstituentMobility(const int64_t cid, const int charge, const double mobility);
  RetCode insertConstituentPKa(const int64_t cid, const int transition, const double pKa);
  RetCode insertConstituentProperties(const int64_t id, const std::vector<std::tuple<int, double, double>> &properties);
  template <typename... Args>
  SQLiteStmtPtr makeStatement(Args ...args);
  RetCode open(const char *path);
  RetCode openDatabaseHandle(const char *path, const bool allowCreate);
  void prepareStatements();

  /* SQLite3 exec callbacks */
  static int checkDatabase_table_callback(void *table_ok, int columns, char **text, char **names);
  static int checkTableLayout_constituents_callback(void *table_ok, int columns, char **text, char **names);
  static int checkTableLayout_pkas_callback(void *table_ok, int columns, char **text, char **names);
  static int checkTableLayout_mobilities_callback(void *table_ok, int columns, char **text, char **names);
  static void sqliteBoundStringDestructor(void *str);

  static const char *CONSTITUENTS_TABLE_NAME;
  static const char *PKAS_TABLE_NAME;
  static const char *MOBILITIES_TABLE_NAME;
  static const char *TBC_COL_ID;
  static const char *TBC_COL_ID_TYPE;
  static const char *TBC_COL_NAME;
  static const char *TBC_COL_NAME_TYPE;
  static const char *TBC_COL_CHARGE_LOW;
  static const char *TBC_COL_CHARGE_LOW_TYPE;
  static const char *TBC_COL_CHARGE_HIGH;
  static const char *TBC_COL_CHARGE_HIGH_TYPE;
  static const char *TBP_COL_ID;
  static const char *TBP_COL_ID_TYPE;
  static const char *TBP_COL_CID;
  static const char *TBP_COL_CID_TYPE;
  static const char *TBP_COL_CHARGE;
  static const char *TBP_COL_CHARGE_TYPE;
  static const char *TBP_COL_TRANSITION;
  static const char *TBP_COL_TRANSITION_TYPE;
  static const char *TBP_COL_PKA;
  static const char *TBP_COL_PKA_TYPE;
  static const char *TBP_COL_MOBILITY;
  static const char *TBP_COL_MOBILITY_TYPE;
};

} // namespace database

#endif // DATABASE_CONSTITUENTSDATABASE_H
