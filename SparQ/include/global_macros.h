/**
 * @file global_macros.h
 * @brief 全局宏定义文件
 * @details 定义编译配置宏、条件控制宏、CUDA 支持宏以及各类工具宏
 */

#pragma once
// Comment this or uncomment this to select single-thread or openmp
#define SINGLE_THREAD

/* Condition check macros — all require every specified condition to hold (logical AND):
 - ConditionSatisfiedNonzeros: register value is nonzero
 - ConditionSatisfiedAllOnes:   every bit in the register is 1
 - ConditionSatisfiedByValue:   register value equals a given constant
 - ConditionSatisfiedByBit:     a specific bit position in the register is 1
 - ConditionSatisfied:          master check combining all of the above
*/

// Utility macros — these remain as preprocessor macros rather than constexpr functions
// because they capture outer-scope condition_variable_* members via text substitution,
// enabling the ClassControllable pattern where each operator carries its own condition set.

/**
 * @def ConditionSatisfiedNonzeros
 * @brief 检查条件寄存器是否全部非零
 */
#define ConditionSatisfiedNonzeros(state_ref) \
	std::all_of(std::begin(condition_variable_nonzeros), std::end(condition_variable_nonzeros),\
	[&](size_t cond_var)\
	{\
		return (state_ref).get(cond_var).as<bool>(System::size_of(cond_var));\
	})

/**
 * @def ConditionSatisfiedAllOnes
 * @brief 检查条件寄存器是否全部为1（全满）
 */
#define ConditionSatisfiedAllOnes(state_ref) \
	std::all_of(std::begin(condition_variable_all_ones), std::end(condition_variable_all_ones),\
	[&](size_t cond_var)\
	{\
		return (state_ref).get(cond_var).value == (pow2(System::size_of(cond_var)) - 1);\
	})

/**
 * @def ConditionSatisfiedByValue
 * @brief 按值检查条件寄存器
 */
#define ConditionSatisfiedByValue(state_ref) \
	std::all_of(std::begin(condition_variable_by_value), std::end(condition_variable_by_value),\
	[&](const std::pair<size_t, uint64_t>& cond_var)\
	{\
		return (state_ref).get(cond_var.first).value == cond_var.second;\
	})

/**
 * @def ConditionSatisfiedByBit
 * @brief 按位检查条件寄存器
 */
#define ConditionSatisfiedByBit(state_ref) \
	std::all_of(std::begin(condition_variable_by_bit), std::end(condition_variable_by_bit),\
	[&](const std::pair<size_t, size_t> &cond_pair)\
	{\
		return (state_ref).get(cond_pair.first).value & (pow2(cond_pair.second));\
	})

/**
 * @def HasCondition
 * @brief 检查是否存在任何条件
 */
#define HasCondition (\
	condition_variable_nonzeros.size() > 0 ||\
	condition_variable_all_ones.size() > 0 ||\
	condition_variable_by_value.size() > 0 ||\
	condition_variable_by_bit.size() > 0)

/**
 * @def ConditionSatisfied
 * @brief 检查所有条件是否满足
 */
#define ConditionSatisfied(state_ref)          \
	(ConditionSatisfiedAllOnes((state_ref))      \
	 && ConditionSatisfiedByBit((state_ref))     \
	 && ConditionSatisfiedNonzeros((state_ref))  \
	 && ConditionSatisfiedByValue((state_ref)) )

/**
 * @def ConditionNotSatisfied
 * @brief 检查条件是否不满足
 */
#define ConditionNotSatisfied(state_ref) ((!ConditionSatisfied(((state_ref)))))


/* CLASS CONTROLLABLE NONZERO */
/********************************************
*            conditioned_by_nonzeros
*********************************************/
/**
 * @def ClassControllableNonzeros
 * @brief 定义非零条件控制成员变量和方法
 * @details 为类添加 condition_variable_nonzeros 成员及相关的条件控制方法
 */
#define ClassControllableNonzeros std::vector<size_t> condition_variable_nonzeros;\
    inline void clear_control_nonzeros()\
	{\
		condition_variable_nonzeros.clear();\
	}\
	inline auto& conditioned_by_nonzeros(std::string_view cond)\
	{\
		clear_control_nonzeros();\
		condition_variable_nonzeros.push_back(System::get(cond));\
		return *this;\
	}\
    inline auto& conditioned_by_nonzeros(const std::vector<std::string_view> &conds)\
	{\
		clear_control_nonzeros();\
		for (auto &cond : conds) condition_variable_nonzeros.push_back(System::get(cond));\
		return *this;\
	}\
    inline auto& conditioned_by_nonzeros(const std::vector<std::string> &conds)\
	{\
		clear_control_nonzeros();\
		for (auto &cond : conds) condition_variable_nonzeros.push_back(System::get(cond));\
		return *this;\
	}\
	inline auto& conditioned_by_nonzeros(size_t cond)\
	{\
		clear_control_nonzeros();\
		condition_variable_nonzeros.push_back(cond);\
		return *this; \
	}\
    inline auto& conditioned_by_nonzeros(const std::vector<size_t> &conds)\
	{\
		clear_control_nonzeros();\
		condition_variable_nonzeros = conds;\
		return *this;\
	}

/**
 * @def ClassControllableAllOnes
 * @brief 定义全1条件控制成员变量和方法
 */
#define ClassControllableAllOnes std::vector<size_t> condition_variable_all_ones;\
    inline void clear_control_all_ones()\
	{\
		condition_variable_all_ones.clear();\
	}\
	inline auto& conditioned_by_all_ones(std::string_view cond)\
	{\
		clear_control_nonzeros();\
		condition_variable_all_ones.push_back(System::get(cond));\
		return *this;\
	}\
    inline auto& conditioned_by_all_ones(const std::vector<std::string_view> &conds)\
	{\
		clear_control_nonzeros();\
		for (auto &cond : conds) condition_variable_all_ones.push_back(System::get(cond));\
		return *this;\
	}\
	inline auto& conditioned_by_all_ones(size_t cond)\
	{\
		clear_control_nonzeros();\
		condition_variable_all_ones.push_back(cond);\
		return *this; \
	}\
    inline auto& conditioned_by_all_ones(const std::vector<size_t> &conds)\
	{\
		clear_control_nonzeros();\
		condition_variable_all_ones = conds;\
		return *this;\
	}

/* CLASS CONTROLLABLE BY BIT */
/********************************************
*            conditioned_at
*********************************************/
/**
 * @def ClassControllableByBit
 * @brief 定义按位条件控制成员变量和方法
 * @details 允许按特定位设置条件控制
 */
#define ClassControllableByBit \
	std::vector<std::pair<size_t, size_t>> condition_variable_by_bit;\
    inline void clear_control_by_bit()\
	{\
		condition_variable_by_bit.clear();\
	}\
    inline auto& conditioned_by_bit(const std::vector<std::pair<std::string, size_t>> &conds)\
	{\
		clear_control_by_bit();\
		for (auto &cond : conds) \
			condition_variable_by_bit.emplace_back(System::get(cond.first), cond.second);\
		return *this;\
	}\
    inline auto& conditioned_by_bit(const std::vector<std::pair<std::string_view, size_t>> &conds)\
	{\
		clear_control_by_bit();\
		for (auto &cond : conds) \
			condition_variable_by_bit.emplace_back(System::get(cond.first), cond.second);\
		return *this;\
	}\
    inline auto& conditioned_by_bit(const std::vector<std::pair<size_t, size_t>> &conds)\
	{\
		clear_control_by_bit();\
		condition_variable_by_bit = conds;\
		return *this;\
	}\
	inline auto& conditioned_by_bit(std::string_view cond, size_t pos)\
	{\
		clear_control_by_bit();\
		condition_variable_by_bit.emplace_back(System::get(cond), pos);\
		return *this;\
	}\
	inline auto& conditioned_by_bit(size_t cond, size_t pos)\
	{\
		clear_control_by_bit();\
		condition_variable_by_bit.emplace_back(cond, pos);\
		return *this; \
	}


/* CLASS CONTROLLABLE BY VALUE*/
/********************************************
*            conditioned_by_value
*********************************************/
/**
 * @def ClassControllableByValue
 * @brief 定义按值条件控制成员变量和方法
 */
#define ClassControllableByValue \
	std::vector<std::pair<size_t, size_t>> condition_variable_by_value;\
    inline void clear_control_by_value()\
	{\
		condition_variable_by_value.clear();\
	}\
    inline auto& conditioned_by_value(const std::vector<std::pair<std::string, size_t>> &conds)\
	{\
		clear_control_by_value();\
		for (auto &cond : conds) condition_variable_by_value.emplace_back(System::get(cond.first), cond.second);\
		return *this;\
	}\
    inline auto& conditioned_by_value(const std::vector<std::pair<std::string_view, size_t>> &conds)\
	{\
		clear_control_by_value();\
		for (auto &cond : conds) condition_variable_by_value.emplace_back(System::get(cond.first), cond.second);\
		return *this;\
	}\
    inline auto& conditioned_by_value(const std::vector<std::pair<size_t, size_t>> &conds)\
	{\
		clear_control_by_value();\
		condition_variable_by_value = conds;\
		return *this;\
	}\
	inline auto& conditioned_by_value(std::string_view cond, size_t pos)\
	{\
		clear_control_by_value();\
		condition_variable_by_value.emplace_back(System::get(cond), pos);\
		return *this;\
	}\
	inline auto& conditioned_by_value(size_t cond, size_t pos)\
	{\
		clear_control_by_value();\
		condition_variable_by_value.emplace_back(cond, pos);\
		return *this; \
	}

/**
 * @def ClassControllable
 * @brief 组合所有条件控制宏
 */
#define ClassControllable \
	ClassControllableNonzeros\
	ClassControllableAllOnes\
	ClassControllableByBit\
	ClassControllableByValue

// #define SELF_ADJOINT inline void dag(std::vector<System>& state) const { (*this)(state); }
// #define SELF_ADJOINT_NONCONST inline void dag(std::vector<System>& state) { (*this)(state); }

/**
 * @def GetAs
 * @brief 以指定类型获取寄存器值
 */
#define GetAs(id, type) get(id).as<type>(System::size_of(id))


/**
 * @def Debug_CheckOverflow
 * @brief 调试宏：检查寄存器值是否溢出
 */
#define Debug_CheckOverflow(regname) \
if(s.get(regname).value >= pow2(s.size_of(regname))) \
{ \
	auto err_info = "Found overflow in data register!\nvalue={}, size={} (maximum value = {}, expect size = {})\n", \
		s.get(regname).value, s.size_of(regname), pow2(s.size_of(regname)) - 1, log2(s.get(regname).value;\
	fmt::print(err_info); \
	throw_general_runtime_error(err_info);\
}

/**
 * @def FORWARD_CONDITIONS
 * @brief 转发所有条件控制设置
 */
#define FORWARD_CONDITIONS conditioned_by_bit(condition_variable_by_bit)\
.conditioned_by_nonzeros(condition_variable_nonzeros)\
.conditioned_by_value(condition_variable_by_value)\
.conditioned_by_all_ones(condition_variable_all_ones)


/**
 * @def INLINE_IMPL
 * @brief 内联实现宏
 */
#define INLINE_IMPL(type) inline void operator()(type& state) const\
{impl<type>(state);}\
inline void dag(type& state) const\
{impl_dag<type>(state);}

/**
 * @def DECLARE_IMPL
 * @brief 声明实现宏
 */
#define DECLARE_IMPL(type) void operator()(type& state) const;\
void dag(type& state) const;

/**
 * @def CU_IMPL
 * @brief CUDA 实现宏
 */
#define CU_IMPL(type) void type::operator()(CuSparseState& state) const {profiler _(#type " cuda");impl<CuSparseState>(state);}\
void type::dag(CuSparseState& state) const {profiler _(#type ".dag cuda");impl_dag<CuSparseState>(state);}

/**
 * @def CU_IMPL_SKIP_GPU
 * @brief CUDA 跳过 GPU 实现宏
 */
#define CU_IMPL_SKIP_GPU(type) void type::operator()(CuSparseState& state) const {state.move_to_cpu();\
impl<std::vector<System>>(state.sparse_state_cpu);}\
void type::dag(CuSparseState& state) const {state.move_to_cpu();\
impl_dag<std::vector<System>>(state.sparse_state_cpu);}

#ifdef USE_CUDA
/**
 * @def COMPOSITE_OPERATION
 * @brief 复合操作宏（CUDA 版本）
 */
#define COMPOSITE_OPERATION INLINE_IMPL(std::vector<System>)\
INLINE_IMPL(SparseState)\
DECLARE_IMPL(CuSparseState)
#else
/**
 * @def COMPOSITE_OPERATION
 * @brief 复合操作宏（CPU 版本）
 */
#define COMPOSITE_OPERATION INLINE_IMPL(std::vector<System>)\
INLINE_IMPL(SparseState)
#endif


#ifdef USE_CUDA
/**
 * @def CuCondition_Args
 * @brief CUDA 条件参数
 */
#define CuCondition_Args condition_variable_nonzeros_gpu.data().get(), condition_variable_nonzeros_gpu.size(),\
condition_variable_all_ones_gpu.data().get(), condition_variable_all_ones_gpu.size(),\
condition_variable_by_bit_gpu.data().get(), condition_variable_by_bit_gpu.size(),\
condition_variable_by_value_gpu.data().get(), condition_variable_by_value_gpu.size()

/**
 * @def CuCondition_Params
 * @brief CUDA 条件参数类型
 */
#define CuCondition_Params thrust::pair<size_t, size_t>* nonzeros_ptr_, size_t nonzeros_size_,\
thrust::pair<size_t, size_t>* all_ones_ptr_, size_t all_ones_size_,\
thrust::pair<size_t, size_t>* by_bit_ptr_, size_t by_bit_size_,\
thrust::tuple<size_t, size_t, size_t>* by_value_ptr_, size_t by_value_size_

/**
 * @def CuCondition_Host_Prepare
 * @brief CUDA 条件主机端准备
 */
#define CuCondition_Host_Prepare thrust::device_vector<thrust::pair<size_t, size_t>> condition_variable_nonzeros_gpu;\
thrust::device_vector<thrust::pair<size_t, size_t>> condition_variable_all_ones_gpu;\
thrust::device_vector<thrust::pair<size_t, size_t>> condition_variable_by_bit_gpu;\
thrust::device_vector<thrust::tuple<size_t, size_t, size_t>> condition_variable_by_value_gpu;\
for (auto& c : condition_variable_nonzeros)\
condition_variable_nonzeros_gpu.push_back(thrust::pair<size_t, size_t>(c, System::size_of(c)));\
for (auto& c : condition_variable_all_ones)\
condition_variable_all_ones_gpu.push_back(thrust::pair<size_t, size_t>(c, System::size_of(c)));\
for (auto& c : condition_variable_by_bit)\
condition_variable_by_bit_gpu.push_back(thrust::pair<size_t, size_t>(c.first, c.second));\
for (auto& c : condition_variable_by_value)\
condition_variable_by_value_gpu.push_back(thrust::tuple<size_t, size_t, size_t>(c.first, System::size_of(c.first), c.second));

/**
 * @def CuConditionLambdaPrepare
 * @brief CUDA 条件 Lambda 准备
 */
#define CuConditionLambdaPrepare CuCondition_Host_Prepare \
auto nonzeros_ptr = condition_variable_nonzeros_gpu.data().get();\
auto all_ones_ptr = condition_variable_all_ones_gpu.data().get();\
auto by_bit_ptr = condition_variable_by_bit_gpu.data().get();\
auto by_value_ptr = condition_variable_by_value_gpu.data().get();\
auto nonzeros_size = condition_variable_nonzeros_gpu.size();\
auto all_ones_size = condition_variable_all_ones_gpu.size();\
auto by_bit_size = condition_variable_by_bit_gpu.size();\
auto by_value_size = condition_variable_by_value_gpu.size();

/**
 * @def CuConditionLambdaCaptures
 * @brief CUDA 条件 Lambda 捕获
 */
#define CuConditionLambdaCaptures nonzeros_ptr, nonzeros_size, all_ones_ptr, all_ones_size, by_bit_ptr, by_bit_size, by_value_ptr, by_value_size

/**
 * @def CuCondition_Functor
 * @brief CUDA 条件仿函数成员
 */
#define CuCondition_Functor thrust::pair<size_t, size_t>* nonzeros_ptr;\
thrust::pair<size_t, size_t>* all_ones_ptr;\
thrust::pair<size_t, size_t>* by_bit_ptr;\
thrust::tuple<size_t, size_t, size_t>* by_value_ptr;\
size_t nonzeros_size;\
size_t all_ones_size;\
size_t by_bit_size;\
size_t by_value_size;

/**
 * @def CuCondition_Init
 * @brief CUDA 条件初始化
 */
#define CuCondition_Init nonzeros_ptr(nonzeros_ptr_), nonzeros_size(nonzeros_size_),\
all_ones_ptr(all_ones_ptr_), all_ones_size(all_ones_size_),\
by_bit_ptr(by_bit_ptr_), by_bit_size(by_bit_size_),\
by_value_ptr(by_value_ptr_), by_value_size(by_value_size_)

/**
 * @def CuConditionSatisfied
 * @brief CUDA 条件检查
 */
#define CuConditionSatisfied(s) bool condition = true;\
for (size_t i = 0; condition && (i < nonzeros_size); i++) condition &= (CuGetAsUint64(s, nonzeros_ptr[i].first, nonzeros_ptr[i].second) != 0);\
for (size_t i = 0; condition && (i < all_ones_size); i++) condition &= (CuGetAsUint64(s, all_ones_ptr[i].first, all_ones_ptr[i].second) == pow2(all_ones_ptr[i].second) - 1);\
for (size_t i = 0; condition && (i < by_bit_size); i++) condition &= (get_digit(CuGetAsUint64(s, by_bit_ptr[i].first, by_bit_ptr[i].second), by_bit_ptr[i].second) == 1);\
for (size_t i = 0; condition && (i < by_value_size); i++) condition &= (CuGetAsUint64(s, thrust::get<0>(by_value_ptr[i]), thrust::get<1>(by_value_ptr[i])) == thrust::get<2>(by_value_ptr[i]));\
if (condition)
#endif

#define OPTIMIZE_HADAM
#define OPTIMIZE_HADAM_INT
#define OPTIMIZE_ROT
// #define OPTIMIZE_PREP

#define SAFE_HASH
#define CHECK_HASH
