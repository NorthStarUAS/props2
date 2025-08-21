#ifdef HAVE_PYBIND11

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include "props2.h"

PYBIND11_MODULE(PropertyTree, m) {
    py::class_<SharedStateWrapper>(m, "SharedStateWrapper")
        ;

    py::class_<PropertyNode>(m, "PropertyNode")
        .def(py::init<>())
        .def(py::init<std::string>())
        .def(py::init<std::string, bool>())

        .def("get_shared_state", &PropertyNode::get_shared_state)
        .def("set_shared_state", &PropertyNode::set_shared_state)

        .def("hasChild", &PropertyNode::hasChild)
        .def("getChild", static_cast<PropertyNode (PropertyNode::*)(const char *)>(&PropertyNode::getChild))
        .def("getChild", static_cast<PropertyNode (PropertyNode::*)(const char *, bool)>(&PropertyNode::getChild))

        .def("isNull", &PropertyNode::isNull)
        .def("isParent", &PropertyNode::isParent)
        .def("isArray", &PropertyNode::isArray)
        .def("isValue", static_cast<bool (PropertyNode::*)(const char *)>(&PropertyNode::isValue))
        .def("isValue", static_cast<bool (PropertyNode::*)(const char *, unsigned int)>(&PropertyNode::isValue))

        .def("getLen", &PropertyNode::getLen)
        // .def("setLen", &PropertyNode::setLen)
        .def("getChildren", &PropertyNode::getChildren)
        // .def("getChildren", static_cast<vector<string> (PropertyNode::*)(bool)>(&PropertyNode::getChildren))

        .def("getBool", static_cast<bool (PropertyNode::*)(const char *)>(&PropertyNode::getBool))
        .def("getInt", static_cast<int (PropertyNode::*)(const char *)>(&PropertyNode::getInt))
        .def("getUInt", static_cast<unsigned int (PropertyNode::*)(const char *)>(&PropertyNode::getUInt))
        .def("getInt64", static_cast<int64_t (PropertyNode::*)(const char *)>(&PropertyNode::getInt64))
        .def("getUInt64", static_cast<uint64_t (PropertyNode::*)(const char *)>(&PropertyNode::getUInt64))
        .def("getDouble", static_cast<double (PropertyNode::*)(const char *)>(&PropertyNode::getDouble))
        .def("getString", static_cast<string (PropertyNode::*)(const char *)>(&PropertyNode::getString))

        // .def("getBool", static_cast<bool (PropertyNode::*)(const char *, unsigned int)>(&PropertyNode::getBool))
        .def("getInt", static_cast<int (PropertyNode::*)(const char *, unsigned int)>(&PropertyNode::getInt))
        // .def("getUInt", static_cast<unsigned int (PropertyNode::*)(const char *, unsigned int)>(&PropertyNode::getUInt))
        // .def("getInt64", static_cast<int64_t (PropertyNode::*)(const char *, unsigned int)>(&PropertyNode::getInt64))
        // .def("getUInt64", static_cast<uint64_t (PropertyNode::*)(const char *, unsigned int)>(&PropertyNode::getUInt64))
        .def("getDouble", static_cast<double (PropertyNode::*)(const char *, unsigned int)>(&PropertyNode::getDouble))
        .def("getString", static_cast<string (PropertyNode::*)(const char *, unsigned int)>(&PropertyNode::getString))

        .def("setBool", static_cast<bool (PropertyNode::*)(const char *, bool)>(&PropertyNode::setBool))
        .def("setInt", static_cast<bool (PropertyNode::*)(const char *, int)>(&PropertyNode::setInt))
        .def("setUInt", static_cast<bool (PropertyNode::*)(const char *, unsigned int)>(&PropertyNode::setUInt))
        .def("setInt64", static_cast<bool (PropertyNode::*)(const char *, int64_t)>(&PropertyNode::setInt64))
        .def("setUInt64", static_cast<bool (PropertyNode::*)(const char *, uint64_t)>(&PropertyNode::setUInt64))
        .def("setDouble", static_cast<bool (PropertyNode::*)(const char *, double)>(&PropertyNode::setDouble))
        .def("setString", static_cast<bool (PropertyNode::*)(const char *, string)>(&PropertyNode::setString))

        // .def("setBool", static_cast<bool (PropertyNode::*)(const char *, unsigned int)>(&PropertyNode::setBool))
        .def("setInt", static_cast<bool (PropertyNode::*)(const char *, int, unsigned int)>(&PropertyNode::setInt))
        .def("setUInt", static_cast<bool (PropertyNode::*)(const char *, unsigned int, unsigned int)>(&PropertyNode::setUInt))
        // .def("setInt64", static_cast<bool (PropertyNode::*)(const char *, unsigned int)>(&PropertyNode::setInt64))
        // .def("setUInt64", static_cast<bool (PropertyNode::*)(const char *, unsigned int)>(&PropertyNode::setUInt64))
        .def("setDouble", static_cast<bool (PropertyNode::*)(const char *, double, unsigned int)>(&PropertyNode::setDouble))
        .def("setString", static_cast<bool (PropertyNode::*)(const char *, string, unsigned int)>(&PropertyNode::setString))

        .def("load", &PropertyNode::load)
        .def("save", &PropertyNode::save)
        .def("pretty_print", &PropertyNode::pretty_print)
        .def("get_json_string", &PropertyNode::get_json_string)
        .def("set_json_string", &PropertyNode::set_json_string)
    ;
}

#endif // HAVE_PYBIND11
