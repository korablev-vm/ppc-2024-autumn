#pragma once

#include <gtest/gtest.h>

#include <boost/mpi.hpp>
#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>
#include <memory>
#include <utility>
#include <vector>

#include "core/task/include/task.hpp"

namespace korablev_v_qucik_sort_simple_merge_mpi {

class QuickSortSimpleMergeSequential : public ppc::core::Task {
 public:
  explicit QuickSortSimpleMergeSequential(std::shared_ptr<ppc::core::TaskData> taskData_)
      : Task(std::move(taskData_)) {}
  bool pre_processing() override;
  bool validation() override;
  bool run() override;
  bool post_processing() override;

 private:
  std::vector<double> input_;
  std::vector<double> output_;

  static std::vector<double> merge(const std::vector<double>& left, const std::vector<double>& right);
  std::vector<double> quick_sort_with_merge(const std::vector<double>& arr);
};

class QuickSortSimpleMergeParallel : public ppc::core::Task {
 public:
  explicit QuickSortSimpleMergeParallel(std::shared_ptr<ppc::core::TaskData> taskData_) : Task(std::move(taskData_)) {}
  bool pre_processing() override;
  bool validation() override;
  bool run() override;
  bool post_processing() override;

 private:
  std::vector<double> input_;
  std::vector<double> output_;
  std::vector<double> local_data_;

  static std::vector<double> merge(const std::vector<double>& left, const std::vector<double>& right);
  std::vector<double> quick_sort_with_merge(const std::vector<double>& arr);
  boost::mpi::communicator world;
};

}  // namespace korablev_v_qucik_sort_simple_merge_mpi
