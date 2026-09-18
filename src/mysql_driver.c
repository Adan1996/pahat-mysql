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

// HAPUS DEKLARASI DBResult DI SINI KARENA SUDAH ADA DI db_driver.h

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
    result->headers = (char**)malloc(sizeof(char*) * num_cols);
    result->data = (char***)malloc(sizeof(char**) * num_rows);

    MYSQL_FIELD *fields = mysql_fetch_fields(res);
    for (int i = 0; i < num_cols; i++) {
        result->headers[i] = strdup(fields[i].name);
    }

    MYSQL_ROW row;
    int r = 0;
    while ((row = mysql_fetch_row(res))) {
        result->data[r] = (char**)malloc(sizeof(char*) * num_cols);
        for (int c = 0; c < num_cols; c++) {
            result->data[r][c] = row[c] ? strdup(row[c]) : strdup("");
        }
        r++;
    }

    mysql_free_result(res);
    return result;
}

DB_EXPORT void db_free_result(DBResult *res) {
    if (!res) return;
    for (int i = 0; i < res->num_cols; i++) free(res->headers[i]);
    for (int r = 0; r < res->num_rows; r++) {
        for (int c = 0; c < res->num_cols; c++) free(res->data[r][c]);
        free(res->data[r]);
    }
    free(res->headers);
    free(res->data);
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