// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef BSPLINE_MODIFIED_CLENSHAW_CURTIS_BASE_HPP
#define BSPLINE_MODIFIED_CLENSHAW_CURTIS_BASE_HPP

#include <cmath>
#include <sgpp/base/operation/hash/common/basis/Basis.hpp>
#include <sgpp/base/operation/hash/common/basis/BsplineBasis.hpp>
#include <sgpp/base/tools/ClenshawCurtisTable.hpp>

#include <sgpp/globaldef.hpp>

namespace SGPP {
  namespace base {

    /**
     * B-spline basis on Clenshaw-Curtis grids.
     */
    template<class LT, class IT>
    class BsplineModifiedClenshawCurtisBasis: public Basis<LT, IT> {
      public:
        /**
         * Default constructor.
         */
        BsplineModifiedClenshawCurtisBasis()
          : BsplineModifiedClenshawCurtisBasis(0) {
        }

        /**
         * Constructor.
         *
         * @param degree        B-spline degree, must be odd
         *                      (if it's even, degree - 1 is used)
         */
        BsplineModifiedClenshawCurtisBasis(size_t degree)
          : degree(degree), xi(std::vector<float_t>(degree + 2, 0.0)) {
          if (degree < 1) {
            this->degree = 1;
          } else if (degree % 2 == 0) {
            this->degree = degree - 1;
          }
        }

        /**
         * @param l     level of the grid point
         * @param i     index of the grid point
         * @return      i-th Clenshaw-Curtis grid point with level l
         */
        inline float_t clenshawCurtisPoint(LT l, IT i) const {
          const IT hInv = static_cast<IT>(1) << l;

          if (i == 0) {
            return -clenshawCurtisTable.getPoint(l, 1);
          } else if (i >= hInv) {
            const float_t offset = static_cast<float_t>((i - 1) / (hInv - 1));
            i = (i - 1) % (hInv - 1) + 1;
            return clenshawCurtisTable.getPoint(l, i) + offset;
          } else {
            return clenshawCurtisTable.getPoint(l, i);
          }
        }

        /**
         * @param l     level of the grid point
         * @param ni    neagtive index -i of the grid point
         * @return      (-ni)-th Clenshaw-Curtis grid point with level l
         */
        inline float_t clenshawCurtisPointNegativeIndex(LT l, IT ni) const {
          const IT hInv = static_cast<IT>(1) << l;
          const float_t offset = -static_cast<float_t>(ni / (hInv - 1)) - 1.0;
          const IT i = hInv - 1 - ni % (hInv - 1);
          return clenshawCurtisTable.getPoint(l, i) + offset;
        }

        /**
         * @param l     level of basis function
         * @param i     index of basis function
         * @param x     evaluation point
         * @return      value of modified Clenshaw-Curtis
         *              B-spline basis function
         */
        inline float_t eval(LT l, IT i, float_t x) {
          if (l == 1) {
            return 1.0;
          }

          const IT hInv = static_cast<IT>(1) << l;

          if (i == 1) {
            return modifiedBSpline(l, x, degree);
          } else if (i == hInv - 1) {
            return modifiedBSpline(l, 1.0 - x, degree);
          } else {
            constructKnots(l, i);
            return nonUniformBSpline(x, degree, 0);
          }
        }

        /**
         * @param l     level of basis function
         * @param i     index of basis function
         * @param x     evaluation point
         * @return      value of derivative of modified Clenshaw-Curtis
         *              B-spline basis function
         */
        inline float_t evalDx(LT l, IT i, float_t x) {
          if (l == 1) {
            return 0.0;
          }

          const IT hInv = static_cast<IT>(1) << l;

          if (i == 1) {
            return modifiedBSplineDx(l, x, degree);
          } else if (i == hInv - 1) {
            return modifiedBSplineDx(l, 1.0 - x, degree);
          } else {
            constructKnots(l, i);
            return nonUniformBSplineDx(x, degree, 0);
          }
        }

        /**
         * @param l     level of basis function
         * @param i     index of basis function
         * @param x     evaluation point
         * @return      value of 2nd derivative of modified Clenshaw-Curtis
         *              B-spline basis function
         */
        inline float_t evalDxDx(LT l, IT i, float_t x) {
          if (l == 1) {
            return 0.0;
          }

          const IT hInv = static_cast<IT>(1) << l;

          if (i == 1) {
            return modifiedBSplineDxDx(l, x, degree);
          } else if (i == hInv - 1) {
            return modifiedBSplineDxDx(l, 1.0 - x, degree);
          } else {
            constructKnots(l, i);
            return nonUniformBSplineDxDx(x, degree, 0);
          }
        }

        /**
         * @return      B-spline degree
         */
        inline size_t getDegree() const {
          return degree;
        }

      protected:
        /// degree of the B-spline
        size_t degree;
        /// temporary helper vector of fixed size p+2 containing B-spline knots
        std::vector<float_t> xi;

        /**
         * Construct the (p+2) Clenshaw-Curtis knots of a
         * B-spline basis function and save them in xi.
         *
         * @param l     level of basis function
         * @param i     index of basis function
         */
        inline void constructKnots(LT l, IT i) {
          const IT degreePlusOneHalved = static_cast<IT>(degree + 1) / 2;

          for (IT k = 0; k < degree + 2; k++) {
            if (i + k >= degreePlusOneHalved) {
              xi[k] = clenshawCurtisPoint(
                        l, i + k - degreePlusOneHalved);
            } else {
              xi[k] = clenshawCurtisPointNegativeIndex(
                        l, degreePlusOneHalved - i - k);
            }
          }
        }

        /**
         * Construct the (p+2) Clenshaw-Curtis knots of a
         * B-spline basis function and save them in xi.
         *
         * @param l     level of basis function
         * @param ni    negative index -i of basis function
         */
        inline void constructKnotsNegativeIndex(LT l, IT ni) {
          const IT degreePlusOneHalved = static_cast<IT>(degree + 1) / 2;

          for (IT k = 0; k < degree + 2; k++) {
            if (k >= degreePlusOneHalved + ni) {
              xi[k] = clenshawCurtisPoint(
                        l, k - ni - degreePlusOneHalved);
            } else {
              xi[k] = clenshawCurtisPointNegativeIndex(
                        l, degreePlusOneHalved + ni - k);
            }
          }
        }

        /**
         * @param x     evaluation point
         * @param p     B-spline degree
         * @param k     index of B-spline in the knot sequence
         * @param xi    knot sequence
         * @return      value of non-uniform B-spline with knots
         *              \f$\{\xi_k, ... \xi_{k+p+1}\}\f$
         */
        inline float_t nonUniformBSpline(float_t x, size_t p, size_t k) const {
          if (p == 0) {
            // characteristic function of [xi[k], xi[k+1])
            return (((x >= xi[k]) && (x < xi[k + 1])) ? 1.0 : 0.0);
          } else if ((x < xi[k]) || (x >= xi[k + p + 1])) {
            // out of support
            return 0.0;
          } else {
            // Cox-de-Boor recursion
            return (x - xi[k]) / (xi[k + p] - xi[k])
                   * nonUniformBSpline(x, p - 1, k)
                   + (1.0 - (x - xi[k + 1]) / (xi[k + p + 1] - xi[k + 1]))
                   * nonUniformBSpline(x, p - 1, k + 1);
          }
        }

        /**
         * @param x     evaluation point
         * @param p     B-spline degree
         * @param k     index of B-spline in the knot sequence
         * @param xi    knot sequence
         * @return      value of derivative of non-uniform B-spline with knots
         *              \f$\{\xi_k, ... \xi_{k+p+1}\}\f$
         */
        inline float_t nonUniformBSplineDx(float_t x, size_t p, size_t k) const {
          if (p == 0) {
            return 0.0;
          } else if ((x < xi[k]) || (x >= xi[k + p + 1])) {
            return 0.0;
          } else {
            const float_t pDbl = static_cast<float_t>(p);

            return pDbl / (xi[k + p] - xi[k]) * nonUniformBSpline(x, p - 1, k)
                   - pDbl / (xi[k + p + 1] - xi[k + 1])
                   * nonUniformBSpline(x, p - 1, k + 1);
          }
        }

        /**
         * @param x     evaluation point
         * @param p     B-spline degree
         * @param k     index of B-spline in the knot sequence
         * @param xi    knot sequence
         * @return      value of 2nd derivative of non-uniform B-spline
         *              with knots \f$\{\xi_k, ... \xi_{k+p+1}\}\f$
         */
        inline float_t nonUniformBSplineDxDx(
          float_t x, size_t p, size_t k) const {
          if (p <= 1) {
            return 0.0;
          } else if ((x < xi[k]) || (x >= xi[k + p + 1])) {
            return 0.0;
          } else {
            const float_t pDbl = static_cast<float_t>(p);
            const float_t alphaKP = pDbl / (xi[k + p] - xi[k]);
            const float_t alphaKp1P = pDbl / (xi[k + p + 1] - xi[k + 1]);
            const float_t alphaKPm1 = (pDbl - 1.0) / (xi[k + p - 1] - xi[k]);
            const float_t alphaKp1Pm1 = (pDbl - 1.0) / (xi[k + p] - xi[k + 1]);
            const float_t alphaKp2Pm1 = (pDbl - 1.0) /
                                        (xi[k + p + 1] - xi[k + 2]);

            return alphaKP * alphaKPm1 * nonUniformBSpline(x, p - 2, k)
                   - (alphaKP + alphaKp1P) * alphaKp1Pm1
                   * nonUniformBSpline(x, p - 2, k + 1)
                   + alphaKp1P * alphaKp2Pm1 *
                   nonUniformBSpline(x, p - 2, k + 2);
          }
        }

        /**
         * @param l     level of basis function
         * @param x     evaluation point
         * @param p     B-spline degree
         * @return      value of modified
         *              Clenshaw-Curtis B-spline (e.g. index == 1)
         */
        inline float_t modifiedBSpline(LT l, float_t x, size_t p) {
          float_t y = 0.0;
          constructKnots(l, 1);
          y += 1.0 * nonUniformBSpline(x, degree, 0);
          constructKnots(l, 0);
          y += 2.0 * nonUniformBSpline(x, degree, 0);

          // the upper summation bound is defined to be ceil((p + 1) / 2.0),
          // which is the same as (p + 2) / 2 written in C
          for (IT k = 2; k <= (p + 2) / 2; k++) {
            constructKnotsNegativeIndex(l, k - 1);
            y += static_cast<float_t>(k + 1) *
                 nonUniformBSpline(x, degree, 0);
          }

          return y;
        }

        /**
         * @param l     level of basis function
         * @param x     evaluation point
         * @param p     B-spline degree
         * @return      value of derivative of modified
         *              Clenshaw-Curtis B-spline (e.g. index == 1)
         */
        inline float_t modifiedBSplineDx(LT l, float_t x, size_t p) {
          float_t y = 0.0;
          constructKnots(l, 1);
          y += 1.0 * nonUniformBSplineDx(x, degree, 0);
          constructKnots(l, 0);
          y += 2.0 * nonUniformBSplineDx(x, degree, 0);

          // the upper summation bound is defined to be ceil((p + 1) / 2.0),
          // which is the same as (p + 2) / 2 written in C
          for (IT k = 2; k <= (p + 2) / 2; k++) {
            constructKnotsNegativeIndex(l, k - 1);
            y += static_cast<float_t>(k + 1) *
                 nonUniformBSplineDx(x, degree, 0);
          }

          return y;
        }

        /**
         * @param l     level of basis function
         * @param x     evaluation point
         * @param p     B-spline degree
         * @return      value of 2nd derivative of modified
         *              Clenshaw-Curtis B-spline (e.g. index == 1)
         */
        inline float_t modifiedBSplineDxDx(LT l, float_t x, size_t p) {
          float_t y = 0.0;
          constructKnots(l, 1);
          y += 1.0 * nonUniformBSplineDxDx(x, degree, 0);
          constructKnots(l, 0);
          y += 2.0 * nonUniformBSplineDxDx(x, degree, 0);

          // the upper summation bound is defined to be ceil((p + 1) / 2.0),
          // which is the same as (p + 2) / 2 written in C
          for (size_t k = 2; k <= (p + 2) / 2; k++) {
            constructKnotsNegativeIndex(l, k - 1);
            y += static_cast<float_t>(k + 1) *
                 nonUniformBSplineDxDx(x, degree, 0);
          }

          return y;
        }
    };

    // default type-def (unsigned int for level and index)
    typedef BsplineModifiedClenshawCurtisBasis<unsigned int, unsigned int>
    SBsplineModifiedClenshawCurtisBase;
  }
}

#endif /* BSPLINE_MODIFIED_CLENSHAW_CURTIS_BASE_HPP */
