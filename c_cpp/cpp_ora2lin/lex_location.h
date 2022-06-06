#ifndef LEX_LOCATION_HH
# define LEX_LOCATION_HH


#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>

#include "lex_position.h"

#include "project_optimization.h"
COMPILER_LOCATIONS_OPTIMIZATATION_PUSH()


#define CL_EMPTY_LOCATION { { 0, 0, 0}, { 0, 0, 0 }  }
#define CL_INIT_LOCATION  { {1, 1, 0}, {1, 1, 0} }
#define CL_PROCENDL_LOCATION  { {std::numeric_limits<cl::position::Line>::max(), \
  std::numeric_limits<uint32_t>::max(), \
  std::numeric_limits<uint32_t>::max()}, {0, 0, 0} }

extern std::string  *globalCurrentFile;


namespace cl {
  class filelocation;
}

namespace SmartLexer {
  std::string textFromFile(const cl::filelocation &floc);
  std::string fullFilename(const cl::filelocation &floc);
  std::string fullFilename(const std::string *floc);
}


namespace cl {

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

  /// Сбросить начальное размещение в конечное мещение
  inline void step() { begin = end; }

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


inline constexpr location emptyLocation() { return CL_EMPTY_LOCATION; }

class filelocation {
public:
  location loc; // = emptyLocation();
  const std::string *file ; //= 0;

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

  inline std::string textFromFile() const { return SmartLexer::textFromFile(*this); }
  inline std::string fullFilename() const { return SmartLexer::fullFilename(*this); }
  static inline std::string fullFilename(const std::string *file) { return SmartLexer::fullFilename(file); }
};


inline  location::operator filelocation() const {
  return {*this, globalCurrentFile};
}


inline filelocation fLoc(location l) { return {l, globalCurrentFile};  }
inline constexpr filelocation fLoc(location l, const std::string *f) { return {l, f};  }

inline constexpr filelocation emptyFLocation() { return { CL_EMPTY_LOCATION, 0 }; }

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



COMPILER_LOCATIONS_OPTIMIZATATION_POP()

#endif // LEX_LOCATION_HH
// vim:foldmethod=marker:foldmarker={,}

