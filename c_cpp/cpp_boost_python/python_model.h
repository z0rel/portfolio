#ifndef PYTHON_MODEL_H
#define PYTHON_MODEL_H

#include <vector>

#include "src/compiler/parser_union.h"
#include <pybind11/pybind11.h>


namespace python_model {

using str11 = pybind11::str;
using list11 = pybind11::list;
using memoryview11 = pybind11::memoryview;


class SyntaxErrorException : public std::exception {
public:
    explicit SyntaxErrorException(const char * m) : message(m) {}
    virtual const char * what() const noexcept override {return message.c_str();}
private:
    std::string message = "";
};


/* Числовое (целое и дробное) значение */
class NumericValue      : public flpx::GrammarBase
{
public:
    str11 val;

    NumericValue() {}
    NumericValue(const NumericValue &o) : flpx::GrammarBase(o), val(o.val) {}
    NumericValue copy() const { return NumericValue(*this); }
};

inline void build_numeric_value(NumericValue &dst, smart::Ptr<flpx::NumericValue> o) {
   if (o && o->val) {
       dst.setLLoc(o->getLLoc());
       dst.val = str11(o->val->c_str());
   }
};

class XYJ_item : public flpx::GrammarBase {
public:
    str11 cat;
    NumericValue val;
    str11 comment;
    bool comment_empty = true;
    const bool has_xyj_conversion = true;


    XYJ_item() {}
    XYJ_item(const XYJ_item &o)
        : flpx::GrammarBase(o),
          cat(o.cat),
          val(o.val), comment(o.comment), comment_empty(o.comment_empty) {}
    XYJ_item copy() const { return XYJ_item(*this); }
};

inline void build_xyj_item(XYJ_item &dst, smart::Ptr<flpx::XYJ_item> o) {
    if (o) {
        dst.setLLoc(o->getLLoc());
        dst.cat = o->cathegory_to_string();
        build_numeric_value(dst.val, o->val);
        if (o->comment) {
            dst.comment = o->comment->c_str();
            dst.comment_empty = false;
        }
    }
}


class Cadr : public flpx::GrammarBase {
public:
    NumericValue num;
    list11 body;
    bool has_number = true;
    bool skip_m = false;

    Cadr() {}
    Cadr(const Cadr &o)
        : flpx::GrammarBase(o), num(o.num), body(o.body), has_number(o.has_number) {}


    Cadr copy() const {
        list11 new_body;

        Cadr res = Cadr(*this);
        for (auto &it : res.body) {
            XYJ_item item = it.cast<XYJ_item>();
            new_body.append(item.copy());
        }

        res.body = new_body;
        return res;
    }
};


inline void build_cadr(Cadr &dst, smart::Ptr<flpx::Cadr> o) {
    if (o) {
        dst.setLLoc(o->getLLoc());
        build_numeric_value(dst.num, o->num);
        if (!o) {
            dst.has_number = false;
        }
        if (o->body) {
            for (flpx::XYJ_ENTRIES::iterator it = o->body->begin(); it != o->body->end(); ++it) {
                XYJ_item item;
                build_xyj_item(item, *it);
                dst.body.append(item);
            }
        }
    }
}


class Program : public flpx::GrammarBase {
public:
    std::string comment;
    list11 commands;
    bool comment_empty = true;
};


inline memoryview11 comment_load(Program& self) {
  //now you wrap that as buffer
    char *data = (char*)(self.comment.c_str());
    Py_ssize_t dataSize = self.comment.length();

    memoryview11 memoryView(pybind11::buffer_info(data, dataSize));
    return memoryView;
}



inline void build_program(Program &dst, smart::Ptr<flpx::Program> o) {
    if (o) {
        dst.setLLoc(o->getLLoc());
        if (o->comment) {
            dst.comment = o->comment->c_str();
            dst.comment_empty = false;
        }
        if (o->commands) {
            for (flpx::Program::Commands::iterator it = o->commands->begin(); it != o->commands->end(); ++it) {
                Cadr cadr;
                build_cadr(cadr, *it);
                dst.commands.append(cadr);
            }
        }
    }
}


Program parse_file(std::string fname);
void init_globals();
std::string text_from_file(const flpx::GrammarBase &self, std::string fname);

inline std::string format_lloc(const cl::filelocation &self, const std::string &/*fmt_spec*/) {
    return self.toString();
}





} // end of namespace python_model

#endif // PYTHON_MODEL_H
