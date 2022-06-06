#ifndef LEX_POSITION_HH
#define LEX_POSITION_HH

#include <iostream>
#include <string>
#include <string.h>
#include <algorithm>
#include <limits>

#include "project_optimization.h"
COMPILER_LOCATIONS_OPTIMIZATATION_PUSH()

namespace cl {

#define CL_POSITION_ADD_COLUMNS(column, bPos, count)    (column) += (count); (bPos) += (count)
#define CL_POSITION_ADD_COLUMNS1(column, bPos)       (++(column)); (++(bPos))
#define CL_POSITION_ADD_COLUMNSN(column, bPos, N)       (column) += N; (bPos) += N;

#define CL_POSITION_LINES_O(obj, count, b)           ((obj).line) += (count); (obj).column = 1; (obj).bytePosition += (b)
#define CL_POSITION_LINES(count, b)                         line  += (count); column = 1; bytePosition += b;
#define CL_POSITION_LINES_O1(obj)                    ++((obj).line);    (obj).column = 1; ++(obj).bytePosition

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
  } __attribute__((packed));

  inline std::ostream& operator<< (std::ostream& ostr, const position& pos) { return ostr << pos.line << ',' << pos.column; }
  inline constexpr cl::position emptyPosition() { return { 0, 0, 0 }; }
  inline constexpr cl::position initPosition()  { return { 1, 1, 0 }; }
} // cl


COMPILER_LOCATIONS_OPTIMIZATATION_POP()

#endif // not LEX_POSITION_HH
// vim:foldmethod=marker:foldmarker={,}
