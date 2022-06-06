#ifndef OBJ_COM_DRIVER_H_
#define OBJ_COM_DRIVER_H_

/**
 *  @file obj_com_driver.h
 *  	Заголовочный файл драйвера com-порта и tcp-ip. \n
 *  	Реализуемая функциональность:
 *  	- отслеживание конца сообщения по интервалу тишины;
 *  	- динамический буфер;
 *  	- возможность выполнения синхронизированных запросов;
 *  	- автоматическое вычисление интервала тишины;
 *  	- функции отладочного вывода
 */

/** @ingroup obj_com_driver */
/// @{

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <cstring>
#include <multithread_resource.h>

#include <netdb.h>
#include <sys/socket.h>
#include <poll.h>
#include <regex.h>
#include <driver_base.h>
#include <ping_algorithm.h>
class listner;


 // Класс-драйвер работы с потоковыми устройствами. Может работать tty-устройствами и tcp сокетами.

class tcp_driver : public driver_base
{
public:
	friend class listner;

	/// Длина опций сокета. По умолчанию - 0 - не выставляется.
	size_t len_opt;

	/// если true - то выводить в протокол после подключения <...> - connected
	/// Нужно, чтобы при подключении вывод в протокол был, а потом - нет
	bool is_conn_func;

	 // Конструктор объекта драйвера
	tcp_driver(
			timeout_handler_t timeout_fcn = timeout_fcn_def,
			int multipler = 5,
			void* data = NULL,
			const char* head = "dir - ",
			bool is_static = false,
			bool from_listner = false);

    // Деструктор
	virtual ~tcp_driver() {};

	// Инициализация драйвера, в случае использования его для обмена по tcp-ip
	int do_init	(const char* addres, const char * port,
				 tpf_parsing parsing_fcn,
				 bool start_pinger = false, bool to_protocol = true, ostream & parser_out = cout);

	// Запуск реконнекта.
	bool reconnect_base(const char* operation = NULL);
    // Запрос реконнекта, ждать время
    bool reconnect_request(__time_t tv_sec, long int tv_nsec);
    // Запрос реконнекта, ждать до бесконечности
    bool reconnect_request(const char* op = NULL);

	// Отправить данные удаленному хосту с предварительной проверкой ошибок и доступности
	// удаленного хоста
	int send_data_to_socket(string & pbuf, bool ping_device = false);

	int request_send_data_to_socket(string & pbuf, bool ping_device,
			__time_t tv_sec, long int tv_nsec, int count_retry = 1);

	string toStatistics();

protected:



	/// Флаг, характеризующий что объект является драйвером принятого слушателем подключения
	const bool is_driver_for_listner;

	/// Блокировка для монопольного доступа к сокету: Либо чтение/запись (с разных потоков),
	/// либо реконнект с одного.
	multithread_resource<void> sync_socket;

	/// Выходной аргумент getaddrinfo.
	struct addrinfo *addr_res;

	/// tcp адрес к которому нужно подключиться
	string addres;

	/// Имя порта к которому нужно подключиться
	string port;

    /// Контекст состояния соединения по результатам пинговки
	ping_algorithm_t ping;

	/// Контекст синхронизации запросов успешности подключения
	const condition_sync_t reconnect_request_obj;

	void clear_and_terminate2(bool do_thread_cancel = true);

	static void *thread_reconnect_func(void *arg)
	{ 	return reinterpret_cast<tcp_driver*>(arg)->ThreadReconnectBody();    }

	virtual void *ThreadReconnectBody();

	// Инициализация tcp подключения
	int tcp_init(ostream & parser_out = cout);

	void set_apd_options(int fd);

	virtual bool error_in_port(int fd);
	int connect2(int sd, ostream & parser_out = cout);

};


#endif /* OBJ_COM_DRIVER_H_ */



/// @}
// конец \ingroup obj_com_driver
