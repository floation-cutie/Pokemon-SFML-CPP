// Client.cpp
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 12345

int main() {
  int sock = 0;
  struct sockaddr_in serv_addr;
  char buffer[1024] = {0};

  // 创建socket文件描述符
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    std::cerr << "Socket creation error" << std::endl;
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // 转换IP地址
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    std::cerr << "Invalid address or Address not supported" << std::endl;
    return -1;
  }

  // 连接到服务器
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    std::cerr << "Connection failed" << std::endl;
    return -1;
  }

  // 发送数据到服务器
  const char *message = "Hello from client";
  send(sock, message, strlen(message), 0);
  std::cout << "Message sent" << std::endl;

  // 读取服务器响应
  int valread = read(sock, buffer, 1024);
  std::cout << "Response: " << buffer << std::endl;

  // 关闭socket
  close(sock);

  return 0;
}
