// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/globaldef.hpp>

#include <sgpp/optimization/sle/solver/Eigen.hpp>
#include <sgpp/optimization/sle/system/CloneableSLE.hpp>
#include <sgpp/optimization/tools/Printer.hpp>

#ifdef USEEIGEN
#include <eigen3/Eigen/Dense>
#endif /* USEEIGEN */

#include <cstddef>
#include <iostream>

namespace SGPP {
  namespace optimization {
    namespace sle_solver {

#ifdef USEEIGEN

#if USE_DOUBLE_PRECISION == 1
      typedef ::Eigen::VectorXd EigenVector;
      typedef ::Eigen::MatrixXd EigenMatrix;
#else
      typedef ::Eigen::VectorXf EigenVector;
      typedef ::Eigen::MatrixXf EigenMatrix;
#endif

      /**
       * @param       A     coefficient matrix
       * @param       A_QR  Householder QR decomposition of coefficient matrix
       * @param       b     right-hand side
       * @param[out]  x     solution of the linear system
       * @return            whether all went well (false if errors occurred)
       */
      bool solveInternal(
        const EigenMatrix& A,
        const ::Eigen::HouseholderQR<EigenMatrix>& A_QR,
        const std::vector<float_t>& b,
        std::vector<float_t>& x) {
        // solve system
        EigenVector bEigen = EigenVector::Map(&b[0], b.size());
        EigenVector xEigen = A_QR.solve(bEigen);

        // check solution
        if ((A * xEigen).isApprox(bEigen)) {
          x = std::vector<float_t>(xEigen.data(),
                                   xEigen.data() + xEigen.size());
          return true;
        } else {
          return false;
        }
      }
#endif /* USEEIGEN */

      bool Eigen::solve(SLE& system, const std::vector<float_t>& b,
                        std::vector<float_t>& x) const {
        std::vector<std::vector<float_t>> B;
        std::vector<std::vector<float_t>> X;
        B.push_back(b);

        if (solve(system, B, X)) {
          x = X[0];
          return true;
        } else {
          return false;
        }
      }

      bool Eigen::solve(SLE& system,
                        const std::vector<std::vector<float_t>>& B,
                        std::vector<std::vector<float_t>>& X) const {
#ifdef USEEIGEN
        printer.printStatusBegin("Solving linear system (Eigen)...");

        const size_t n = system.getDimension();
        EigenMatrix A = EigenMatrix::Zero(n, n);
        size_t nnz = 0;

        // parallelize only if the system is cloneable
        #pragma omp parallel if (system.isCloneable()) \
        shared(system, A, nnz, printer) default(none)
        {
          SLE* system2 = &system;
#ifdef _OPENMP
          std::unique_ptr<CloneableSLE> clonedSLE;

          if (system.isCloneable() && (omp_get_max_threads() > 1)) {
            dynamic_cast<CloneableSLE&>(system).clone(clonedSLE);
            system2 = clonedSLE.get();
          }

#endif /* _OPENMP */

          // copy system matrix to Eigen matrix object
          #pragma omp for ordered schedule(dynamic)

          for (size_t i = 0; i < n; i++) {
            for (size_t j = 0; j < n; j++) {
              A(i, j) = system2->getMatrixEntry(i, j);

              // count nonzero entries
              // (not necessary, you can also remove that if you like)
              if (A(i, j) != 0) {
                #pragma omp atomic
                nnz++;
              }
            }

            // status message
            if (i % 100 == 0) {
              #pragma omp ordered
              {
                char str[10];
                snprintf(str, 10, "%.1f%%",
                static_cast<float_t>(i) / static_cast<float_t>(n) * 100.0);
                printer.printStatusUpdate("constructing matrix (" +
                std::string(str) + ")");
              }
            }
          }
        }

        printer.printStatusUpdate("constructing matrix (100.0%)");
        printer.printStatusNewLine();

        // print ratio of nonzero entries
        {
          char str[10];
          float_t nnz_ratio = static_cast<float_t>(nnz) /
                              (static_cast<float_t>(n) *
                               static_cast<float_t>(n));
          snprintf(str, 10, "%.1f%%", nnz_ratio * 100.0);
          printer.printStatusUpdate("nnz ratio: " + std::string(str));
          printer.printStatusNewLine();
        }

        // calculate QR factorization of system matrix
        printer.printStatusUpdate("step 1: Householder QR factorization");
        ::Eigen::HouseholderQR<EigenMatrix> A_QR = A.householderQr();

        std::vector<float_t> x;
        X.clear();

        // solve system for each RHS
        for (size_t i = 0; i < B.size(); i++) {
          const std::vector<float_t>& b = B[i];
          printer.printStatusNewLine();

          if (B.size() == 1) {
            printer.printStatusUpdate("step 2: solving");
          } else {
            printer.printStatusUpdate("step 2: solving (RHS " +
                                      std::to_string(i + 1) +
                                      " of " + std::to_string(B.size()) + ")");
          }

          if (solveInternal(A, A_QR, b, x)) {
            X.push_back(x);
          } else {
            printer.printStatusEnd("error: could not solve linear system!");
            return false;
          }
        }

        printer.printStatusEnd();
        return true;
#else
        std::cerr << "Error in sle_solver::Eigen::solve: "
                  << "SG++ was compiled without Eigen support!\n";
        return false;
#endif /* USEEIGEN */
      }

    }
  }
}