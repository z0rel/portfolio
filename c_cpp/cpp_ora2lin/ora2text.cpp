#include <iostream>
#include "ora2text.h"
#include "siter.h"
#include "constants.h"

using namespace std;

void varcharToText(string &result, const void *value, size_t size) {
  string s((char *)value, size);
  PlsqlHelper::replaceCr(s, "~");
  PlsqlHelper::quotingAndEscapingSpec(result, s, '\'', '\'');
}

int extractXmlOffset(const void *value, const size_t &size) {
  if (size <= 8)
    return 0;

  size_t xmlSize = ((unsigned char *)value)[2];
  int offset = 8;
  if (xmlSize != size)
    offset = 12;
  return offset;
}

void xmlToText(string &result, const void *value, size_t size) {
  int offset = extractXmlOffset(value, size);
  if (!offset)
    result += "default";
  else
    varcharToText(result, ((char *)value) + offset, size - offset);
}

void stubToText(string &result, const void *, size_t) { result += "default"; }

void DateTime::normalizeFracSeconds() {
  leadZeros = 9;
  if (fracSecond)
    while (!(fracSecond % 10)) { // уменьшение количества завершающих нулей в
                                 // дробной части наносекунд
      fracSecond /= 10;
      leadZeros--;
    }
  else
    leadZeros = 0;
  if (leadZeros < 0) // проверка для корректного вывода ведущих нулей
    leadZeros = 0;
}

void DateTime::toString(std::string &result) {
  static char strBuf[128];

  if (fracSecond)
    sprintf(strBuf, "\'%02u.%02u.%04i %02u:%02u:%02u.%0*u\'", day, month, year,
            hour, minute, second, leadZeros, fracSecond);
  else
    sprintf(strBuf, "\'%02u.%02u.%04i %02u:%02u:%02u\'", day, month, year, hour,
            minute, second);

  result += strBuf;
}

void timestampToObj(DateTime &dateTime, const void *value, size_t size) {
#define SHIFT_STAMP(pos, shift)                                                \
  ((unsigned int)(((unsigned char *)(value))[pos]) << ((shift) * 8))

  dateTime.year =
      (int(SHIFT_STAMP(0, 0)) - 100) * 100 + (int(SHIFT_STAMP(1, 0)) - 100);
  dateTime.month  = SHIFT_STAMP(2, 0);
  dateTime.day    = SHIFT_STAMP(3, 0);
  dateTime.hour   = SHIFT_STAMP(4, 0) - 1;
  dateTime.minute = SHIFT_STAMP(5, 0) - 1;
  dateTime.second = SHIFT_STAMP(6, 0) - 1;

  if (!dateTime.year)
    dateTime.year = 2000;

  dateTime.fracSecond = 0;
  for (size_t i = 7; i < size; i++)
    dateTime.fracSecond |= SHIFT_STAMP(i, size - i - 1);
  dateTime.normalizeFracSeconds();
}

/**
 * @brief timestampToText
 *   Преобразование TIMESTAMP-ов к дате.
 *   Отрицательные TIMESTAMP (до нашей эры) конвертируются в null
 * @param result
 *  Выходной буфер токенов
 * @param value
 *  Буфер с преобразуемым значением
 * @param size
 *  Размер буфера value
 */
void timestampToText(string &result, const void *value, size_t size) {
  DateTime dateTime;
  timestampToObj(dateTime, value, size);

  if (dateTime.year >= 0)
    dateTime.toString(result);
  else
    result += "default";
}

void intervalToObj(DateTime &d, const void *value, size_t size) {
#define BYTE_SHIFT(pos, shift)                                                 \
  ((unsigned int)(((unsigned char *)(value))[pos]) << (shift * 8))
#define NORMALIZE_SHIFTED_INT(name, p)                                         \
  BYTE_SHIFT(p, 3) | BYTE_SHIFT(p + 1, 2) | BYTE_SHIFT(p + 2, 1) |             \
      BYTE_SHIFT(p + 3, 0);                                                    \
  name -= 0x80000000;
#define NORMALIZE_HMS(pos) (int)(((char *)(value))[pos]) - 60;

  if (size >= 11) {
    d.day        = NORMALIZE_SHIFTED_INT(d.day, 0);
    d.fracSecond = NORMALIZE_SHIFTED_INT(d.fracSecond, 7);
    d.hour       = NORMALIZE_HMS(4);
    d.minute     = NORMALIZE_HMS(5);
    d.second     = NORMALIZE_HMS(6);

    d.month = 1;

    d.normalizeFracSeconds();
    if (d.day >= 0 && d.fracSecond >= 0 && d.hour >= 0 && d.minute >= 0 &&
        d.second >= 0) {
      d.day += 1;
      d.year = 1;
    } else {
      d.day        = abs(d.day) + 1;
      d.hour       = abs(d.hour);
      d.fracSecond = abs(d.fracSecond);
      d.hour       = abs(d.hour);
      d.minute     = abs(d.minute);
      d.second     = abs(d.second);
      d.year       = 2;
    }
  } else if (size >= 5) {
    d.year  = NORMALIZE_SHIFTED_INT(d.year, 0);
    d.month = NORMALIZE_HMS(4);
    if (d.year >= 0 && d.month >= 0) {
      d.year += 1;
      d.day   = 1;
    } else {
      d.year  = abs(d.year) + 1;
      d.month = abs(d.month);
      d.day   = 2;
    }
  }
}

/**
 * @brief intervalDayToSecondToText
 *  Преобразование типа @code INTERVAL DAY TO SECOND @endcode
 *  в эквивалентный формат Линтер.
 *
 *  Тип INTERVAL DAY TO SECOND занимает 11 байт.
 *
 *  @code
 *  SQL> CREATE TABLE t1 (c1 INTERVAL DAY(5) TO SECOND(3));
 *  Table created.
 *  SQL> INSERT INTO t1 VALUES (TO_DSINTERVAL('2 10:20:30.456'));
 *  1 row created.
 *
 *  SQL> SELECT * FROM t1;
 *  C1
 *  ----------------------
 *  +00002 10:20:30.456
 *
 *  SQL> SELECT dump(c1) FROM t1;
 *  DUMP(C1)
 *  ---------------------------------------------
 *  Typ=183 Len=11: 128,0,0,2,70,80,90,155,46,2,0
 *                   0  1 2 3  4  5  6   7  8 9 10
 *  128,0,0,2  & 7F FF FF = 2  дня.
 *  70 - 60               = 10 часов.
 *  80 - 60               = 20 минут.
 *  90 - 60               = 30 секунд.
 *  155,46,2,0 & 7F FF FF = 456000000 наносекунд
 *
 *  @endcode
 *
 *  days + 2147483648
 *  hours + 60
 *  minutes + 60
 *  seconds + 60
 *  nanoseconds + 2147483648
 *
 * @param result
 *  Выходной буфер токенов
 * @param value
 *  Буфер с преобразуемым значением
 * @param size
 *  Размер буфера value
 */
void intervalToText(string &result, const void *value, size_t size) {
  if (size < 5)
    result += "default";
  else {
    DateTime d;
    intervalToObj(d, value, size);
    d.toString(result);
  }
}

void dateToObj(DateTime &d, const void *value, size_t sz) {
  struct OracleDate {
    unsigned char century, year, month, day;
    unsigned char hour, minute, second;
  } *date = (OracleDate *)value;
  if (sz == 7) {
    d.day    = date->day;
    d.month  = date->month;
    d.year   = int(date->century - 100) * 100 + (date->year - 100);
    d.hour   = date->hour - 1;
    d.minute = date->minute - 1;
    d.second = date->second - 1;
  }
}

void dateToText(string &result, const void *value, size_t sz) {
  DateTime d;
  dateToObj(d, value, sz);

  if (sz == 7)
    d.toString(result);
  else
    result += "default";
}

void clobToText(string &result, const void *, size_t) {
  result += "default"; // Не null, т.к. может быть ограничение IS NOT NULL
}

void blobToText(string &result, const void *, size_t) { result += "default"; }
