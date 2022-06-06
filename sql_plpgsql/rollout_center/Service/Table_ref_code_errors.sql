-- Table: ref_code_errors

DROP TABLE ref_code_errors;

CREATE TABLE ref_code_errors
(
  err_code bigint,
  err_str character varying
)
WITH (
  OIDS=FALSE
);
ALTER TABLE ref_code_errors
  OWNER TO postgres;
COMMENT ON TABLE ref_code_errors
  IS 'Таблица содержит стандартные коды возврата.';

insert into ref_code_errors(err_code, err_str)
values(5, 'Сертификат с данным именем издателя и серийным номером уже зарегистрирован.');

insert into ref_code_errors(err_code, err_str)
values(4, 'Изменения не требуются.');

insert into ref_code_errors(err_code, err_str)
values(3, 'Удаляемый объект не существует.');

insert into ref_code_errors(err_code, err_str)
values(2, 'Все существующие записи уже обработаны.');

insert into ref_code_errors(err_code, err_str)
values(1, 'Создаваемый объект уже существует');

insert into ref_code_errors(err_code, err_str)
values(-1, 'Нет прав на выполнение данного действия с данным объектом.');

insert into ref_code_errors(err_code, err_str)
values(-2, 'Объекта с нужным именем и типом нет.');

insert into ref_code_errors(err_code, err_str)
values(-3, 'Передаваемые аргументы взаимоисключающие.');

insert into ref_code_errors(err_code, err_str)
values(-4, 'Временная таблица не найдена');

insert into ref_code_errors(err_code, err_str)
values(-5, 'Неверный формат аргумента.');

insert into ref_code_errors(err_code, err_str)
values(-6, 'NULL недопустим.');

insert into ref_code_errors(err_code, err_str)
values(-7, 'Просроченный сертификат.');

insert into ref_code_errors(err_code, err_str)
values(-8, 'Объекта с данным идентификатором нет.');

insert into ref_code_errors(err_code, err_str)
values(-9, 'Уже есть объект с таким же именем.');

insert into ref_code_errors(err_code, err_str)
values(-10, 'Такого типа объекта не существует.');

insert into ref_code_errors(err_code, err_str)
values(-11, 'Сертификат с данным серийным номером и именем издателя не зарегистрирован.');

insert into ref_code_errors(err_code, err_str)
values(-12, 'Не найден объект по значениям числового и строкового идентификатора.');

insert into ref_code_errors(err_code, err_str)
values(-13, 'Объекта с данным строковым идентификатором нет.');

insert into ref_code_errors(err_code, err_str)
values(-14, 'Редактирование объекта не допускается.');

insert into ref_code_errors(err_code, err_str)
values(-15, 'Объект должен иметь уникальное имя.');

insert into ref_code_errors(err_code, err_str)
values(-16, 'Пустой аргумент недопустим.');

insert into ref_code_errors(err_code, err_str)
values(-17, 'Ресурс с заданным именем уже существует');

insert into ref_code_errors(err_code, err_str)
values(-18, 'Внутренний адрес ресурса уже используется');

insert into ref_code_errors(err_code, err_str)
values(-19, 'Внешний адрес ресурса уже используется');

insert into ref_code_errors(err_code, err_str)
values(-20, 'Редактирование объекта невозможно, состояние было изменено.');

insert into ref_code_errors(err_code, err_str)
values(-21, 'Сертификат имеет неизвестный формат');

insert into ref_code_errors(err_code, err_str)
values(-22, 'Ошибка аутентификации пользователя');

insert into ref_code_errors(err_code, err_str)
values(-23, 'Невозможно удалить последнего администратора безопасности');

insert into ref_code_errors(err_code, err_str)
values(-24, 'Невозможно редактировать доступ к ресурсам у заблокированного сертификата');

insert into ref_code_errors(err_code, err_str)
values(-25, 'Для данного издателя уже зарегистрирован более свежий CRL');

insert into ref_code_errors(err_code, err_str)
values(-26, 'Корневой сертификат с данным именем и идентификатором ключа не зарегестрирован');

insert into ref_code_errors(err_code, err_str)
values(-27, 'Свойства с таким именем или идентификатором нет');

insert into ref_code_errors(err_code, err_str)
values(-28, 'Данное свойство устанавливается только автоматически');

insert into ref_code_errors(err_code, err_str)
values(-29, 'Корневой сертификат уже зарегестрирован');

insert into ref_code_errors(err_code, err_str)
values(-30, 'Сертификат был отозван');

insert into ref_code_errors(err_code, err_str)
values(-31, 'Не найден CRL для корневого сертификата');

insert into ref_code_errors(err_code, err_str)
values(-32, 'Лицензия не загружена');

insert into ref_code_errors(err_code, err_str)
values(-33, 'Достигнуто максимальное число пользователей для данной лицензии');

insert into ref_code_errors(err_code, err_str)
values(-100, 'Внутренняя ошибка при выполнении.');

insert into ref_code_errors(err_code, err_str)
values(-200, 'Нет соединения с БД.');
