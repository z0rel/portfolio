#include <string.h>
#include "ora2call.h"
#include "ora2text.h"
#include "constants.h"

#include "dlldefs.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "lintypes.h"
#include "decimals.h"
#include "tick.h"

#ifdef __cplusplus
}
#endif

using namespace std;

// typedef INT  (DLLAPI *StrConverter)(L_CHAR *S, DECIMAL	D);
typedef void (*Oracle2textConverter)(string &result, const void *value, size_t sz);

template <typename TYPE>
static inline bool verifyBufSize(size_t &bufsize, string &datesrc) {
  if (bufsize < (sizeof(TYPE) + 2))
    return false;

  datesrc.assign(0, '\0');
  return true;
}

static inline void writeLengthToBuf(char *&pbuf, size_t &bufsize, L_SWORD length) {
  L_WORD *len = (L_WORD *)pbuf;
  *len = (L_WORD)length;
  pbuf    += sizeof(L_WORD);
  bufsize -= sizeof(L_WORD);
}

template <typename TYPE>
static inline void writeLengthToBuf(char *&pbuf, size_t &bufsize) {
  writeLengthToBuf(pbuf, bufsize, sizeof(TYPE));
}

static inline bool toDecimals(char *&pbuf, size_t &bufsize, const void *value, size_t size) {
  static string datesrc;
  if (!verifyBufSize<L_DECIMAL>(bufsize, datesrc))
    return false;
  numberToText(datesrc, value, size);
  if (datesrc.empty())
    return nullToLinT(pbuf, bufsize, value, size);

  writeLengthToBuf<L_DECIMAL>(pbuf, bufsize);

  L_DECIMAL d;
  if (!STRTODEC((L_CHAR *)(datesrc.c_str()), d)) {
    // throw Ошибка преобразования даты
  }
  memcpy(pbuf, d, sizeof(L_DECIMAL));
  pbuf += sizeof(L_DECIMAL);
  bufsize -= sizeof(L_DECIMAL);
  return true;
}

template <typename TYPE>
static void numberToHost(TYPE &result, const void *value, size_t size) {
  int sign = ~(*(signed char *)value >> 7);
  int pow = (unsigned char)((*(unsigned char *)value ^ sign) << 1);
  int numSub = (101 & sign) + 1;

  size_t numLen;
  const unsigned char *numPtr;

  result = 0;

  // calc effective length
  numLen = size - 1;
  if (*((char *)value + numLen) == 102)
    numLen--;
  if (!numLen || *(unsigned char *)value == SIGN_MASK)
    // that is 0
    return;

  int num = 0;
  int leftNum = 0;

  pow -= SIGN_MASK;
  numPtr = (unsigned char *)value + 1;
  if (pow > 0) {
    // skip leading 0
    num = (*numPtr - numSub) ^ sign;
    leftNum = num / 10;
    num %= 10;
    if (!leftNum) {
      pow--;
      leftNum = num;
      numPtr++;
      numLen--;
    }
  }

  TYPE pow10 = 1;
  if (pow > 0)
    for (int i = 0; i < pow - 1; i++)
      pow10 *= 10;
  else
    for (int i = 0; i < -pow - 1; i++)
      pow10 /= 10;
  TYPE leftNumValue;

  // main loop
  do {
    if (pow & 1) {
      // skip tailing 0 after point
      if (pow < 0 && !num && !numLen)
        break;
    } else {
      num = (*numPtr - numSub) ^ sign;
      leftNum = num / 10;
      num %= 10;
      numPtr++;
      numLen--;
    }
    // put next digit

    leftNumValue = leftNum;
    leftNumValue *= pow10;
    result += leftNumValue;

    leftNum = num;
    pow--;
    pow10 /= 10;
  } while ((pow & 1) || numLen);

  // add sign
  if (sign)
    result = -result;
}

template <typename TYPE>
static inline bool writeHostToCall(char *&pbuf, size_t &bufsize, const void *value, size_t size) {
  if (bufsize < (sizeof(TYPE) + 2))
    return false;

  TYPE d = 0;
  numberToHost(d, value, size);
  writeLengthToBuf<TYPE>(pbuf, bufsize);
  memcpy(pbuf, &d, sizeof(TYPE));
  pbuf += sizeof(TYPE);
  bufsize -= sizeof(TYPE);
  return true;
}

bool nullToLinT(char *&pbuf, size_t &bufsize, const void *, size_t) {
  if (bufsize < 2)
    return false;
  writeLengthToBuf(pbuf, bufsize, -1);
  return true;
}

bool defaultToLinT(char *&pbuf, size_t &bufsize, const void *, size_t) {
  if (bufsize < 2)
    return false;
  writeLengthToBuf(pbuf, bufsize, -2);
  return true;
}

bool dateTimeToLinT(char *&pbuf, size_t &bufsize, DateTime &dt) {
  static const L_DLONG vectLength[] = {0l, // 0 разрядов
                                       1l,
                                       10l, // 2 разряда
                                       100l,       1000l,       10000l,
                                       100000l,    1000000l,    10000000l,
                                       100000000l, 1000000000l, 10000000000l};
  if (bufsize < (sizeof(L_DECIMAL) + 2))
    return false;

  L_LONG newageDays; /* количество дней от начала н.э. */
  L_LONG lastDayTicks; /* время в тиках в последнем дне */
  L_DECIMAL D;           /* дата во внутреннем формате */

  newageDays = DAYNUMBERDATE(dt.day, dt.month, dt.year);

  lastDayTicks =  dt.hour   * 360000; // в тиках
  lastDayTicks += dt.minute * 6000;
  lastDayTicks += dt.second * 100;

  if (dt.fracSecond && dt.leadZeros) {
    while (dt.fracSecond >= vectLength[dt.leadZeros + 1])
      dt.fracSecond /= 10;
    if (dt.leadZeros > 2 && dt.fracSecond >= vectLength[dt.leadZeros - 1])
      lastDayTicks += L_LONG(dt.fracSecond / vectLength[dt.leadZeros - 1]);
    else if (dt.leadZeros <= 2 && dt.fracSecond <= 99 && dt.fracSecond > 0)
      lastDayTicks += dt.fracSecond;
  }

  DATETOTICK(newageDays, lastDayTicks, D);

  writeLengthToBuf<L_DECIMAL>(pbuf, bufsize);

  memcpy(pbuf, D, sizeof(L_DECIMAL));
  pbuf    += sizeof(L_DECIMAL);
  bufsize -= sizeof(L_DECIMAL);
  return true;
}

bool dateToLinT(char *&pbuf, size_t &bufsize, const void *value, size_t size) { // -> date
  if (size != 7)
    return defaultToLinT(pbuf, bufsize, value, size);

  DateTime dt;
  dateToObj(dt, value, size);
  return dateTimeToLinT(pbuf, bufsize, dt);
}

bool intervalToLinT(char *&pbuf, size_t &bufsize, const void *value, size_t size) { // -> date
  if (size < 5)
    return defaultToLinT(pbuf, bufsize, value, size);
  DateTime d;
  intervalToObj(d, value, size);
  return dateTimeToLinT(pbuf, bufsize, d);
}

bool timestampToLinT(char *&pbuf, size_t &bufsize, const void *value, size_t size) { // -> date
  DateTime d;
  timestampToObj(d, value, size);
  if (d.year > 0)
    return dateTimeToLinT(pbuf, bufsize, d);
  else
    return defaultToLinT(pbuf, bufsize, value, size);
}

bool numToIntLinT(char *&pbuf, size_t &bufsize, const void *value, size_t size) { // -> int
  return writeHostToCall<L_LONG>(pbuf, bufsize, value, size);
}

bool numToRealLinT(char *&pbuf, size_t &bufsize, const void *value, size_t size) { // -> real
  return writeHostToCall<L_LONG>(pbuf, bufsize, value, size);
}

bool numToNumLinT(char *&pbuf, size_t &bufsize, const void *value, size_t size) { // -> decimals
  return toDecimals(pbuf, bufsize, value, size);
}

// Должен вызывать rawToCall и юзать функциональность поределения длины из
// ora2text
bool xmlToLinT(char *&pbuf, size_t &bufsize, const void *value, size_t size) {
  if (bufsize < 2)
    return false;

  int xmlOffset = extractXmlOffset(value, size);
  if (!xmlOffset)
    writeLengthToBuf(pbuf, bufsize, -1);
  else
    return rawToLinT(pbuf, bufsize, ((char *)value) + xmlOffset,
                     size - xmlOffset);
  return true;
}

// Все, что конвертится в DT_CHAR DT_BYTE DT_VARCHAR DT_VARBYTE DT_NCHAR
// DT_NVARCHAR
bool rawToLinT(char *&pbuf, size_t &bufsize, const void *value, size_t size) {
  if (bufsize < (size + 2))
    return false;
  writeLengthToBuf(pbuf, bufsize, size);

  memcpy(pbuf, value, size);
  pbuf += size;
  bufsize -= size;
  return true;
}

// Все, что конвертится в DT_CHAR DT_BYTE DT_VARCHAR DT_VARBYTE DT_NCHAR
// DT_NVARCHAR
bool varcharToLinT(char *&pbuf, size_t &bufsize, const void *value,
                   size_t size) {
  if (bufsize < (size + 2 + 2))
    return false;

  writeLengthToBuf(pbuf, bufsize, size + 2);
  memset(pbuf, 0, 2);
  pbuf += 2;
  bufsize -= 2;

  memcpy(pbuf, value, size);
  pbuf += size;
  bufsize -= size;
  return true;
}

bool blobToLinT(char *&pbuf, size_t &bufsize, const void *, size_t) {
  return defaultToLinT(pbuf, bufsize, 0, 0);
}

bool clobToLinT(char *&pbuf, size_t &bufsize, const void *, size_t) {
  return defaultToLinT(pbuf, bufsize, 0, 0);
}

bool stubToLinT(char *&pbuf, size_t &bufsize, const void *, size_t) {
  return defaultToLinT(pbuf, bufsize, 0, 0);
}
