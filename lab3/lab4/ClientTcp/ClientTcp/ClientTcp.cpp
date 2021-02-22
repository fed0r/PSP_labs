#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <windows.h>

#include <iostream>
#include <vector>
#include <sstream>
#include <chrono>
#include <string>
#include <ctime>
#include <fstream>
#pragma warning(disable: 4996)

using namespace std;



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


int main(int argc, char* argv[])
{
    //загрузка версии библиотеки для вызова сетевых функций
    WSAData wsaData;

    //запрашиваемая версия для библиотеки winsock
    WORD DLLVersion = MAKEWORD(2, 1);
    //фуекция для загрузки библиотеки
    if (WSAStartup(DLLVersion, &wsaData) != 0) {
        std::cout << "Error lib\n";
        exit(1);
    }

    char ip_server[15];

    std::cin >> ip_server;

    //std::vector<int> size_segment(2);


    //структура для хранения адреса (для интернет протоколов)
    SOCKADDR_IN socket_addres;
    int sizeof_socket_addres = sizeof(socket_addres);
    //для хранения ip-адреса
    socket_addres.sin_addr.s_addr = inet_addr(ip_server);
    //для хранения порта
    socket_addres.sin_port = htons(1111);
    //указание семейства протоколов - af_inet -сокет интернета
    socket_addres.sin_family = AF_INET;

    //создаем сокет (параметры: протокол, тип сокета - потоковый или асинхронный
    SOCKET client_Socket = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(client_Socket, (SOCKADDR*)&socket_addres, sizeof(socket_addres)) != 0) {
        std::cout << "Error connection to Server\n";
        return 1;
    }

    std::cout << "Connected to Server\n";


    //////////////////////////////// ОТПРАВКА
    int size;
    int number;
    std::cout << "Input size\n";

    cin >> size;
    std::cout << "Input numbers\n";

    vector<int> vec = { };

    for (int i = 0; i < size; i++) {
        cin >> number;
        vec.push_back(number);
    }

    string str = fromVectorToString(vec);

    int res_size = str.length();
    send(client_Socket, (char*)&res_size, sizeof(int), 0);
    send(client_Socket, str.c_str(), res_size, 0);


    int msg_size;
    recv(client_Socket, (char*)&msg_size, sizeof(int), 0);
    char* msg = new char[msg_size + 1];
    msg[msg_size] = '\0';
    recv(client_Socket, msg, msg_size, 0);

    vector<int> result = fromStringToVector(string(msg));

    cout << "result vector: ";
    printVector(result);



    system("pause");
    closesocket(client_Socket);
    WSACleanup();

    return(0);
}




