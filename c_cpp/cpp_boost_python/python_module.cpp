#include <pybind11/pybind11.h>

#include "src/compiler/python_model.h"


namespace python_model {

namespace py = pybind11;

PYBIND11_MODULE(behrens_gparser, m) {
    m.doc() = "behrens_gparser module"; // optional module docstring

    m.def("parse_file", &parse_file, "", py::arg("fname"));
    m.def("init_globals", &init_globals);

    py::class_<cl::position>(m, "position")
        .def_readwrite("line", &cl::position::line)
        .def_readwrite("column", &cl::position::column)
        .def_readwrite("bytePosition", &cl::position::bytePosition)
    ;
    py::class_<cl::location>(m, "location")
        .def_readwrite("begin", &cl::location::begin)
        .def_readwrite("end", &cl::location::end)
    ;
    py::class_<cl::filelocation>(m, "filelocation")
        .def_readwrite("loc", &cl::filelocation::loc)
        .def("__str__", &cl::filelocation::toString)
        .def("__format__", &format_lloc)
    ;

    py::class_<flpx::GrammarBase>(m, "GrammarBase")
        .def("lloc", &flpx::GrammarBase::getLLoc, py::return_value_policy::reference)
        .def("loc", &flpx::GrammarBase::loc, "", py::arg("l"))
        .def("ltext", &text_from_file)
    ;

    py::class_<NumericValue, flpx::GrammarBase>(m, "NumericValue")
        .def_readwrite("val", &NumericValue::val)
        .def("copy", &NumericValue::copy)
    ;
    py::class_<XYJ_item, flpx::GrammarBase>(m, "XYJ_item")
        .def_readwrite("cat", &XYJ_item::cat)
        .def_readwrite("val", &XYJ_item::val)
        .def_readwrite("comment", &XYJ_item::comment)
        .def_readwrite("comment_empty", &XYJ_item::comment_empty)
        .def_readonly("has_xyj_conversion", &XYJ_item::has_xyj_conversion)
        .def("copy", &XYJ_item::copy)
    ;
    py::class_<Cadr, flpx::GrammarBase>(m, "Cadr")
        .def_readwrite("num", &Cadr::num)
        .def_readwrite("body", &Cadr::body)
        .def_readwrite("has_number", &Cadr::has_number)
        .def_readonly("skip_m", &Cadr::skip_m)
        .def("copy", &Cadr::copy)
    ;
    py::class_<Program, flpx::GrammarBase>(m, "Program")
        .def("comment_load", &comment_load)
        .def_readwrite("comment_empty", &Program::comment_empty)
        .def_readwrite("commands", &Program::commands)
    ;

}


} // end of namespace python model
