#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <vector>
#include <stdbool.h>
#include <iostream>
#include <math.h>
using namespace std;
char SERVERADDR[11];

typedef double(*pointFunc)(double);

double f(double x) {
    return (x*x*pow(2, pow(x*cos(x)*sin(x), 1.0/3)));
}

double rectangle_integral(pointFunc f, double a, double b, int n) {
    double x, h;
    double sum = 0.0;
    double fx;
    h = (b - a) / n;  //шаг

    for (int i = 0; i < n; i++) {
        x = a + i * h;
        fx = f(x);
        sum += fx;
    }
    return (sum * h); //приближенное значение интеграла равно
                      //сумме площадей прямоугольников
}

double eps = 0.0001;

int main() {

  // create a socket
  int network_socket = 0;
  fd_set master; // create a master set
  fd_set clients; // create set for clients

  printf("TCP CLIENT\n");
  int choose;
  cout << "Choose:\n1.Local IP\n2.Enter IP\n";
  cin >> choose;
  switch (choose)
  {
	  case 1:
		  strcpy(SERVERADDR, "127.0.0.1");
		  break;
	  case 2:
		  cout << "Enter server's ip address:";
		  cin >> SERVERADDR;
		  break;
  }

  // define the server address
  struct sockaddr_in server_address;

  network_socket = socket(AF_INET, SOCK_STREAM, 0); // (Domain, Type of socket, Protocol)
  if(network_socket == -1){
    perror("socket");
    exit(1);
  }
  //
  server_address.sin_family = AF_INET; // knows what type of server_address
  server_address.sin_port = htons(9002); // htons() converts data format so that our struct can understand the port number
  server_address.sin_addr.s_addr = inet_addr(SERVERADDR); // bind to any available address
  memset(server_address.sin_zero, '\0', sizeof(server_address.sin_zero)); // for padding, to make it the same size as struct sockaddr

  // connect with server socket
  int connection_status = connect(network_socket, (struct sockaddr*) &server_address, sizeof(server_address));
  // check for error with connection
  if(connection_status == -1){
    perror("connect");
    exit(-1);
  }else{
    printf("Connected to remote socket \n");
  }
  FD_ZERO(&master); // zero the sets
  FD_ZERO(&clients);

  FD_SET(0, &master);  // add zero to the set so that we can watch for input
  FD_SET(network_socket, &master); // add the connected socket
  //Получение размера массива
  double A, B;
  int nsize;
  nsize = recv(network_socket, (char*)&A, sizeof(double), 0);
  nsize = recv(network_socket, (char*)&B, sizeof(double), 0);
  cout << "A: " << A << "\n" << "B: " << B << "\n" << "eps: " << eps;
  //Сортировка
  double s1, s;
  int n = 1; //начальное число шагов
  s1 = rectangle_integral(f, A, B, n); //первое приближение для интеграла
  do {
      s = s1;     //второе приближение
      n = 2 * n;  //увеличение числа шагов в два раза,
                    //т.е. уменьшение значения шага в два раза
      s1 = rectangle_integral(f, A, B, n);
  } while (fabs(s1 - s) > eps);  //сравнение приближений с заданной точностью
  cout << "\n" << "S: " << s1;
  //Отправка отсортированного массива обратно на сервер
  send(network_socket, (char*)&s1, sizeof(double), 0);

  close(network_socket);
  return 0;
}
