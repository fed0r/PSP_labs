#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <windows.h>
#include "ThreadPool.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <chrono>
#include <string>
#include <ctime>
#include <fstream>
#pragma warning(disable: 4996)

using namespace std;

const double epsilon = 1e-2;

//прототип функции 
DWORD WINAPI ThreadFunc(LPVOID client_socket);
SOCKET server_socket;

void printVector(vector<int> vec) {
    for (int i = 0; i < vec.size(); i++) {
        cout << vec[i] << " ";
    }
    cout << "\n";
}

vector<int> fromStringToVector(string str) {
    vector<int> result(0);
    int start = 0;
    int end = 0;
    for (int i = 0; i < str.length(); i++) {
        if (!isspace(str[i])) end = i;
        else {
            string numberString = str.substr(start, end - start + 1);
            result.push_back(atoi(numberString.c_str()));
            start = i + 1;
        }
    }
    string numberString = str.substr(start);
    result.push_back(atoi(numberString.c_str()));

    return result;
}
string fromVectorToString(vector<int> vec) {
    string str;
    for (int i = 0; i < vec.size(); i++) {
        if (i != 0) str += " ";
        str += to_string(vec[i]);
    }

    return str;
}


vector<int> merge(vector<int> left, vector<int> right) {
    vector<int> resultVector(0);
    int leftIndex = 0, rightIndex = 0;

    while (leftIndex < left.size() && rightIndex < right.size()) {
        if (left[leftIndex] < right[rightIndex]) {
            resultVector.push_back(left[leftIndex]);
            leftIndex++;
        }
        else {
            resultVector.push_back(right[rightIndex]);
            rightIndex++;
        }
    }
    while (leftIndex < left.size()) {
        resultVector.push_back(left[leftIndex]);
        leftIndex++;
    }
    while (rightIndex < right.size()) {
        resultVector.push_back(right[rightIndex]);
        rightIndex++;
    }

    return resultVector;
}

vector<int> slice(vector<int> vectorToSlice, int from, int to) {
    vector<int> newVector(0);
    for (int i = from; i < to; i++) {
        newVector.push_back(vectorToSlice[i]);
    }

    return newVector;
}

vector<int> quickSort(vector<int> vec) {
    if (vec.size() <= 1) return vec;

    int middle = vec.size() / 2;
    vector<int> left = slice(vec, 0, middle);
    vector<int> right = slice(vec, middle, vec.size());
    return merge(quickSort(left), quickSort(right));
}

DWORD WINAPI ThreadFunc(LPVOID client_socket)
{

    SOCKET mySock;
    mySock = ((SOCKET*)client_socket)[0];

    int msg_size;
    recv(mySock, (char*)&msg_size, sizeof(int), 0);
    char* msg = new char[msg_size + 1];
    msg[msg_size] = '\0';
    recv(mySock, msg, msg_size, 0);


    vector<int> initial = fromStringToVector(string(msg));

    cout << "initial vector: ";
    printVector(initial);
    delete msg;

    vector<int> sorted = quickSort(initial);


    cout << "Sorted array: ";
    printVector(sorted);


    string result = fromVectorToString(sorted);
    int res_size = result.length();
    send(mySock, (char*)&res_size, sizeof(int), 0);
    send(mySock, result.c_str(), res_size, 0);

    return 0;
}




int main(int argc, char* argv[])
{
    //загрузка версии библиотеки для вызова сетевых функций
    WSAData wsaData;

    //запрашиваемая версия для библиотеки winsock
    WORD DLLVersion = MAKEWORD(2, 1);
    //функция для загрузки библиотеки
    if (WSAStartup(DLLVersion, &wsaData) != 0) {
        std::cout << "Error lib\n";
        exit(1);
    }

    //структура для хранения адреса (для интернет протоколов)
    SOCKADDR_IN server_socket_addres;

    //для хранения ip-адреса
    server_socket_addres.sin_addr.s_addr = 0;
    //для хранения порта
    server_socket_addres.sin_port = htons(1111);
    //указание семейства протоколов - af_inet -сокет интернета
    server_socket_addres.sin_family = AF_INET;

    //создаем сокет (параметры: протокол, тип сокета - потоковый
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    //связываем сокет с его локальным адресом
    bind(server_socket, (SOCKADDR*)&server_socket_addres, sizeof(server_socket_addres));

    //прослушиваем порт(какой порт + количество клиентов в очереди)
    listen(server_socket, SOMAXCONN);



    //принемаем соединение, создавая новый сокет для клиента
    SOCKET client_socket;
    SOCKADDR_IN client_socket_addres;
    int sizeof_client_socket_addres = sizeof(server_socket_addres);


    while ((client_socket = accept(server_socket, (sockaddr*)&client_socket_addres, &sizeof_client_socket_addres)))
    {
        if (client_socket == 0) {
            std::cout << "Error with client connection\n";
        }
        else {
            std::cout << "Client connected\n";
        }

        QueueUserWorkItem(ThreadFunc, &client_socket, WT_EXECUTEDEFAULT);

    }

    closesocket(server_socket);
    system("pause");
    WSACleanup();
    return(0);
}


