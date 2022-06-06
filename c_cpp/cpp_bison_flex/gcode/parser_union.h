#ifndef PARSER_UNION_H
#define PARSER_UNION_H

#include <string>
#include <memory>
#include <vector>
#include <list>
#include <initializer_list>

#define YYDEBUG 1

#include "src/compiler/lex_location.h"
#include "src/compiler/smartptr.h"


namespace flpx  {

typedef cl::filelocation FLoc;
typedef const cl::filelocation& CLoc;

template <typename T> class List;
class XYJ_item;

typedef List<smart::Ptr<XYJ_item> > XYJ_ENTRIES;

class GrammarBase {
  cl::filelocation lloc = cl::emptyFLocation();
public:
  inline void loc(const FLoc & l) { lloc = l; }
  inline CLoc getLLoc() const { return lloc; }

  GrammarBase() {}
  GrammarBase(CLoc l) : lloc(l) {}
  GrammarBase(const GrammarBase &o) : lloc(o.lloc) {}
  inline bool beginedFrom(uint32_t line, uint32_t column) const { return lloc.beginedFrom(line, column); }
  inline bool beginedFrom(uint32_t line) const { return lloc.beginedFrom(line); }

  void setLLoc(const FLoc &l) { lloc = l; }
};


class String : public smart::Smart, public std::string {
protected:
public:
  inline String()                           : std::string()                {}
  inline String(const char *str)            : std::string(str)             {}
  inline String(const char *str, size_t sz) : std::string(str, sz)         {}
  inline String(size_t count, char c)       : std::string(count, c)        {}
  inline String(const std::string &str)     : std::string(str)             {}
  inline String(const String &str)          : smart::Smart(), std::string(str) {}

  inline const String &operator=(const String &str) { std::string::operator=(str); return *this; }
};


class GrammarBaseSmart : public virtual smart::Smart, public GrammarBase {
protected:
public:
    GrammarBaseSmart() {}
    GrammarBaseSmart(CLoc l) : GrammarBase(l) {}
    GrammarBaseSmart(const GrammarBaseSmart & o) : Smart(o), GrammarBase(o) {}
    inline virtual ~GrammarBaseSmart() {}
};


template <typename Element>
class List : public GrammarBaseSmart, public std::list<Element>
{
public:
  inline List() {}
  inline List(const List& o) : Smart(o), std::list<Element>(o) {}
  inline List(const Element &x) : std::list<Element>(1, x) {}
  inline List(CLoc l, const Element &x) : Smart(), GrammarBaseSmart(l), std::list<Element>(1, x) {}

  List(std::initializer_list<smart::Ptr<Element> > lst) : std::list<Element>(lst) {}
  List(CLoc l, std::initializer_list<smart::Ptr<Element> > lst)
      : Smart(), GrammarBaseSmart(l), std::list<Element>(lst) {}
};


template <typename T>
inline List<smart::Ptr<T> >* mkList(CLoc l, T *first) {
    return new List<smart::Ptr<T> >(l, first);
}


class NumericValue      : public GrammarBaseSmart
{ /* Числовое (целое и дробное) значение */
public:
    smart::Ptr<flpx::String> val;
    int i_value = 0;
    double d_value = 0.0;
    bool is_ivalue = true;

    NumericValue() {}
    NumericValue(CLoc l, const char* str, size_t len) : GrammarBaseSmart(l), val(new flpx::String(str, len)) {}
    NumericValue(const NumericValue& o) : Smart(o), GrammarBaseSmart(o), val(o.val) {}
    NumericValue(int value) : i_value(value) {}

    virtual inline ~NumericValue() {}
};


class Cadr;

class Program : public GrammarBaseSmart  {
public:
    smart::Ptr<String> comment;
    typedef flpx::List<smart::Ptr<flpx::Cadr> > Commands;
    smart::Ptr<Commands> commands;

    inline Program() {}
    inline Program(CLoc l) : GrammarBaseSmart(l) {}
    inline Program(const Program& o) : Smart(o), GrammarBaseSmart(o), comment(o.comment), commands(o.commands) {}

    inline Program(CLoc l, smart::Ptr<String > comment_, Commands *commands_)
        : GrammarBaseSmart(l), comment(comment_), commands(commands_) {}
    virtual inline ~Program() {}
};


class ParsingContext {
public:
    smart::Ptr<Program> program;
};


class Cadr : public GrammarBaseSmart {
public:
    smart::Ptr<NumericValue> num;
    smart::Ptr<XYJ_ENTRIES> body;

    inline Cadr() {}
    inline Cadr(CLoc l) : GrammarBaseSmart(l) {}
    inline Cadr(const Cadr& o) : Smart(o), GrammarBaseSmart(o), num(o.num), body(o.body) {}
    virtual inline ~Cadr() {}

    inline Cadr(CLoc l, smart::Ptr<NumericValue> num_, smart::Ptr<XYJ_ENTRIES> body_)
        : GrammarBaseSmart(l), num(num_), body(body_) {}
};

class XYJ_item : public GrammarBaseSmart {
public:
    enum XYJ_cathegory {
        CMD_X,
        CMD_Y,
        CMD_H,
        CMD_F,
        CMD_S,
        CMD_E,
        CMD_I,
        CMD_J,
        CMD_G,
        CMD_T,
        CMD_R,
        CMD_TR,
        CMD_M,
        CMD_L,
        CMD_PERCENT
    };
    enum XYJ_cathegory cat;
    smart::Ptr<NumericValue> val;
    smart::Ptr<String> comment;


    inline XYJ_item() {}
    inline XYJ_item(CLoc l) : GrammarBaseSmart(l) {}
    inline XYJ_item(const XYJ_item& o)
        : Smart(o), GrammarBaseSmart(o), cat(o.cat), val(o.val), comment(o.comment) {}
    virtual inline ~XYJ_item() {}

    inline XYJ_item(CLoc l,
                    enum XYJ_cathegory cat_,
                    smart::Ptr<NumericValue> val_,
                    flpx::String *comment)
        : GrammarBaseSmart(l), cat(cat_), val(val_), comment(comment) {}


    inline const char* cathegory_to_string() const {
        switch (cat) {
            case CMD_X: return "X";
            case CMD_Y: return "Y";
            case CMD_H: return "H";
            case CMD_F: return "F";
            case CMD_S: return "S";
            case CMD_E: return "E";
            case CMD_I: return "I";
            case CMD_J: return "J";
            case CMD_G: return "G";
            case CMD_T: return "T";
            case CMD_R: return "R";
            case CMD_TR: return "TR";
            case CMD_M: return "M";
            case CMD_L: return "L";
            case CMD_PERCENT: return "%";
            default: return "UNKNOWN";
        }
    }
};


} // namespace flpx



namespace cl {

union semantic_type {
  flpx::String          *comment;
  flpx::NumericValue    *numval;
  flpx::Program         *program;
  flpx::Cadr            *cadr;
  flpx::XYJ_item        *xyj_item;
  flpx::XYJ_ENTRIES     *xyj_entries;
  flpx::List<smart::Ptr<flpx::Cadr> > *cadres_list;
};

} // namespace cl


#endif // PARSER_UNION_H
