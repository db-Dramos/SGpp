// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#pragma once

#include <sgpp/combigrid/GeneralFunction.hpp>
#include <sgpp/combigrid/common/MultiIndexIterator.hpp>
#include <sgpp/combigrid/definitions.hpp>
#include <sgpp/combigrid/grid/TensorGrid.hpp>
#include <sgpp/combigrid/grid/hierarchy/AbstractPointHierarchy.hpp>
#include <sgpp/combigrid/operation/multidim/fullgrid/AbstractFullGridLinearEvaluator.hpp>
#include <sgpp/combigrid/operation/onedim/AbstractLinearEvaluator.hpp>
#include <sgpp/combigrid/storage/AbstractCombigridStorage.hpp>
#include <sgpp/combigrid/storage/tree/CombigridTreeStorage.hpp>
#include <sgpp/combigrid/storage/tree/TreeStorage.hpp>
#include <sgpp/combigrid/threading/PtrGuard.hpp>
#include <sgpp/combigrid/threading/ThreadPool.hpp>

#include <vector>

namespace sgpp {
namespace combigrid {

typedef GeneralFunction<std::shared_ptr<TreeStorage<double>>, std::shared_ptr<TensorGrid>>
    GridFunction;

/**
 * Implementation of the AbstractFullGridLinearEvaluator class using a callback function that
 * If you want to be able to use different function values at the same point in different levels
 * (for example because you are implementing a PDE solver), set exploitNesting to false in the
 * constructor of CombigridTreeStorage.
 */
template <typename V>
class FullGridLinearGridBasedEvaluator : public AbstractFullGridLinearEvaluator<V> {
 protected:
  GridFunction gridFunction;

  // We cannot use bool because that would involve vector<bool> problems...
  std::shared_ptr<TreeStorage<uint8_t>> precomputedLevels;

  void addResults(MultiIndex const &level, std::shared_ptr<TreeStorage<double>> results) {
    std::vector<bool> orderingConfiguration(this->evaluatorPrototypes.size());
    for (size_t d = 0; d < this->evaluatorPrototypes.size(); ++d) {
      orderingConfiguration[d] = this->evaluatorPrototypes[d]->needsOrderedPoints();
    }
    MultiIndex multiBounds(this->pointHierarchies.size());
    for (size_t d = 0; d < multiBounds.size(); ++d) {
      multiBounds[d] = this->pointHierarchies[d]->getNumPoints(level[d]);
    }
    MultiIndexIterator multiIt(multiBounds);

    auto storageIt = this->storage->getGuidedIterator(level, multiIt, orderingConfiguration);

    while (storageIt->isValid()) {
      storageIt->value() = results->get(multiIt.getMultiIndex());
      storageIt->moveToNext();
    }
  }

 public:
  /**
   * Constructor.
   *
   * @param storage Storage that stores and provides the function values for each grid point.
   * @param evaluatorPrototypes prototype objects for the evaluators that are cloned to get an
   * evaluator for each dimension and each level.
   * @param pointHierarchies PointHierarchy objects for each dimension providing the points for
   * each level and information about their ordering.
   * @param gridFunction callback function that is called with a grid as parameters and should
   * return a TreeStorage that contains the values at these grid points
   */
  FullGridLinearGridBasedEvaluator(
      std::shared_ptr<AbstractCombigridStorage> storage,
      std::vector<std::shared_ptr<AbstractLinearEvaluator<V>>> evaluatorPrototypes,
      std::vector<std::shared_ptr<AbstractPointHierarchy>> pointHierarchies,
      GridFunction gridFunction)
      : AbstractFullGridLinearEvaluator<V>(storage, evaluatorPrototypes, pointHierarchies),
        gridFunction(gridFunction),
        precomputedLevels(std::make_shared<TreeStorage<uint8_t>>(pointHierarchies.size())) {}

  virtual ~FullGridLinearGridBasedEvaluator() {}

  std::shared_ptr<TensorGrid> getTensorGrid2(MultiIndex const &level) {
    std::vector<bool> orderingConfiguration(this->evaluatorPrototypes.size());
    for (size_t d = 0; d < this->evaluatorPrototypes.size(); ++d) {
      orderingConfiguration[d] = this->evaluatorPrototypes[d]->needsOrderedPoints();
    }
    return this->getTensorGrid(level, orderingConfiguration);
  }

  /**
     * @return a vector of tasks which can be precomputed in parallel to make the (serialized)
     * execution of eval() faster. This class only returns one task in the vector.
     * @param level the level which one wants to compute
     * @param callback This callback is called (with already locked mutex) from inside one of the
     * returned tasks when all tasks for the given level are completed and the level can be added.
     */
  std::vector<ThreadPool::Task> getLevelTasks(MultiIndex const &level, ThreadPool::Task callback) {
    auto grid = getTensorGrid2(level);

    std::vector<ThreadPool::Task> tasks;

    tasks.push_back(ThreadPool::Task([grid, level, this, callback]() {
      auto results = gridFunction(grid);

      // now we need locking
      PtrGuard guard(this->mutexPtr);
      addResults(level, results);
      precomputedLevels->set(level, 1);
      callback();
    }));

    return tasks;
  }

  virtual V eval(MultiIndex const &level) {
    if (!precomputedLevels->containsIndex(level)) {
      addResults(level, gridFunction(getTensorGrid2(level)));
      precomputedLevels->set(level, 1);
    }
    // call the base eval
    return this->AbstractFullGridLinearEvaluator<V>::eval(level);
  }
};

} /* namespace combigrid */
} /* namespace sgpp */