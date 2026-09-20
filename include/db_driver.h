#ifndef DB_DRIVER_H
#define DB_DRIVER_H

#ifdef _WIN32
  #define DB_EXPORT __declspec(dllexport)
#else
  #define DB_EXPORT
#endif

// Forward declaration koneksi
typedef struct DBConn DBConn;

// Struct penampung data hasil fetch
typedef struct {
    int num_rows;
    int num_cols;
    char ***data;    // Matrix string [row][col]
    char **headers;  // Nama-nama kolom
} DBResult;

#ifdef __cplusplus
extern "C" {
#endif

// --- PROTOTIPE FUNGSI EKSPOR UNTUK PUSTAKA/DLL DRIVER ---
DB_EXPORT DBConn*          db_connect(const char* host, const char* user, const char* pass, const char* dbname, unsigned int port);
DB_EXPORT DBConn*          db_connect_options(const char* host, const char* user, const char* pass, const char* dbname, unsigned int port, const char* charset, unsigned int timeout, int ssl);
DB_EXPORT int              db_ping(DBConn* conn);
DB_EXPORT int              db_select_db(DBConn* conn, const char* db_name);
DB_EXPORT const char*      db_error_msg(DBConn* conn);
DB_EXPORT unsigned int     db_errno_code(DBConn* conn);
DB_EXPORT int              db_query(DBConn* conn, const char* query);
DB_EXPORT unsigned long long db_last_insert_id(DBConn* conn);
DB_EXPORT long long        db_affected_rows(DBConn* conn);
DB_EXPORT DBResult*        db_fetch(DBConn* conn, const char* query);
DB_EXPORT DBResult*        db_fetch_one(DBConn* conn, const char* query);
DB_EXPORT void             db_free_result(DBResult *res);
DB_EXPORT void             db_close(DBConn* conn);

// --- TYPE DEFINITIONS UNTUK DYNAMIC FUNCTION POINTER (dlsym / GetProcAddress) ---
typedef DBConn*          (*fn_db_connect)(const char*, const char*, const char*, const char*, unsigned int);
typedef DBConn*          (*fn_db_connect_options)(const char*, const char*, const char*, const char*, unsigned int, const char*, unsigned int, int);
typedef int              (*fn_db_ping)(DBConn*);
typedef int              (*fn_db_select_db)(DBConn*, const char*);
typedef const char*      (*fn_db_error_msg)(DBConn*);
typedef unsigned int     (*fn_db_errno_code)(DBConn*);
typedef int              (*fn_db_query)(DBConn*, const char*);
typedef unsigned long long (*fn_db_last_insert_id)(DBConn*);
typedef long long        (*fn_db_affected_rows)(DBConn*);
typedef DBResult*        (*fn_db_fetch)(DBConn*, const char*);
typedef DBResult*        (*fn_db_fetch_one)(DBConn*, const char*);
typedef void             (*fn_db_free_result)(DBResult*);
typedef void             (*fn_db_close)(DBConn*);

#ifdef __cplusplus
}
#endif

#endif