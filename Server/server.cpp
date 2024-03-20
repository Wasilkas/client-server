#pragma comment(lib, "Ws2_32.lib") // подключение внешней библиотеки Winsock
#include <WinSock2.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <ws2tcpip.h>

using namespace std; // подключение пространства имен для string
const int MaxNumUsers = 10; // максимальное число пользователей на сервере
int NumUsers = 0; // текущее число пользователей на сервере

// информация о пользователях
SOCKET clientsSockets[MaxNumUsers]; // массив клиентских сокетов
SOCKADDR_IN clientsSADDR[MaxNumUsers]; // массив структур, описывающих соответствующие сокеты клиентов для работы с протоколами IP
USHORT ports[MaxNumUsers]; // массив номеров портов сокетов клиентов
char names[MaxNumUsers + 1][256]; // массив имен клиентов

DWORD WINAPI chat(LPVOID clientSocket)
{
	int retVal= 0; // код ошибки
	char bufRec[1000]; // буфер для приема данных
	SOCKET clientSock = *((SOCKET*)clientSocket);
	char bufSend[1000]; // буфер для отправки данных

	while (true) // пока клиент не покинет чат
	{
		// получить сообщение от клиента
		bufRec[0] = '\0'; // обнуляем буфер для приема данных
		retVal = recv(clientSock, bufRec, 1000, 0);
		// clientSock-дескриптор сокета, из которого читаются данные, buf-имя буфера для приема данных, 1000-размер этого буфера
		// 0-отсутствие списока специальных констант, с помощью которых можно запрашивать специальные опции

		if (retVal == SOCKET_ERROR) // если не получилось принять данные от клиента
		{
			printf("Unable to recv\n"); // выводим сообщение о невозможности принять данные
			closesocket(clientSock); // закрываем сокет клиента
			printf("Connection closed\n"); // выводим сообщение о прекращении соединения
			return SOCKET_ERROR; // возвращаемое значение функции chat(-1)
		}
		printf("Data received\n"); // выводим сообщение о получении данных
		int CurNum = 0; // номер текущего клиента
		SOCKADDR_IN sin; // структура, которая описывает сокет для работы с протоколами IP
		for (int i = 0; i < NumUsers; i++) // идем по массиву сокетов клиентов
		{
			if (clientsSockets[i] == clientSock) // если нашли в массиве сокет текущего клиента
			{
				sin = clientsSADDR[i]; // запоминаем его структуру, которая описывает его сокет для работы с протоколами IP
				CurNum = i; // запоминаем номер текущего клиента
			}
		}

		if (!strcmp(bufRec, "exit")) // если пользователь ввел команду для выхода (сообщение "exit")
		{
			// оповестить всех др пользователей о его выходе
			bufSend[0] = '\0'; // обнуляем буфер для отправки данных
			strcat_s(bufSend, names[CurNum]); // добавляем в буфер имя пользователя, который хочет выйти
			strcat_s(bufSend, " left the chat");  // добавляем в буфер строку " left the chat"
			for (int i = 0; i < NumUsers; i++) // оповещаем всех других пользователей
			{
				if (clientsSockets[i] != clientSock) // кроме текущего (который хочет выйти)
				{
					retVal = send(clientsSockets[i], bufSend, 1000, 0);
					// clientsSockets[i]–дескриптор сокета, через который проводится передача; 
// bufSend–имя буфера, содержащего данные о пользователе, 
					// который выходит из чата, размер буфера - 1000 
					// 0-отсутствие списока специальных констант, с помощью которых можно запрашивать специальные опции
				}	
			}
			closesocket(clientSock); // Закрываем клиентский сокет
			printf("Client disconnected\n"); // вывод сообщения о выходе клиента

			// удаляем информацию о пользователе
			for (int j = CurNum; j < NumUsers; j++)
			{
				clientsSockets[j] = clientsSockets[j + 1];
				clientsSADDR[j] = clientsSADDR[j + 1];
				ports[j] = ports[j + 1];
				strcpy_s(names[j], names[j + 1]);
			}
			clientsSockets[NumUsers - 1] = SOCKET_ERROR; // обнуляем сокет
			NumUsers--; // количество пользователей уменьшается на единицу
			printf("Current amount of clients: %i\n", NumUsers); // выводим текущее количество пользователей
			return SOCKET_ERROR; // возвращаемое значение функции chat(-1)
		}
		
		if (bufRec[0] != '\0') //если принятое сообщение не пустая строка
		{
			printf("%s: %s\n", names[CurNum], bufRec); // выводим на экран имя пользователя и его сообщение
			bufSend[0] = '\0'; // обнуляем буфер для отправки данных
			strcat_s(bufSend, names[CurNum]); // добавляем в буфер имя пользователя
			strcat_s(bufSend, ": ");// добавляем в буфер строку ": "
			strcat_s(bufSend, bufRec); // добавляем в буфер сообщение пользователя
			printf("Sending response from server\n"); // выводим сообщение об отправки ответа с сервера
			
			for (int i = 0; i < NumUsers; i++) // отсылаем всем пользователем кроме него самого его имя и сообщение
			{
				if (clientsSockets[i] != clientSock)
					retVal = send(clientsSockets[i], bufSend, 1000, 0);
				// clientsSockets[i]–дескриптор сокета, через который проводится передача; 
// bufSend–имя буфера, содержащего сообщение пользователя, 
				// который выходит из чата, размер буфера - 1000 
				// 0-отсутствие списока специальных констант, с помощью которых можно запрашивать специальные опции
				if (retVal == SOCKET_ERROR) // если не получилось отправить сообщение клиентy
				{
					printf("Unable to send\n"); // вывод сообщения о невозможности отправки
					return SOCKET_ERROR; // возвращаемое значение функции chat(-1)
				}
			}
		}
	}
}

int main()
{
	WORD sockVer = MAKEWORD(2, 2); // необходимая версия Winsock 
	WSADATA wsaData; // структура данных, которая должна получать сведения о реализации Windows Sockets.
	int retVal = 0; // код ошибки
	WSAStartup(sockVer, &wsaData); // инициализация Winsock

	// создаем сокет для подключения к серверу
	SOCKET servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	// PF_INET-имя коммуникационного домена,используемого сокетом (принимает значение AF_INET)
	// SOCK_STREAM-тип связи, который будет использоваться в сокете (потоковая передача с установлением логического соединения)
	// IPPROTO_TCP-транспортный протокол

	if (servSock == INVALID_SOCKET) // если не получилось создать сокет
	{
		printf("Unable to create socket\n"); // выводим сообщение о невозможности создания сокета
		WSACleanup(); // функция прерывания Winsock
		system("pause"); // Заморозка консоли
		return SOCKET_ERROR; // возвращаемое значение функции main(-1)
	}

	// обнуляем массив сокетов клиентов
	for (int i = 0; i < MaxNumUsers; i++)
	{
		clientsSockets[i] = SOCKET_ERROR;
	}

	SOCKADDR_IN sin; // структура, которая описывает сокет для работы с протоколами IP
	sin.sin_family = PF_INET; // семейство адресов, принимает значение PF_INET
	sin.sin_port = htons(2014); // номер порта 
	// htons преобразует число из системного пордка в сетевой порядок байтов TCP/IP (который является прямым порядком байтов)
	sin.sin_addr.s_addr = INADDR_ANY; // 32х битное значение IP адреса, к которому будет привязан сокет
	
	// Привязка сокета к конкретному адресу, который лежит в структуре sin
	retVal = bind(servSock, (LPSOCKADDR)&sin, sizeof(sin)); // servSock-дескриптор сокета,
	// (LPSOCKADDR)&sin–адрес структуры sin, в которой задается конкретный адрес привязки, sizeof(sin)–размер структуры sin. 

	if (retVal == SOCKET_ERROR) // если не получилось привязать сокет к конкретному адресу, который лежит в структуре sin
	{
		printf("Unable to bind\n"); // выводим сообщение о невозможности привязки
		WSACleanup(); // функция прерывания Winsock
		system("pause"); // Заморозка консоли
		return SOCKET_ERROR; // возвращаемое значение функции main(-1)
	}
	printf_s("Server started\n"); // выводим сообщение о запуске сервера 

	while (true) // пока не отключат сервер
	{
		// пытаемся начать слушать сокет
		retVal = listen(servSock, 10); // servSock-дескриптор сокета, максимальный размер очереди запросов на соединение = 10
		if (retVal == SOCKET_ERROR) // если не получается начать слушать сокет
		{
			printf("Unable to listen\n"); // выводим сообщение о невозможности прослушки сокета
			WSACleanup(); // функция прерывания Winsock
			system("pause"); // Заморозка консоли
			return SOCKET_ERROR; // возвращаемое значение функции main(-1)
		}

		// ждем клиента
		SOCKET clientSock; // clientSock-клиентский сокет
		SOCKADDR_IN from; // структура, которая описывает сокет для работы с протоколами IP
		int fromlen = sizeof(from); // fromlen-размер структуры from
		
		// принимаем клиентский сокет
		clientSock = accept(servSock, (struct sockaddr*) & from, &fromlen);
		// servSock-дескриптор слушающего сокета, (struct sockaddr*)&from–структура, содержащая адресную информацию о клиенте
		// &fromlen–размер структуры from. 

		if (clientSock == INVALID_SOCKET) // если не получилось принять сокет
		{
			printf("Unable to accept\n"); // выводим сообщение о невозможности принять сокет
			WSACleanup(); // функция прерывания Winsock
			system("pause"); // Заморозка консоли
			return SOCKET_ERROR; // возвращаемое значение функции main(-1)
		}

		char ip[16]; // символьный буфер для хранения ip-адреса
		inet_ntop(AF_INET, &from.sin_addr.S_un.S_addr, ip, 16);
		// inet_ntop-преобразует структуру ip-адреса, который хранится в структуре from, в строку символов с ip-адресом (типа AF_INET), 
		// которая затем копируется в символьный буфер ip; размер этого буфера = 16
		
		printf("New connection accepted from %s, port %d\n", ip, htons(from.sin_port));
		// выводим сообщение об установлении соединения сервера с устройством с ip-адресом ip, с номером порта htons(from.sin_port)
		
		printf("Current amount of clients: %i\n", NumUsers + 1); // выводим сообщение о текущем количестве клиентов

		// получаем имя нового клиента
		retVal = recv(clientSock, names[NumUsers], 256, 0);
		// clientSock-дескриптор сокета, из которого читаются данные, names[NumUsers]-имя буфера для приема данных, 256-размер этого буфера
		// 0-отсутствие списока специальных констант, с помощью которых можно запрашивать специальные опции

		if (retVal == SOCKET_ERROR) // если не получилось принять данные от клиента
		{
			printf("Unable to recv\n"); // выводим сообщение о невозможности принять данные
			system("pause"); // Заморозка консоли
			return SOCKET_ERROR; // возвращаемое значение функции main(-1)
		}

		// если имя команда на выключение сервера
		if (!strcmp(names[NumUsers], "shutdown"))
		{
			string res = "Server shutdown"; // результат = выключение сервера
			// посылаем сообщение о закрытии всем клиентам и закрываем сокеты
			for (int i = 0; i < NumUsers; i++)
			{
				retVal = send(clientsSockets[i], res.c_str(), 256, 0);
				// clientsSockets[i]–дескриптор сокета, через который проводится передача; 
// res.c_str()–имя буфера, содержащего данные, размера 256 
			   	// 0-отсутствие списока специальных констант, с помощью которых можно запрашивать специальные опции
				closesocket(clientsSockets[i]); // Закрываем клиентский сокет
			}
			retVal = send(clientSock, res.c_str(), 256, 0); // отправляем сообщение текущему клиенту, который хочет присоединится
			closesocket(clientSock); // Закрываем сокет текущего клиента
			break; // прекращение работы сервера
		}
		else
		{
			if (NumUsers < MaxNumUsers) // если номер текущего клиента меньше максмума пользователей
			{
				char NewClient[1000];  // символьный буфер для хранения данных нового клиента
				// сохраняем и выводим информацию о текущем пользователе
				ports[NumUsers] = from.sin_port; // добавляем в массив новый портов номер порта нового клиента
				clientsSockets[NumUsers] = clientSock; // добавляем в массив клиентских сокетов сокет нового клиента
				clientsSADDR[NumUsers] = from; // добавляем в массив структур новую структуру, 
// описывающую сокет нового клиента для работы с протоколами IP 
				NewClient[0] = '\0'; // обнуляем массив для хранения данных нового клиента
				strcat_s(NewClient, "New client, ip: "); // добавляем в буфер строку "New client, ip: "
				inet_ntop(AF_INET, &from.sin_addr, ip, 16); 
				// inet_ntop-преобразует структуру ip-адреса, который хранится в структуре from,
// в строку символов с ip-адресом (типа AF_INET), 
				// которая затем копируется в символьный буфер ip; размер этого буфера = 16
				strcat_s(NewClient, ip); // добавляем в буфер IP нового клиента
				strcat_s(NewClient, "; Name: "); // добавляем в буфер строку "; Name: 
				strcat_s(NewClient, names[NumUsers]); // добавляем в буфер имя нового клиента
				printf_s("%s\n", NewClient); // выводим на экран сообщение о новом клиенте
				NumUsers++; // количество пользователей увеличивается на единицу

				// рассылаем всем клиентам собщение о новом пользователе, кроме него самого
				for (int i = 0; i < NumUsers; i++)
				{
					if (clientsSockets[i] != clientSock) // если сокет не является сокетом нового клиента
						retVal = send(clientsSockets[i], NewClient, 1000, 0);
					// clientSock–дескриптор сокета, через который проводится передача; 
// NewClient–данные о новом пользователе, размера 1000
					// 0-отсутствие списока специальных констант, с помощью которых можно запрашивать специальные опции
				}
			}
			else // если номер текущего клиента больше максмума пользователей
			{
				printf("Maximum amount of clients\n"); // выводим на экран сообщение о достижении максимум пользователей
				retVal = send(clientSock, "Sorry, too much people on the line", 256, 0); 
// отправляем пользователю сообщение о достижении максимума пользователей
				closesocket(clientSock); // Закрываем клиентский сокет
			}
			// чат
			DWORD threadID; // переменная, которая принимает идентификатор потока
			CreateThread(NULL, NULL, chat, &clientSock, NULL, &threadID); //  возвращаемое значение - дескриптор нового потока
			// NULL - Указатель на структуру SECURITY_ATTRIBUTES, который обуславливает, 
// что возвращенный дескриптор не может быть унаследован дочерними процессами
			// NULL - новый поток использует по умолчанию размер стека исполняемой программы
			// chat - указатель на адрес входной функции потока
			// &clientSock - параметр, который будет передан внутрь функции потока
			// NULL - поток запускается немедленно после создания
			// threadID - Указатель на переменную, которая принимает идентификатор потока
		}
	}
	closesocket(servSock); // Закрываем сокет сервера
	WSACleanup(); // функция прерывания Winsock
	return 0; // возвращаемое значение функции main(0)
}
