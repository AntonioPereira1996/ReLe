/*
 * rele,
 *
 *
 * Copyright (C) 2016 Davide Tateo
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
#include "rele/approximators/features/TilesCoder.h"
#include "rele/approximators/regressors/others/LinearApproximator.h"
#include "rele/approximators/basis/IdentityBasis.h"
#include "rele/approximators/basis/PolynomialFunction.h"
#include "rele/approximators/tiles/BasicTiles.h"
#include "rele/approximators/tiles/LogTiles.h"

#include "rele/policy/parametric/differentiable/LinearPolicy.h"
#include "rele/policy/parametric/differentiable/NormalPolicy.h"
#include "rele/statistics/DifferentiableNormals.h"

#include "rele/environments/LQR.h"
#include "rele/solvers/lqr/LQRsolver.h"
#include "rele/core/PolicyEvalAgent.h"

#include "rele/utils/FileManager.h"

#include "rele/IRL/algorithms/BayesianCoordinateAscend.h"
#include "rele/IRL/algorithms/MLEDistribution.h"
#include "rele/IRL/algorithms/LinearMLEDistribution.h"
#include "rele/IRL/algorithms/EGIRL.h"
#include "rele/IRL/algorithms/SDPEGIRL.h"
#include "rele/IRL/algorithms/CurvatureEGIRL.h"

#include "../RewardBasisLQR.h"

#include "rele/feature_selection/PrincipalComponentAnalysis.h"

using namespace std;
using namespace arma;
using namespace ReLe;

int main(int argc, char *argv[])
{
    if(argc != 5)
    {
        cout << "Wrong argument number: dimension, degree, n_episode, n_experiment must be provided" << endl;
        return -1;
    }

    string dimension(argv[1]);
    string degree(argv[2]);
    string n_episodes(argv[3]);
    string n_experiment(argv[4]);

    FileManager fm("nips/lqr_mle/" + dimension + "/" + degree + "/" + n_episodes + "/" + n_experiment);
    fm.createDir();
    fm.cleanDir();
    std::cout << std::setprecision(OS_PRECISION);

    unsigned int dim = stoi(dimension);
    unsigned int deg = stoi(degree);
    unsigned int nbEpisodes = stoi(n_episodes);

    //Set reward policy
    arma::vec eReward;

    if(dim == 2)
    {
        eReward = {0.3, 0.7};
    }
    else
    {
        eReward = {0.2, 0.7, 0.1};
    }

    int rewardDim = dim;
    LQR mdp(dim, rewardDim, LQR::S0Type::RANDOM);

    BasisFunctions basis = IdentityBasis::generate(dim);

    SparseFeatures phi;
    phi.setDiagonal(basis);

    DetLinearPolicy<DenseState> expertPolicy(phi);

    // solve the problem in exact way
    LQRsolver solver(mdp,phi);
    solver.setRewardWeights(eReward);
    mat K = solver.computeOptSolution();
    arma::vec p = K.diag();
    arma::mat SigmaExpert = arma::eye(dim, dim)*1e-2;
    ParametricNormal expertDist(p, SigmaExpert);

    std::cout << "Rewards: ";
    for (int i = 0; i < eReward.n_elem; ++i)
    {
        std::cout << eReward(i) << " ";
    }
    std::cout << "| Params: " << expertDist.getParameters().t() << std::endl;


    PolicyEvalDistribution<DenseAction, DenseState> expert(expertDist, expertPolicy);

    // Generate LQR expert dataset
    Core<DenseAction, DenseState> expertCore(mdp, expert);
    CollectorStrategy<DenseAction, DenseState> collection;
    expertCore.getSettings().loggerStrategy = &collection;
    expertCore.getSettings().episodeLength = mdp.getSettings().horizon;
    expertCore.getSettings().testEpisodeN = nbEpisodes;
    expertCore.runTestEpisodes();
    Dataset<DenseAction,DenseState>& data = collection.data;

    // recover approximate
    BasisFunctions basisImitator = PolynomialFunction::generate(deg, dim);
    SparseFeatures phiImitator(basisImitator, dim);

    unsigned int dp = phiImitator.rows();

    std::cout << "Parameters Number" << std::endl;
    std::cout << dp << std::endl;

    arma::mat SigmaPolicy = arma::eye(dim, dim)*1e-3;
    MVNPolicy policyFamily(phiImitator, SigmaPolicy);
    LinearMLEDistribution alg(phiImitator, SigmaPolicy);

    std::cout << "Recovering Distribution" << std::endl;
    alg.compute(data);



    ParametricNormal imitatorDist = alg.getDistribution();

    // Reduce policy
    arma::mat theta = alg.getParameters();


    PrincipalComponentAnalysis featureSelector(0.90, false);

    featureSelector.createFeatures(theta);

    arma::mat T = featureSelector.getTransformation();
    arma::mat thetaN = featureSelector.getNewFeatures();
    arma::vec meanN(thetaN.n_rows, arma::fill::zeros);
    arma::mat SigmaN = T*imitatorDist.getCovariance()*T.t();

    ParametricNormal imitatorDistN(meanN, SigmaN);

    //Run EGIRL
    BasisFunctions basisReward;
    for(unsigned int i = 0; i < eReward.n_elem; i++)
        basisReward.push_back(new LQR_RewardBasis(i, dim));
    DenseFeatures phiReward(basisReward);


    LinearApproximator rewardRegressor(phiReward);
    auto* irlAlg =  new EGIRL<DenseAction, DenseState>(data, thetaN, imitatorDistN,
            rewardRegressor, mdp.getSettings().gamma, IrlEpGrad::PGPE_BASELINE);

    irlAlg->run();
    arma::vec omega = rewardRegressor.getParameters();

    //Print results
    cout << "Weights (EGIRL): " << omega.t();

    //save weights
    omega.save(fm.addPath("Weights.txt"),  arma::raw_ascii);

}
