#ifndef TTY_DRIVER_H_
#define TTY_DRIVER_H_


#include <driver_base.h>
#include <termios.h>
#include <fcntl.h>

class tty_driver : public driver_base
{
public:
	typedef void (t_set_RS232_flags)(termios *options);

	// Конструктор объекта драйвера
	tty_driver(
			timeout_handler_t timeout_fcn,
			int multipler = 5,
			void* data = NULL,
			const char* head = "dir - ",
			bool is_static = false);

	// Инициализация из опций командной строки
	void init_args(int argc, char *argv[], const char* config_file = NULL);

    ssize_t request_write(const string & pbuf, __time_t tv_sec, long int tv_nsec = 0);

	// Инициализация драйвера, в случае использования его для обмена по tty интерфейсу
    int     do_init (t_set_RS232_flags* set_RS232_flags, tpf_parsing parsing_fcn, bool console = false);

	// Обертка для корректной инициализации строки из stdin в многопоточном окружении
	static int  init_stream_buffers(string *inp_buf);

	string com_port_name;
protected:
	void clear_and_terminate2(bool do_thread_cancel = true);

	// функция для открытия порта. вызываетя всеми do_init*
	int open_port (const char* PortName, t_set_RS232_flags* set_RS232_flags);

	// Получение скорости com-порта
	speed_t get_termios_speed(int port_id);

	// Получение длины одного байта, в битах
	int get_byte_length(int port_id);

	// Получение времени, которое будет считаться интервалом тишины для заданных настроек терминала
	struct timespec get_port_sleeptime ( int port_id, int multipler = 1 );

	// Преобразование скорости терминала в время, которое будет считаться интервалом тишины
	struct timespec speed_to_timeout ( speed_t speed, int byte_length, int multipler );

	// Преобразование posix константы, определяющей скорость терминала в фактическое значение
	int const_to_speed ( speed_t speed );
};

#endif /* TTY_DRIVER_H_ */
