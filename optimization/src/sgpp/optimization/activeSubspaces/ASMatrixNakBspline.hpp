// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#pragma once
//#ifdef USE_EIGEN

#include <sgpp/base/grid/generation/functors/SurplusRefinementFunctor.hpp>
#include <sgpp/base/grid/type/NakBsplineGrid.hpp>
#include <sgpp/base/operation/hash/common/basis/NakBsplineBasis.hpp>
#include <sgpp/base/operation/hash/common/basis/NakBsplineBoundaryBasis.hpp>
#include <sgpp/base/operation/hash/common/basis/NakBsplineModifiedBasis.hpp>
#include <sgpp/optimization/activeSubspaces/ASMatrix.hpp>
#include <sgpp/optimization/activeSubspaces/GaussQuadrature.hpp>
#include <sgpp/optimization/function/scalar/InterpolantScalarFunction.hpp>
#include <sgpp/optimization/function/scalar/InterpolantScalarFunctionGradient.hpp>
#include <sgpp/optimization/sle/solver/Armadillo.hpp>
#include <sgpp/optimization/sle/system/HierarchisationSLE.hpp>
#include <sgpp/optimization/tools/RandomNumberGenerator.hpp>

namespace sgpp {
namespace optimization {

/**
 * Used to create, store and use the matrix C for the detection of active subspaces using a B-spline
 * interpolant. So
 * C_{i,j} = \int \nabla f \nabla f^T dx \approx \int \nabla \hat{f} \nabla \hat{f}^T dx
 * where \hat{f} is a B-splien interpolant for f
 */
class ASMatrixNakBspline : public ASMatrix {
 public:
  /**
   * Constructor
   *
   * @param objectiveFunction objective Function f
   * @param gridType          type of the grid for the interpolant
   * @param degree            degree for the B-spline basis functions
   */
  ASMatrixNakBspline(WrapperScalarFunction objectiveFunc, sgpp::base::GridType gridType,
                     size_t degree)
      : ASMatrix(objectiveFunc), gridType(gridType), degree(degree) {
    if (gridType == sgpp::base::GridType::NakBspline) {
      grid = std::make_shared<sgpp::base::NakBsplineGrid>(numDim, degree);
    } else if (gridType == sgpp::base::GridType::NakBsplineBoundary) {
      grid = std::make_shared<sgpp::base::NakBsplineBoundaryGrid>(numDim, degree);
    } else if (gridType == sgpp::base::GridType::NakBsplineModified) {
      grid = std::make_shared<sgpp::base::NakBsplineModifiedGrid>(numDim, degree);
    } else {
      throw sgpp::base::generation_exception("ASMatrixNakBspline: gridType not supported.");
    }
  }

  /**
   * Create a regular interpolant of the objective function f
   *
   * @param level	level of the underlying grid
   */
  void buildRegularInterpolant(size_t level);

  /**
   * Create a spatially adaptive interpolant of the objective function f
   *
   * @param maxNumGridPoints	upper threshold for the number of grid points
   * @param initialLevel		the refinement needs an initial regular grid of initialLevel
   */
  void buildAdaptiveInterpolant(size_t maxNumGridPoints, size_t initialLevel = 1);

  /**
   * General routine to create the Matrix C, currently simply wraps createMatrixMonteCarlo.
   * Usually createMatrixGauss() is better and should be preferred
   */
  void createMatrix(size_t numPoints);

  /**
   * creates the matrix C from the interpolant by using Monte Carlo quadrature
   *
   * @param numPoints	number of Monte Carlo points
   */
  void createMatrixMonteCarlo(size_t numMCPoints);

  /**
   * creates the matrix C using the exact integrals of the underlying B-spline functions from a
   * Gauss quadrature of sufficient degree
   */
  void createMatrixGauss();

  // auxiliary routines

  /**
   * refine the current interpolant surplus adaptive by ading the children of <= refinementsNum
   * points
   *
   * @param refinementsNum maximum number of points to be refined
   */
  void refineSurplusAdaptive(size_t refinementsNum);

  /**
   * calculates the coefficients for the interpolant based on the objective function and grid.
   * Must be called after every change to the grid!
   */
  void calculateInterpolationCoefficients();

  /**
   * calculates the entry C_{i,j} of the matrix,
   * C_{i,j} = int df/dxi df/dxj dx
   *
   * @param i row
   * @param j column
   * @param coordinates coordinates for the Gauss quadrature
   * @param weights weights for the Gauss quadrature
   * @return matrix entry C_{i,j}
   */
  double matrixEntryGauss(size_t i, size_t j, base::DataVector coordinates,
                          base::DataVector weights);

  /**
   * calculates \int d/dx_i b_k(x) d/dx_j b_l(x) dx
   * This is done by evaluating one dimensional integrals of b_{k_d} b_{l_d},
   * d/dx_i b_{k_i} b_{l_i}, b_{k_j} d/dx_j b_{l_j}, d/dx_i b_{k_i} d/dx_i b_{l_i}
   *
   *@param i index of matrix row
   *@param j index of matrix column
   *@param k index of first basis function
   *@param l index of second basis function
   *@param coordinates coordinates for the Gauss quadrature
   * @param weights weights for the Gauss quadrature
   * @return integral \int d/dx_i b_k(x) d/dx_j b_l(x) dx
   */
  double scalarProductDxbiDxbj(size_t i, size_t j, size_t k, size_t l, base::DataVector coordinates,
                               base::DataVector weights);

  /**
   * calculates the one diensional integral \int f*g dx where f and g are B-spline basis functions
   * or first derivatives of B-spline basis functions
   *
   * @param level1 level of the first B-spline
   * @param index1 index of the first B-spline
   * @param dx1 evaluate B-spline if False, evaluate d/dx B-spline if True
   * @param level2 level of the second B-spline
   * @param index2 index of the second B-spline
   * @param coordinates coordinates for the Gauss quadrature
   * @param weights weights for the Gauss quadrature
   * @param dx2 evaluate B-spline if False, evaluate d/dx B-spline if True
   *
   * @return  integral (derivative of) first basis function * (derivative of) second basis function
   */
  double univariateScalarProduct(size_t level1, size_t index1, bool dx1, size_t level2,
                                 size_t index2, bool dx2, base::DataVector coordinates,
                                 base::DataVector weights);

  /**
   * used to get the support segments of a b-splien basis functions. Needed for Gauss quadrature
   *
   * @param level	level of the B-spline basis function
   * @param index	index of the B-spline basis function
   *
   * @return the indices of the segments of the B-spline basis functions support
   */
  sgpp::base::DataVector nakBSplineSupport(size_t level, size_t index);

 private:
  sgpp::base::GridType gridType;
  size_t degree;
  sgpp::base::DataVector coefficients;
  std::shared_ptr<sgpp::base::Grid> grid;
  typedef std::tuple<size_t, size_t, bool, size_t, size_t, bool> asMatrixHashType;
  std::map<asMatrixHashType, double> innerProducts;
};

}  // namespace optimization
}  // namespace sgpp

//#endif /* USE_EIGEN */
