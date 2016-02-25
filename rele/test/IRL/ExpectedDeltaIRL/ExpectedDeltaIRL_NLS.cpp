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

#include "rele/approximators/features/SparseFeatures.h"
#include "rele/approximators/features/DenseFeatures.h"
#include "rele/approximators/regressors/others/GaussianMixtureModels.h"
#include "rele/approximators/basis/IdentityBasis.h"
#include "rele/approximators/basis/PolynomialFunction.h"
#include "rele/approximators/basis/GaussianRbf.h"

#include "rele/policy/parametric/differentiable/NormalPolicy.h"

#include "rele/environments/NLS.h"

#include "rele/core/PolicyEvalAgent.h"
#include "rele/core/Core.h"
#include "rele/IRL/ParametricRewardMDP.h"
#include "rele/IRL/algorithms/ExpectedDeltaIRL.h"
#include "rele/algorithms/policy_search/gradient/onpolicy/GPOMDPAlgorithm.h"

#include "rele/utils/FileManager.h"

using namespace std;
using namespace arma;
using namespace ReLe;

//#define PRINT
#define RUN_GIRL
#define RECOVER

int main(int argc, char *argv[])
{
//  RandomGenerator::seed(45423424);
//  RandomGenerator::seed(8763575);


    IrlGrad atype = IrlGrad::REINFORCE_BASELINE;
    int nbEpisodes = 5000;

    FileManager fm("nls", "ExpectedDeltaIRL");
    fm.createDir();
    fm.cleanDir();
    std::cout << std::setprecision(OS_PRECISION);


    NLS mdp;

    //Setup expert policy
    int dim = mdp.getSettings().continuosStateDim;

    BasisFunctions basis = IdentityBasis::generate(dim);
    DenseFeatures phi(basis);

    BasisFunctions stdBasis = PolynomialFunction::generate(1, dim);
    DenseFeatures stdPhi(stdBasis);
    arma::vec stdWeights(stdPhi.rows());
    stdWeights.fill(0.1);

    NormalStateDependantStddevPolicy expertPolicy(phi, stdPhi, stdWeights);

    arma::vec p(2);
    p(0) = 6.5178;
    p(1) = -2.5994;

    expertPolicy.setParameters(p);

    PolicyEvalAgent<DenseAction, DenseState> expert(expertPolicy);

    // Generate expert dataset
    Core<DenseAction, DenseState> expertCore(mdp, expert);
    CollectorStrategy<DenseAction, DenseState> collection;
    expertCore.getSettings().loggerStrategy = &collection;
    expertCore.getSettings().episodeLength = mdp.getSettings().horizon;
    expertCore.getSettings().testEpisodeN = nbEpisodes;
    expertCore.runTestEpisodes();
    Dataset<DenseAction,DenseState>& data = collection.data;


    // Create parametric reward
    //BasisFunctions basisReward = IdentityBasis::generate(2);
    unsigned int nbasis = 3;
    double anglePart = nbasis;
    double sigma = 1.0/anglePart;

    /*arma::vec cT = {0, 0};
    arma::vec cF = {10, 10};
    BasisFunction* bfT = new GaussianRbf(cT, sigma);
    BasisFunction* bfF = new GaussianRbf(cF, sigma);
    BasisFunctions basisReward;
    basisReward.push_back(bfT);
    basisReward.push_back(bfF);

    for(unsigned int i = 0; i < nbasis; i++)
    {
        double angle = i*2.0/anglePart*M_PI;
        basisReward.push_back(new GaussianRbf({cos(angle), sin(angle)}, sigma));
    }*/

    BasisFunctions basisReward = GaussianRbf::generate({5, 5}, {-2, 2, -2, 2});

    DenseFeatures phiReward(basisReward);

    LinearApproximator rewardRegressor(phiReward);
    ExpectedDeltaIRL<DenseAction,DenseState> irlAlg(data, expertPolicy, rewardRegressor,
                                        mdp.getSettings().gamma, atype);

    //Info print
    std::cout << "Basis size: " << phiReward.rows();
    std::cout << " | Params: " << expertPolicy.getParameters().t() << std::endl;
    std::cout << "Features Expectation " << data.computefeatureExpectation(phiReward, mdp.getSettings().gamma).t();

    ofstream ofs(fm.addPath("TrajectoriesExpert.txt"));
    data.writeToStream(ofs);


    //Run GIRL
#ifdef RUN_GIRL
    irlAlg.run();
    arma::vec weights = rewardRegressor.getParameters();

    cout << "weights :" << weights.t();

#endif

#ifdef RECOVER

    for(unsigned int i = 0; i < weights.n_cols; i++)
    {
        rewardRegressor.setParameters(weights.col(i));

        //Try to recover the initial policy
        int episodesPerPolicy = 1;
        int policyPerUpdate = 100;
        int updates = 400;
        int episodes = episodesPerPolicy*policyPerUpdate*updates;

        NormalStateDependantStddevPolicy imitatorPolicy(phi, stdPhi, stdWeights);
        AdaptiveStep stepRule(0.01);
        int nparams = phi.rows();
        arma::vec mean(nparams, fill::zeros);

        imitatorPolicy.setParameters(mean);
        GPOMDPAlgorithm<DenseAction, DenseState> imitator(imitatorPolicy, policyPerUpdate,
                mdp.getSettings().horizon, stepRule, GPOMDPAlgorithm<DenseAction, DenseState>::BaseLineType::MULTI);

        ParametricRewardMDP<DenseAction, DenseState> prMdp(mdp, rewardRegressor);
        Core<DenseAction, DenseState> imitatorCore(prMdp, imitator);
        EmptyStrategy<DenseAction, DenseState> emptyStrategy;
        imitatorCore.getSettings().loggerStrategy = &emptyStrategy;
        imitatorCore.getSettings().episodeLength = mdp.getSettings().horizon;
        imitatorCore.getSettings().episodeN = episodes;
        imitatorCore.getSettings().testEpisodeN = nbEpisodes;
        imitatorCore.runEpisodes();

        cout << "----------------------------------------------------------" << endl;
        cout << "Learned Parameters: " << imitatorPolicy.getParameters().t();
        cout << arma::as_scalar(imitatorCore.runBatchTest()) << endl;

        //Evaluate policy against the real mdp
        Core<DenseAction, DenseState> evaluationCore(mdp, imitator);
        CollectorStrategy<DenseAction, DenseState> collector2;
        evaluationCore.getSettings().loggerStrategy = &collector2;
        evaluationCore.getSettings().episodeLength = mdp.getSettings().horizon;
        evaluationCore.getSettings().episodeN = episodes;
        evaluationCore.getSettings().testEpisodeN = nbEpisodes;

        evaluationCore.runTestEpisodes();

        Dataset<DenseAction,DenseState>& data2 = collector2.data;

        double gamma = mdp.getSettings().gamma;
        cout << "Features Expectation ratio: " << (data2.computefeatureExpectation(phiReward, gamma)/data.computefeatureExpectation(phiReward, gamma)).t();
        cout << "reward: " << arma::as_scalar(evaluationCore.runBatchTest()) << endl;


        stringstream ss;
        ss << "TrajectoriesImitator" << i << ".txt";
        ofstream ofs(fm.addPath(ss.str()));
        data2.writeToStream(ofs);
    }
#endif


#ifdef PRINT
    //calculate full grid function
    int samplesParams = 101;
    arma::vec valuesG(samplesParams);
    arma::vec valuesJ(samplesParams);
    arma::vec valuesD(samplesParams);

    for(int i = 0; i < samplesParams; i++)
    {
        cerr << i << endl;
        double step = 0.01;
        arma::vec wm(2);
        wm(0) = i*step;
        wm(1) = 1.0 - wm(0);
        rewardRegressor.setParameters(wm);
        arma::mat gGrad(2, 2);
        arma::vec dJ;
        arma::vec dD;
        arma::vec g = irlAlg.ReinforceBaseGradient(gGrad);

        double Je = irlAlg.computeJ(dJ);
        double D = irlAlg.computeDisparity(dD);
        double G2 = as_scalar(g.t()*g);
        valuesG(i) = std::sqrt(G2);
        valuesJ(i) = Je;
        valuesD(i) = D;
    }

    valuesG.save("/tmp/ReLe/G.txt", arma::raw_ascii);
    valuesJ.save("/tmp/ReLe/J.txt", arma::raw_ascii);
    valuesD.save("/tmp/ReLe/D.txt", arma::raw_ascii);

#endif

    // Save Reward Function
    weights.save(fm.addPath("Weights.txt"),  arma::raw_ascii);

    return 0;
}