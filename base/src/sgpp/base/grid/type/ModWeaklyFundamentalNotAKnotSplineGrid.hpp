// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef MODWEAKLYFUNDAMENTALNOTAKNOTSPLINEGRID_HPP
#define MODWEAKLYFUNDAMENTALNOTAKNOTSPLINEGRID_HPP

#include <sgpp/base/grid/Grid.hpp>
#include <sgpp/base/operation/hash/common/basis/WeaklyFundamentalNotAKnotSplineModifiedBasis.hpp>
#include <sgpp/base/grid/generation/StandardGridGenerator.hpp>

#include <sgpp/globaldef.hpp>

namespace sgpp {
namespace base {

/**
 * Grid with modified weakly fundamental spline basis functions with not-a-knot boundary conditions
 */
class ModWeaklyFundamentalNotAKnotSplineGrid : public Grid {
 protected:
  /**
   * This constructor creates a new GridStorage out of the stream.
   *
   * @param istr inputstream that contains the grid information
   */
  explicit ModWeaklyFundamentalNotAKnotSplineGrid(std::istream& istr);

 public:
  /**
   * Constructor
   *
   * @param dim the dimension of the grid
     * @param degree the bspline's degree
   */
  explicit ModWeaklyFundamentalNotAKnotSplineGrid(size_t dim, size_t degree);

  /**
   * Destructor.
   */
  ~ModWeaklyFundamentalNotAKnotSplineGrid() override;

  /**
   * @return string that identifies the grid type uniquely
   */
  sgpp::base::GridType getType() override;

  /**
   * @return B-spline basis
   */
  SBasis& getBasis() override;

  /**
   * @return pointer to a GridGenerator object
   */
  GridGenerator& getGenerator() override;

  /**
   * reads a grid out of a string
   *
   * @param istr string that contains the grid information
   * @return grid
   */
  static Grid* unserialize(std::istream& istr);

  /**
   * Serializes the grid.
   *
   * @param ostr stream to which the grid is written
   * @param version the serialization version of the file
   */
  void serialize(std::ostream& ostr, int version = SERIALIZATION_VERSION) override;

  /**
   * @return B-spline degree
   */
  virtual size_t getDegree();

 protected:
  /// grid generator
  StandardGridGenerator generator;
  /// B-spline degree
  size_t degree;
  /// B-spline basis
  std::unique_ptr<SWeaklyFundamentalNotAKnotSplineModifiedBase> basis_;
};

}  // namespace base
}  // namespace sgpp

#endif /* MODWEAKLYFUNDAMENTALNOTAKNOTSPLINEGRID_HPP */