// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#pragma once

#include <sgpp/base/grid/generation/functors/SurplusRefinementFunctor.hpp>
#include <sgpp/base/grid/type/NakBsplineBoundaryGrid.hpp>
#include <sgpp/base/grid/type/NakBsplineGrid.hpp>
#include <sgpp/base/grid/type/NakBsplineModifiedGrid.hpp>
#include <sgpp/base/operation/hash/common/basis/NakBsplineBasis.hpp>
#include <sgpp/base/operation/hash/common/basis/NakBsplineBoundaryBasis.hpp>
#include <sgpp/base/operation/hash/common/basis/NakBsplineModifiedBasis.hpp>
#include <sgpp/optimization/activeSubspaces/ASResponseSurface.hpp>

namespace sgpp {
namespace optimization {

class ASResponseSurfaceNakBspline : public ASResponseSurface {
 public:
  ASResponseSurfaceNakBspline(Eigen::MatrixXd W1, sgpp::base::GridType gridType, size_t degree = 3)
      : ASResponseSurface(W1), degree(degree), gridType(gridType) {
    initialize();
  };

  /**
   * sets numDim, grid and basis according to gridType and W1
   */
  void initialize();

  /**
   * creates a regular grid of the dimension of the reduced space ( = # columns of W1)
   * then performs regression for the B-spline coefficients with the given evaluationPoints and
   * functionValues. These usually come from the detection of the active  subspace
   */
  void createRegularSurfaceFromDetectionPoints(sgpp::base::DataMatrix evaluationPoints,
                                               sgpp::base::DataVector functionValues, size_t level);

  /**
   * creates a regular grid of the dimension of the reduced space ( = # columns of W1)
   * then interpolates in these points. The function values are calculated by using the
   * Moore-Penrose (pseudo-) inverse of W1, pinvW1. So for a point x the right hand side is
   * f(pinvW1 x)
   */
  void createRegularResponseSurfaceWithPseudoInverse(
      size_t level, sgpp::optimization::WrapperScalarFunction objectiveFunc);

  /**
   * creates a surplus adaptive grid of the dimension of the reduced space ( = # columns of W1)
   * then interpolates in these points. The function values are calculated by using the
   * Moore-Penrose (pseudo-) inverse of W1, pinvW1. So for a point x the right hand side is
   * f(pinvW1 x)
   */
  void createAdaptiveResponseSurfaceWithPseudoInverse(
      size_t maxNumGridPoints, sgpp::optimization::WrapperScalarFunction objectiveFunc,
      size_t initialLevel = 1);

  double eval(sgpp::base::DataVector v);
  double evalGradient(sgpp::base::DataVector v, sgpp::base::DataVector& gradient);

 private:
  size_t degree = 3;
  sgpp::base::GridType gridType;
  size_t numDim;
  std::unique_ptr<sgpp::base::Grid> grid;
  std::unique_ptr<sgpp::base::SBasis> basis;

  void refineSurplusAdaptive(size_t refinementsNum,
                             sgpp::optimization::WrapperScalarFunction objectiveFunc,
                             sgpp::base::DataVector& alpha);

  sgpp::base::DataVector calculateInterpolationCoefficientsWithPseudoInverse(
      sgpp::optimization::WrapperScalarFunction objectiveFunc);
};

}  // namespace optimization
}  // namespace sgpp