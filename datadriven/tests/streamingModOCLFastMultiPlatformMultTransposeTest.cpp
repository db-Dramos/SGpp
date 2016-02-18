// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#if USE_OCL == 1

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <zlib.h>

#include <random>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "test_datadrivenCommon.hpp"
#include "sgpp/globaldef.hpp"
#include "sgpp/base/operation/hash/OperationMultipleEval.hpp"
#include "sgpp/datadriven/DatadrivenOpFactory.hpp"
#include "sgpp/base/operation/BaseOpFactory.hpp"
#include "sgpp/datadriven/tools/ARFFTools.hpp"
#include "sgpp/base/grid/generation/functors/SurplusRefinementFunctor.hpp"
#include "sgpp/base/tools/ConfigurationParameters.hpp"

BOOST_AUTO_TEST_SUITE(TestStreamingModOCLFastMultiPlatformMultTranspose)

BOOST_AUTO_TEST_CASE(Simple) {
  std::vector<std::string> fileNames = {"datadriven/tests/data/friedman_4d.arff.gz",
                                        "datadriven/tests/data/friedman_10d.arff.gz"};
  std::vector<double> errors = {10E-11, 10E-11};

  uint32_t level = 4;

  std::shared_ptr<SGPP::base::OCLOperationConfiguration> parameters =
      getConfigurationDefaultsSingleDevice();

  (*parameters)["INTERNAL_PRECISION"].set("double");

  std::vector<std::reference_wrapper<json::Node>> deviceNodes = parameters->getAllDeviceNodes();
  for (json::Node &deviceNode : deviceNodes) {
    deviceNode.replaceIDAttr("KERNEL_USE_LOCAL_MEMORY", true);
    deviceNode.replaceIDAttr("KERNEL_DATA_BLOCK_SIZE", 1ul);
    deviceNode.replaceIDAttr("KERNEL_TRANS_GRID_BLOCK_SIZE", 1ul);
    deviceNode.replaceIDAttr("KERNEL_TRANS_DATA_BLOCK_SIZE", 1ul);
    deviceNode.replaceTextAttr("KERNEL_STORE_DATA", "array");
    deviceNode.replaceIDAttr("KERNEL_MAX_DIM_UNROLL", 1ul);
  }

  SGPP::datadriven::OperationMultipleEvalConfiguration configuration(
      SGPP::datadriven::OperationMultipleEvalType::STREAMING,
      SGPP::datadriven::OperationMultipleEvalSubType::OCLFASTMP, *parameters);

  for (size_t i = 0; i < fileNames.size(); i++) {
    double mse = compareToReferenceTranspose(SGPP::base::GridType::ModLinear, fileNames[i], level,
                                             configuration);
    BOOST_CHECK(mse < errors[i]);
  }
}

BOOST_AUTO_TEST_CASE(Blocking) {
  std::vector<std::string> fileNames = {"datadriven/tests/data/friedman_4d.arff.gz",
                                        "datadriven/tests/data/friedman_10d.arff.gz"};
  std::vector<double> errors = {10E-11, 10E-11};

  uint32_t level = 4;

  std::shared_ptr<SGPP::base::OCLOperationConfiguration> parameters =
      getConfigurationDefaultsSingleDevice();

  (*parameters)["INTERNAL_PRECISION"].set("double");

  std::vector<std::reference_wrapper<json::Node>> deviceNodes = parameters->getAllDeviceNodes();

  for (json::Node &deviceNode : deviceNodes) {
    deviceNode.replaceIDAttr("KERNEL_USE_LOCAL_MEMORY", false);
    deviceNode.replaceIDAttr("KERNEL_DATA_BLOCK_SIZE", 2ul);
    deviceNode.replaceIDAttr("KERNEL_TRANS_GRID_BLOCK_SIZE", 2ul);
    deviceNode.replaceIDAttr("KERNEL_TRANS_DATA_BLOCK_SIZE", 2ul);
    deviceNode.replaceTextAttr("KERNEL_STORE_DATA", "register");
    deviceNode.replaceIDAttr("KERNEL_MAX_DIM_UNROLL", 10ul);
  }

  SGPP::datadriven::OperationMultipleEvalConfiguration configuration(
      SGPP::datadriven::OperationMultipleEvalType::STREAMING,
      SGPP::datadriven::OperationMultipleEvalSubType::OCLFASTMP, *parameters);

  for (size_t i = 0; i < fileNames.size(); i++) {
    double mse = compareToReferenceTranspose(SGPP::base::GridType::ModLinear, fileNames[i], level,
                                             configuration);
    BOOST_CHECK(mse < errors[i]);
  }
}

BOOST_AUTO_TEST_CASE(MultiDevice) {
  std::vector<std::string> fileNames = {"datadriven/tests/data/friedman_4d.arff.gz",
                                        "datadriven/tests/data/friedman_10d.arff.gz"};
  std::vector<double> errors = {10E-12, 10E-12};

  uint32_t level = 4;

  std::shared_ptr<SGPP::base::OCLOperationConfiguration> parameters =
      getConfigurationDefaultsMultiDevice();

  (*parameters)["INTERNAL_PRECISION"].set("double");

  std::vector<std::reference_wrapper<json::Node>> deviceNodes = parameters->getAllDeviceNodes();

  for (json::Node &deviceNode : deviceNodes) {
    deviceNode.replaceIDAttr("KERNEL_USE_LOCAL_MEMORY", false);
    deviceNode.replaceIDAttr("KERNEL_DATA_BLOCK_SIZE", 2ul);
    deviceNode.replaceIDAttr("KERNEL_TRANS_GRID_BLOCK_SIZE", 2ul);
    deviceNode.replaceIDAttr("KERNEL_TRANS_DATA_BLOCK_SIZE", 2ul);
    deviceNode.replaceTextAttr("KERNEL_STORE_DATA", "register");
    deviceNode.replaceIDAttr("KERNEL_MAX_DIM_UNROLL", 10ul);
  }

  SGPP::datadriven::OperationMultipleEvalConfiguration configuration(
      SGPP::datadriven::OperationMultipleEvalType::STREAMING,
      SGPP::datadriven::OperationMultipleEvalSubType::OCLFASTMP, *parameters);

  for (size_t i = 0; i < fileNames.size(); i++) {
    double mse = compareToReferenceTranspose(SGPP::base::GridType::ModLinear, fileNames[i], level,
                                             configuration);
    BOOST_CHECK(mse < errors[i]);
  }
}

BOOST_AUTO_TEST_CASE(MultiPlatform) {
  std::vector<std::string> fileNames = {"datadriven/tests/data/friedman_4d.arff.gz",
                                        "datadriven/tests/data/friedman_10d.arff.gz"};
  std::vector<double> errors = {10E-12, 10E-12};

  uint32_t level = 4;

  std::shared_ptr<SGPP::base::OCLOperationConfiguration> parameters =
      getConfigurationDefaultsMultiPlatform();

  (*parameters)["INTERNAL_PRECISION"].set("double");

  std::vector<std::reference_wrapper<json::Node>> deviceNodes = parameters->getAllDeviceNodes();

  for (json::Node &deviceNode : deviceNodes) {
    deviceNode.replaceIDAttr("KERNEL_USE_LOCAL_MEMORY", false);
    deviceNode.replaceIDAttr("KERNEL_DATA_BLOCK_SIZE", 2ul);
    deviceNode.replaceIDAttr("KERNEL_TRANS_GRID_BLOCK_SIZE", 2ul);
    deviceNode.replaceIDAttr("KERNEL_TRANS_DATA_BLOCK_SIZE", 2ul);
    deviceNode.replaceTextAttr("KERNEL_STORE_DATA", "register");
    deviceNode.replaceIDAttr("KERNEL_MAX_DIM_UNROLL", 10ul);
  }

  SGPP::datadriven::OperationMultipleEvalConfiguration configuration(
      SGPP::datadriven::OperationMultipleEvalType::STREAMING,
      SGPP::datadriven::OperationMultipleEvalSubType::OCLFASTMP, *parameters);

  for (size_t i = 0; i < fileNames.size(); i++) {
    double mse = compareToReferenceTranspose(SGPP::base::GridType::ModLinear, fileNames[i], level,
                                             configuration);
    BOOST_CHECK(mse < errors[i]);
  }
}

BOOST_AUTO_TEST_CASE(SimpleSinglePrecision) {
  std::vector<std::string> fileNames = {"datadriven/tests/data/friedman_4d.arff.gz",
                                        "datadriven/tests/data/friedman_10d.arff.gz"};
  std::vector<double> errors = {2.0E+04, 1.0E+4};

  uint32_t level = 4;

  std::shared_ptr<SGPP::base::OCLOperationConfiguration> parameters =
      getConfigurationDefaultsSingleDevice();

  (*parameters)["INTERNAL_PRECISION"].set("float");

  std::vector<std::reference_wrapper<json::Node>> deviceNodes = parameters->getAllDeviceNodes();

  for (json::Node &deviceNode : deviceNodes) {
    deviceNode.replaceIDAttr("KERNEL_USE_LOCAL_MEMORY", false);
    deviceNode.replaceIDAttr("KERNEL_DATA_BLOCK_SIZE", 1ul);
    deviceNode.replaceIDAttr("KERNEL_TRANS_GRID_BLOCK_SIZE", 1ul);
    deviceNode.replaceIDAttr("KERNEL_TRANS_DATA_BLOCK_SIZE", 1ul);
    deviceNode.replaceTextAttr("KERNEL_STORE_DATA", "array");
    deviceNode.replaceIDAttr("KERNEL_MAX_DIM_UNROLL", 1ul);
  }

  SGPP::datadriven::OperationMultipleEvalConfiguration configuration(
      SGPP::datadriven::OperationMultipleEvalType::STREAMING,
      SGPP::datadriven::OperationMultipleEvalSubType::OCLFASTMP, *parameters);

  for (size_t i = 0; i < fileNames.size(); i++) {
    double mse = compareToReferenceTranspose(SGPP::base::GridType::ModLinear, fileNames[i], level,
                                             configuration);
    BOOST_CHECK(mse < errors[i]);
  }
}

BOOST_AUTO_TEST_CASE(BlockingSinglePrecision) {
  std::vector<std::string> fileNames = {"datadriven/tests/data/friedman_4d.arff.gz",
                                        "datadriven/tests/data/friedman_10d.arff.gz"};
  std::vector<double> errors = {2.0E+04, 1.0E+4};

  uint32_t level = 4;

  std::shared_ptr<SGPP::base::OCLOperationConfiguration> parameters =
      getConfigurationDefaultsSingleDevice();

  (*parameters)["INTERNAL_PRECISION"].set("float");

  std::vector<std::reference_wrapper<json::Node>> deviceNodes = parameters->getAllDeviceNodes();

  for (json::Node &deviceNode : deviceNodes) {
    deviceNode.replaceIDAttr("KERNEL_USE_LOCAL_MEMORY", false);
    deviceNode.replaceIDAttr("KERNEL_DATA_BLOCK_SIZE", 2ul);
    deviceNode.replaceIDAttr("KERNEL_TRANS_GRID_BLOCK_SIZE", 2ul);
    deviceNode.replaceIDAttr("KERNEL_TRANS_DATA_BLOCK_SIZE", 2ul);
    deviceNode.replaceTextAttr("KERNEL_STORE_DATA", "register");
    deviceNode.replaceIDAttr("KERNEL_MAX_DIM_UNROLL", 10ul);
  }

  SGPP::datadriven::OperationMultipleEvalConfiguration configuration(
      SGPP::datadriven::OperationMultipleEvalType::STREAMING,
      SGPP::datadriven::OperationMultipleEvalSubType::OCLFASTMP, *parameters);

  for (size_t i = 0; i < fileNames.size(); i++) {
    double mse = compareToReferenceTranspose(SGPP::base::GridType::ModLinear, fileNames[i], level,
                                             configuration);
    BOOST_CHECK(mse < errors[i]);
  }
}

BOOST_AUTO_TEST_CASE(MultiDeviceSinglePrecision) {
  std::vector<std::string> fileNames = {"datadriven/tests/data/friedman_4d.arff.gz",
                                        "datadriven/tests/data/friedman_10d.arff.gz"};
  std::vector<double> errors = {2.0E+04, 1.0E+4};

  uint32_t level = 4;

  std::shared_ptr<SGPP::base::OCLOperationConfiguration> parameters =
      getConfigurationDefaultsMultiDevice();

  (*parameters)["INTERNAL_PRECISION"].set("float");

  std::vector<std::reference_wrapper<json::Node>> deviceNodes = parameters->getAllDeviceNodes();

  for (json::Node &deviceNode : deviceNodes) {
    deviceNode.replaceIDAttr("KERNEL_USE_LOCAL_MEMORY", false);
    deviceNode.replaceIDAttr("KERNEL_DATA_BLOCK_SIZE", 2ul);
    deviceNode.replaceIDAttr("KERNEL_TRANS_GRID_BLOCK_SIZE", 2ul);
    deviceNode.replaceIDAttr("KERNEL_TRANS_DATA_BLOCK_SIZE", 2ul);
    deviceNode.replaceTextAttr("KERNEL_STORE_DATA", "register");
    deviceNode.replaceIDAttr("KERNEL_MAX_DIM_UNROLL", 10ul);
  }

  SGPP::datadriven::OperationMultipleEvalConfiguration configuration(
      SGPP::datadriven::OperationMultipleEvalType::STREAMING,
      SGPP::datadriven::OperationMultipleEvalSubType::OCLFASTMP, *parameters);

  for (size_t i = 0; i < fileNames.size(); i++) {
    double mse = compareToReferenceTranspose(SGPP::base::GridType::ModLinear, fileNames[i], level,
                                             configuration);
    BOOST_CHECK(mse < errors[i]);
  }
}

BOOST_AUTO_TEST_CASE(MultiPlatformSinglePrecision) {
  std::vector<std::string> fileNames = {"datadriven/tests/data/friedman_4d.arff.gz",
                                        "datadriven/tests/data/friedman_10d.arff.gz"};
  std::vector<double> errors = {2.0E+04, 1.0E+4};

  uint32_t level = 4;

  std::shared_ptr<SGPP::base::OCLOperationConfiguration> parameters =
      getConfigurationDefaultsMultiPlatform();

  (*parameters)["INTERNAL_PRECISION"].set("float");

  std::vector<std::reference_wrapper<json::Node>> deviceNodes = parameters->getAllDeviceNodes();

  for (json::Node &deviceNode : deviceNodes) {
    deviceNode.replaceIDAttr("KERNEL_USE_LOCAL_MEMORY", false);
    deviceNode.replaceIDAttr("KERNEL_DATA_BLOCK_SIZE", 2ul);
    deviceNode.replaceIDAttr("KERNEL_TRANS_GRID_BLOCK_SIZE", 2ul);
    deviceNode.replaceIDAttr("KERNEL_TRANS_DATA_BLOCK_SIZE", 2ul);
    deviceNode.replaceTextAttr("KERNEL_STORE_DATA", "register");
    deviceNode.replaceIDAttr("KERNEL_MAX_DIM_UNROLL", 10ul);
  }

  SGPP::datadriven::OperationMultipleEvalConfiguration configuration(
      SGPP::datadriven::OperationMultipleEvalType::STREAMING,
      SGPP::datadriven::OperationMultipleEvalSubType::OCLFASTMP, *parameters);

  for (size_t i = 0; i < fileNames.size(); i++) {
    double mse = compareToReferenceTranspose(SGPP::base::GridType::ModLinear, fileNames[i], level,
                                             configuration);
    BOOST_CHECK(mse < errors[i]);
  }
}

BOOST_AUTO_TEST_SUITE_END()

#endif
