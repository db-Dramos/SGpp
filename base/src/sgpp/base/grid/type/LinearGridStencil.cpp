// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/base/grid/type/LinearGridStencil.hpp>

#include <sgpp/base/grid/generation/StandardGridGenerator.hpp>
#include <sgpp/base/operation/hash/common/basis/LinearBasis.hpp>

#include <sgpp/base/exception/factory_exception.hpp>

#include <sgpp/globaldef.hpp>

#include <iostream>
#include <exception>

namespace SGPP {
namespace base {

LinearGridStencil::LinearGridStencil(std::istream& istr) : GridStencil(istr) {
}

LinearGridStencil::LinearGridStencil(size_t dim) : GridStencil(dim) {
}

LinearGridStencil::LinearGridStencil(BoundingBox& BB) : GridStencil(BB) {
}

LinearGridStencil::~LinearGridStencil() {
}

SGPP::base::GridType LinearGridStencil::getType() {
  return SGPP::base::GridType::LinearStencil;
}

const SBasis& LinearGridStencil::getBasis() {
  throw new factory_exception("Not implemented");
  // it should never get so far, code just for compilation reasons
  // If there will be a meaningful basis, this following lines should be changed
  static SLinearBase basis;
  return basis;
}

std::unique_ptr<Grid> LinearGridStencil::unserialize(std::istream& istr) {
  return std::unique_ptr<Grid>(new LinearGridStencil(istr));
}

/**
 * Creates new GridGenerator
 * This must be changed if we add other storage types
 */
std::unique_ptr<GridGenerator> LinearGridStencil::createGridGenerator() {
  return std::unique_ptr<GridGenerator>(new StandardGridGenerator(storage));
}


}  // namespace base
}  // namespace SGPP
