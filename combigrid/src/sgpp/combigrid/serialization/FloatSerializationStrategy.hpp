// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef COMBIGRID_SRC_SGPP_COMBIGRID_SERIALIZATION_FLOATSERIALIZATIONSTRATEGY_HPP_
#define COMBIGRID_SRC_SGPP_COMBIGRID_SERIALIZATION_FLOATSERIALIZATIONSTRATEGY_HPP_

#include <sgpp/combigrid/serialization/AbstractSerializationStrategy.hpp>
#include <sgpp/globaldef.hpp>

#include <cmath>
#include <limits>
#include <sstream>
#include <string>

namespace sgpp {
namespace combigrid {

/**
 * This class provides a serialization strategy for floating-point types (float, double), though not
 * particularly well-compressed (roughly 70 characters per float will be used).
 */
template <typename T>
class FloatSerializationStrategy : public AbstractSerializationStrategy<T> {
  static const int64_t SHIFT_AMOUNT = 58;

  static const char normalPrefix = 'd';
  static const char infPrefix = 'u';
  static const char minusInfPrefix = 'l';
  static const char nanPrefix = 'n';

 public:
  virtual ~FloatSerializationStrategy() {}

  virtual std::string serialize(T const &value) {
    if (std::isnan(value)) {
      return std::string(1, nanPrefix);
    } else if (std::isinf(value)) {
      if (value < 0) {
        return std::string(1, minusInfPrefix);
      } else {
        return std::string(1, infPrefix);
      }
    }

    int exponent = 0;

    T normalized = frexp(value, &exponent);

    int64_t shifted =
        static_cast<int64_t>(normalized * static_cast<T>(static_cast<int64_t>(1) << SHIFT_AMOUNT));

    std::ostringstream str;
    str << shifted << "e" << exponent;

    return normalPrefix + str.str();
  }

  virtual T deserialize(std::string const &input) {
    if (input[0] == infPrefix) {
      return std::numeric_limits<T>::infinity();
    } else if (input[0] == minusInfPrefix) {
      return -std::numeric_limits<T>::infinity();
    } else if (input[0] == nanPrefix) {
      return std::numeric_limits<T>::quiet_NaN();
    }

    std::istringstream str(input.substr(1));

    int exponent;
    int64_t shifted;

    str >> shifted;
    str.get();
    str >> exponent;

    int64_t one = 1;

    if (exponent >= 0) {
      return (static_cast<T>(shifted) / static_cast<T>(one << SHIFT_AMOUNT)) *
             static_cast<T>(one << exponent);
    } else {
      return (static_cast<T>(shifted) / static_cast<T>(one << SHIFT_AMOUNT)) /
             static_cast<T>(one << (-exponent));
    }
  }
};

} /* namespace combigrid */
} /* namespace sgpp*/

#endif /* COMBIGRID_SRC_SGPP_COMBIGRID_SERIALIZATION_FLOATSERIALIZATIONSTRATEGY_HPP_ */
