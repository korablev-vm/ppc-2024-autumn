#include "seq/korablev_v_jacobi_method/include/ops_seq.hpp"

#include <cmath>
#include <iostream>

bool korablev_v_jacobi_method_seq::JacobiMethodSequential::isNeedToComplete(const std::vector<double>& x_old,
                                                                            const std::vector<double>& x_new) const {
  double sum_up = 0;
  double sum_low = 0;
  for (size_t k = 0; k < x_old.size(); k++) {
    sum_up += (x_new[k] - x_old[k]) * (x_new[k] - x_old[k]);
    sum_low += x_new[k] * x_new[k];
  }
  return (sqrt(sum_up / sum_low) < epsilon_);
}

bool korablev_v_jacobi_method_seq::JacobiMethodSequential::pre_processing() {
  internal_order_test();
  size_t n = *reinterpret_cast<size_t*>(taskData->inputs[0]);
  A_.resize(n * n);
  b_.resize(n);
  x_.resize(n, 1.0);

  for (size_t i = 0; i < n; ++i) {
    for (size_t j = 0; j < n; ++j) {
      A_[i * n + j] = reinterpret_cast<double*>(taskData->inputs[1])[i * n + j];
    }
    b_[i] = reinterpret_cast<double*>(taskData->inputs[2])[i];
  }

  return true;
}

bool korablev_v_jacobi_method_seq::JacobiMethodSequential::validation() {
  internal_order_test();

  if (taskData->inputs_count.size() != 3 || taskData->outputs_count.size() != 1) {
    std::cerr << "Error: Invalid number of inputs or outputs." << std::endl;
    return false;
  }

  size_t n = *reinterpret_cast<size_t*>(taskData->inputs[0]);
  if (n <= 0) {
    std::cerr << "Error: Matrix size must be positive." << std::endl;
    return false;
  }

  for (size_t i = 0; i < n; ++i) {
    double diag = std::fabs(reinterpret_cast<double*>(taskData->inputs[1])[i * n + i]);
    double sum = 0.0;

    for (size_t j = 0; j < n; ++j) {
      if (i != j) {
        sum += std::fabs(reinterpret_cast<double*>(taskData->inputs[1])[i * n + j]);
      }
    }

    if (diag <= sum) {
      std::cerr << "Error: Matrix is not diagonally dominant at row " << i << "." << std::endl;
      return false;
    }

    if (diag == 0.0) {
      std::cerr << "Error: Zero element on the diagonal at row " << i << "." << std::endl;
      return false;
    }
  }

  return true;
}

bool korablev_v_jacobi_method_seq::JacobiMethodSequential::run() {
  internal_order_test();
  size_t n = b_.size();
  std::vector<double> x_prev(n);

  size_t numberOfIter = 0;

  while (numberOfIter < maxIterations_) {
    std::copy(x_.begin(), x_.end(), x_prev.begin());

    for (size_t k = 0; k < n; k++) {
      double S = 0;
      for (size_t j = 0; j < n; j++) {
        if (j != k) {
          S += A_[k * n + j] * x_[j];
        }
      }
      x_[k] = (b_[k] - S) / A_[k * n + k];
    }

    if (isNeedToComplete(x_prev, x_)) break;
    numberOfIter++;
  }

  if (numberOfIter == maxIterations_) {
    std::cerr << "Warning: Maximum iterations reached without convergence." << std::endl;
  }

  return true;
}

bool korablev_v_jacobi_method_seq::JacobiMethodSequential::post_processing() {
  internal_order_test();
  for (size_t i = 0; i < x_.size(); ++i) {
    reinterpret_cast<double*>(taskData->outputs[0])[i] = x_[i];
  }
  return true;
}