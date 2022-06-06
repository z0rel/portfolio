/**
 *  @file tcp_driver.cpp
 *  	Файл драйвера com-порта и tcp-ip.
 */

#include <tcp_driver.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <sstream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sigrtmanager.h>
#include <fcntl.h>
#include <limits.h>
#include <wt_connections.h>
/**
 * Конструктор объекта драйвера
 * @param max_read_bytes
 * 		Максимальный размер части сообщеня, которая может быть считана за 1 вызов
 * 		read (для com-порта), либо recv (для tcp-ip)
 * @param multipler
 *		Множитель, на который будет умножаться вычисленное время
 *		интервала тишины для com-порта,	либо множитель константы,
 *		задающей время интервала тишины для tcp-ip
 * @param data
 *		Контекст внешнего кода, либо какие-то другие дополнительные данные
 */
tcp_driver::tcp_driver(
		timeout_handler_t timeout_fcn,
		int multipler,
		void* data,
		const char* head,
		bool is_static,
		bool from_listner)
: driver_base(timeout_fcn, multipler, data, head, is_static),
  is_driver_for_listner(from_listner),
  ping(this, 1, 0),
  reconnect_request_obj(this, NULL)
{
	is_conn_func = false;
	addr_res = NULL;
	len_opt = 0;
}


string tcp_driver::toStatistics()
{
	stringstream str;
	str << reinterpret_cast<driver_base*>(this)->toStatistics() << endl
			<< "is_conn_func = " << is_conn_func << endl
			<< "is_driver_for_listner = " << is_driver_for_listner << endl
			<< "addr_res = " << addr_res << endl
			<< "addres = " << addres << endl
			<< "port = " << port << endl
			<< "ping = " << ping.toStatistics() << endl;
	return str.str();
}

bool tcp_driver::error_in_port(int fd)
{
	char errbuf[256];
	int err;
	socklen_t len = sizeof(err);

	// После того, как poll сообщил о возможности операций с сокетом,
	// используется getsockopt(2), чтобы прочитать флаг SO_ERROR на уровне SOL_SOCKET
	// и определить,
	// -> в случае записи: успешно ли завершился connect
	// -> в случае чтения: нет ли других ошибок на сокете?
	// Если все нормально, то err = SO_ERROR = 0.
	// Если есть ошибка,   то err = SO_ERROR равен одному из обычных кодов ошибок

	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) == -1 || err) // ?  Какие ошибки можно получить с уровня TCP
	{
		errno = (err) ? err : errno;
		strerror_r(errno, errbuf, 256);
		if_printf("%serror getsockopt: %s\n", header.c_str(), errbuf);
		return true;
	}
	set_apd_options(fd);
	return false;
}

void tcp_driver::set_apd_options(int fd)
{
	if (len_opt > 0)
	{
		uint8_t opt_buf[] = {0xFC, 0x08, 0xFF, 0x33, 0xCC, 0xFF, 0xAA, 0x00};
		setsockopt(fd, SOL_IP, IP_OPTIONS, opt_buf, len_opt);
	}
}

/**
 * Инициализация tcp подключения
 * @return
 * 		Успех или неудача
 * 		@retval EXIT_SUCCESS
 * 			Успех
 * 		@retval EXIT_FAILURE
 * 			Неудача, открыть com-порт не удалось или не удалось создать поток-читатель
 * 		@retval 2
 * 			Неудача, создать сокет неудалось
 * 		@retval 3
 * 			Неудача, неверный адрес, порт, или произошла ошибка получения адреса
 * 		@retval 4
 * 			Неудача, не удалось сделать сокет неблокирующим.
 * 		@retval 5
 * 			Неудача, не удалось подключиться к заданному адресу
 */
int tcp_driver::tcp_init(ostream & parser_out)
{
	int res;
	int sd;
	char errbuf[256];
    memset(errbuf, 0, sizeof (errbuf));

    if ((sd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		strerror_r(errno, errbuf, sizeof(errbuf));
		if_printf("%serror create socket: %s", header.c_str(), errbuf);
		return (2);
	}
    set_apd_options(sd);
    if ((res = getaddrinfo (addres.c_str(), port.c_str(), NULL, &addr_res)) != 0)
	{
    	if_printf("%s - error getaddrinfo: %s %s %s___\n", header.c_str(), gai_strerror (res), addres.c_str(), port.c_str());
		close(sd);
		return (3);
	}
    int flags = fcntl(sd, F_GETFL, 0);
    if((flags != -1) && (fcntl(sd, F_SETFL, flags | O_NONBLOCK) == -1))
	{
    	strerror_r(errno, errbuf, sizeof(errbuf));
    	if_printf("%serror fcntl: %s", header.c_str(), errbuf);
		close(sd);
		freeaddrinfo(addr_res);
		addr_res = NULL;
		return (4);
	}
    return connect2(sd, parser_out);
}

int tcp_driver::connect2(int sd, ostream & parser_out)
{
	int res = connect(sd, addr_res->ai_addr, addr_res->ai_addrlen);
	set_apd_options(sd);
    bool fl_conn_err = true;
    if (res == -1 && errno == EINPROGRESS)
    	fl_conn_err = poll_to_write(sd);
	else if (!res)
		fl_conn_err = false;
    if(fl_conn_err)
    {
        close(sd);
        freeaddrinfo(addr_res);
        addr_res = NULL;
        return 5;
    }
    port_id = sd;
    task_worked = true;
    if (is_conn_func)
    	parser_out << header << "connected" << endl;
    ping.connect_state = true;
    return EXIT_SUCCESS;
}


/**
 *	Запуск реконнекта.
 * @param operation
 * 		Операция, в которой происходит реконнект - нужно для отладки
 * @return
 * 		Происходит реконнект или нет
 * 		@retval true   В данный момент происходит реконнект
 * 		@retval false  Реконнекта не происходило, но он был запущен
 */
bool tcp_driver::reconnect_base(const char* operation)
{
	if (is_reconnect)
		return true;
	is_reconnect = true;
	ping.connect_state = false;
	if (is_driver_for_listner)
	{
		task_worked = false;
		return true;
	}
	if_printf("%sdisconnect in %s\nreconnecting...\n", header.c_str(), operation);
	pthread_t thr;
	if (pthread_create(&thr, NULL, thread_reconnect_func, (void*)this))
	{
		task_worked = false;
		exit(EXIT_FAILURE);
	}
	wt_connections_pool.push_thread_reconnect(thr);

	return false;
}


void *tcp_driver::ThreadReconnectBody()
{
	sync_socket.lock();
	clear_and_terminate(false);

	while (tcp_init() != EXIT_SUCCESS)
		usleep(300*1000);
	if_printf("%sreconnecting succees\n", header.c_str());
	sync_socket.unlock();
	is_reconnect = false;
	task_worked = true;
	wt_connections_pool.push_connection(this);
	reconnect_request_obj.accept();
	return 0;
}



int tcp_driver::request_send_data_to_socket(string & pbuf, bool ping_device,
			__time_t tv_sec, long int tv_nsec, int count_retry)
{
	request.prev_sync(tv_sec, tv_nsec);
	int retval = 0;

	for (int i = 0; i < count_retry; i++)
		retval |= send_data_to_socket(pbuf, ping_device);
	if (!retval)
		request.post_sync();
	else
		request.cancel();
	return retval;
}

int tcp_driver::send_data_to_socket(string & pbuf, bool ping_device)
{
	if (!task_worked)
	{
		if (is_driver_for_listner)
			return 1;
		if (PING_DEV && ping_device)
			if (!ping(1,3, 1,5))
				goto error_finally;

		if (do_init(addres.c_str(), port.c_str(),
				parser_context.pf_parsing,
				signum_timer, false) != EXIT_SUCCESS
		)
			goto error_finally;
	}

	if (PING_DEV && ping_device)
	{
//		cout << "                          state = " << ping.state() << endl;
		if (!ping.state())
			goto error_finally;
		if (!ping(1,3, 1,5))
			goto error_finally;
	}

	if (sync_socket.timedlock(1))
		goto error_finally;

	sync_socket.unlock();

	if (poll_to_write(port_id.resource))
		goto error_finally;

	while (write(port_id, pbuf.c_str(), pbuf.length()) < 0)
	{
		if (errno == EPIPE) // читающий конец сокета закрылся с той стороны.
		{
			if (is_driver_for_listner)
				return 1;
			reconnect_request(1, 0);
			if (is_reconnect)
				goto error_finally;
		}
		else
		{
			if_printf("%serror sending\n", header.c_str());
			return 1;
		}
	}
	return 0;
error_finally:
	parser_context.timeout_handler(this);
	return 1;
}


bool tcp_driver::reconnect_request(__time_t tv_sec, long int tv_nsec)
{
	reconnect_request_obj.prev_sync(tv_sec, tv_nsec);
	reconnect("reconnect_request");
	reconnect_request_obj.post_sync();
	return reconnect_request_obj.timeout();
}

bool tcp_driver::reconnect_request(const char* op)
{
	reconnect_request_obj.prev_sync(0, 0);
	reconnect(op);
	reconnect_request_obj.post_sync_inf();
	return reconnect_request_obj.timeout();
}

/**
 * Инициализация драйвера, в случае использования его для обмена по tcp-ip
 * @param addres
 * 		Адрес, к которому нужно подключиться
 * @param port
 * 		Порт, к которому нужно подключиться
 * @param parsing_fcn
 *		Указатель на функцию обратного вызова, которая будет парсить принятые сообщения
 * @param timeout_fcn
 * 		Указатель на функцию обратного вызова, которая будет обрабатывать событие таймаута,
 * 		которое может возникнуть при установке режима "запрос"
 * @param signum
 * 		Номер сигнала, на который будет повешено событие "конец сообщения".
 * 		Это событие происходит по истечению времени "интервала тишины", за который
 * 		не произошло нового приема данных.
 * 		Данное событие и вызывает функцию парсинга parsing_fcn.
 * 		Типичные значения параметра - SIGUSR1, SIGUSR2 и т.д.
 * @param start_pinger true => запустить поток сканирующий наличие удаленного хоста в живых
 *                     false => не делать этого
 * @return
 * 		Успех или неудача
 *		\see tcp_driver::tcp_reinit
 */
int tcp_driver::do_init (const char* addres, const char * port,
				tpf_parsing parsing_fcn,
				bool start_pinger, bool to_protocol, ostream & parser_out)
{
	allready_cleared = false;
	this->addres.assign(addres);
	this->port.assign(port);

	ping.addres(this->addres);

	parser_context.pf_parsing = parsing_fcn;

	int latency = 30000; // 30 мкс.
	///Инициализация времени интервала тишины для ethernet
	make_time(&timeval_for_slience_timer, 0, latency);

	is_conn_func = true && to_protocol;

	if (PING_DEV)
		if (!ping(1,3, 1,5))
		{
			if_printf("%serror: remote host unreachable\n", header.c_str());
			return EXIT_FAILURE;
		}
	int res = tcp_init(parser_out);
	is_conn_func = false;
	if (res != EXIT_SUCCESS)
		return res;

	string str = header;
	str += "error:  create thread reader in do_tcp_init\n";

	if (start_reader(str) == EXIT_FAILURE)
		return EXIT_FAILURE;

	if (start_pinger)
		if (ping.start() != EXIT_SUCCESS)
		{
			if_printf("%serror: create thread pinger\n", header.c_str());
			return EXIT_FAILURE;
		}
	return EXIT_SUCCESS;
}


void tcp_driver::clear_and_terminate2(bool )
{
	port_id.lock();
	if (port_id.resource > 0)
		close(port_id.resource);
	port_id.unlock();

	if (addr_res != NULL)
	{
		freeaddrinfo(addr_res);
		addr_res = NULL;
	}
}


