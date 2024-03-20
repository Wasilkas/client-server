#pragma comment (lib,"Ws2_32.lib") // подключение внешней библиотеки Winsock
#include <WinSock2.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <string>
#include <ws2tcpip.h>

using namespace std; // подключение пространства имен для string
bool fout;

DWORD WINAPI receive(LPVOID clientSocket)
{
	int retVal = 0; // retVal - код ошибки
	SOCKET clientSock; // сокет клиента
	clientSock = *((SOCKET*)clientSocket);

	char bufRec[1000]; // буфер для приема данных

	retVal = recv(clientSock, bufRec, 1000, 0); // получение сообщения от сервера
	//clientSock – дескриптор сокета клиента
	//bufRec - имя буфера для приема данных, размер этого буфера 1000
	//0-отсутствие списока специальных констант, с помощью которых можно запрашивать специальные опции

	if (!strcmp(bufRec, "Server shutdown")) //если получили сообщение о завершении работы сервера
	{
		printf("Server shutdown\n"); // выводим сообщение о завершении работы сервера
		fout = true; // завершение работы клиента
		return 0; // возвращаемое значение функции receive(0)
	}

	if (!strcmp(bufRec, "Sorry, too much people on the line")) //если получили сообщение о переполнении сервера
	{
		printf("Sorry, too much people on the line\n"); // выводим сообщение о переполнение сервера
		fout = true; // завершение работы клиента
		return 0; // возвращаемое значение функции receive(0)
	}

	if (!fout) // если клиент все еще работает
	{
		if (retVal == SOCKET_ERROR) // если не удалось получить ответ от сервера
		{
			retVal = 0; // retVal - код ошибки о завершении работы
			printf("Unable to recv\n"); // выводим сообщение о невозможности получения сообщения 
			fout = true; // завершение работы клиента
			return 0; // возвращаемое значение функции receive(0)
		}
		else // если удалось получить ответ от сервера
		{
			printf("%s\n", bufRec); // печатаем сообщение
		}
	}
	return 1; // останавливаем функцию и возвращаемся к предыдущим
}

DWORD WINAPI send(LPVOID clientSocket)
{
	int retVal = 0; // retVal - код ошибки
	char bufSend[1000]; // символьный буфер для отправки данных
	SOCKET clientSock = *((SOCKET*)clientSocket); // сокет клиента

	gets_s(bufSend); // считываем строку и сохраняем ее в буфер
	if (!strcmp(bufSend, "exit")) // если введенная строка - команда на завершение работы клиента (сообщение "exit")
	{
		fout = true; // завершение работы клиента
		retVal = send(clientSock, bufSend, 1000, 0); // отправляем сообщение серверу о прекращении работы клиента
		//clientSock – дескриптор сокета клиента
		//bufSend - имя буфера, в котором содержатся данные для отправки, размер этого буфера 1000
		//0-отсутствие списока специальных констант, с помощью которых можно запрашивать специальные опции
		return 0; // возвращаемое значение функции send(0)
	}
	else
	{
		retVal = send(clientSock, bufSend, 1000, 0); //отправить сообщение серверу
		//clientSock – дескриптор сокета клиента
		//bufSend -имя буфера, в котором содержатся данные для отправки, размер этого буфера 1000
		//0-отсутствие списока специальных констант, с помощью которых можно запрашивать специальные опции

		if (retVal == SOCKET_ERROR) // если не получилось передать данные на сервер
		{
			printf("Unable to send\n"); // выводим сообщения о невозможности передачи данных
			WSACleanup(); //  Функция прерывания Winsock 
			system("pause");  // заморозка консоли 
			return SOCKET_ERROR; // возвращаемое значение функции send(-1)
		}
	}
	return 1; // останавливаем функцию и возвращаемся к предыдущим
}
int main()
{
	struct addrinfo hints, * res; // hints - структура addrinfo с типом запрашиваемой службы
	// res - указатель, указывающий на новую структуру addrinfo с информацией, запрошенной после успешного завершения функции
	WORD ver = MAKEWORD(2, 2); // необходимая версия Winsock
	WSADATA wsaData; // структура данных, которая должна получать сведения о реализации Windows Sockets
	int retVal = 0; // retVal - код ошибки

	WSAStartup(ver, (LPWSADATA)&wsaData); // инициализация Winsock
	// преобразование текстовой строки, представляющий имена хостов или IP-адреса, 
// в динамически выделенный связанный список структур struct addrinfo
	DWORD hostEnt = getaddrinfo("localhost", NULL, &hints, &res);
	// localhost - доменное имя
	// NULL - номер порта 
	// hints - структура addrinfo с типом запрашиваемой службы
	// res - указатель, указывающий на новую структуру addrinfo с информацией, запрошенной после успешного завершения функции

	if (!hostEnt) // если не удалось преобразовать строку
	{
		printf("Unable to collect getaddrinfo\n"); // выводим сообщение о невозможности преобразовать строку
		WSACleanup(); //  Функция прерывания Winsock 
		system("pause"); // заморозка консоли 
		return 1; // останавливаем функцию и возвращаемся к предыдущим
	}

	SOCKET clientSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // создаем сокет для подключения к клиенту 
	// PF_INET-имя коммуникационного домена,используемого сокетом (принимает значение AF_INET)
	// SOCK_STREAM-тип связи, который будет использоваться в сокете (потоковая передача с установлением логического соединения)
	// IPPROTO_TCP-транспортный протокол

	if (clientSock == SOCKET_ERROR) // если не получилось создать сокет
	{
		printf("Unable to create socket\n"); // выводим сообщение о невозможности создания сокета
		WSACleanup(); //  Функция прерывания Winsock 
		system("pause");  // заморозка консоли 
		return SOCKET_ERROR; // возвращаемое значение функции main(-1)
	}

	string ip; // ip - строка, содержащая ip-адрес
	cout << "Enter ip - server>"; // выводим сообщение "Введите ip-сервера"
	cin >> ip; // ввод ip пользователем
	cin.ignore(); // функция, которая считывает символ и игнорирует его

	SOCKADDR_IN serverInfo; // структура, которая описывает сокет для работы с протоколами IP
	serverInfo.sin_family = PF_INET; // семейство адресов, принимает значение PF_INET
	inet_pton(AF_INET, ip.c_str(), (PVOID)&serverInfo.sin_addr.S_un.S_addr);
	// inet_pton-преобразует строку ip-адреса (типа AF_INET), который хранится в ip.c_str(),
	// в численный двоичный формат, в котором он хранится в структуре sin 

	serverInfo.sin_port = htons(2014); // номер порта
	// htons преобразует число из системного пордка в сетевой порядок байтов TCP/IP (который является прямым порядком байтов)

	//Пытаемся присоединится к серверу по ip и port
	retVal = connect(clientSock, (LPSOCKADDR)&serverInfo, sizeof(serverInfo));
	//clientSock – дескриптор сокета клиента
	//(LPSOCKADDR)&serverInfo – адрес структуры serverInfo, в которой задается конкретный адрес привязки  и содержащая адрес сервера
	//sizeof(serverInfo) – размер этой структуры

	if (retVal == SOCKET_ERROR) // если не получилось присоединиться к серверу
	{
		printf("Unable to connect\n"); // выводим сообщение о невозможности присоединения
		WSACleanup();  //  Функция прерывания Winsock 
		system("pause");  //  Функция прерывания Winsock 
		return SOCKET_ERROR; // возвращаемое значение функции main(-1)
	}

	printf("Connection made successfully\n"); // выводим сообщение об успешном подключении
	printf("Enter your name: "); // выводим сообщение о просьбе вести имя клиента

	char name[256]; // 256-символьный буфер для хранения имени клиента
	scanf_s("%s", name, sizeof(name)); // считываем имя клиента

	//отправить имя серверу
	retVal = send(clientSock, name, 256, 0);
	//clientSock – дескриптор сокета клиента
	//name – имя  буфера, содержащего данные, 256 - размер данного буфера
	//0-отсутствие списока специальных констант, с помощью которых можно запрашивать специальные опции

	if (retVal == SOCKET_ERROR) // если не получилось передать данные на сервер
	{
		printf("Unable to send\n"); // выводим сообщения о невозможности передачи данных
		WSACleanup(); //  Функция прерывания Winsock 
		system("pause");  // заморозка консоли 
		return SOCKET_ERROR; // возвращаемое значение функции main(-1)
	}
	printf("Enter 'exit' to finish chat\n"); // выводим сообщение о просьбе вести клиента exit, чтобы закончить работу с сервером 

	while (!fout) //пока не завершена работа клиента/сервера
	{
		DWORD threadID; // переменная, которая принимает идентификатор потока
		CreateThread(NULL, NULL, send, &clientSock, NULL, &threadID); // возвращаемое значение - дескриптор нового потока
		CreateThread(NULL, NULL, receive, &clientSock, NULL, &threadID);
		// NULL - Указатель на структуру SECURITY_ATTRIBUTES, который обуславливает, 
// что возвращенный дескриптор не может быть унаследован дочерними процессами
		// NULL - новый поток использует по умолчанию размер стека исполняемой программы
		// send, receive - указатель на адрес входной функции потока
		// &clientSock - параметр, который будет передан внутрь функции потока
		// NULL - поток запускается немедленно после создания
		// threadID - Указатель на переменную, которая принимает идентификатор потока
	}
	closesocket(clientSock); //  закрываем клиентский сокет
	WSACleanup(); //  Функция прерывания Winsock 
	return 0; // возвращаемое значение функции main(0)
}
