/*
 * wt_connections.cpp
 *
 *  Created on: 24.04.2012
 *      Author: root
 */

#include "wt_connections.h"
#include <algorithm>
#include <functional>
#include <sched.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
using namespace std;

wt_connections wt_connections_pool;

wt_connections::wt_connections(size_t MAX_CONNECTION)
{
	ios_base::sync_with_stdio(true);
	connections.lock();
	connections.resource.reserve(64);
	connections.resource.clear();
	connections.unlock();
	max_connection = MAX_CONNECTION;
	ufds = new pollfd[max_connection];
	nfds = 0;
	recieved_buffer.reserve(4096);
	reconnect_queue.resource.reserve(64);

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	struct sched_param sp;
	sp.__sched_priority = sched_get_priority_min(SCHED_RR) + 5;
	pthread_attr_setschedparam(&attr, &sp);
	pthread_create(&thr_run, &attr, thread_func, (void*)this);

	pthread_attr_destroy(&attr);


	pthread_create(&thr_idle, NULL, idle_body, (void*)this);
}

string wt_connections::toStatistics()
{
	stringstream str;
	str << "max_connection = " << max_connection << endl
			<< "nfds = " << nfds << endl
			<< "------- connections ----------" << endl;
	for (vector<task_interface*>::iterator it = connections.resource.begin();
			it != connections.resource.end(); ++it)
		str << (*it)->toStatistics() << endl << endl;
	str << "reconnect_queue size = " << reconnect_queue.resource.size() << endl;
	for (vector<pthread_t>::iterator it = reconnect_queue.resource.begin();
			it != reconnect_queue.resource.end(); ++it)
		str << (*it) << " ";
	str << endl;
	return str.str();
}


wt_connections::~wt_connections()
{
	pthread_cancel(thr_run);
	void* retval;
	pthread_join(thr_run, &retval);
	pthread_cancel(thr_idle);
	pthread_join(thr_idle, &retval);

	delete[] ufds;
}


void wt_connections::push_thread_reconnect(const pthread_t &th)
{
	reconnect_queue.lock();
	reconnect_queue.resource.push_back(th);
	reconnect_queue.unlock();
}

void wt_connections::push_connection(task_interface* connection)
{
	connections.lock();
	connections.resource.push_back(connection);
	// сортировать по возрастанию приоритета;
	stable_sort(connections.resource.begin(), connections.resource.end(), cmp_tasks);
	connections.unlock();
}

void* wt_connections::idle_body(void* arg)
{
	wt_connections * pool = reinterpret_cast<wt_connections*>(arg);
	pthread_t v;
	void* retval;
	while (true)
	{
		if (!pool->reconnect_queue.size())
		{
			sleep(5);
			continue;
		}
		pool->reconnect_queue.lock();
		v = pool->reconnect_queue.resource.back();
		pool->reconnect_queue.resource.pop_back();
		pool->reconnect_queue.unlock();
		pthread_join(v, &retval);
	}
	return 0;
}

/// сформировать параметры вызова poll
void wt_connections::make_ufds()
{
	if (max_connection < connections.resource.size())
	{
		delete[] ufds;
		max_connection = connections.resource.size();
		ufds = new pollfd[max_connection];
	}
	nfds = connections.resource.size();
	for (size_t i = 0; i < nfds; i++)
	{
		ufds[i].fd = connections.resource[i]->get_port_id();
		ufds[i].events = POLLIN | POLLPRI | POLLRDHUP;
		ufds[i].revents = 0;
	}
}

/// перед вызовом - заблокировать ресурс connections
bool wt_connections::verify_connections()
{
	bool retval = false;
	task_interface* task;
	vector<task_interface*>::iterator it = connections.resource.begin();
	while (it != connections.resource.end())
	{
		task = *it;
		if (task->wt_delete_me)
		{
			task->clear();
			delete task;
			it = connections.resource.erase(it);
		}
		// задача по возврату обратно в очередь - на потоке реконнекта
		else if (task->is_reconnect)
		{
			task->delete_me = true;
			it = connections.resource.erase(it);
		}
		else if (!task->task_worked)
		{
			task->clear();
			task->delete_me = true;
			it = connections.resource.erase(it);
		}
		else
			++it;
	}
	retval = connections.resource.empty();
	if (retval) connections.unlock();
	return retval;
}

void wt_connections::parse_poll_result(nfds_t i)
{
	// Другие потоки могут только добавлять элементы в конец вектора.
	// Поэтому после вызова poll, незаблокированного мьютексом
	// вполне корректно можно считать, что индексы ufds соответствуют индексам
	// из вектора connections.resource;

	pollfd c_ufds = ufds[i];
	task_interface * task = connections.resource[i];

	if (normal_state_or_reconnect(c_ufds, task, do_reconnect_op)
			&& (c_ufds.revents & POLLIN))
	{
		ssize_t size;
		// XXX: !!! открытый дескриптор должен быть неблокирующим
		recieved_buffer.clear();
		do
		{
			size = read(c_ufds.fd, read_buf, 1024);
			if (size > 0)
				recieved_buffer.append(read_buf, size);
		}
		while (size == 1024);
		if (recieved_buffer.length())
			connections.resource[i]->parse(recieved_buffer);
		recieved_buffer.clear();
	}
}

void *wt_connections::ThreadBody()
{
	int retval;
	nfds_t index;
	while (true)
	{
		if (!connections.size()) sleep (1);
		else
		{
			// на основе вектора connections сформировать массив подключений
			// pollfd[], в процессе формирования масива проверять -
			// а не завершено ли подключение? task_interface::task_worked
			// Если завершено -
			//  -> вызывать для него ф-цию очистки
			//  -> удалять его из очереди и вообще;

			// после poll обработать готовые, затем обработать ошибки.
			connections.lock();
			if (verify_connections())	continue;
			make_ufds();
			connections.unlock();
			retval = poll(ufds, nfds, 1000);
			if (retval > 0)
			{
				connections.lock();
				for (index = nfds - 1; index != 0; index--)
					parse_poll_result(index);
				parse_poll_result(0);
				connections.unlock();
			}
		}
	}
	return 0;
}


