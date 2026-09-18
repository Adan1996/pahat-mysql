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

// Prototipe Fungsi Ekspor untuk Pustaka/DLL Driver
DB_EXPORT DBConn*   db_connect(const char* host, const char* user, const char* pass, const char* dbname, unsigned int port);
DB_EXPORT int       db_query(DBConn* conn, const char* query);
DB_EXPORT DBResult* db_fetch(DBConn* conn, const char* query);
DB_EXPORT void      db_free_result(DBResult *res);
DB_EXPORT void      db_close(DBConn* conn);

// Type definitions untuk dynamic function pointer (dlsym / GetProcAddress)
typedef DBConn*   (*fn_db_connect)(const char*, const char*, const char*, const char*, unsigned int);
typedef int       (*fn_db_query)(DBConn*, const char*);
typedef DBResult* (*fn_db_fetch)(DBConn*, const char*);
typedef void      (*fn_db_free_result)(DBResult*);
typedef void      (*fn_db_close)(DBConn*);

#ifdef __cplusplus
}
#endif

#endif