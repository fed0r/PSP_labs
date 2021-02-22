// Пример простого TCP-эхо-сервера

#include <stdio.h>
#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h> // Wincosk2.h должен быть раньше windows!
#include <windows.h>
#include <locale.h>
#include <iostream>
#include <vector>
#include <ctime>
using namespace std;

#define MY_PORT 666 // Порт, который слушает сервер 666

// глобальная переменная - количество активных пользователей
int nclients = 0;

int CountOfClients;
double A, B, STEP;
long start_time, end_time;


int main()
{
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
    setlocale(0, "");
    char buff[1024];
    cout << "TCP SERVER \n";
    // Шаг 1 - Инициализация Библиотеки Сокетов
    // т.к. возвращенная функцией информация не используется
    // ей передается указатель на рабочий буфер, преобразуемый к указателю
    // на структуру WSADATA.
    // Такой прием позволяет сэкономить одну переменную, однако, буфер
    // должен быть не менее полкилобайта размером (структура WSADATA
    // занимает 400 байт)
    if (WSAStartup(0x0202, (WSADATA*)&buff[0]))
    {
        // Ошибка!
        cout << "Error WSAStartup" << WSAGetLastError() << "\n";
        return -1;
    }

    // Шаг 2 - создание сокета
    SOCKET mysocket;
    // AF_INET - сокет Интернета
    // SOCK_STREAM - потоковый сокет (с установкой соединения)
    // 0 - по умолчанию выбирается TCP протокол
    if ((mysocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        // Ошибка!
        cout << "Error socket" << WSAGetLastError() << "\n";
        WSACleanup(); // Деиницилизация библиотеки Winsock
        return -1;
    }

    // Шаг 3 - связывание сокета с локальным адресом
    sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(MY_PORT); // не забываем о сетевом порядке!!!
    local_addr.sin_addr.s_addr = 0; // сервер принимает подключения
                                    // на все свои IP-адреса

    // вызываем bind для связывания
    if (bind(mysocket, (sockaddr*)&local_addr, sizeof(local_addr)))
    {
        // Ошибка
        cout << "Error bind" << WSAGetLastError() << "\n";
        closesocket(mysocket); // закрываем сокет!
        WSACleanup();
        return -1;
    }

    // Шаг 4 - ожидание подключений
    // размер очереди - 0x100
    if (listen(mysocket, 0x100))
    {
        // Ошибка
        cout << "Error listen" << WSAGetLastError() << "\n";
        closesocket(mysocket);
        WSACleanup();
        return -1;
    }

    cout << "Ожидание подключений...\n";

    // Шаг 5 - извлекаем сообщение из очереди
    SOCKET client_socket; // сокет для клиента
    sockaddr_in client_addr; // адрес клиента (заполняется системой)

    // функции accept необходимо передать размер структуры
    int client_addr_size = sizeof(client_addr);
    double border, border_step, recv_sum, sum = 0;
    border = A;
    border_step = A + STEP;
    vector<SOCKET> client_sockets(CountOfClients);
    // цикл извлечения запросов на подключение из очереди
    while ((client_socket = accept(mysocket, (sockaddr*)&client_addr, \
        & client_addr_size)))
    {
        client_sockets[nclients] = client_socket;
        nclients++; // увеличиваем счетчик подключившихся клиентов

        // пытаемся получить имя хоста
        HOSTENT* hst;
        hst = gethostbyaddr((char*)&client_addr.sin_addr.s_addr, 4, AF_INET);

        // вывод сведений о клиенте
        
        cout << hst->h_name << inet_ntoa(client_addr.sin_addr) << "new connect!\n";

        
        // Вызов нового потока для обслужвания клиента
        // Да, для этого рекомендуется использовать _beginthreadex
        // но, поскольку никаких вызовов функций стандартной Си библиотеки
        // поток не делает, можно обойтись и CreateThread
        DWORD thID;
        
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
                cout << "-disconnect\n";
                {
                    // закрываем сокет
                    closesocket(client_sockets[i]);
                }
            }
            break;
        }
        
    }
    cout << "sum: " << sum;
    end_time = clock() - start_time;
    cout << "\ntime: " << end_time;
    char s[1];
    cin >> s;
    return 0;
}
