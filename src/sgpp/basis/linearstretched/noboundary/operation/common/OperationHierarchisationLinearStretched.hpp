/******************************************************************************
* Copyright (C) 2011 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Alexander Heinecke (Alexander.Heinecke@mytum.de), Sarpkan Selcuk (Sarpkan.Selcuk@mytum.de)


#ifndef OPERATIONHIERARCHISATIONLINEARSTRETCHED_HPP
#define OPERATIONHIERARCHISATIONLINEARSTRETCHED_HPP

#include "operation/common/OperationHierarchisation.hpp"
#include "grid/GridStorage.hpp"

namespace sg
{

/**
 * Hierarchisation on sparse grid, linear grid without boundaries
 */
class OperationHierarchisationLinearStretched : public OperationHierarchisation
{
public:
	/**
	 * Construtor of OperationHierarchisationLinear
	 *
	 * @param storage Pointer to the grid's gridstorage obejct
	 */
	OperationHierarchisationLinearStretched(GridStorage* storage) : storage(storage) {}

	/**
	 * Destructor
	 */
	virtual ~OperationHierarchisationLinearStretched() {}

	virtual void doHierarchisation(DataVector& node_values);
	virtual void doDehierarchisation(DataVector& alpha);

protected:
	/// Pointer to the grid's gridstorage object
	GridStorage* storage;
};

}

#endif /* OPERATIONHIERARCHISATIONLINEARSTRETCHED_HPP */