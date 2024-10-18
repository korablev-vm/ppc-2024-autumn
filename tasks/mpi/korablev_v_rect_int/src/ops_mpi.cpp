#include "mpi/korablev_v_rect_int/include/ops_mpi.hpp"

#include <algorithm>
#include <boost/mpi.hpp>
#include <chrono>
#include <functional>
#include <numeric>
#include <random>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

bool korablev_v_rect_int_mpi::RectangularIntegrationSequential::pre_processing() {
  internal_order_test();

  auto* tmp_ptr_a = reinterpret_cast<double*>(taskData->inputs[0]);
  auto* tmp_ptr_b = reinterpret_cast<double*>(taskData->inputs[1]);
  auto* tmp_ptr_n = reinterpret_cast<int*>(taskData->inputs[2]);

  a_ = *tmp_ptr_a;
  b_ = *tmp_ptr_b;
  n_ = *tmp_ptr_n;

  if (!func_) {
    std::cerr << "Error: func not found!" << std::endl;
    return false;
  }

  return true;
}

bool korablev_v_rect_int_mpi::RectangularIntegrationSequential::validation() {
  internal_order_test();
  return taskData->outputs_count[0] == 1;
}

bool korablev_v_rect_int_mpi::RectangularIntegrationSequential::run() {
  internal_order_test();
  result_ = integrate(func_, a_, b_, n_);
  return true;
}

bool korablev_v_rect_int_mpi::RectangularIntegrationSequential::post_processing() {
  internal_order_test();
  *reinterpret_cast<double*>(taskData->outputs[0]) = result_;
  return true;
}

double korablev_v_rect_int_mpi::RectangularIntegrationSequential::integrate(const std::function<double(double)>& f,
                                                                            double a, double b, int n) {
  double h = (b - a) / n;
  double sum = 0.0;

  for (int i = 0; i < n; ++i) {
    double x = a + i * h;
    sum += f(x) * h;
  }

  return sum;
}

void korablev_v_rect_int_mpi::RectangularIntegrationSequential::set_function(
    const std::function<double(double)>& func) {
  func_ = func;
}

bool korablev_v_rect_int_mpi::RectangularIntegrationParallel::pre_processing() {
  internal_order_test();

  unsigned int delta = 0;
  if (world.rank() == 0) {
    delta = n_ / world.size();
  }
  broadcast(world, delta, 0);

  if (world.rank() == 0) {
    auto* tmp_ptr_a = reinterpret_cast<double*>(taskData->inputs[0]);
    auto* tmp_ptr_b = reinterpret_cast<double*>(taskData->inputs[1]);
    auto* tmp_ptr_n = reinterpret_cast<int*>(taskData->inputs[2]);

    a_ = *tmp_ptr_a;
    b_ = *tmp_ptr_b;
    n_ = *tmp_ptr_n;
  }

  broadcast(world, a_, 0);
  broadcast(world, b_, 0);
  broadcast(world, n_, 0);

  return true;
}

bool korablev_v_rect_int_mpi::RectangularIntegrationParallel::validation() {
  internal_order_test();
  if (world.rank() == 0) {
    return taskData->outputs_count[0] == 1;
  }
  return true;
}

bool korablev_v_rect_int_mpi::RectangularIntegrationParallel::run() {
  internal_order_test();

  local_result_ = parallel_integrate(func_, a_, b_, n_);

  reduce(world, local_result_, global_result_, std::plus<>(), 0);

  return true;
}

bool korablev_v_rect_int_mpi::RectangularIntegrationParallel::post_processing() {
  internal_order_test();
  if (world.rank() == 0) {
    *reinterpret_cast<double*>(taskData->outputs[0]) = global_result_;
  }
  return true;
}

double korablev_v_rect_int_mpi::RectangularIntegrationParallel::parallel_integrate(
    const std::function<double(double)>& f, double a, double b, int n) {
  int rank = world.rank();
  int size = world.size();

  double h = (b - a) / n;
  int local_n = n / size;
  int remainder = n % size;

  if (rank < remainder) {
    local_n += 1;
  }

  double local_a = a + rank * local_n * h;

  double local_sum = 0.0;
  for (int i = 0; i < local_n; ++i) {
    double x = local_a + i * h;
    local_sum += f(x) * h;
  }

  return local_sum;
}

void korablev_v_rect_int_mpi::RectangularIntegrationParallel::set_function(const std::function<double(double)>& func) {
  func_ = func;
}