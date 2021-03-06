/*
 * rele,
 *
 *
 * Copyright (C) 2015 Davide Tateo & Matteo Pirotta
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

#ifndef INCLUDE_RELE_APPROXIMATORS_BASIS_NORMBASIS_H_
#define INCLUDE_RELE_APPROXIMATORS_BASIS_NORMBASIS_H_

#include "rele/approximators/BasisFunctions.h"

#include <string>

namespace ReLe
{

/*!
 * This class implements function to return the p-norm of an
 * input feature.
 */
class NormBasis : public BasisFunction
{
public:
    /*!
     * Constructor.
     * \param p the p of the p-norm
     */
    NormBasis(unsigned int p = 2);

    virtual double operator() (const arma::vec& input) override;
    virtual void writeOnStream (std::ostream& out) override;
    virtual void readFromStream(std::istream& in) override;

private:
    unsigned int p;
};

/*!
 * This class implements function to return the infinite norm of an
 * input feature.
 */
class InfiniteNorm : public BasisFunction
{
public:
    /*!
     * Constructor.
     * \param max flag to indicate wheter to use inf or -inf for the p of the p-norm
     */
    InfiniteNorm(bool max = true);

    virtual double operator() (const arma::vec& input) override;
    virtual void writeOnStream (std::ostream& out) override;
    virtual void readFromStream(std::istream& in) override;

private:
    std::string type;
};

}




#endif /* INCLUDE_RELE_APPROXIMATORS_BASIS_NORMBASIS_H_ */
