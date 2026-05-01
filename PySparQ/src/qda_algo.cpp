#include "qda_algo.h"

using namespace qram_simulator;

size_t _compute_step_rate(double step_rate, double kappa)
{
    constexpr int StepConstant = 2305;
    size_t steps = size_t(step_rate * StepConstant * kappa);
    if (steps % 2 != 0)
    {
        steps += 1;
    }

    return steps;
}

Eigen::VectorXd QDA_solve(
    const Eigen::MatrixXd &A_in,
    const Eigen::VectorXd &b_in,
    std::optional<double> given_kappa,
    double p,
    double step_rate)
{
    auto [A_q, b_q, recover_x] = QDA_classical2quantum(A_in, b_in);

    DenseMatrix<double> A(A_q);
    DenseVector<double> b(b_q);

    size_t n = A.n_row();

    double kappa;
    if (given_kappa.has_value())
    {
        kappa = given_kappa.value();
    }
    else
    {
        kappa = get_kappa_general(A);
    }
    //PYPRINT("kappa: ", kappa);

    size_t steps = _compute_step_rate(step_rate, kappa);
    //PYPRINT("steps: ", steps);

    size_t data_size = 50;
    size_t rational_size = 51;

    int exponent = 15;
    size_t log_column_size = log2(n);
    if (n != pow2(log_column_size))
    {
        throw_invalid_input("Error: matrix dimension is not a power of 2\n");
    }

    /* Generate QRAM */
    std::vector<size_t> conv_A = scaleAndConvertVector(A, exponent, data_size);
    std::vector<size_t> conv_b = scaleAndConvertVector(b, exponent, data_size);
    memory_t data_tree_A = make_vector_tree(conv_A, data_size);
    memory_t data_tree_b = make_vector_tree(conv_b, data_size);
    size_t addr_size = log_column_size * 2 + 1;

    qram_qutrit::QRAMCircuit qram_A(addr_size, data_size, data_tree_A);
    qram_qutrit::QRAMCircuit qram_b(log_column_size + 1, data_size, data_tree_b);
    //PYPRINT("QRAMCircuit ok");

    // Initialize system and registers.
    SparseState state;

    auto main_reg = AddRegister("main_reg", UnsignedInteger, log_column_size)(state);
    auto anc_UA = AddRegister("anc_UA", UnsignedInteger, log_column_size)(state);
    auto anc_4 = AddRegister("anc_4", Boolean, 1)(state);
    auto anc_3 = AddRegister("anc_3", Boolean, 1)(state);
    auto anc_2 = AddRegister("anc_2", Boolean, 1)(state);
    auto anc_1 = AddRegister("anc_1", Boolean, 1)(state);

    state_prep::State_Prep_via_QRAM(&qram_b, "main_reg", data_size, rational_size)(state);

    QDA::QDA_via_QRAM::WalkSequence_via_QRAM_Debug(
        &qram_A, &qram_b, A, b,
        "main_reg", "anc_UA",
        "anc_1", "anc_2", "anc_3", "anc_4",
        steps, kappa, p, data_size, rational_size)(state);

    double prob_inv0 = PartialTraceSelect({anc_UA, anc_2, anc_3}, {0, 0, 0})(state);
    double prob0 = (1.0 / prob_inv0) * (1.0 / prob_inv0);

    //PYPRINT("Success probability after walk sequence: ", prob0);

    (StatePrint(StatePrintDisplay::Detail))(state);

    auto result = PartialTraceSelect({anc_UA, anc_1, anc_2, anc_3, anc_4}, {0, 1, 0, 0, 0}).get_projected_full(state);

    Eigen::Map<Eigen::VectorXcd> sol_complex(
        reinterpret_cast<std::complex<double> *>(result.first.data()),
        result.first.size());

    auto sol_real = sol_complex.real();

    auto sol = recover_x(sol_real);

    return sol;
}

// Helper to check if a matrix is approximately Hermitian
// (A good check, though for the transformation, we enforce it)
bool is_hermitian(const Eigen::MatrixXd &A, double precision = 1e-9)
{
    if (A.rows() != A.cols())
    {
        return false;
    }
    return A.isApprox(A.adjoint(), precision);
}

// Helper to find the next power of 2
int next_power_of_2(int n)
{
    if (n <= 0)
        return 1; // Or handle error
    if ((n > 0) && ((n & (n - 1)) == 0))
        return n; // Already a power of 2
    return static_cast<int>(std::pow(2, std::ceil(std::log2(static_cast<double>(n)))));
}

/**
 * @brief Converts a linear system Ax=b to a form suitable for quantum linear solvers.
 * The resulting matrix A_q is Hermitian and has dimensions 2^k x 2^k.
 * The solution x to the original system can be recovered from the solution x_q
 * of A_q x_q = b_q using the provided recovery function.
 *
 * @param A_in The input matrix (Eigen::MatrixXd).
 * @param b_in The input vector (Eigen::VectorXd).
 * @return A tuple containing:
 *         1. Eigen::MatrixXd: The transformed Hermitian matrix A_q.
 *         2. Eigen::VectorXd: The transformed vector b_q.
 *         3. std::function<Eigen::VectorXd(const Eigen::VectorXd&)>:
 *            A function to recover the original solution x from x_q.
 */
// std::tuple<Eigen::MatrixXd, Eigen::VectorXd>
std::tuple<Eigen::MatrixXd, Eigen::VectorXd, std::function<Eigen::VectorXd(const Eigen::VectorXd &)>>
QDA_classical2quantum(const Eigen::MatrixXd &A_in, const Eigen::VectorXd &b_in)
{
    if (A_in.rows() != A_in.cols())
    {
        throw std::invalid_argument("Input matrix A_in must be square.");
    }
    if (A_in.rows() != b_in.size())
    {
        throw std::invalid_argument("Dimensions of A_in and b_in are incompatible.");
    }

    Eigen::MatrixXd current_A;
    Eigen::VectorXd current_b;
    int original_dim = A_in.rows();
    bool hermitian_transform_done = false;

    // Step 1: Hermitization (if necessary)
    // A simple check, though the problem implies it might not be
    // For robustness, we can always apply the embedding if not explicitly told it's Hermitian.
    // Or, if A_in IS Hermitian, we can skip. For now, let's assume we check.
    // A more robust check than `is_hermitian` for very small matrices or specific structures
    // might be needed, but `isApprox` is generally good.
    if (is_hermitian(A_in))
    {
        current_A = A_in;
        current_b = b_in;
        //PYPRINT("Input A is already Hermitian.");
    }
    else
    {
        //PYPRINT("Input A is not Hermitian. Applying transformation.");
        hermitian_transform_done = true;
        int n = A_in.rows();
        current_A = Eigen::MatrixXd::Zero(2 * n, 2 * n);
        current_A.block(0, n, n, n) = A_in;
        current_A.block(n, 0, n, n) = A_in.adjoint();

        current_b = Eigen::VectorXd::Zero(2 * n);
        current_b.head(n) = b_in;
    }

    // Step 2: Padding to power of 2
    int current_dim = current_A.rows();
    int padded_dim = next_power_of_2(current_dim);

    Eigen::MatrixXd A_q;
    Eigen::VectorXd b_q;

    if (padded_dim == current_dim)
    {
        //PYPRINT("Dimension is already a power of 2.");
        A_q = current_A;
        b_q = current_b;
    }
    else
    {
        //PYPRINT("Padding dimension from ", current_dim, " to ", padded_dim, ".");
        A_q = Eigen::MatrixXd::Zero(padded_dim, padded_dim);
        b_q = Eigen::VectorXd::Zero(padded_dim);

        A_q.block(0, 0, current_dim, current_dim) = current_A;
        // Pad with identity on the diagonal for the matrix A
        for (int i = current_dim; i < padded_dim; ++i)
        {
            A_q(i, i) = 1.0; // Real identity is fine
        }

        b_q.head(current_dim) = current_b;
        // The rest of b_q is already zero
    }

    // Step 3: Normalize the matrix A_q and vector b_q
    b_q = b_q / b_q.norm();
    A_q = A_q / A_q.norm();

    // Step 4: Create the recovery function
    std::function<Eigen::VectorXd(const Eigen::VectorXd &)> recover_x =
        [original_dim, hermitian_transform_done, current_dim, padded_dim](const Eigen::VectorXd &x_q)
    {
        if (x_q.size() != padded_dim)
        {
            throw std::runtime_error("Solution vector x_q has incorrect dimension for recovery.");
        }

        Eigen::VectorXd x_intermediate = x_q.head(current_dim);

        if (hermitian_transform_done)
        {
            // Original x was in the second half of the [0, x]^T solution vector
            // for the system [0 A; A_dag 0] [y; z] = [b; 0]
            // where solution is y=0, z=x. So x_intermediate = [0; x]
            // The dimension of this x_intermediate is 2 * original_dim
            if (x_intermediate.size() != 2 * original_dim)
            {
                throw std::runtime_error("Mismatch in dimensions during hermitian recovery logic.");
            }
            return x_intermediate.segment(original_dim, original_dim).eval();
        }
        else
        {
            // No hermitian transform was done, x_intermediate is directly the solution
            // (potentially padded, but head(current_dim) already handled that)
            // and current_dim == original_dim in this case.
            if (x_intermediate.size() != original_dim)
            {
                throw std::runtime_error("Mismatch in dimensions during non-hermitian recovery logic.");
            }
            return x_intermediate;
        }
    };

    return std::make_tuple(A_q, b_q, recover_x);
}

int main(int argc, const char **argv)
{
    Eigen::MatrixXd A(4, 4);
    A << 1.0, 2.0, 3.0, 4.0,
        2.0, 1.0, 4.0, 5.0,
        3.0, 4.0, 1.0, 6.0,
        4.0, 5.0, 6.0, 1.0;

    Eigen::VectorXd b(4);
    b << 3.0, 4.5, 11.8, 0.2;

    auto solution = QDA_solve(A, b);

    std::cout << "Solution: " << solution.transpose() << std::endl;
}
