#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
int main() {
  int server_fd;
  struct sockaddr_in server_addr;
  socklen_t addrlen = sizeof(server_addr);

  // 创建套接字
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // 设置服务器地址结构
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY; // 监听所有接口
  server_addr.sin_port = htons(0);          // 假设绑定的端口号是 8080

  // 绑定套接字到本地地址
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
      -1) {
    perror("bind failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  // 获取套接字的本地地址信息
  if (getsockname(server_fd, (struct sockaddr *)&server_addr, &addrlen) == -1) {
    perror("getsockname failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  // 打印本地地址信息
  printf("Local address: %s\n", inet_ntoa(server_addr.sin_addr));
  printf("Local port: %d\n", ntohs(server_addr.sin_port));

  // 清理资源
  close(server_fd);

  return 0;
}
