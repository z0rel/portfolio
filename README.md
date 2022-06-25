В настоящем портфолио представлены примеры кода, разработанного Зориным А.Н. в разные периоды времени.


# [/js](/js)
Фрагменты проектов на JavaScript, TypeScript и SCSS


## [react](/js/react)
Проект веб-интерфейса ERP-системы на React.

Используемые технологии: JavaScript, TypeScript, React, Antd, GraphQL Apollo,
Google Protobuf protobufjs, SCSS, styled-components, react-router.

Годы разработки: 2020&mdash;2021

Исходных файлов: 160, строк: 23989, функций: 1405, средняя цикломатическая сложность: 4.0 


## [gulp/innostan](/js/gulp/innostan)
Фрагмент системы сборки статического сайта на основе gulpfile.

В файле [gulpfile.js](/js/gulp/innostan/gulpfile.js) находится основной код вызова задач сборки.

В файле [images.js](/js/gulp/innostan/images.js) находится функциональность экстремального сжатия картинок.

Год разработки: 2019

Исходных файлов: 2, строк: 698, функций: 134, средняя цикломатическая сложность: 1.4 


## [gulp/ptc](/js/gulp/ptc)
Gulp-проект сборки и удаленного развертывания по ssh+rsync фронтэнда приложения
на базе react-create-app. 

Помимо сборки и развертывания основного приложения реализованы команды
обновления кода js-интерфейса Google Protobuf.

Год разработки 2020

Исходных файлов: 17, строк: 939, функций: 111, средняя цикломатическая сложность: 2.0 


## [nextjs](/js/nextjs)
Проект статического сайта на Next.js

Используемые технологии: Next.js, SCSS.

Год разработки: 2021

Исходных файлов: 42, строк: 1897, функций: 100, средняя цикломатическая сложность: 1.9 


## [scraper](/js/scraper)
Код скрапера сайта 

Используемые технологии: JavaScript, cheerio, nightmare

Год разработки: 2019

Исходных файлов: 1, строк: 176, функций: 85, средняя цикломатическая сложность: 1.1 


## [scss](/js/scss)
Примеры таблиц scss стилей, разработанных для различных проектов

Годы разработки: 2018&mdash;2021


# [/python](/python)
Фрагменты проектов на Python и Cython.


## [cv2](/python/cv2)
Пример использования библиотеки OpenCV для сканирования QR кодов чеков

Год разработки: 2021

Исходных файлов: 1, строк: 50, функций: 2, средняя цикломатическая сложность: 5.0 


## [cx_oracle](/python/cx_oracle)
Код дампера модели базы Oracle с использованием библиотеки cx_oracle

Год разработки: 2015

Исходных файлов: 1, строк: 763, функций: 59, средняя цикломатическая сложность: 3.2 


## [cython/estimate_generator](/python/cython/estimate_generator)
Компилируемый код Cython системы автоматизированного генератора сметных расчетов

Скриншот рабочего процесса генерации находится [screenshots](/python/cython/estimate_generator/screenshots).

Годы разработки: 2019&mdash;2021


## [cython/science](/python/cython/science)
Код Cython для решающих T-норм системы нечетких множеств (fuzzy_norms.pyx), поиска циклов во вложенных подмножествах (min_pre_images.pyx), модуля битового массива (lp_bitarray.pyx)

Год разработки: 2018


## [django](/python/django)
Элементы кода бекенда макретинговой ERP-системы на Django.

Скриншот API находится в каталоге [screenshots](/python/django/screenshots).

Технологии, используемые в проекте: 
1. Django models, 
2. Django QuerySet + django-postgres-extra, 
3. GraphQL graphene + graphene-django + graphql_relay,
4. Google ProtoBuf,
5. Fabric,
6. python-docx,
7. openpyxl.

Система реализует функции: 
1. Отдача API на graphql, с оптимизацией больших запросов на Google Protobuf;
2. Управление СУБД на Postgres;
3. Обсчет смет на основе множества критериев;
4. Генерация XLSX (Excel) отчетов;
5. Генерация docx документов (смет и прайс-листов);
6. CLI интерфейс автоматического дампинга удаленной базы и автоматического удаленного развертывания обновлений.

Годы разработки: 2020&mdash;2021

Исходных файлов: 245, строк: 22485, функций: 721, средняя цикломатическая сложность: 3.7 


## [docx](/python/docx)
Код библиотеки для генерации word-файлов коммерческих предложений.

Годы разработки: 2020&mdash;2021

Исходных файлов: 8, строк: 1155, функций: 68, средняя цикломатическая сложность: 3.2 


## [fabric](/python/fabric)
Код различных применений библиотеки fabric для автоматизированного администрирования удаленных серверов 

Годы разработки: 2020&mdash;2021

Исходных файлов: 4, строк: 531, функций: 63, средняя цикломатическая сложность: 1.5 


## [fns_api](/python/fns_api)
Код работы с API ФНС, выполняющих запрос содержимого чека по ФН, ФД, ФПД и дате.

Год разработки: 2021

Исходных файлов: 1, строк: 122, функций: 5, средняя цикломатическая сложность: 5.2 


## [gdbhelpers](/python/gdbhelpers)
Код различных помощников отладчика GDB для разработанных на C++ и C библиотек.
Помощники GDB выполяют человекочитаемое отображение структур данных в консоли отладчика и в структурном представлении WatchWindow IDE.

[ora2lin](/python/gdbhelpers/ora2lin): код помощников отладчика конвертера модели Oracle в модель СУБД Линтер

[scheduler](/python/gdbhelpers/scheduler): код помощников отладчика user-space планировщика задач на корутинах

Годы разработки: 2015&mdash;2017

Исходных файлов: 11, строк: 1727, функций: 225, средняя цикломатическая сложность: 2.1 


## [localization](/python/localization)
Пример кода локализации программы на Python.

Год разработки: 2017

Исходных файлов: 1, строк: 24, функций: 2, средняя цикломатическая сложность: 1.5 


## [/python/ml](/python/ml)
Различные примеры кода, использующие библиотеки машинного обучения 


### [lstm_check_titles_cathegorizer](/python/ml/lstm_check_titles_cathegorizer)
Код рекуррентной нейросети, выполняющей тематическую классификацию категорий товаров по их наименованиям 

Год разработки: 2021

Исходных файлов: 4, строк: 196, функций: 16, средняя цикломатическая сложность: 2.7 


### [rule-based](/python/ml/rule-based)
Код классификатора, основанного на правилах, 
умеющего обрабатывать некоторые отсутствующие входные признаки. 

Год разработки: 2017&mdash;2018

Исходных файлов: 2, строк: 404, функций: 42, средняя цикломатическая сложность: 3.2 


### [find_clustering.py](/python/ml/find_clustering.py)
Код DBSCAN кластеризации данных статистической выборки.

Год разработки: 2016


## [google-translator](/python/pyqt5)
Интерфейс к google-translator на Python3+Python2. 

Программа работает как плагин для Vim и как отдельное оконное приложение на PyQt5.
В Windows переводит автоматически то, что скопировано в буфер обмена.
В Linux переводит автоматически новое выделение (gtk-selection). 
На Python2 написан модуль работы с gtk-selection, т.к. в Python3 такого модуля нет.

Годы разработки: 2015&mdash;2020

Исходных файлов: 11, строк: 11836, функций: 131, средняя цикломатическая сложность: 3.5 


## [scraper](/python/scraper)
Код скраперов сайтов, использующий библиотеку scrapy.

+ [bronses_scrapy](/python/scraper/bronses_scrapy): скрапер сайта www.splav-kharkov.com
+ [gsmarena_scrapy](/python/scraper/gsmarena_scrapy): скрапер сайта gsmarena.com

Годы разработки: 2018-2019

Исходных файлов: 12, строк: 1404, функций: 83, средняя цикломатическая сложность: 3.6 


## [tests](/python/tests)
Код различных модульных тестов.

Годы разработки: 2015&mdash;2021

Исходных файлов: 4, строк: 663, функций: 51, средняя цикломатическая сложность: 2.8 


## [transpliator](/python/transpliator)
Код модуля формальных преобразований транспилятора, преобразующего G-код диалекта Siemens в G-код диалекта БалтСистем.

Год разработки: 2019

Исходных файлов: 2, строк: 1188, функций: 79, средняя цикломатическая сложность: 7.2 


## [vk_api](/python/vk_api)
Код дампинга страниц пользователей VK с загрузкой полученных данных в базу данных PostgreSQL.

Годы разработки: 2016

Исходных файлов: 1, строк: 770, функций: 44, средняя цикломатическая сложность: 3.9 


## [xlwings_scipy_optimize](/python/xlwings_scipy_optimize)
Python-код вычисления локального минимума функции в Excel (с помощью xlwings и scipy).

Годы разработки: 2019

Исходных файлов: 1, строк: 171, функций: 8, средняя цикломатическая сложность: 6.4 


# [/c_cpp](/c_cpp)
Фрагменты проектов, разработанных на C и C++


## [cpp_ora2lin](/c_cpp/cpp_ora2lin)
Фрагмент большого проекта транспилятора DDL-определений языка Oracle включая
PL/SQL в код хранимых процедур и DDL-определений СУБД Линтер. 

В каталоге [screenshots](/c_cpp/cpp_ora2lin/screenshots) представлена иллюстрация работы и возможностей
транспилятора, а также фрагмент документации.

Используемые технологии: С++, pthread, fcntl syscall.

Годы разработки: 2012&mdash;2016

Исходных файлов: 99, строк: 51632, функций: 6641, средняя цикломатическая сложность: 2.5 


## [c_cpp_tests](/c_cpp/c_cpp_tests)
Код различных тестов на C и C++. 

Годы разработки: 2015&mdash;2016

Исходных файлов: 6, строк: 1126, функций: 51, средняя цикломатическая сложность: 3.5 


### [cpp_boost_unittest](/c_cpp/c_cpp_tests/cpp_boost_unittest)
Код юниттестов с использованием библиотеки Boost.

Год разработки: 2016


## [c_multiarch](/c_cpp/c_multiarch)
Фрагмент кода модуля поддержки мультиплатформенных функций для Windows и Linux.

Год разработки: 2015

Исходных файлов: 4, строк: 664, функций: 30, средняя цикломатическая сложность: 5.3 


## [c_scheduler_on_coroutines](/c_cpp/c_scheduler_on_coroutines)
Проект планировщика задач на корутинах, выполняемого в пользовательском пространстве.

Проект кроссплатформенный для Windows и Linux для платформ x86 и amd64.

Планировщик задач является портом MuQSS scheduler из ядра Linux.
Корутины работают как обычные функции, стек вызовов выгружается планировщиком.  
Помимо проекта корутин планировщик поддерживает синхронизацию с помощью реализованных user-space мьютексов. 

Год разработки: 2016

Исходных файлов: 37, строк: 3813, функций: 224, средняя цикломатическая сложность: 2.5 


## [c_spinlocks](/c_cpp/c_spinlocks)
Проект userspace spin-блокировок.

Год разработки: 2016

Исходных файлов: 10, строк: 394, функций: 24, средняя цикломатическая сложность: 2.5 


## [cpp_backend_webserver](/c_cpp/cpp_backend_webserver)
Фрагмент проекта бекэнда, обрабатывающего FastCGI запросы и управляющего базой данных на PostgreSQL.

Используемые технологии: C++, Boost, Libfcgi, poll syscall, libpq, Apache Thrift.

Год разработки: 2016

Исходных файлов: 21, строк: 3974, функций: 196, средняя цикломатическая сложность: 2.9 


## [/c_cpp/cpp_bison_flex](/c_cpp/cpp_bison_flex)
Код различных грамматик лексического анализа Flex и синтаксического glr-анализа Bison


### [gcode](/c_cpp/cpp_bison_flex/gcode)
Проект грамматик лексического анализа Flex и синтаксического glr-анализа Bison 
языка G-кодов диалекта Siemens.

Используемые технологии: Flex, Bison, C++.  

Год разработки: 2019


### [oracle_plpgsql](/c_cpp/cpp_bison_flex/oracle_plpgsql)
Проект грамматик лексического анализа Flex и синтаксического glr-анализа Bison
DDL-определений Oracle, включая язык хранимых процедур и триггеров Oracle
PL/SQL.

Используемые технологии: Flex, Bison, C++.  

Годы разработки: 2012&mdash;2015


### [cpp_boost_python](/c_cpp/cpp_boost_python)
Фрагмент проекта привязки быстрого кода синтаксического анализатора на C++ к коду Python.
Проект компилируется в ddl-библиотеку и импортируется в Python как модуль.

Применяемые технологии: C++, pybind11.

Год разработки: 2019

Исходных файлов: 2, строк: 177, функций: 18, средняя цикломатическая сложность: 1.7 


## [cpp_boost_spirit](/c_cpp/cpp_boost_spirit)
Код синтаксического анализатора URL-ов путей отдаваемых REST API бекенда.

Для реализации использовался Boost Spirit, т.к. стек технологий Заказчика
ограничивался только заголовочной частью Boost, и Заказчик ввел запрет на
подключение разделяемых библиотек включая разделяемые библиотеки boost, в т.ч.
статически компонуемые.

Используемые технологии: С++, Boost, Boost Spirit.

Год разработки: 2016

Исходных файлов: 2, строк: 907, функций: 45, средняя цикломатическая сложность: 3.2 


## [cpp_embedded](/c_cpp/cpp_embedded)
Фрагмент проекта прошивки embedded-устройства (вязкозиметра) для процессора AVR atmega128. 

Год разработки: 2021

Исходных файлов: 74, строк: 3861, функций: 218, средняя цикломатическая сложность: 2.6 


## [cpp_game](/c_cpp/cpp_game)
Код 2d игры "Арканоид" для Windows.

Год разработки: 2012

Исходных файлов: 5, строк: 452, функций: 28, средняя цикломатическая сложность: 3.0 


## [cpp_makefiles](/c_cpp/cpp_makefiles)
Примеры проектов различных систем сборки (make, cmake, qmake) для проектов C/С++


## [cpp_network](/c_cpp/cpp_network)
Linux-библиотека API для подключения и обмена по TCP/IP и RS232.

Год разработки: 2011

Исходных файлов: 4, строк: 547, функций: 28, средняя цикломатическая сложность: 4.4 


## [cpp_poll](/c_cpp/cpp_poll)
Код использования системного вызова poll для обработки TCP/IP соединений с применением паттерна WorkerThread.

Год разработки: 2011

Исходных файлов: 2, строк: 240, функций: 14, средняя цикломатическая сложность: 2.7 


## [cpp_proteus_vsm](/c_cpp/cpp_proteus_vsm)
Код библиотеки виртуальной модели микросхемы памяти DS1744 для ее использования в модели Proteus.

Год разработки: 2022

Исходных файлов: 19, строк: 1577, функций: 80, средняя цикломатическая сложность: 3.7 


## [cpp_pthread](/c_cpp/cpp_pthread)
Пример использования примитива синхронизации Condition Variable.

Используемые технологии: C++, pthread.

Год разработки: 2012

Исходных файлов: 2, строк: 121, функций: 12, средняя цикломатическая сложность: 1.5 


## [/c_cpp/cpp_qt](/c_cpp/cpp_qt)
Различные проекты на C++ использующе библиотеку Qt5


### [behrens_gui](/c_cpp/cpp_qt/behrens_gui)
Графический интерфейс, выполняющий формирование файлов конфигурации.

Используемые технологии: C++, Qt5, pybind11

Год разработки: 2019

Исходных файлов: 3, строк: 151, функций: 15, средняя цикломатическая сложность: 1.1 


### [conductivity](/c_cpp/cpp_qt/conductivity)
Приложение, строящее 3d модель теплопроводности бруса.

Скриншот приложения находится в каталоге [screenshots](/c_cpp/cpp_qt/conductivity/screenshots).

Используемые технологии: Qt5, OpenGL, C++.  

Год разработки: 2012


### [fractals](/c_cpp/cpp_qt/fractals)
Приложение, строящие разнообразные фракталы и L-системы. 

Скриншоты фракталов, которые строит приложение представлены в каталоге [screenshots](/c_cpp/cpp_qt/fractals/screenshots).

Используемые технологии: Qt5, OpenGL, C++.  

Год разработки: 2012

Исходных файлов: 15, строк: 785, функций: 100, средняя цикломатическая сложность: 1.5 


### [gl_widget](/c_cpp/cpp_qt/gl_widget)
Qt-виджет, отображающий сцену OpenGL, объекты на которой можно увеличивать, уменьшать, перетаскивать и вращать.

Используемые технологии: С++, Qt4, GLC_lib, OpenGL

Год разработки: 2011

Исходных файлов: 2, строк: 123, функций: 10, средняя цикломатическая сложность: 1.7 


## [cpp_random](/c_cpp/cpp_random)
Реализация генератора случайных чисел "Вихрь Мерсенна" на C

Год разработки: 2016

Исходных файлов: 2, строк: 91, функций: 6, средняя цикломатическая сложность: 2.7 


# [/sql_plpgsql](/sql_plpgsql)
Фрагменты кода разработанных моделей баз данных с триггерами и хранимыми процедурами для PostgreSQL.


## [ptc](/sql_plpgsql/ptc)
Хранимые процедуры и триггеры для базы PostgreSQL ERP системы. Модель базы создается из Django.

Годы разработки: 2020&mdash;2021


## [rollout_center](/sql_plpgsql/rollout_center)
Модель и хранимые процедуры базы данных бекенда системы генерации раздачи ключей ЭЦП.

Годы разработки: 2016



# [/mics](/mics)
Код, разработанный на неосновных языках Java и C#


## [io_modbus_raspberrypi](/mics/c#/io_modbus_raspberrypi)
Проект системы сбора и передачи данных для Raspberry Pi на C#. 

Программа собирает данные по Modbus RTU RS485 и передает их в систему управления УЧПУ Delta NC30EB по API протоколу этой системы (по Ethernet).

Используемые технологии: C# (.NET Core 3), NModbus, Unosquare.RaspberryIO, YamlDotNet

Год разработки: 2021

Исходных файлов: 6, строк: 631, функций: 25, средняя цикломатическая сложность: 3.1 


## [suo](/mics/c#/suo)
Фрагмент проекта обучающей программы "стрельба и управление огнём артиллерии" на C#.

Год разработки: 2010

Исходных файлов: 5, строк: 3674, функций: 112, средняя цикломатическая сложность: 7.2 


## [java](/mics/java)
Проект тестового приложения на Java для Android.

Год разработки: 2016

Исходных файлов: 3, строк: 141, функций: 13, средняя цикломатическая сложность: 1.8 


# [/science](/science)
Основные научные статьи. 

До 08.2017 года публиковался под фамилией Шмарин, после &mdash; поменял фамилию на Зорин.

Годы разработки: 2013&mdash;2018

