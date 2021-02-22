// Пример простого TCP-клиента
#include <stdio.h>
#include <string.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <vector>
using namespace std;

#define PORT 666
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

int main(int argc, char* argv[])
{
    char buff[1024];
    printf("TCP CLIENT\n");
    int choose;
    cout << "Choose:\n1.Local IP\n2.Enter IP\n";
    cin >> choose;
    switch (choose)
    {
    case 1:
        strcpy_s(SERVERADDR, "127.0.0.1");
        break;
    case 2:
        cout << "Enter server's ip address:";
        cin >> SERVERADDR;
        break;
    }


    // Шаг 1 - инициализация библиотеки Winsock
    if (WSAStartup(0x202, (WSADATA*)&buff[0]))
    {
        printf("WSAStart error %d\n", WSAGetLastError());
        return -1;
    }

    // Шаг 2 - создание сокета
    SOCKET my_sock;
    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock < 0)
    {
        printf("Socket() error %d\n", WSAGetLastError());
        return -1;
    }

    // Шаг 3 - установка соединения
    // заполнение структуры sockaddr_in - указание адреса и порта сервера
    sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    HOSTENT* hst;

    // преобразование IP адреса из символьного в сетевой формат
    if (inet_addr(SERVERADDR) != INADDR_NONE)
        dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);
    else
    {
        // попытка получить IP адрес по доменному имени сервера
        if (hst = gethostbyname(SERVERADDR))
            // hst->h_addr_list содержит не массив адресов,
            // а массив указателей на адреса
            ((unsigned long*)&dest_addr.sin_addr)[0] =
            ((unsigned long**)hst->h_addr_list)[0][0];
        else
        {
            printf("Invalid address %s\n", SERVERADDR);
            closesocket(my_sock);
            WSACleanup();
            return -1;
        }
    }

    // адрес сервера получен - пытаемся установить соединение
    if (connect(my_sock, (sockaddr*)&dest_addr, sizeof(dest_addr)))
    {
        printf("Connect error %d\n", WSAGetLastError());
        return -1;
    }

    printf("Connection to %s successfully\n \
            Type quit for quit\n\n", SERVERADDR);

    // Шаг 4 - чтение и передача сообщений
    int nsize;
    double a, b;
    nsize = recv(my_sock, (char*)&a, sizeof(double), NULL) != SOCKET_ERROR;
    nsize = recv(my_sock, (char*)&b, sizeof(double), NULL) != SOCKET_ERROR;
    // ставим завершающий ноль в конце строки
    double s1, s;
    int n = 1; //начальное число шагов
    s1 = rectangle_integral(f, a, b, n); //первое приближение для интеграла
    do {
        s = s1;     //второе приближение
        n = 2 * n;  //увеличение числа шагов в два раза, 
                    //т.е. уменьшение значения шага в два раза
        s1 = rectangle_integral(f, a, b, n);
    } while (fabs(s1 - s) > eps);  //сравнение приближений с заданной точностью

    // передаем строку клиента серверу
    send(my_sock, (char*)&s1, sizeof(double), 0);
    closesocket(my_sock);
    WSACleanup();
    int i;
    cin >> i;
    return 0;
}