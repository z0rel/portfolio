/*
 * conditionsynct.h
 *
 *  Created on: 02.05.2012
 *      Author: root
 */

#ifndef CONDITIONSYNCT_H_
#define CONDITIONSYNCT_H_

#include <pthread.h>
#include <time.h>
#include <errno.h>

#include <task_interface.h>

/// Функциональность синхронизированных запросов
class condition_sync_t
{
	/// Синоним типа функции обработки таймаута
	typedef task_interface::timeout_handler_t timeout_handler_t;

	/// Переменная_условия;
	mutable pthread_cond_t  cond;
	/// Мьютекс для синхронизации;
	mutable pthread_mutex_t mutex;
	/// Таймаут вермени ожидания
	mutable struct timespec timeout_time;
	/// Предикат, чтобы не делать broadcast, когда никто не ждет;
	mutable int predicate;
	/// Сколько всего произошло таймаутов
	mutable int count_timeouts;
	/// Произошел ли в течение последней операции синхронизации таймаут.
	mutable bool is_timeout;
	/// Внешний пользователь класса синхронизации
	driver_base * parent;
	/// Внешняя функция обработки таймаута
	timeout_handler_t timeout_handler;

	// Парсинг результатов ожидания изменения состояния
	void parse_wait_res(int res) const;

public:
	string toStatistics();

	// Конструктор контекста синхронизированного запроса
	condition_sync_t(driver_base * _parent, timeout_handler_t ptimeout_handler);

	// Деструктор контекста синхронизированного запроса
	virtual ~condition_sync_t();
	// Подготовить контекст состояния к синхронизированному запросу
	void prev_sync(__time_t tv_sec, long int tv_nsec) const;
	// После того, как запрос выполнен - ожидать подтверждения ответа на запрос
	void post_sync() const;
	// После того, как запрос выполнен - ожидать (до бесконечности) подтверждения ответа на запрос
	void post_sync_inf() const;
	// Отменить синхронизированный запрос, когда после выполнения операции
	// становится известно, что он уже не нужен.
	void cancel() const;
	// Остановить синхронизированный запрос request_sync, послав ему команду успешно завершиться.
	void accept() const;
	// Произошел ли таймаут во время последнего синхронизированного запроса
	bool timeout() const;
	/// Сколько всего произошло таймаутов
	/// @return Сколько всего произошло таймаутов
	int count() const {return count_timeouts;}
	/**
	 * Выполнить синхронизированный запрос. Если по прошествии заданного времени запрос не
	 * подтвердится вызовом accept_request_sync, то:
	 * будет установлен в @c true флаг - @c is_timeout, который затем внешний код сможет запросить
	 * через get-метод get_is_timeout, а также будет вызвана функция-обработчик таймаута
	 * th_context.timeout_handler
	 * @param tv_sec
	 * 		Секунды - целая часть максимального времени запроса
	 * @param tv_nsec
	 * 		Наносекунды - дробная часть максимального времени запроса
	 */
	template <typename OPERATION>
	void operator()(__time_t tv_sec, long int tv_nsec, const OPERATION &op) const
	{
		prev_sync(tv_sec, tv_nsec);
		op();
		post_sync();
	}
};

#endif /* CONDITIONSYNCT_H_ */
