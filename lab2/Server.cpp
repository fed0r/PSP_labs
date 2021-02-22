#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <vector>
#include <ctime>
#include <sys/select.h>
#include <iostream>
using namespace std;

int nclients = 0;

int CountOfClients;
long start_time, end_time;
double A, B, STEP;

int main() {
  // create a socket
  int server_socket = 0;
  int checkSelect, checkBind, checkListen, checkSend, checkSockopt;
  fd_set master; // create a master set
  fd_set clients; // create set for clients
  printf("TCP SERVER \n");
  cout << "Enter the number of clients:\n";
  cin >> CountOfClients;
  do {
        cout << "[-1.5; 1.5]\n";
        cout << "Enter the left border:\n";
        cin >> A;
        cout << "Enter the right border:\n";
        cin >> B;
  } while (A >= B);
  STEP = (B - A) / CountOfClients;
  char buff[1024];
  setlocale(0, "");

  // define the server address
  struct sockaddr_in server_address;
  struct sockaddr_in client_address;

  FD_ZERO(&master); // zero the set

  // (Domain, Type of socket, Protocol)
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(server_socket == -1){
    perror("socket");
    exit(1);
  }

  // bind the socket to our specified IP and port
  server_address.sin_family = AF_INET; // // knows what type of server_address
  server_address.sin_port = htons(9002); // htons() converts data format so that our struct can understand the port number
  server_address.sin_addr.s_addr = INADDR_ANY; // bind to any available address
  memset(server_address.sin_zero, '\0', sizeof server_address.sin_zero); // for padding, to make it the same size as struct sockaddr

  int yes = 1;
  checkSockopt = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  if(checkSockopt == -1){
    perror("setsockopt");
    exit(1);
  }

  checkBind = bind(server_socket, (struct sockaddr *) &server_address, sizeof(struct sockaddr));
  if(checkBind == -1){
    perror("unable to bind");
    exit(1);
  }

  // begin listening for clients attempting to connect
  checkListen = listen(server_socket, CountOfClients);
  if(checkListen == -1){
    perror("listen");
    exit(1);
  }

  fflush(stdout);

  //printf("server socket: \t%d\n", server_socket);
  FD_SET(server_socket, &master); // sets the bit for the server_socket in the master socket set
  printf("Ожидание подключений...\n");

  double border, border_step, recv_sum ,sum = 0;
  border = A;
  border_step = A + STEP;

  //My
  socklen_t addrlen;
  int client_socket;
  addrlen = sizeof(struct sockaddr_in);
  vector<int> client_sockets(CountOfClients);
  while(client_socket = accept(server_socket, (struct sockaddr *)&client_address, &addrlen)){
	  client_sockets[nclients] = client_socket;
	  nclients++; // увеличиваем счетчик подключившихся клиентов
	  // пытаемся получить имя хоста
	  hostent* hst;
	  hst = gethostbyaddr((char*)&client_address.sin_addr.s_addr, 4, AF_INET);

	  // вывод сведений о клиенте
	  printf("+%s [%s] new connect!\n",
		  (hst) ? hst->h_name : "", inet_ntoa(client_address.sin_addr));
	  if (nclients == CountOfClients) {
		  start_time = clock();
		  for (int i = 0; i < CountOfClients; i++) {
			  send(client_sockets[i], (char*)&border, sizeof(double), 0);
			  send(client_sockets[i], (char*)&border_step, sizeof(double), 0);
			  int bytes_recv;
			  bytes_recv = recv(client_sockets[i], (char*)&recv_sum, sizeof(double), 0);
			  sum += recv_sum;
              border += STEP;
              border_step += STEP;
			  nclients--; // уменьшаем счетчик активных клиентов
			  printf("-disconnect\n");
			  {
				  // закрываем сокет
				  close(client_sockets[i]);
			  }
		  }
		  break;
	  }

  }
  //EndMy
end_time = clock() - start_time;
cout << "sum: " << sum << "\n" << "time: " << end_time;
close(server_socket);
return 0;
}
