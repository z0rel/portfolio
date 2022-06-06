#ifndef LEX_LOCATION_HH
#define LEX_LOCATION_HH


#include <cstdint>
#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <string.h>
#include <limits>


#define CL_POSITION_ADD_COLUMNS(column, bPos, count)    (column) += (count); (bPos) += (count)
#define CL_POSITION_ADD_COLUMNS1(column, bPos)       (++(column)); (++(bPos))
#define CL_POSITION_ADD_COLUMNSN(column, bPos, N)       (column) += N; (bPos) += N;

#define CL_POSITION_LINES_O(obj, count, b)           ((obj).line) += (count); (obj).column = 1; (obj).bytePosition += (b)
#define CL_POSITION_LINES(count, b)                         line  += (count); column = 1; bytePosition += b;
#define CL_POSITION_LINES_O1_NO_BYTES(obj)           ++((obj).line);    (obj).column = 1;
#define CL_POSITION_LINES_O1(obj)                    ++((obj).line);    (obj).column = 1; ++(obj).bytePosition

#define CL_EMPTY_LOCATION { { 0, 0, 0}, { 0, 0, 0 }  }
#define CL_INIT_LOCATION  { {1, 1, 0}, {1, 1, 0} }
#define CL_PROCENDL_LOCATION  \
  { \
    { \
      std::numeric_limits<cl::position::Line>::max(), \
      std::numeric_limits<uint32_t>::max(), \
      std::numeric_limits<uint32_t>::max() \
    }, \
    {0, 0, 0}  \
  }


extern std::string  *globalCurrentFile;


namespace cl {
  class filelocation;
}

namespace smart_lexer {
  std::string textFromFile(const cl::filelocation &floc);
}


namespace cl {


/// Abstract a position.
class position
{
public:
  typedef uint32_t Line;
  typedef uint32_t Column;
  typedef uint32_t BytePosition;


  /// Initialization.
  inline void initialize () { *this = {1, 1, 0}; }

  /// (line related) Advance to the COUNT next lines.
  inline void lines(Line count, BytePosition byteOffset) { CL_POSITION_LINES(count, byteOffset); }

  /// (column related) Advance to the COUNT next columns.
  inline void columns(int count = 1) { CL_POSITION_ADD_COLUMNS(column, bytePosition, count); }

  /// Сравнить два объекта position
  inline bool operator== (position pos2) const { return !memcmp(this, &pos2, sizeof(position)); }
  inline bool operator!= (position pos2) const { return  memcmp(this, &pos2, sizeof(position)); }

public:
  /// Номер текущей строки
  Line         line;
  /// Номер текущего столбца
  Column       column;
  BytePosition bytePosition;
};

inline std::ostream& operator<< (std::ostream& ostr, const position& pos) {
    return ostr << pos.line << ',' << pos.column;
}

class filelocation;

/// Абстракция размещения
class location
{
public:
  /// Инициализация
  inline void initialize() { *this = CL_INIT_LOCATION; }

  inline void initProcendl() { *this = CL_PROCENDL_LOCATION; }

  inline bool beginedFrom(uint32_t line) const { return begin.line == line; }
  inline bool beginedFrom(uint32_t line, uint32_t col) const { return begin.line == line && begin.column == col; }
  inline bool beginedFrom(cl::position pos) const { return begin == pos; }

  inline void step0() { begin = end; }
  inline void step1() { begin = end; CL_POSITION_ADD_COLUMNS1(end.column, end.bytePosition); }
  inline void step2() { begin = end; CL_POSITION_ADD_COLUMNSN(end.column, end.bytePosition, 2); }
  inline void step(unsigned int count) { memcpy(&begin, &end, sizeof(position)); CL_POSITION_ADD_COLUMNS(end.column, end.bytePosition, count); }

  /// Расширить текущее размещение на COUNT следующих столбцов.
  inline void columns () { CL_POSITION_ADD_COLUMNS1(end.column, end.bytePosition); }
  inline void columns2() { CL_POSITION_ADD_COLUMNSN(end.column, end.bytePosition, 2); }
  inline void columns3() { CL_POSITION_ADD_COLUMNSN(end.column, end.bytePosition, 3); }
  inline void columns7() { CL_POSITION_ADD_COLUMNSN(end.column, end.bytePosition, 7); }
  inline void columns (unsigned int count) { CL_POSITION_ADD_COLUMNS(end.column, end.bytePosition, count); }

  /// Расшиendь текущее размещение на COUNT следующих строк.
  inline void lines () { CL_POSITION_LINES_O1(end); }
  inline void lines_no_bytes() { CL_POSITION_LINES_O1_NO_BYTES(end); }
  inline void lines (uint32_t count, uint32_t bytes) { CL_POSITION_LINES_O(end, count, bytes); }
  inline void bytes() { ++(end.bytePosition); }

  /// cравнить два объекта location.
  inline bool operator== (const location& loc2) { return !memcmp(this, &loc2, sizeof(location)); }
  inline bool operator!= (const location& loc2) { return  memcmp(this, &loc2, sizeof(location)); }

public:
  /// Начало размещенного региона.
  position begin;   /// Конец  размещенного региона.
  position end;

  inline operator filelocation() const;
}; // end of location

inline location operator+ (const location &lhs, const location& rhs) {
   return { lhs.begin, rhs.end };
}

inline std::ostream& operator<< (std::ostream& ostr, const location& loc)
{
  position last = loc.end;
  if (last.column > 1)
   last.column -= 1;
  ostr << loc.begin;
  if (loc.begin.line != last.line)
    ostr << '-' << last.line  << ',' << last.column;
  else if (loc.begin.column != last.column)
    ostr << '-' << last.column;
  return ostr;
}


class filelocation {
public:
  inline bool beginedFrom(int line)          const { return loc.beginedFrom(line); }
  inline bool beginedFrom(int line, int col) const { return loc.beginedFrom(line, col); }
  inline bool beginedFrom(cl::position pos)  const { return loc.beginedFrom(pos); }

  inline std::string toString() const;

  bool empty() const { return loc.begin.line && !loc.begin.column && !loc.end.line && loc.end.column; }

  inline std::string text(const std::string &s) const {
    return s.substr(loc.begin.bytePosition, loc.end.bytePosition - loc.begin.bytePosition);
  }

  inline std::string textUp(const std::string &s) const {
    std::string o = text(s);
    std::transform(o.begin(), o.end(), o.begin(), ::toupper);
    return o;
  }

  inline std::string locText() const;
  inline std::string locTextNoEndl() const;

  inline std::string textFromFile() const { return smart_lexer::textFromFile(*this); }

  location loc; // = emptyLocation();
  const std::string *file ; //= 0;
};


inline constexpr filelocation emptyFLocation() { return { CL_EMPTY_LOCATION, 0 }; }

inline  location::operator filelocation() const {
  return {*this, 0};
}


inline std::ostream& operator<< (std::ostream& ostr, const filelocation& loc) {
  const std::string *s = loc.file;
  if (s)
    ostr << *s << ':';
  ostr << loc.loc;
  return ostr;
}

inline std::string filelocation::toString() const {
  std::stringstream str;
  str << *this;
  return str.str();
}


inline std::string filelocation::locText() const {
  std::stringstream str;
  str << *this << ": " << this->textFromFile();
  return str.str();
}

inline std::string::value_type toNoEndl(std::string::value_type ch) {
  if (ch == '\n' || ch == '\r')
    return ' ';
  return ch;
}


inline std::string filelocation::locTextNoEndl() const {
  std::string res = locText();
  std::transform(res.begin(), res.end(), res.begin(), cl::toNoEndl);
  return res;
}

}

#define LEX_STRINGBUFFER_ADD_CHAR(buf, c) *buf.nextPos_++ = c;

class LexStringBuffer {
public:
  char* nextPos_;
private:
  char* p;
  char* endPos_;
  int capacity_;


  void growth() {
    capacity_ *= 2;
    char* p2 = new char[capacity_];
    memcpy(p2, p, nextPos_ - p);
    delete[] p;
    p = p2;
  }
public:
  LexStringBuffer()
    : capacity_(65535)
  {
    p = new char[capacity_];
    nextPos_ = p;
    endPos_ = p + capacity_;
  }

  inline void push_back(int c) {
    if (nextPos_ == endPos_)
      growth();
    *nextPos_++ = c;
  }

  inline void clear() { nextPos_ = p; }

  inline char* ptr() const { return p; }
  inline char* end() const { return endPos_; }
  inline size_t size() const { return nextPos_ - p; }

  ~LexStringBuffer() {
    if (p)
      delete[] p;
  }
};


#undef CL_EMPTY_LOCATION
#undef CL_INIT_LOCATION
#undef CL_PROCENDL_LOCATION
#undef CL_POSITION_ADD_COLUMNS
#undef CL_POSITION_ADD_COLUMNS1
#undef CL_POSITION_ADD_COLUMNSN

#undef CL_POSITION_LINES_O
#undef CL_POSITION_LINES
#undef CL_POSITION_LINES_O1
#undef CL_POSITION_LINES_O1_NO_BYTES

#endif // LEX_LOCATION_HH
