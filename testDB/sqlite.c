#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[]) {
  sqlite3 *ppdb;
  //创建数据库
  int ret = sqlite3_open("Server.db", &ppdb);
  if (ret != SQLITE_OK) {
    printf("Sqlite3_open : %s\n", sqlite3_errmsg(ppdb));
    exit(1);
  }

  //创表
  char sql[128] = {0};
  sprintf(sql, "create table if not exists student (id integer, name text, age "
               "integer);");
  ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
  if (ret != SQLITE_OK) {
    printf("Sqlite3_open : %s\n", sqlite3_errmsg(ppdb));
    exit(1);
  }
  return 0;
}
