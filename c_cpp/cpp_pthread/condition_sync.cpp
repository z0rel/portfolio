#include "condition_sync.h"
#include <time.h>
#include <sstream>
#include <iostream>
using namespace std;


/// Конструктор контекста синхронизированного запроса
condition_sync_t::condition_sync_t(driver_base * _parent, timeout_handler_t ptimeout_handler)
: predicate(1),
  count_timeouts(0),
  is_timeout(false),
  parent(_parent),
  timeout_handler(ptimeout_handler)
{
	pthread_cond_init(&cond, NULL);
	pthread_mutex_init(&mutex, NULL);
}


string condition_sync_t::toStatistics()
{
	stringstream str;
	str     << "timeout_time: sec = " << timeout_time.tv_sec
						<< " nsec = " << timeout_time.tv_nsec << endl
			<< "predicate = " << predicate << endl
			<< "count_timeouts = " << count_timeouts << endl
			<< "is_timeout = " << is_timeout << endl
			<< "parent = " << parent << endl
			<< "timeout_handler = " << timeout_handler;
	return str.str();
}

/// Деструктор контекста синхронизированного запроса
condition_sync_t::~condition_sync_t()
{
	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&mutex);
}

/**
 * Подготовить контекст состояния к синхронизированному запросу
 * @param tv_sec    Сколько секунд ожидать подтверждения запроса
 * @param tv_nsec   Сколько наносекунд ожидать подтверждения запроса
 */
void condition_sync_t::prev_sync(__time_t tv_sec, long int tv_nsec) const
{
	pthread_mutex_lock(&mutex);

	time(&timeout_time.tv_sec);
	timeout_time.tv_sec  += tv_sec;
	timeout_time.tv_nsec = tv_nsec;

	predicate = 0;
	is_timeout = false;
}


/// После того, как запрос выполнен - ожидать подтверждения ответа на запрос
void condition_sync_t::post_sync() const
{
	parse_wait_res(pthread_cond_timedwait(&cond, &mutex, &timeout_time));
}

// После того, как запрос выполнен - ожидать подтверждения ответа на запрос
void condition_sync_t::post_sync_inf() const
{
	parse_wait_res(pthread_cond_wait(&cond, &mutex));
}

/**
 * Парсинг результатов ожидания изменения состояния
 * @param res - возврат из pthread_cond_timedwait или pthread_cond_wait
 */
void condition_sync_t::parse_wait_res(int res) const
{
	if (
			/// парсер не обработал запрос синхронизации
			(res == ETIMEDOUT) ||
			/// во время запроса синхронизации произошла ошибка
			((res) && (res != ETIMEDOUT) && (!predicate))
	)
	{
		is_timeout = true;
		count_timeouts++;
		if (timeout_handler)
			timeout_handler(parent);
	}
	else // парсер уже обработал запрос синхронизации вызвав accept_request_sync()
		count_timeouts = 0;
	predicate = 1;
	pthread_mutex_unlock(&mutex);
}


/// Отменить синхронизированный запрос, когда после выполнения операции
/// становится известно, что он уже не нужен.
void condition_sync_t::cancel() const
{
	predicate = 1;
	pthread_mutex_unlock(&mutex);
}

/// Остановить синхронизированный запрос request_sync, послав ему команду успешно завершиться.
void condition_sync_t::accept() const
{
	pthread_mutex_lock(&mutex);
	if (!predicate)
	{
		pthread_mutex_unlock(&mutex);
		pthread_cond_broadcast(&cond);
	}
	else
		pthread_mutex_unlock(&mutex);
}

/**
 * Произошел ли таймаут во время последнего синхронизированного запроса
 * @return
 * 		@retval true
 * 			Таймаут произошел
 * 		@retval false
 * 			Таймаут не произошел
 */
bool condition_sync_t::timeout() const
{
	return is_timeout;
}
