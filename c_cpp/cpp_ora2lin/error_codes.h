#ifndef __ERROR_CODES
#define __ERROR_CODES

#include <string>

#include "hash_table.h"

using namespace BackportHashMap;

namespace dmperr {
  /// Коды исключений, генерируемых в ходе преобразователя
  typedef enum Errors {
    OK = 0,
    /// В курсоре после IS ничего не идет
    CURSOR_BAD_SYNTAX,
    /// Курсор не заканчивается точкой с запятой.
    CURSOR_NO_SEMICOLON,
    /// Запись не заканчивается точкой с запятой.
    RECORD_NO_SEMICOLON,
    /// В курсоре не определен запрос SELECT
    CURSOR_NOT_HAVE_REQUEST,
    /// При попытке выполнить операцию с курсором, оказалось что он не определен
    CURSOR_NOT_FOUND,
    /// При открытии курсора встретилась конструкция FOR
    OPENED_CURSOR_CANNOT_HAVE_FOR,
    /// Открываемая курсорная переменная не имеет оператора FOR.
    OPEN_CURSOR_VARIABLE_NOT_HAVE_FOR,
    /// Оператор OPEN для курсорной переменной не имеет оператора SELECT
    CURSOR_VARIABLE_BAD_SYNTAX,
    /// Аргумент операции OPEN имеет тип не курсора и не курсорной переменной
    ARGUMENT_NOT_HAVE_CURSOR_TYPE,
    /// Оператор FETCH не содержит конструкции INTO
    FETCH_NOT_HAVE_INTO,
    /// Оператор FETCH не содержит завершающей точки с запятой
    FETCH_NO_SEMICOLON,
    /// Оператор CLOSE не содержит завершающей точки с запятой
    CLOSE_NO_SEMICOLON,

    /// Нарушение структуры DMP-файла
    DMP_STRUCT_ERROR,
    /// Создаваемый @c ScopeItem уже существует, но имеет другой тип
    NAME_ALREDY_EXIST_WITH_OTHER_TYPE,

    /**
     * Ошибка при парсинге конструкции @c INSERT_INTO
     * - имя таблицы пусто
     * - список полей пуст
     * - таблица с заданным именем еще не создана
     * - не удалось разобрать типы значений для полей
     * .
     */
    INSERT_INTO,
    /**
     * Ошибка при парсинге конструкции @c ALTER_TABLE
     * - таблица с заданным именем еще не создана
     * - нет тега, указывающего как измененять талицу
     * .
     */
    ALTER_TABLE,
    /**
     * Ошибка при парсинге комментария к таблице:
     * - таблица с заданным именем еще не создана
     * - нет тега @c IS
     * - после тега @c IS есть другие символы
     * - комментарий пуст
     * .
     */
    COMMENT_ON_TABLE,
   /**
    * Ошибка при парсинге комментария к столбцу:
    * - таблица с заданным именем еще не создана
    * - нет точки-разделителя после имени таблицы
    * - столбец с заданным именем еще не создан
    * - нет тега @c IS
    * - после тега @c IS есть другие символы
    * - комментарий пуст
    * .
    */
    COMMENT_ON_COLUMN,

    /**
     * Ошибка добавления первичного ключа:
     * - список ключевых полей пуст
     * - для таблицы не удалось найти или создать индекс с заданными полями
     * .
     */
    ADD_PRIMARY_KEY,
    /**
     * Ошибка добавления внешнего ключа:
     * - список ключевых полей пуст
     * - для таблицы не удалось найти или создать индекс с заданными полями
     */
    ADD_FOREIGN_KEY,
    /**
    * Ошибка добавления ограничения на уникальность:
    * - список ограничиваемых полей пуст
    * - для таблицы не удалось найти или создать индекс с заданными полями
    * .
    */
    ADD_UNIQUE     ,

    /// Для @c CHECK список ограничиваемых полей пуст.
    ADD_CHECK      ,

    /**
     *  Конструкция @code ADD CONSTRAINT @endcode не завершена
     *  элементом @code (PRIMARY KEY | FOREIGN KEY | CHECK) <список полей> @endcode
     */
    ADD_CONSTRAINT, //UNCOMPLETED_CONSTRUCTION
    /// После @code MODIFY DEFAULT @endcode нет (?) комментария
    MODIFY_DEFAULT,
    /// Для @c MODIFY не найдено полей с заданными именами, либо нет тега @c DEFAULT
    /// За тегом @c MODIFY нет открывающей скобки, либо список полей пуст
    MODIFY_UNCOMPLETED_CONSTRUCRION,

    /* Ошибки, определяемые вызовом valid() */
    CREATE_SEQUENCE     ,
    CREATE_TABLE        ,
    CREATE_INDEX        ,
    CREATE_UNIQUE_INDEX ,
    CREATE_SYNONYM      ,
    CREATE_VIEW         ,

    /* Ошибки ввода-вывода */
    /// @c fread не смог прочесть запись
    FREAD_RDERROR     ,
    /// Длина типа меньше чем количество байтов, которые нужно считать
    FREAD_OVERFLOW    ,
    /// В шаблонную функцию @c fread передан нецелый тип
    FREAD_NONINTEGRAL ,

    /// Функции @c fseek не удалось выполнить заданное перемещение
    FSEEK_ERROR       ,

    /* Ошибки анализа списков значений */
    // для setValues
    /// Длина блоба не выровнена по границе последнего поля
    SV_UNALIGNED_BLOBLEN ,
    /// В файле дампа базы данных встретился неизвестный код
    UNKNOWN_DMP_MAGIC_CODE ,
    SV_END_OF_DATASET_MUST_BE_FIRST_IN_ROW ,
    // для setValueTypes
    /// Следующая длина поля < 0
    SVT_BAD_OPERATION_SYNTAX ,
    /// Неизвестный тип
    UNKNOWN_DMP_CODE_FOR_TYPE ,

    // Ошибки, идущие через throw
    /// Нет закрывающей круглой скобки в выражении @code IN( ... ) @endcode
    EXPR_IN_NO_CLOSE_BRACKET ,

    /// Нет закрывающей круглой скобки в вызове функции
    FUNCTION_NO_CLOSE_BRACKET ,

    /// У функции не задано имяSV_ROWLEN_NE_0_AND_NE_1
    FUNCTION_UNCOMPLETED_CONSTRUCRION, // UNCOMPLETED_CONSTRUCRION

    /// В выражении нет закрывающей скобки перед открывающей
    OPERNAD_NO_CLOSE_BRACKET,
    /// Имя операнда пусто
    OPERAND_NAME_EMPTY,

    /// В списке параметров нет разделяющего пробела перед тегом направления
    /// @c IN или @c OUT либо (?) запятой
    PARAMS_LIST_BAD_OPERATION_SYNTAX,
    /// В списке параметров нет завершающей круглой скобки
    PARAMS_LIST_NO_CLOSE_BRACKET ,
    /// В списке параметров функции нет завершающего @c RETURN
    PARAMS_LIST_UNCOMPLETED_CONSTRUCTION ,

    /// Заголовок не завершается тегом @c begin или точкой с запятой
    HEADER_NO_SEMICOLON ,

    /// При парсинге операции перед @c exception встретились @c when или @c exception
    OPERATION_UNEXPECTED_TAG ,
    /// Перед when нет тега exception
    OPERATION_BAD_OPERATION_SYNTAX ,
    /// После тега when нет тега then
    OPERATION_UNCOMPLETED_CONSTRUCTION ,

    /// Перед тегом @c then встретились теги @c then или @c else
    IF_UNEXPECTED_TAG ,
    /// Перед тегом @c else нет тега @c then
    /// Перед тегом @c elseif нет тега @c then
    IF_BAD_OPERATION_SYNTAX ,
    /// После тега end нет тега @c if
    /// После @c IF или @c ELSIF не было никаких тегов
    IF_UNCOMPLETED_CONSTRUCTION ,

    /// В проверяемом операторе отсутствует необходимая точка с запятой
    OPERATOR_NO_SEMICOLON ,

    /// После тега @c END нет тега @c LOOP
    LOOP_UNCOMPLETED_CONSTRUCTION ,
    /// В начале оператора отутствует тег @c LOOP
    LOOP_BAD_OPERATION_SYNTAX ,


    /// В начале оператора отутствует тег @c IN
    FOR_BAD_OPERATION_SYNTAX ,
    /// В после тега @c IN не встретился тег @c LOOP, либо после тега @c END нет тега @c LOOP
    FOR_UNCOMPLETED_CONSTRUCTION ,

    /// В @c select встретился недопустимый тег
    SELECT_UNEXPECTED_TAG ,
    /// В @c select, до точки с запятой встретился нераспознанный символ
    SELECT_UNEXPECTED_CHARACTER ,
    /// В @c select не задано, что выбирать
    SELECT_UNCOMPLETED_CONSTRUCTION ,

    INSERT_UNCOMPLETED_CONSTRUCTION ,
    INSERT_NO_CLOSE_BRACKET         ,
    INSERT_NO_OPEN_BRACKET          ,
    INSERT_UNEXPECTED_TAG           ,

    DELETE_UNCOMPLETED_CONSTRUCTION ,
    DELETE_UNEXPECTED_TAG   ,

    EXECUTE_UNEXPECTED_TAG  ,

    PRAGMA_NO_CLOSE_BRACKET ,
    PRAGMA_NO_OPEN_BRACKET  ,
    PRAGMA_NO_SEMICOLON     ,
    PRAGMA_NO_COMMA         ,

    /// Незавершенное определение типа в конструкции package
    PACKAGE_UNCOMPLETE_TYPE ,
    PACKAGE_NO_SEMICOLON    ,
    PACKAGE_UNCOMPLETED_CONSTRUCTION,

    DECLARE_NO_SEMICOLON,

    SCOPE_ADD_FUNCTION_ERROR,

    REFTYPE_NO_CLOSE_BRACKET    ,
    REFTYPE_PRECISION_EMPTY     ,
    REFTYPE_PRECISION_UNEXPECTED,
    REFTYPE_UNEXPECTED_TAG      ,
    REFTYPE_UNEXPECTED_TYPE     ,
    REFTYPE_WIDTH_EMPTY         ,
    REFTYPE_WIDTH_UNEXPECTED    ,

    /// Для типа значения не задан конвертор в строку
    NOT_TYPE_CONVERTER_FOR_ROW_VALUE,
    /// Неизвестный тип в SQL-тексте.
    UNKNOWN_TYPE_SQL_DECLARATION,
    /// После распознанного типа идет неизвестный текст
    UNKNOWN_TEXT_AFTER_TYPE_SQL_DECLARATION,
    /// При парсинге конструкции CREATE TABLE, программа вошла в бесконечный цикл
    CREATE_TABLE_INFINITE_LOOP,

    /// В конструкции @c CASE после конструкции @c WHEN нет конструкции @c THEN
    CASE_WHEN_NOT_THEN,
    /// В конструкции @c CASE нет завершающего терминала @c END
    CASE_NOT_END,

    /// В конструкции @c CASE после конструкции @c WHEN нет конструкции @c THEN
    CAST_NOT_AS,
    /// В конструкции @c CASE нет завершающего терминала @c END
    CAST_NOT_BRACKET,
    /// В конструкции @c UPDATE нет лексемы @c SET
    UPDATE_NOT_SET,
    /// В конструкции UPDATE нет завершающей точки с запятой
    UPDATE_NOT_SEMICOLON,
    /// В выражении @c RETURNING нет тега @c INTO
    RETURNING_CLAUSE_NOT_INTO,

    /// В операторе @c EXTRACT нет открывающей скобки
    EXTRACT_NO_OPEN_BRACKET,
    /// В операторе @c EXTRACT нет закрывающей скобки
    EXTRACT_NO_CLOSE_BRACKET,
    /// В операторе @c EXTRACT нет секции @c FROM
    EXTRACT_NO_FROM,

    EXTRACT_UNKNOWN_PART_NAME,

    OVER_HAS_NOT_OPEN_BRACKET,
    OVER_HAS_NOT_CLOSE_BRACKET,

    LABEL_HAS_NOT_CLOSE_BRACKETS,

    LOCK_TABLE_HAS_NOT_IN,
    LOCK_TABLE_HAS_NOT_MODE,
    /// Неизвестный wait-тег в операторе @c LOCK @c TABLE
    LOCK_TABLE_UNKNOWN_WAIT,
    /// В секции @c WITH оператора @c SELECT нет терминального символа @c AS
    SELECT_WITH_HAS_NOT_AS,

    /// Встретилась неизвестная конструкция в парсинге @c CREATE @c TYPE
    CREATE_TYPE_UNEXPECTED_SYNTAX,
    /// После ключевого слова @c CREATE @c TYPE не идет нулевого маркера 0xFFFD
    CREATE_TYPE_HAS_NOT_NULL_MARKER,

    /**
     * Секция блоб идет перед строкой обычных полей.
     * Данный формат ранее не встречался и дожен быть исследован.
     */
    BLOB_BEFORE_DATA_ROW,
    INCORRECT_NUMBER_OF_BLOB_RECORDS,

    MERGE_HAS_NOT_INTO,
    MERGE_HAS_NOT_USING,
    MERGE_HAS_NOT_ON,
    MERGE_INTO_HAS_NOT_FIELD_LIST,
    MERGE_INTO_HAS_NOT_VALUES,


    /// Нет открывающей круглой скобки в вызове функции
    TRIM_NO_OPEN_BRACKET,
    TRIM_NO_CLOSE_BRACKET,
    TRIM_HAS_NOT_FROM,

    BRACKET_ITEM_EMPTY,

    EXCEPTION_VALUE_EMPTY,

    // Некорректное использование повышающего приведения типов
    BAD_CAST

  } errors;

  const char* toString(errors err);

  struct ErrorInfo {
    ErrorInfo( const std::string & _parsedString,
               const std::string & _errorBegins, errors _code);
    /// Анализируемая строка
    std::string parsedString;
    /// Строка, начинающаяся с позиции ошибки
    std::string errorBegins;
    /// Код ошибки
    errors code;

    void toString(std::string & str) const;
    std::string toString() const { std::string s; toString(s); return s; }
    void toString(std::string & str, const std::string & parsedStr ) const;
  };


  struct ErrorCodes {
    std::string engCode;
    std::string ruCode;
  };

  void handleError( const std::string       & str      ,
                    size_t                    streamlen,
                    const dmperr::ErrorInfo & err      ,
                    const std::string       & pkgName  );

  extern HashMap<int, dmperr::ErrorCodes> errorCodeConverter;

  void errorCodeInit(HashMap<int, ErrorCodes> &hash, const std::string &fname);
  void errorseInit(HashMap<int, ErrorCodes> &hash, const std::string &fname);
}




#endif
