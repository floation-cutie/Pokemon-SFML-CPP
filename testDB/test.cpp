#include <iostream>
#include <sqlite3.h>

// 定义一个回调函数，用于处理查询结果
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
  // 输出查询结果
  for (int i = 0; i < argc; i++) {
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return SQLITE_OK; // 表示处理了当前行，继续处理下一行
}

int main() {
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;

  // 打开数据库
  rc = sqlite3_open("test.db", &db);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return 1;
  }

  // 执行查询
  rc = sqlite3_exec(db, "SELECT * FROM stu", callback, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    sqlite3_close(db);
    return 1;
  }
  sqlite3_free(zErrMsg);

  // 关闭数据库
  sqlite3_close(db);

  return 0;
}
