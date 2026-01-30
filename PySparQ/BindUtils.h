#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/complex.h"
#include "pybind11/functional.h"
#include "pybind11/operators.h"
#include "pybind11/numpy.h"

#include "sparse_state_simulator.h"

using namespace pybind11::literals;
namespace py = pybind11;
using namespace qram_simulator;

using namespace std;

// Bind ClassControllable classes extra methods and attributes
#define BIND_CONTROLLABLE_METHODS(CLASS_NAME)                                                                                                                               \
    .def_readonly("condition_variable_nonzeros", &CLASS_NAME::condition_variable_nonzeros)                                                                                  \
        .def_readonly("condition_variable_all_ones", &CLASS_NAME::condition_variable_all_ones)                                                                              \
        .def_readonly("condition_variable_by_value", &CLASS_NAME::condition_variable_by_value)                                                                              \
        .def_readonly("condition_variable_by_bit", &CLASS_NAME::condition_variable_by_bit) /* conditioned_by_nonzeros */                                                    \
        .def("conditioned_by_nonzeros", static_cast<CLASS_NAME &(CLASS_NAME::*)(std::string_view)>(&CLASS_NAME::conditioned_by_nonzeros),                                   \
             py::arg("cond"), py::return_value_policy::reference_internal)                                                                                                  \
        .def("conditioned_by_nonzeros", static_cast<CLASS_NAME &(CLASS_NAME::*)(const std::vector<std::string_view> &)>(&CLASS_NAME::conditioned_by_nonzeros),              \
             py::arg("conds"), py::return_value_policy::reference_internal)                                                                                                 \
        .def("conditioned_by_nonzeros", static_cast<CLASS_NAME &(CLASS_NAME::*)(size_t)>(&CLASS_NAME::conditioned_by_nonzeros),                                             \
             py::arg("cond"), py::return_value_policy::reference_internal)                                                                                                  \
        .def("conditioned_by_nonzeros", static_cast<CLASS_NAME &(CLASS_NAME::*)(const std::vector<size_t> &)>(&CLASS_NAME::conditioned_by_nonzeros),                        \
             py::arg("conds"), py::return_value_policy::reference_internal) /* conditioned_by_all_ones */                                                                   \
        .def("conditioned_by_all_ones", static_cast<CLASS_NAME &(CLASS_NAME::*)(std::string_view)>(&CLASS_NAME::conditioned_by_all_ones),                                   \
             py::arg("cond"), py::return_value_policy::reference_internal)                                                                                                  \
        .def("conditioned_by_all_ones", static_cast<CLASS_NAME &(CLASS_NAME::*)(const std::vector<std::string_view> &)>(&CLASS_NAME::conditioned_by_all_ones),              \
             py::arg("conds"), py::return_value_policy::reference_internal)                                                                                                 \
        .def("conditioned_by_all_ones", static_cast<CLASS_NAME &(CLASS_NAME::*)(size_t)>(&CLASS_NAME::conditioned_by_all_ones),                                             \
             py::arg("cond"), py::return_value_policy::reference_internal)                                                                                                  \
        .def("conditioned_by_all_ones", static_cast<CLASS_NAME &(CLASS_NAME::*)(const std::vector<size_t> &)>(&CLASS_NAME::conditioned_by_all_ones),                        \
             py::arg("conds"), py::return_value_policy::reference_internal) /* conditioned_by_bit */                                                                        \
        .def("conditioned_by_bit", static_cast<CLASS_NAME &(CLASS_NAME::*)(std::string_view, size_t)>(&CLASS_NAME::conditioned_by_bit),                                     \
             py::arg("cond"), py::arg("pos"), py::return_value_policy::reference_internal)                                                                                  \
        .def("conditioned_by_bit", static_cast<CLASS_NAME &(CLASS_NAME::*)(const std::vector<std::pair<std::string_view, size_t>> &)>(&CLASS_NAME::conditioned_by_bit),     \
             py::arg("conds"), py::return_value_policy::reference_internal)                                                                                                 \
        .def("conditioned_by_bit", static_cast<CLASS_NAME &(CLASS_NAME::*)(size_t, size_t)>(&CLASS_NAME::conditioned_by_bit),                                               \
             py::arg("cond"), py::arg("pos"), py::return_value_policy::reference_internal)                                                                                  \
        .def("conditioned_by_bit", static_cast<CLASS_NAME &(CLASS_NAME::*)(const std::vector<std::pair<size_t, size_t>> &)>(&CLASS_NAME::conditioned_by_bit),               \
             py::arg("conds"), py::return_value_policy::reference_internal) /* conditioned_by_value */                                                                      \
        .def("conditioned_by_value", static_cast<CLASS_NAME &(CLASS_NAME::*)(std::string_view, size_t)>(&CLASS_NAME::conditioned_by_value),                                 \
             py::arg("cond"), py::arg("pos"), py::return_value_policy::reference_internal)                                                                                  \
        .def("conditioned_by_value", static_cast<CLASS_NAME &(CLASS_NAME::*)(const std::vector<std::pair<std::string_view, size_t>> &)>(&CLASS_NAME::conditioned_by_value), \
             py::arg("conds"), py::return_value_policy::reference_internal)                                                                                                 \
        .def("conditioned_by_value", static_cast<CLASS_NAME &(CLASS_NAME::*)(size_t, size_t)>(&CLASS_NAME::conditioned_by_value),                                           \
             py::arg("cond"), py::arg("pos"), py::return_value_policy::reference_internal)                                                                                  \
        .def("conditioned_by_value", static_cast<CLASS_NAME &(CLASS_NAME::*)(const std::vector<std::pair<size_t, size_t>> &)>(&CLASS_NAME::conditioned_by_value),           \
             py::arg("conds"), py::return_value_policy::reference_internal) /* clear methods */                                                                             \
        .def("clear_control_nonzeros", &CLASS_NAME::clear_control_nonzeros)                                                                                                 \
        .def("clear_control_all_ones", &CLASS_NAME::clear_control_all_ones)                                                                                                 \
        .def("clear_control_by_bit", &CLASS_NAME::clear_control_by_bit)                                                                                                     \
        .def("clear_control_by_value", &CLASS_NAME::clear_control_by_value)

#define BIND_BASE_OPERATOR(NAME) \
    py::class_<NAME, BaseOperator>(m, #NAME)

/* Workaround for binding inherited operator() of SelfAdjointOperator
*
    Author: Agony5757
    Date:   2025/2/26

   This is used to solve the issue for Windows platform which has a trouble if
   calling operator() in Python. However, the underlying mechanism is unclear
   to me, so I just use a workaround to avoid the issue. */
#ifdef _WIN32
#define BIND_SELF_ADJOINT_OPERATOR(NAME)            \
    py::class_<NAME, SelfAdjointOperator>(m, #NAME) \
        .def("__call__", (void (NAME::*)(SparseState &) const) & NAME::operator(), py::arg("state"))
#else
#define BIND_SELF_ADJOINT_OPERATOR(NAME) \
    py::class_<NAME, SelfAdjointOperator>(m, #NAME)
#endif

#define BIND_BASE_OPERATOR_SUBNAME(NAME, PYNAME) \
    py::class_<NAME, BaseOperator>(m, #PYNAME)

#define BIND_DAG_METHODS(NAME) \
    .def("dag", (void (NAME::*)(SparseState &) const) & NAME::dag, py::arg("state"))
