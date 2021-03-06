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

#ifndef INCLUDE_RELE_SOLVERS_DYNAMIC_PROGRAMMING_BASICDYNAMICPROGRAMMING_H_
#define INCLUDE_RELE_SOLVERS_DYNAMIC_PROGRAMMING_BASICDYNAMICPROGRAMMING_H_

#include "rele/core/Solver.h"
#include "rele/core/FiniteMDP.h"
#include "rele/policy/nonparametric/DeterministicPolicy.h"

#include <armadillo>

namespace ReLe
{

/*!
 * This class implements the Dynamic Programming (DP) algorithm.
 */
class DynamicProgrammingAlgorithm : public Solver<FiniteAction, FiniteState>
{
public:
    /*!
     * Constructor.
     * \param mdp the mdp to solve
     */
    DynamicProgrammingAlgorithm(FiniteMDP& mdp);

    virtual Policy<FiniteAction, FiniteState>& getPolicy() override;
    virtual Dataset<FiniteAction, FiniteState> test() override;

    /*!
     * Getter
     * \return the value function
     */
    inline arma::vec getValueFunction()
    {
        return V;
    }

protected:
    FiniteMDP& mdp;
    size_t stateN;
    unsigned int actionN;
    double gamma;

    const arma::cube& P;
    const arma::cube& R;

    DeterministicPolicy pi;
    arma::vec V;
};

/*!
 * This class implements the Policy Iteration algorithm.
 */
class PolicyIteration : public DynamicProgrammingAlgorithm
{
public:
    /*!
     * Constructor.
     * \param mdp the mdp to solve
     */
    PolicyIteration(FiniteMDP& mdp);

    virtual void solve() override;

    /*!
     * Destructor.
     */
    virtual ~PolicyIteration();

private:
    void computeValueFunction();
    void computePolicy();

private:
    bool changed;
};

/*!
 * This class implements the Value Iteration algorithm.
 */
class ValueIteration : public DynamicProgrammingAlgorithm
{
public:
    /*!
     * Constructor.
     * \param mdp the mdp to solve
     * \param eps threshold to be evaluated to stop the algorithm
     */
    ValueIteration(FiniteMDP& mdp, double eps);

    virtual void solve() override;

    /*!
     * Destructor.
     */
    virtual ~ValueIteration();

private:
    void computeValueFunction();
    void computePolicy();

private:
    double eps;

    arma::vec Vold;
};



}


#endif /* INCLUDE_RELE_SOLVERS_DYNAMIC_PROGRAMMING_BASICDYNAMICPROGRAMMING_H_ */
