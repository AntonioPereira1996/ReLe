/*
 * rele,
 *
 *
 * Copyright (C) 2015 Davide Tateo
 * Versione 1.0
 *
 * This file is part of rele.
 *
 * rele is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rele is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with rele.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDE_RELE_APPROXIMATORS_FEATURES_H_
#define INCLUDE_RELE_APPROXIMATORS_FEATURES_H_

#include "BasisFunctions.h"

namespace ReLe
{

class BasisMatrix
{
public:
    virtual ~BasisMatrix()
    {
    }

    virtual arma::mat operator()(const arma::vec& input) = 0;
    virtual size_t rows() const = 0;
    virtual size_t cols() const = 0;

};

class DenseBasisMatrix: public BasisMatrix
{
public:
    DenseBasisMatrix(BasisFunctions& basisVector);
    DenseBasisMatrix(BasisFunctions& basisVector, unsigned int rows, unsigned int cols);
    virtual ~DenseBasisMatrix();
    virtual arma::mat operator()(const arma::vec& input);

    inline virtual size_t rows() const
    {
        return basis.n_rows;
    }

    inline virtual size_t cols() const
    {
        return basis.n_cols;
    }

private:
    arma::field<BasisFunction*> basis;

};

class SparseBasisMatrix: public BasisMatrix
{
public:
    SparseBasisMatrix();

    SparseBasisMatrix(BasisFunctions& basis,
                      unsigned int nbReplication = 1,
                      bool indipendent = true);

    // AbstractBasisMatrix interface
public:
    arma::mat operator ()(const arma::vec& input);
    inline size_t rows() const
    {
        return n_rows;
    }

    size_t cols() const
    {
        return n_cols;
    }

    void addBasis(unsigned int row, unsigned int col, BasisFunction* bfs);

private:
    //an element of the matrix is given by (rowsIdxs[i], colsIdxs[i], values[i])
    std::vector<unsigned int> rowsIdxs, colsIdxs;
    std::vector<BasisFunction*> values;
    unsigned int n_rows, n_cols;
};

}

#endif /* INCLUDE_RELE_APPROXIMATORS_FEATURES_H_ */
