// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at 
// sgpp.sparsegrids.org

#include <sgpp/base/basis/modpoly/ModifiedPolyBasis.hpp>
#include <sgpp/base/basis/modlinear/operation/OperationHierarchisationModLinear.hpp>
#include <sgpp/base/basis/modlinear/algorithm_sweep/HierarchisationModLinear.hpp>
#include <sgpp/base/basis/modlinear/algorithm_sweep/DehierarchisationModLinear.hpp>

#include <sgpp/base/algorithm/sweep.hpp>


#include <sgpp/globaldef.hpp>


namespace SGPP {
  namespace base {
    /**
     * Implements the hierarchisation on a sprase grid with mod linear base functions
     *
     * @param node_values the functions values in the node base
     *
     * @todo (heinecke, nice) Implement the hierarchisation on the sparse grid with mod linear base functions
     */
    void OperationHierarchisationModLinear::doHierarchisation(DataVector& node_values) {
      HierarchisationModLinear func(this->storage);
      sweep<HierarchisationModLinear> s(func, this->storage);

      // Execute hierarchisation in every dimension of the grid
      for (size_t i = 0; i < this->storage->dim(); i++) {
        s.sweep1D(node_values, node_values, i);
      }
    }

    /**
     * Implements the dehierarchisation on a sprase grid with mod linear base functions
     *
     * @param alpha the coefficients of the sparse grid's base functions
     *
     * @todo (heinecke, nice) Implement the dehierarchisation on the sparse grid with mod linear base functions
     */
    void OperationHierarchisationModLinear::doDehierarchisation(DataVector& alpha) {
      DehierarchisationModLinear func(this->storage);
      sweep<DehierarchisationModLinear> s(func, this->storage);

      // Execute hierarchisation in every dimension of the grid
      for (size_t i = 0; i < this->storage->dim(); i++) {
        s.sweep1D(alpha, alpha, i);
      }
    }

  }
}