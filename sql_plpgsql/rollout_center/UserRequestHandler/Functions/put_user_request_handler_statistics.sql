-- function put_user_request_handler_statistics( timestamp, int, int, int );

CREATE OR REPLACE FUNCTION put_user_request_handler_statistics( server_time timestamp,          -- время на сервере, когда были собраны данные статистики
                                                                server_duration_seconds int,    -- период времени в секундах, за который были собраны данные
                                                                server_new_connections int,     -- количество новых подключений за период
                                                                server_active_connections int)  -- текущее количество активных подключений
RETURNS void AS
$$
BEGIN
        insert into 
          user_request_handler_statistics(  stat_time, 
                                            period_seconds, 
                                            new_connections, 
                                            total_connections  ) 
        values ( server_time, 
                 server_duration_seconds, 
                 server_new_connections, 
                 server_active_connections );
END;
$$
LANGUAGE plpgsql;
ALTER FUNCTION put_user_request_handler_statistics( timestamp, int, int, int )
OWNER TO postgres;
COMMENT ON FUNCTION put_user_request_handler_statistics( timestamp, int, int, int )
  IS 'Добавляет данные статистики в БД';