#include <mysql.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/db_driver.h"

#ifndef DB_EXPORT
  #ifdef _WIN32
    #define DB_EXPORT __declspec(dllexport)
  #else
    #define DB_EXPORT
  #endif
#endif

struct DBConn {
    MYSQL *mysql;
};

// --- CONNECT STANDARD ---
DB_EXPORT DBConn* db_connect(const char* host, const char* user, const char* pass, const char* dbname, unsigned int port) {
    DBConn* conn = (DBConn*)malloc(sizeof(DBConn));
    if (!conn) return NULL;

    conn->mysql = mysql_init(NULL);
    if (!conn->mysql) {
        free(conn);
        return NULL;
    }

    unsigned int actual_port = (port == 0) ? 3306 : port;

    if (!mysql_real_connect(conn->mysql, host, user, pass, dbname, actual_port, NULL, 0)) {
        fprintf(stderr, "[MySQL Driver Error] %s\n", mysql_error(conn->mysql));
        mysql_close(conn->mysql);
        free(conn);
        return NULL;
    }
    return conn;
}

// --- CONNECT WITH OPTIONS ---
DB_EXPORT DBConn* db_connect_options(const char* host, const char* user, const char* pass, 
                                     const char* dbname, unsigned int port, const char* charset, 
                                     unsigned int timeout, int ssl) {
    DBConn* conn = (DBConn*)malloc(sizeof(DBConn));
    if (!conn) return NULL;

    conn->mysql = mysql_init(NULL);
    if (!conn->mysql) {
        free(conn);
        return NULL;
    }

    if (timeout > 0) {
        mysql_options(conn->mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    }
    if (charset && strlen(charset) > 0) {
        mysql_options(conn->mysql, MYSQL_SET_CHARSET_NAME, charset);
    }
    if (ssl) {
        enum mysql_ssl_mode ssl_mode = SSL_MODE_REQUIRED;
        mysql_options(conn->mysql, MYSQL_OPT_SSL_MODE, &ssl_mode);
    }

    unsigned int actual_port = (port == 0) ? 3306 : port;

    if (!mysql_real_connect(conn->mysql, host, user, pass, dbname, actual_port, NULL, 0)) {
        fprintf(stderr, "[MySQL Driver Error] %s\n", mysql_error(conn->mysql));
        mysql_close(conn->mysql);
        free(conn);
        return NULL;
    }
    return conn;
}

// --- UTILITIES ---
DB_EXPORT int db_ping(DBConn* conn) {
    if (!conn || !conn->mysql) return -1;
    return mysql_ping(conn->mysql);
}

DB_EXPORT int db_select_db(DBConn* conn, const char* db_name) {
    if (!conn || !conn->mysql || !db_name) return -1;
    return mysql_select_db(conn->mysql, db_name);
}

DB_EXPORT const char* db_error_msg(DBConn* conn) {
    if (!conn || !conn->mysql) return "Invalid Connection Handle";
    return mysql_error(conn->mysql);
}

DB_EXPORT unsigned int db_errno_code(DBConn* conn) {
    if (!conn || !conn->mysql) return 2000; // CR_UNKNOWN_ERROR
    return mysql_errno(conn->mysql);
}

// --- EXECUTION & METADATA ---
DB_EXPORT int db_query(DBConn* conn, const char* query) {
    if (!conn || !conn->mysql) return -1;

    if (mysql_query(conn->mysql, query) != 0) {
        fprintf(stderr, "[MySQL Query Error] %s\n", mysql_error(conn->mysql));
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(conn->mysql);
    if (result) {
        mysql_free_result(result);
    } else {
        if (mysql_field_count(conn->mysql) > 0) {
            fprintf(stderr, "[MySQL Fetch Error] %s\n", mysql_error(conn->mysql));
            return -1;
        }
    }

    return 0;
}

DB_EXPORT unsigned long long db_last_insert_id(DBConn* conn) {
    if (!conn || !conn->mysql) return 0;
    return mysql_insert_id(conn->mysql);
}

DB_EXPORT long long db_affected_rows(DBConn* conn) {
    if (!conn || !conn->mysql) return -1;
    return (long long)mysql_affected_rows(conn->mysql);
}

// --- FETCH ALL ROWS ---
DB_EXPORT DBResult* db_fetch(DBConn* conn, const char* query) {
    if (!conn || !conn->mysql) return NULL;

    if (mysql_query(conn->mysql, query) != 0) {
        fprintf(stderr, "[MySQL Fetch Error] %s\n", mysql_error(conn->mysql));
        return NULL;
    }

    MYSQL_RES *res = mysql_store_result(conn->mysql);
    if (!res) return NULL;

    int num_rows = (int)mysql_num_rows(res);
    int num_cols = mysql_num_fields(res);

    DBResult *result = (DBResult*)malloc(sizeof(DBResult));
    result->num_rows = num_rows;
    result->num_cols = num_cols;
    
    // Perlindungan alokasi memori
    result->headers = (num_cols > 0) ? (char**)malloc(sizeof(char*) * num_cols) : NULL;
    result->data = (num_rows > 0) ? (char***)malloc(sizeof(char**) * num_rows) : NULL;

    if (num_cols > 0 && result->headers) {
        MYSQL_FIELD *fields = mysql_fetch_fields(res);
        for (int i = 0; i < num_cols; i++) {
            result->headers[i] = strdup(fields[i].name);
        }
    }

    MYSQL_ROW row;
    int r = 0;
    while ((row = mysql_fetch_row(res)) && r < num_rows) {
        result->data[r] = (char**)malloc(sizeof(char*) * num_cols);
        for (int c = 0; c < num_cols; c++) {
            result->data[r][c] = row[c] ? strdup(row[c]) : strdup("");
        }
        r++;
    }

    mysql_free_result(res);
    return result;
}

// --- FETCH ONE ROW ---
DB_EXPORT DBResult* db_fetch_one(DBConn* conn, const char* query) {
    if (!conn || !conn->mysql) return NULL;

    if (mysql_query(conn->mysql, query) != 0) {
        fprintf(stderr, "[MySQL Fetch One Error] %s\n", mysql_error(conn->mysql));
        return NULL;
    }

    MYSQL_RES *res = mysql_store_result(conn->mysql);
    if (!res) return NULL;

    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        mysql_free_result(res);
        return NULL;
    }

    int num_cols = mysql_num_fields(res);
    MYSQL_FIELD *fields = mysql_fetch_fields(res);

    DBResult *result = (DBResult*)malloc(sizeof(DBResult));
    result->num_rows = 1;
    result->num_cols = num_cols;
    result->headers = (char**)malloc(sizeof(char*) * num_cols);
    result->data = (char***)malloc(sizeof(char**) * 1);

    for (int i = 0; i < num_cols; i++) {
        result->headers[i] = strdup(fields[i].name);
    }

    result->data[0] = (char**)malloc(sizeof(char*) * num_cols);
    for (int c = 0; c < num_cols; c++) {
        result->data[0][c] = row[c] ? strdup(row[c]) : strdup("");
    }

    mysql_free_result(res);
    return result;
}

// --- MEMORY CLEANUP ---
DB_EXPORT void db_free_result(DBResult *res) {
    if (!res) return;
    if (res->headers) {
        for (int i = 0; i < res->num_cols; i++) {
            if (res->headers[i]) free(res->headers[i]);
        }
        free(res->headers);
    }
    if (res->data) {
        for (int r = 0; r < res->num_rows; r++) {
            if (res->data[r]) {
                for (int c = 0; c < res->num_cols; c++) {
                    if (res->data[r][c]) free(res->data[r][c]);
                }
                free(res->data[r]);
            }
        }
        free(res->data);
    }
    free(res);
}

DB_EXPORT void db_close(DBConn* conn) {
    if (conn) {
        if (conn->mysql) {
            mysql_close(conn->mysql);
        }
        free(conn);
    }
}