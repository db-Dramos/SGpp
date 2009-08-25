/*****************************************************************************/
/* This file is part of sgpp, a program package making use of spatially      */
/* adaptive sparse grids to solve numerical problems                         */
/*                                                                           */
/* Copyright (C) 2009 Alexander Heinecke (Alexander.Heinecke@mytum.de)       */
/*                                                                           */
/* sgpp is free software; you can redistribute it and/or modify              */
/* it under the terms of the GNU Lesser General Public License as published  */
/* by the Free Software Foundation; either version 3 of the License, or      */
/* (at your option) any later version.                                       */
/*                                                                           */
/* sgpp is distributed in the hope that it will be useful,                   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU Lesser General Public License for more details.                       */
/*                                                                           */
/* You should have received a copy of the GNU Lesser General Public License  */
/* along with sgpp; if not, write to the Free Software                       */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/* or see <http://www.gnu.org/licenses/>.                                    */
/*****************************************************************************/

#ifndef OPERATIONLAPLACELINEARBOUNDARY_HPP
#define OPERATIONLAPLACELINEARBOUNDARY_HPP

#include "basis/linearboundary/algorithm_sweep/LaplaceDownLinearBoundary.hpp"
#include "basis/linearboundary/algorithm_sweep/LaplaceUpLinearBoundary.hpp"

#include "operation/OperationMatrix.hpp"

#include "algorithm/classification/UnidirGradient.hpp"
#include "algorithm/common/sweep.hpp"

#include "grid/GridStorage.hpp"
#include "data/DataVector.hpp"

namespace sg
{

/**
 * Implementation of Laplace for linear functions with boundaries diagonal cut through subspace scheme
 */
class OperationLaplaceLinearBoundary: public OperationMatrix, public UnidirGradient
{
public:
	/**
	 * Constructor
	 *
	 * @param storage the grid's GridStorage object
	 */
	OperationLaplaceLinearBoundary(GridStorage* storage) : UnidirGradient(storage)
	{
	}

	/**
	 * Destructor
	 */
	virtual ~OperationLaplaceLinearBoundary() {}


	virtual void mult(DataVector& alpha, DataVector& result)
	{
		this->updown(alpha, result);
	}

protected:
	virtual void up(DataVector& alpha, DataVector& result, size_t dim)
	{
		detail::LaplaceUpLinearBoundary func(this->storage);
		sweep<detail::LaplaceUpLinearBoundary> s(func, this->storage);
		s.sweep1D_Boundary(alpha, result, dim);
	}

	virtual void down(DataVector& alpha, DataVector& result, size_t dim)
	{
		detail::LaplaceDownLinearBoundary func(this->storage);
		sweep<detail::LaplaceDownLinearBoundary> s(func, this->storage);
		s.sweep1D_Boundary(alpha, result, dim);
	}

	virtual void downGradient(DataVector& alpha, DataVector& result, size_t dim)
	{
		// init the coefficients of the ansatz functions on the boundary
		result.setAll(0.0);

		// traverse all basis function by sequence number
		for(size_t i = 0; i < storage->size(); i++)
		{
			GridStorage::index_type::level_type level;
			GridStorage::index_type::index_type index;
			(*storage)[i]->get(dim, level, index);
			if (level == 0)
			{
				//only affects the diagonal of the stiffness matrix
				result[i] += alpha[i];

				// down
				if (index == 0)
				{
					GridIndex index_one = (*storage)[i];
					index_one.set(dim, 0, 1);
					result[(*storage)[&index_one]] += ((-1) * alpha[i]);
				}
			}
			//only affects the diagonal of the stiffness matrix
			else
			{
				result[i] = alpha[i]*pow(2.0, static_cast<int>(level+1));
			}
		}
	}

	virtual void upGradient(DataVector& alpha, DataVector& result, size_t dim)
	{
		// init the coefficients of the ansatz functions on the boundary
		result.setAll(0.0);

		// traverse all basis function by sequence number
		for(size_t i = 0; i < storage->size(); i++)
		{
			GridStorage::index_type::level_type level;
			GridStorage::index_type::index_type index;
			(*storage)[i]->get(dim, level, index);
			if (level == 0)
			{
				// up
				if (index == 1)
				{
					GridIndex index_zero = (*storage)[i];
					index_zero.set(dim, 0, 0);
					result[(*storage)[&index_zero]] += ((-1) * alpha[i]);
				}
			}
		}
	}
};

}

#endif /* OPERATIONLAPLACELINEAROSCALED_HPP */
