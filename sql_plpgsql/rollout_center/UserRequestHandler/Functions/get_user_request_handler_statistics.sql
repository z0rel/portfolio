-- function get_user_request_handler_statistics( int );

-- на вход принимает период времени в секундах, на какой выводить статистику
CREATE OR REPLACE FUNCTION get_user_request_handler_statistics( stat_seconds int )
RETURNS TABLE( max_conn_freq real, avg_conn_freq real, avg_connections int ) AS
$$
BEGIN
  select max( cast (new_connections as real) / period_seconds ), 
         avg( cast (new_connections as real) / period_seconds ), 
         avg ( total_connections )
  from user_request_handler_statistics
  where (stat_time + stat_seconds * interval '1 second') > now();
END;
$$
LANGUAGE plpgsql;
ALTER FUNCTION get_user_request_handler_statistics( int )
OWNER TO postgres;
COMMENT ON FUNCTION get_user_request_handler_statistics( int )
  IS 'Возвращает общестстемные показатели статистики: пиковая частота подключений, средняя частота подключений, всего активных подключений';