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

// ============================================================================
// Shared docstrings for controllable methods (used by 38+ operators)
// ============================================================================
namespace pysparq_docs {

inline constexpr const char* DOC_CONDITIONED_BY_NONZEROS =
    "Condition this operation on registers with nonzero values.\n\n"
    "Args:\n"
    "    cond: Register name (str) or ID (int) to condition on.\n"
    "    conds: List of register names or IDs for multi-condition.\n\n"
    "Returns:\n"
    "    Self, for method chaining.\n\n"
    "Example:\n"
    "    op.conditioned_by_nonzeros('control_reg')(state)";

inline constexpr const char* DOC_CONDITIONED_BY_ALL_ONES =
    "Condition this operation on registers where all bits are 1.\n\n"
    "Args:\n"
    "    cond: Register name (str) or ID (int) to condition on.\n"
    "    conds: List of register names or IDs for multi-condition.\n\n"
    "Returns:\n"
    "    Self, for method chaining.";

inline constexpr const char* DOC_CONDITIONED_BY_BIT =
    "Condition this operation on a specific bit position.\n\n"
    "Args:\n"
    "    cond: Register name (str) or ID (int).\n"
    "    pos: Bit position to check (0-indexed).\n"
    "    conds: List of (register, position) pairs.\n\n"
    "Returns:\n"
    "    Self, for method chaining.";

inline constexpr const char* DOC_CONDITIONED_BY_VALUE =
    "Condition this operation on registers holding a specific value.\n\n"
    "Args:\n"
    "    cond: Register name (str) or ID (int).\n"
    "    pos: Value to match.\n"
    "    conds: List of (register, value) pairs.\n\n"
    "Returns:\n"
    "    Self, for method chaining.";

inline constexpr const char* DOC_DAG =
    "Apply the adjoint (inverse) of this operation.\n\n"
    "Args:\n"
    "    state: The quantum state to operate on.\n\n"
    "Note: Only available for self-adjoint operators.";

inline constexpr const char* DOC_CLEAR_CONTROL =
    "Clear all control conditions of the specified type.";

} // namespace pysparq_docs

// Bind ClassControllable classes extra methods and attributes
#define BIND_CONTROLLABLE_METHODS(CLASS_NAME)                                                                                                                               \
    .def_readonly("condition_variable_nonzeros", &CLASS_NAME::condition_variable_nonzeros)                                                                                  \
        .def_readonly("condition_variable_all_ones", &CLASS_NAME::condition_variable_all_ones)                                                                              \
        .def_readonly("condition_variable_by_value", &CLASS_NAME::condition_variable_by_value)                                                                              \
        .def_readonly("condition_variable_by_bit", &CLASS_NAME::condition_variable_by_bit) /* conditioned_by_nonzeros */                                                    \
        .def("conditioned_by_nonzeros", static_cast<CLASS_NAME &(CLASS_NAME::*)(std::string_view)>(&CLASS_NAME::conditioned_by_nonzeros),                                   \
             py::arg("cond"), py::return_value_policy::reference_internal, pysparq_docs::DOC_CONDITIONED_BY_NONZEROS)                                                       \
        .def("conditioned_by_nonzeros", static_cast<CLASS_NAME &(CLASS_NAME::*)(const std::vector<std::string_view> &)>(&CLASS_NAME::conditioned_by_nonzeros),              \
             py::arg("conds"), py::return_value_policy::reference_internal, pysparq_docs::DOC_CONDITIONED_BY_NONZEROS)                                                     \
        .def("conditioned_by_nonzeros", static_cast<CLASS_NAME &(CLASS_NAME::*)(size_t)>(&CLASS_NAME::conditioned_by_nonzeros),                                             \
             py::arg("cond"), py::return_value_policy::reference_internal, pysparq_docs::DOC_CONDITIONED_BY_NONZEROS)                                                      \
        .def("conditioned_by_nonzeros", static_cast<CLASS_NAME &(CLASS_NAME::*)(const std::vector<size_t> &)>(&CLASS_NAME::conditioned_by_nonzeros),                        \
             py::arg("conds"), py::return_value_policy::reference_internal, pysparq_docs::DOC_CONDITIONED_BY_NONZEROS) /* conditioned_by_all_ones */                       \
        .def("conditioned_by_all_ones", static_cast<CLASS_NAME &(CLASS_NAME::*)(std::string_view)>(&CLASS_NAME::conditioned_by_all_ones),                                   \
             py::arg("cond"), py::return_value_policy::reference_internal, pysparq_docs::DOC_CONDITIONED_BY_ALL_ONES)                                                      \
        .def("conditioned_by_all_ones", static_cast<CLASS_NAME &(CLASS_NAME::*)(const std::vector<std::string_view> &)>(&CLASS_NAME::conditioned_by_all_ones),              \
             py::arg("conds"), py::return_value_policy::reference_internal, pysparq_docs::DOC_CONDITIONED_BY_ALL_ONES)                                                    \
        .def("conditioned_by_all_ones", static_cast<CLASS_NAME &(CLASS_NAME::*)(size_t)>(&CLASS_NAME::conditioned_by_all_ones),                                             \
             py::arg("cond"), py::return_value_policy::reference_internal, pysparq_docs::DOC_CONDITIONED_BY_ALL_ONES)                                                     \
        .def("conditioned_by_all_ones", static_cast<CLASS_NAME &(CLASS_NAME::*)(const std::vector<size_t> &)>(&CLASS_NAME::conditioned_by_all_ones),                        \
             py::arg("conds"), py::return_value_policy::reference_internal, pysparq_docs::DOC_CONDITIONED_BY_ALL_ONES) /* conditioned_by_bit */                           \
        .def("conditioned_by_bit", static_cast<CLASS_NAME &(CLASS_NAME::*)(std::string_view, size_t)>(&CLASS_NAME::conditioned_by_bit),                                     \
             py::arg("cond"), py::arg("pos"), py::return_value_policy::reference_internal, pysparq_docs::DOC_CONDITIONED_BY_BIT)                                           \
        .def("conditioned_by_bit", static_cast<CLASS_NAME &(CLASS_NAME::*)(const std::vector<std::pair<std::string_view, size_t>> &)>(&CLASS_NAME::conditioned_by_bit),     \
             py::arg("conds"), py::return_value_policy::reference_internal, pysparq_docs::DOC_CONDITIONED_BY_BIT)                                                         \
        .def("conditioned_by_bit", static_cast<CLASS_NAME &(CLASS_NAME::*)(size_t, size_t)>(&CLASS_NAME::conditioned_by_bit),                                               \
             py::arg("cond"), py::arg("pos"), py::return_value_policy::reference_internal, pysparq_docs::DOC_CONDITIONED_BY_BIT)                                          \
        .def("conditioned_by_bit", static_cast<CLASS_NAME &(CLASS_NAME::*)(const std::vector<std::pair<size_t, size_t>> &)>(&CLASS_NAME::conditioned_by_bit),               \
             py::arg("conds"), py::return_value_policy::reference_internal, pysparq_docs::DOC_CONDITIONED_BY_BIT) /* conditioned_by_value */                              \
        .def("conditioned_by_value", static_cast<CLASS_NAME &(CLASS_NAME::*)(std::string_view, size_t)>(&CLASS_NAME::conditioned_by_value),                                 \
             py::arg("cond"), py::arg("pos"), py::return_value_policy::reference_internal, pysparq_docs::DOC_CONDITIONED_BY_VALUE)                                         \
        .def("conditioned_by_value", static_cast<CLASS_NAME &(CLASS_NAME::*)(const std::vector<std::pair<std::string_view, size_t>> &)>(&CLASS_NAME::conditioned_by_value), \
             py::arg("conds"), py::return_value_policy::reference_internal, pysparq_docs::DOC_CONDITIONED_BY_VALUE)                                                       \
        .def("conditioned_by_value", static_cast<CLASS_NAME &(CLASS_NAME::*)(size_t, size_t)>(&CLASS_NAME::conditioned_by_value),                                           \
             py::arg("cond"), py::arg("pos"), py::return_value_policy::reference_internal, pysparq_docs::DOC_CONDITIONED_BY_VALUE)                                        \
        .def("conditioned_by_value", static_cast<CLASS_NAME &(CLASS_NAME::*)(const std::vector<std::pair<size_t, size_t>> &)>(&CLASS_NAME::conditioned_by_value),           \
             py::arg("conds"), py::return_value_policy::reference_internal, pysparq_docs::DOC_CONDITIONED_BY_VALUE) /* clear methods */                                   \
        .def("clear_control_nonzeros", &CLASS_NAME::clear_control_nonzeros, pysparq_docs::DOC_CLEAR_CONTROL)                                                                \
        .def("clear_control_all_ones", &CLASS_NAME::clear_control_all_ones, pysparq_docs::DOC_CLEAR_CONTROL)                                                                \
        .def("clear_control_by_bit", &CLASS_NAME::clear_control_by_bit, pysparq_docs::DOC_CLEAR_CONTROL)                                                                    \
        .def("clear_control_by_value", &CLASS_NAME::clear_control_by_value, pysparq_docs::DOC_CLEAR_CONTROL)

// Macro with optional class docstring (variadic macro for backward compatibility)
#define BIND_BASE_OPERATOR(NAME, ...) \
    py::class_<NAME, BaseOperator>(m, #NAME, ##__VA_ARGS__)

/* Workaround for binding inherited operator() of SelfAdjointOperator
*
    Author: Agony5757
    Date:   2025/2/26

   This is used to solve the issue for Windows platform which has a trouble if
   calling operator() in Python. However, the underlying mechanism is unclear
   to me, so I just use a workaround to avoid the issue. */
#ifdef _WIN32
#define BIND_SELF_ADJOINT_OPERATOR(NAME, ...)                                                                     \
    py::class_<NAME, SelfAdjointOperator>(m, #NAME, ##__VA_ARGS__)                                                \
        .def("__call__", (void (NAME::*)(SparseState &) const) & NAME::operator(), py::arg("state"))
#else
#define BIND_SELF_ADJOINT_OPERATOR(NAME, ...) \
    py::class_<NAME, SelfAdjointOperator>(m, #NAME, ##__VA_ARGS__)
#endif

#define BIND_BASE_OPERATOR_SUBNAME(NAME, PYNAME, ...) \
    py::class_<NAME, BaseOperator>(m, #PYNAME, ##__VA_ARGS__)

#define BIND_DAG_METHODS(NAME) \
    .def("dag", (void (NAME::*)(SparseState &) const) & NAME::dag, py::arg("state"), pysparq_docs::DOC_DAG)
