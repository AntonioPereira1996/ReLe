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


#ifndef INCLUDE_RELE_ALGORITHMS_BATCH_DOUBLEFQI_H_
#define INCLUDE_RELE_ALGORITHMS_BATCH_DOUBLEFQI_H_

#include "FQI.h"
#include "Ensemble.h"


namespace ReLe
{

class DoubleFQIEnsemble: public Ensemble
{
public:
    DoubleFQIEnsemble(Features& phi,
                      BatchRegressor& QRegressorA,
                      BatchRegressor& QRegressorB) :
        Ensemble(phi, 1),
        QRegressorA(QRegressorA),
        QRegressorB(QRegressorB)
    {
    }

    arma::vec operator()(const arma::vec& input, const arma::vec& output)
    {
        return (QRegressorA(input, output) + QRegressorB(input, output)) / 2;
    }

    virtual BatchRegressor& getQRegressorA()
    {
        return QRegressorA;
    }

    virtual BatchRegressor& getQRegressorB()
    {
        return QRegressorB;
    }

    virtual void writeOnStream(std::ofstream& out) override
    {
        // TODO: Implement
    }

    virtual void readFromStream(std::ifstream& in) override
    {
        // TODO: Implement
    }

protected:
    BatchRegressor& QRegressorA;
    BatchRegressor& QRegressorB;
};


template<class StateC>
class DoubleFQI: public FQI<StateC>
{
public:

    /* This class implements the Double FQI algorithm. As a batch algorithm, it takes
     * a dataset of (s, a, r, s') transitions, together with the regressors that
     * it is used to approximate the target distribution of Q values.
     */
    DoubleFQI(Dataset<FiniteAction, StateC>& data,
              Features& phi,
              BatchRegressor& QRegressorA,
              BatchRegressor& QRegressorB,
              unsigned int nStates,
              unsigned int nActions,
              double gamma) :
        QRegressorEnsemble(phi, QRegressorA, QRegressorB),
        FQI<StateC>(data, QRegressorEnsemble, nStates, nActions, gamma)
    {
    }

    void step(arma::mat input, arma::mat& output, const arma::mat rewards) override
    {
        unsigned int selectedQ = RandomGenerator::sampleUniformInt(0, 1);

        if(selectedQ == 0)
            doubleFQIStep(QRegressorEnsemble.getQRegressorA(), QRegressorEnsemble.getQRegressorB(), input, output, rewards);
        else
            doubleFQIStep(QRegressorEnsemble.getQRegressorB(), QRegressorEnsemble.getQRegressorA(), input, output, rewards);
    }

    void doubleFQIStep(BatchRegressor& trainingRegressor,
                       BatchRegressor& evaluationRegressor,
                       arma::mat input,
                       arma::mat& output,
                       const arma::mat rewards)
    {
        // Loop on each dataset sample (i.e. on each transition)
        unsigned int i = 0;

        for(auto& episode : this->data)
        {
            for(auto& tr : episode)
            {
                /* For the current s', Q values for each action are stored in
                 * Q_xn. The optimal Bellman equation can be computed
                 * finding the maximum value inside Q_xn. They are zero if
                 * xn is an absorbing state. Note that here we exchange the
                 * regressor according to Double Q-Learning algorithm.
                 */
                arma::vec Q_xn(this->nActions, arma::fill::zeros);
                if(!tr.xn.isAbsorbing())
                    for(unsigned int u = 0; u < this->nActions; u++)
                        Q_xn(u) = arma::as_scalar(trainingRegressor(tr.xn, FiniteAction(u)));

                double qmax = Q_xn.max();
                arma::uvec maxIndex = arma::find(Q_xn == qmax);
                unsigned int index = RandomGenerator::sampleUniformInt(0,
                                     maxIndex.n_elem - 1);
                output(i) = arma::as_scalar(rewards(0, i) + this->gamma * evaluationRegressor(tr.xn, FiniteAction(index)));

                i++;
            }
        }

        // The regressor is trained
        BatchDataFeatures<arma::vec, arma::vec> featureDataset(input, output);
        trainingRegressor.trainFeatures(featureDataset);
    }

protected:
    DoubleFQIEnsemble QRegressorEnsemble;
};

}

#endif /* INCLUDE_RELE_ALGORITHMS_BATCH_DOUBLEFQI_H_ */