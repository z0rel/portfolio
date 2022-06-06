/*
 * wt_connections.h
 *
 *  Created on: 24.04.2012
 *      Author: root
 */

#ifndef WT_CONNECTIONS_H_
#define WT_CONNECTIONS_H_

#include <vector>
#include <task_interface.h>
#include <multithread_resource.h>
#include <poll.h>
#include <unistd.h>
#include <pthread.h>
using namespace std;

/**
 * Класс, реализующий паттерн WorkerThread для дескрипторов подключений
 */

class wt_connections
{
	/// максимальное число соединений
	size_t max_connection;
	/// массив дескрипторов соединений для системного вызова poll;
	struct pollfd *ufds;
	/// количество актуальных дескрипторов в массиве ufds;
	nfds_t nfds;
	/// Очередь соединений.
	multithread_resource< vector<task_interface*> > connections;
	/// Общий буфер, в который будет считываться все содержимое системного буфера.
	string recieved_buffer;
	/// временный буфер для read
	char read_buf[1024];
	/// основной поток рабочей нити.
	pthread_t thr_run;
	/// Фоновый поток рабочей нити, для уничтожения завершившихся потоков реконнекта.
	pthread_t thr_idle;
	/// Очередь потоков реконнекта
	multithread_resource< vector<pthread_t> > reconnect_queue;

	static void* idle_body(void* arg);

	static void * thread_func(void *arg)
	{ return reinterpret_cast<wt_connections*>(arg)->ThreadBody(); }

	// сформировать параметры вызова poll
	void make_ufds();
	// перед вызовом - заблокировать ресурс connections
	bool verify_connections();
	void parse_poll_result(nfds_t i);
	void *ThreadBody();

	static bool cmp_tasks(task_interface* t1, task_interface* t2)
	{	return (t1->priority < t2->priority);	}


	static void do_reconnect_op(task_interface *task)
	{ task->reconnect(); }

public:
	wt_connections(size_t MAX_CONNECTION = 512);

	string toStatistics();

	/// @retval true если все нормально
	/// @retval false если в соединении найдена ошибка
	template <typename Operation>
	static bool normal_state_or_reconnect(const pollfd &c_ufds, task_interface *task,
			Operation op)
	{
		task->sync_validate_error.lock();
		if ( task->is_reconnect )
			goto is_reconnect_label;

		if ( task->reconnect_me || task->validate_connection(c_ufds) )
		{
			op(task);
			goto is_reconnect_label;
		}

		task->sync_validate_error.unlock();
		return true;
		is_reconnect_label:
		task->sync_validate_error.unlock();
		return false;
	}

	virtual ~wt_connections();

	void push_thread_reconnect(const pthread_t &th);
	void push_connection(task_interface* connection);
};

extern wt_connections wt_connections_pool;

#endif /* WT_CONNECTIONS_H_ */
