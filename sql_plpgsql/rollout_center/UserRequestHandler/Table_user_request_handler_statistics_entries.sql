-- Table: user_request_handler_statistics

DROP TABLE IF EXISTS user_request_handler_statistics;

CREATE TABLE user_request_handler_statistics
(
  stat_time timestamp without time zone,        -- время записи статистики
  period_seconds int,                           -- количество секунд, за которые были собраны данные для записи
  new_connections int,                          -- количество новых подключений за period_seconds секунд
  total_connections int                         -- общее количество активных подключений в течении period_seconds секунд
)
WITH (
  OIDS=FALSE
);
ALTER TABLE users_certificates
  OWNER TO postgres;
COMMENT ON TABLE users_certificates
  IS 'Статистика работы обработчика запросов пользователей';
