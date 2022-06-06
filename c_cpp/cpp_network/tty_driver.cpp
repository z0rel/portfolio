#include "tty_driver.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

tty_driver::tty_driver(
		timeout_handler_t timeout_fcn,
		int multipler,
		void* data,
		const char* head,
		bool is_static)
: driver_base(timeout_fcn, multipler, data, head, is_static)
{
	parsing_type = parsing_thread;
	com_port_name = "/dev/ttyS0";
}


void tty_driver::clear_and_terminate2(bool )
{
	port_id.lock();
	if (port_id.resource > 0)
		close(port_id.resource);
	port_id.unlock();
}

/**
 * Получение скорости com-порта
 * @param port_id
 * 		файловый дескриптор открытого терминала
 * @return
 * 		termios-константа, описывающая скорость терминала
 */
speed_t tty_driver::get_termios_speed(int port_id)
{
	struct termios options;
	tcgetattr(port_id, &options); 		  ///< Получение текущих опций для порта
	return cfgetispeed(&options);
}

/**
 * Запись в com-порт с запросом
 * @param pbuf
 * @param tv_sec
 * @param tv_nsec
 */
ssize_t tty_driver::request_write(const string & pbuf, __time_t tv_sec, long int tv_nsec)
{
	request.prev_sync(tv_sec, tv_nsec);
    ssize_t ret = write(get_port_id(), pbuf.c_str(), pbuf.size());
	request.post_sync();
    return ret;
}


/**
 * Получение длины одного байта, в битах
 * @param port_id
 * 		файловый дескриптор открытого терминала
 * @return
 * 		длина одного байта, в битах
 */
int tty_driver::get_byte_length(int port_id)
{
	struct termios options;
	tcgetattr(port_id, &options); 		  ///< Получение текущих опций для порта
	return  (((options.c_cflag & PARENB) == PARENB) ? 1 : 0) +
			(((options.c_cflag & CSTOPB) == CSTOPB) ? 2 : 1) +
			(((options.c_cflag & CS8) == CS8) ? 8 :
					(((options.c_cflag & CS7) == CS7) ? 7 :
							(((options.c_cflag & CS6) == CS6) ? 6 :
									(((options.c_cflag & CS5) == CS5) ? 5 : 10)
									)
							)
					);
}


/**
 * Получение времени, которое будет считаться интервалом тишины для заданных настроек терминала
 * @param port_id
 * 		файловый дескриптор открытого терминала
 * @param multipler
 * 		множитель, на который будет помножено вычисленное значение интервала тишины
 * 		по умолчанию равен 1
 * @return
 * 		вычисленное значение времени интервала тишины, в виде структуры timespec
 */
struct timespec tty_driver::get_port_sleeptime ( int port_id, int multipler )
{
	return speed_to_timeout ( get_termios_speed(port_id),
			get_byte_length(port_id) , multipler );
}

/**
 * Преобразование скорости терминала в время, которое будет считаться интервалом тишины
 * @param speed
 *  	termios-константа, описывающая скорость терминала
 * @param byte_length
 * 		длина одного байта на физическом уровне (от 5+1 до 8+1+2)
 * 		по умолчанию равна 9
 * @param multipler
 * 		множитель, на который будет помножено вычисленное значение интервала тишины
 * 		по умолчанию равен 1
 * @return
 * 		вычисленное значение времени интервала тишины, в виде структуры timespec
 */

struct timespec tty_driver::speed_to_timeout ( speed_t speed, int byte_length, int multipler )
{
	struct timespec result;
	result.tv_nsec = 0;
	result.tv_sec  = 1;
	if ( const_to_speed(speed) )
	{
		double res =  8.0 * (double)byte_length * ( 1.0 / (double)const_to_speed( speed ) ) *
				(double) multipler * 1000000000.0;
		result.tv_sec = (__time_t)(res / 1000000000.0);
		result.tv_nsec = (long int)(res - (double)result.tv_sec * 1000000000.0);
	}
	return result;
}
/**
 * Преобразование posix константы, определяющей скорость терминала в фактическое значение
 * @param speed
 * 		Posix-константа определяющая скорость терминала.
 * 		Все возможные константы описаны в <bits/termios.h>
 * @return
 * 		Фактическое значение скорости терминала (в бодах)
 */
int tty_driver::const_to_speed ( speed_t speed )
{
	switch (speed)
	{
	case B0:  			return 0;
	case B50:  			return 50;
	case B75:  			return 75;
	case B110:  		return 110;
	case B134:  		return 134;
	case B150:  		return 150;
	case B200:  		return 200;
	case B300:  		return 300;
	case B600:  		return 600;
	case B1200:  		return 1200;
	case B1800:  		return 1800;
	case B2400:  		return 2400;
	case B4800:  		return 4800;
	case B9600:  		return 9600;
	case B19200:  		return 19200;
	case B38400:  		return 38400;
	case B57600:  		return 57600;
	case B115200:  		return 115200;
	case B230400:  		return 230400;
	case B460800:  		return 460800;
	case B500000:  		return 500000;
	case B576000:  		return 576000;
	case B921600:  		return 921600;
	case B1000000:  	return 1000000;
	case B1152000:  	return 1152000;
	case B1500000:  	return 1500000;
	case B2000000:  	return 2000000;
	case B2500000:  	return 2500000;
	case B3000000:  	return 3000000;
	case B3500000:  	return 3500000;
	case B4000000:  	return 4000000;
	}
	return -1;
}


/// инициализация из опций командной строки
void tty_driver::init_args(int argc, char *argv[], const char* config_file)
{
	int c;

	while( -1 != ( c = getopt( argc, argv, "hp:" ) ) )
		switch( c )
		{
		case 'p':
			com_port_name = optarg;
			break;
		case 'h':
		default :
			printf(
			"option:\n"
			" -p - com-port name\n"
			" -h — this help\n");
			exit( 'h' == c ? EXIT_SUCCESS : EXIT_FAILURE );
		}
	if (config_file)
	{
		FILE * f = fopen(config_file, "r");
		if (f)
		{
			char buf1[512], buf2[512];
			fgets(buf1, sizeof(buf1), f);
			sscanf(buf1, "%s", buf2);
			com_port_name.assign(buf2);
			fclose(f);
		}
	}
}

/**
 * Считать из cin. Сделано для того, чтобы уменьшить шанс пустого чтения, т.к.
 * функция сначала очищает cin и входной буфер
 * @param inp_buf
 * 		Указатель на входной буфер типа string
 * @return
 * 		Успех или неудача
 * @retval EXIT_SUCCESS
 * 		Данные успешно считаны из cin
 * @retval EXIT_FAILURE
 * 		Произошла ошибка чтения и считана пустая строка
 */
int tty_driver::init_stream_buffers(string *inp_buf)
{
	inp_buf->clear();
	cin.clear();
	cin >> *inp_buf;
	if (cin.failbit && !(*inp_buf)[0])
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}


/**
 * Инициализация драйвера, в случае использования его для обмена по tty интерфейсу
 * (т.е. по com-порту)
 * @param PortName
 * 		Имя порта - обычно ttyS0, ttyS1, ttyUSB0
 * @param set_RS232_flags
 * 		Указатель на функцию обратного вызова, которая устанавливает необходимые
 * 		настройки com-порта
 * @param parsing_fcn
 * 		Указатель на функцию обратного вызова, которая будет парсить принятые сообщения
 * @param timeout_fcn
 * 		Указатель на функцию обратного вызова, которая будет обрабатывать событие таймаута,
 * 		которое может возникнуть при установке режима "запрос"
 * @param signum
 * 		Номер сигнала, на который будет повешено событие "конец сообщения".
 * 		Это событие происходит по истечению времени "интервала тишины", за который
 * 		не произошло нового приема данных.
 * 		Данное событие и вызывает функцию парсинга parsing_fcn.
 * 		Типичные значения параметра - SIGUSR1, SIGUSR2 и т.д.
 * @return
 * 		успех или неудача
 * 		@retval EXIT_SUCCESS
 * 			успех
 * 		@retval EXIT_FAILURE
 * 			неудача, открыть com-порт не удалось или не удалось создать поток-читатель
 */
int tty_driver::do_init(t_set_RS232_flags* set_RS232_flags, tpf_parsing parsing_fcn, bool console)
{
	allready_cleared = false;
    if (console)
    {
        port_id = STDIN_FILENO;
    }
    else
    {
        port_id = open_port(com_port_name.c_str(), set_RS232_flags);
        if (port_id == -1)
        {
            if_printf("%serror: open port %s\n", header.c_str(), com_port_name.c_str());
            return EXIT_FAILURE;
        }
    }
	task_worked = true;
	parser_context.pf_parsing = parsing_fcn;

	string str = header;
	str += "error: create thread reader in do_init\n";

	if (start_reader(str.c_str()) == EXIT_FAILURE)
		return EXIT_FAILURE;

	/// Инициализация времени интервала тишины для com-порта
	timeval_for_slience_timer.it_value =
			get_port_sleeptime (port_id, slience_time_multipler );
	timeval_for_slience_timer.it_interval.tv_nsec = 0;
	timeval_for_slience_timer.it_interval.tv_sec = 0;

	return EXIT_SUCCESS;
}

/**
 * Функция, осуществляющая открытие com-порта
 * @param PortName
 * 		Имя порта
 * @param set_RS232_flags
 * 		указатель на функцию обратного вызова, которая устанавливает необходимые
 * 		настройки com-порта
 * @return
 *		Успех или неудача
 * 		@retval != -1
 * 			Дескриптор открытого com-порта
 * 		@retval -1
 * 			Неудача, открыть com-порт не удалось
 */
int tty_driver::open_port(const char* PortName, t_set_RS232_flags* set_RS232_flags)
{
  int32_t fd; ///< Файловый дескриптор для порта
  struct termios options;
  fd = open(PortName, O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd == -1)
	  if_printf("%serror open port %s\n", header.c_str(), PortName);
  else
  {
	  	fcntl(fd, F_SETFL, 0);
		tcgetattr(fd, &options); 		  ///< Получение текущих опций для порта
		set_RS232_flags(&options);
		tcsetattr(fd, TCSANOW, &options); ///< Установка новых опций для порта
  }
  return (fd);
}


