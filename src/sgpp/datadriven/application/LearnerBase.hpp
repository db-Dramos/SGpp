/******************************************************************************
* Copyright (C) 2012 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Alexander Heinecke (Alexander.Heinecke@mytum.de)

#ifndef LEARNERBASE_HPP
#define LEARNERBASE_HPP

#include "base/grid/Grid.hpp"
#include "base/datatypes/DataVector.hpp"
#include "base/datatypes/DataMatrix.hpp"

#include "solver/SLESolver.hpp"

#include "datadriven/algorithm/DMSystemMatrixBase.hpp"

namespace sg
{

namespace datadriven
{

/**
 * struct to encapsulate the classsifiers quality by its
 * characteristic numbers
 */
struct ClassificatorQuality
{
	/// number of true positive classified instances
    size_t truePositive_;
    /// number of true negative classified instances
    size_t trueNegative_;
    /// number of false positive classified instances
    size_t falsePositive_;
    /// number of false negative classified instances
    size_t falseNegative_;
};

/**
 * strcut to encapsulate the learner's timings
 * during training
 */
struct LearnerTiming
{
	/// complete learning time
	double timeComplete_;
	/// time to apply B (including data transfer to eventually used accelerators)
	double timeMultComplete_;
	/// pure application time of B
	double timeMultCompute_;
	/// time to apply B^T (including data transfer to eventually used accelerators)
	double timeMultTransComplete_;
	/// pure application time of B^T
	double timeMultTransCompute_;
	/// time of regularization
	double timeRegularization_;
};

/**
 * Abstract class that implements a regression/classification learner
 * based on spatial adaptive Sparse Grids.
 *
 * Furthermore this class is intended to provide a framework
 * for adavanded regression and classification methods
 * by allowing to override basic routines like train
 * and test.
 */
class LearnerBase
{
protected:
	/// the grid's coefficients
	sg::base::DataVector* alpha_;
	/// sparse grid object
	sg::base::Grid* grid_;
	/// is verbose output enabled
	bool isVerbose_;
	/// is regression selected
	bool isRegression_;
	/// is the grid trained
	bool isTrained_;

	/**
	 * Hook-Method for post-processing after each
	 * refinement learning.
	 *
	 * can be overwritten by derived classes
	 */
	virtual void postProcessing();

	/**
	 * Initialize the grid and its coefficients
	 *
	 * @param GridCongif structure which describes the regular start grid
	 */
	virtual void InitializeGrid(const sg::base::RegularGridConfiguration& GridConfig);

	/**
	 * abstract method that constructs the corresponding system of linear equations
	 * Derived classes MUST overwrite this functions!
	 *
	 * @param trainDataset training dataset
	 * @param lambda lambda regularization parameter
	 */
	virtual sg::datadriven::DMSystemMatrixBase* createDMSystem(sg::base::DataMatrix trainDataset, double lambda) = 0;

public:
	/**
	 * Constructor
	 *
	 * @param isRegression
	 * @param verbose
	 */
	LearnerBase(const bool isRegression, const bool isVerbose = true);

	/**
	 * Constructor
	 *
	 * @param tGridFilename path to file that contains a serialized grid
	 * @param tAlphaFilenment path to file that contains the grid's coefficients
	 * @param isRegression set to true if a regression task should be executed
	 * @param verbose set to true in order to allow console output
	 */
	LearnerBase(std::string tGridFilename, std::string tAlphaFilename, const bool isRegression, const bool isVerbose = true);

	/**
	 * Copy-Constructor
	 *
	 * @param copyMe LearnerBase that should be duplicated
	 */
	LearnerBase(const LearnerBase& copyMe);

	/**
	 * Destructor
	 */
	virtual ~LearnerBase();

	/**
	 * Learning a dataset with spatially adaptive sparse grids
	 *
	 * @param testDataset the training dataset
	 * @param classes classes corresponding to the training dataset
	 * @param GridConfig configuration of the regular start grid
	 * @param SolverConfigRefine configuration of the SLE solver during the adaptive refinements of the grid
	 * @param SolverConfigFinal configuration of the final SLE solving step on the refined grid
	 * @param AdaptConfig configuration of the adaptivity strategy
	 * @param testAccDuringAdapt set to true if the training accuracy should be determined in evert refinement step
	 * @param lambda regularization parameter lambda
	 */
	virtual LearnerTiming train(sg::base::DataMatrix& testDataset, sg::base::DataVector& classes,
			const sg::base::RegularGridConfiguration& GridConfig, const sg::solver::SLESolverConfiguration& SolverConfigRefine,
			const sg::solver::SLESolverConfiguration& SolverConfigFinal, const sg::base::AdpativityConfiguration& AdaptConfig,
			bool testAccDuringAdapt, const double lambda);

	/**
	 * Learning a dataset with regular sparse grids
	 *
	 * @param testDataset the training dataset
	 * @param classes classes corresponding to the training dataset
	 * @param GridConfig configuration of the regular grid
	 * @param SolverConfig configuration of the SLE solver
	 * @param lambda regularization parameter lambda
	 */
	virtual LearnerTiming train(sg::base::DataMatrix& testDataset,  sg::base::DataVector& classes,
			const sg::base::RegularGridConfiguration& GridConfig, const sg::solver::SLESolverConfiguration& SolverConfig,
			const double lamda);

	/**
	 * executes a Regression test for a given dataset and returns the result
	 *
	 * @param testDataset dataset that is evaluated with the current learner
	 *
	 * @return regression values of testDataset
	 */
	virtual sg::base::DataVector test(sg::base::DataMatrix& testDataset);

	/**
	 * compute the accuracy for given testDataset. test is automatically called
	 * in order to determine the regression values of the current learner
	 *
	 * In case if classification (isRegression == false) this routine returns the learner's accuracy
	 * In case of regressions (isRegression == true) this routine returns the learner's MSE
	 *
	 * @param testDataset dataset to be tested
	 * @param classesReference reference labels of the test dataset
	 * @param threshold threshold used for classification, ignored when performing regressions
	 *
	 * @return accuracy, percent or MSE, depending on the execution mode
	 */
	virtual double getAccuracy(sg::base::DataMatrix& testDataset,
			const sg::base::DataVector& classesReference, const double threshold = 0.0);

	/**
	 * compute the accuracy for given testDataset.
	 *
	 * In case if classification (isRegression == false) this routine returns the learner's accuracy
	 * In case of regressions (isRegression == true) this routine returns the learner's MSE
	 *
	 * @param classesComputed regression results of the test dataset
	 * @param classesReference reference labels of the test dataset
	 * @param threshold threshold used for classification, ignored when performing regressions
	 *
	 * @return accuracy, percent or MSE, depending on the execution mode
	 */
	virtual double getAccuracy(const sg::base::DataVector& classesComputed,
			const sg::base::DataVector& classesReference, const double threshold = 0.0);

	/**
	 * compute the quality for given testDataset, classification ONLY!
	 * test is automatically called
	 * in order to determine the regression values of the current learner
	 *
	 * @param testDataset dataset to be tested
	 * @param classesReference reference labels of the test dataset
	 * @param threshold threshold used for classification, ignored when performing regressions
	 *
	 * @return quality structure containing tp, tn, fp, fn counts
	 */
	virtual ClassificatorQuality getCassificatorQuality(sg::base::DataMatrix& testDataset,
			const sg::base::DataVector& classesReference, const double threshold = 0.0);

	/**
	 * compute the quality for given testDataset, classification ONLY!
	 *
	 * @param classesComputed regression results of the test dataset
	 * @param classesReference reference labels of the test dataset
	 * @param threshold threshold used for classification, ignored when performing regressions
	 *
	 * @return quality structure containing tp, tn, fp, fn counts
	 */
	virtual ClassificatorQuality getCassificatorQuality(const sg::base::DataVector& classesComputed,
			const sg::base::DataVector& classesReference, const double threshold = 0.0);

	/**
	 * store the grid and its current coefficients into files for
	 * further usage.
	 *
	 * @param tGridFilename filename of grid file
	 * @param tAlphaFilename filename of coefficient file
	 */
	void store(std::string tGridFilename, std::string tAlphaFilename);

	/**
	 * determines the current mode
	 *
	 * @return returns whether the current mode is regression or not
	 */
	bool getIsRegression() const;

	/**
	 * determines the current verbose mode of learner
	 *
	 * @return returns whether the current learner has verbose output
	 */
	bool getIsVerbose() const;

	/**
	 * sets the current verbose mode of learner
	 *
	 * @param verbose the current learner's verbose output
	 */
	void setIsVerbose(const bool isVerbose);
};

}

}

#endif /* LEARNERBASE_HPP */
