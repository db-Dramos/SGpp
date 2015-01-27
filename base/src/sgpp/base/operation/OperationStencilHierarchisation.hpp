// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at 
// sgpp.sparsegrids.org

#ifndef OPERATIONSTENCILHIERARCHISATION_HPP
#define OPERATIONSTENCILHIERARCHISATION_HPP

#include <sgpp/base/operation/OperationHierarchisation.hpp>
#include <sgpp/base/datatypes/DataVector.hpp>
#include <vector>

#include <sgpp/globaldef.hpp>


namespace SGPP {
  namespace base {

    /**
     * This class implements abstract hierarchisation and dehierarchisation routines
     * on the sparse grid by logging the operations into three arrays rather than
     * actually applying them to the data vector object.
     * To apply the operations to e.g. a valid DataVector obejct alpha associated with
     * the grid, just run a loop over the stencils and perform
     *
     * > alpha[surplus[i]] += weight[i]*alpha[neighbor[i]]
     *
     * Obviously the inverse operation (dehierarchisation resp. hierarchisation) can be
     * applied by reversing the loop and using "-" instead of "+".
     */
    class OperationStencilHierarchisation : public OperationHierarchisation {
      public:

        typedef std::vector<unsigned int> IndexStencil;

        typedef std::vector<float>      WeightStencil;


        /**
         * Constructor
         */
        OperationStencilHierarchisation() {}

        /**
         * Destructor
         */
        virtual ~OperationStencilHierarchisation() {}

        /**
         * Implements the abstract hierarchisation on a sparse grid.
         * The results are queried via get{Surpluses,Neighbors,Weights}.
         *
         * @param node_values dummy array
         */
        virtual void doHierarchisation(DataVector& node_values) = 0;

        /**
         * Implements the dehierarchisation on a sparse grid
         * The results are queried via get{Surplus,Neighbor,Weight}Stencil().
         *
         * @param alpha dummy array
         */
        virtual void doDehierarchisation(DataVector& alpha) = 0;

        /**
         * Access to the results of the hierarchisation operation.
         * This is an index array whose contents refer to the indices
         * associated with the grid points by means of the storage
         * scheme used.
         *
         * @return the surpluses array
         */
        virtual const IndexStencil&
        getSurplusStencil() const = 0;

        /**
         * Access to the results of the hierarchisation operation.
         * This is an index array whose contents refer to the indices
         * associated with the grid points by means of the storage
         * scheme used.
         *
         * @return the neighbors array
         */
        virtual const IndexStencil&
        getNeighborStencil() const = 0;

        /**
         * Access to the results of the hierarchisation operation.
         *
         * @return the weights array
         */
        virtual const WeightStencil&
        getWeightStencil() const = 0;

        /**
         * The number of operations performed, i.e. the length of
         * each of the three arrays.
         *
         * @return the total number of operations
         */
        virtual size_t
        getStencilSize() const = 0;
    };

  }
}

#endif /* OPERATIONSTENCILHIERARCHISATION_HPP */