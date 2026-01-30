#pragma once

#include <Eigen/Dense>
#include "BlockEncoding/block_encoding_via_QRAM.h"
#include "DiscreteAdiabatic/qda_via_QRAM.h"
#include "block_encoding.h"
#include "matrix.h"
#include "BlockEncoding/make_qram.h"
#include "DiscreteAdiabatic/qda_fundamental.h"
#include "qram_circuit_qutrit.h"
#include "state_preparation.h"
#include "sparse_state_simulator.h"

Eigen::VectorXd QDA_solve(
    const Eigen::MatrixXd &A_in,
    const Eigen::VectorXd &b_in,
    std::optional<double> kappa = std::nullopt,
    double p = 1.3,
    double step_rate = 0.01);

std::tuple<Eigen::MatrixXd, Eigen::VectorXd, std::function<Eigen::VectorXd(const Eigen::VectorXd &)>>
QDA_classical2quantum(
    const Eigen::MatrixXd &A_in,
    const Eigen::VectorXd &b_in);
