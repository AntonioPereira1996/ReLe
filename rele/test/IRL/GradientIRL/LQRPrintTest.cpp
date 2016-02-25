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
#include "rele/approximators/regressors/others/LinearApproximator.h"
#include "rele/approximators/basis/IdentityBasis.h"

#include "rele/policy/parametric/differentiable/NormalPolicy.h"

#include "rele/environments/LQR.h"
#include "rele/solvers/LQRsolver.h"
#include "rele/core/PolicyEvalAgent.h"

#include "rele/IRL/utils/GradientCalculatorFactory.h"
#include "rele/IRL/utils/HessianCalculatorFactory.h"

#include "rele/utils/FileManager.h"

#include "../RewardBasisLQR.h"

using namespace std;
using namespace arma;
using namespace ReLe;

int main(int argc, char *argv[])
{
//  RandomGenerator::seed(45423424);
//  RandomGenerator::seed(8763575);

    IrlGrad atype = IrlGrad::REINFORCE;
    vec eReward =
    { 0.3, 0.7 };
    int nbEpisodes = 5000;
    int dim = eReward.n_elem;

    FileManager fm("lqr", "GIRL");
    fm.createDir();
    fm.cleanDir();
    std::cout << std::setprecision(OS_PRECISION);

    // create policy basis functions
    BasisFunctions basis = IdentityBasis::generate(dim);
    SparseFeatures phi;
    phi.setDiagonal(basis);

    // solve the problem in exact way
    LQR mdp(dim, dim);
    LQRsolver solver(mdp, phi);
    solver.setRewardWeights(eReward);
    mat K = solver.computeOptSolution();
    arma::vec p = K.diag();

    // Create expert policy
    MVNPolicy expertPolicy(phi);
    expertPolicy.setParameters(p);

    std::cout << "Rewards: ";
    for (int i = 0; i < eReward.n_elem; ++i)
    {
        std::cout << eReward(i) << " ";
    }
    std::cout << "| Params: " << expertPolicy.getParameters().t() << std::endl;

    PolicyEvalAgent<DenseAction, DenseState> expert(expertPolicy);

    // Generate LQR expert dataset
    Core<DenseAction, DenseState> expertCore(mdp, expert);
    CollectorStrategy<DenseAction, DenseState> collection;
    expertCore.getSettings().loggerStrategy = &collection;
    expertCore.getSettings().episodeLength = mdp.getSettings().horizon;
    expertCore.getSettings().testEpisodeN = nbEpisodes;
    expertCore.runTestEpisodes();
    Dataset<DenseAction, DenseState>& data = collection.data;

    // Create parametric reward
    BasisFunctions basisReward;
    for (unsigned int i = 0; i < eReward.n_elem; i++)
        basisReward.push_back(new LQR_RewardBasis(i, dim));
    DenseFeatures phiReward(basisReward);


    // init calculators methods
    auto gradientCalculator = GradientCalculatorFactory<DenseAction, DenseState>::build(atype,
                              phiReward, data, expertPolicy,
                              mdp.getSettings().gamma);

    auto hessianCalculator = HessianCalculatorFactory<DenseAction, DenseState>::build(atype,
                             phiReward, data, expertPolicy,
                             mdp.getSettings().gamma);

    arma::vec rvec = data.computefeatureExpectation(phiReward, mdp.getSettings().gamma);


    unsigned int samplesParams = 101;
    arma::vec valuesJ(samplesParams, arma::fill::zeros);
    arma::vec valuesG(samplesParams, arma::fill::zeros);
    arma::vec valuesT(samplesParams, arma::fill::zeros);
    arma::vec valuesF(samplesParams, arma::fill::zeros);
    arma::vec valuesFs(samplesParams, arma::fill::zeros);


    // sample functions
    for (int i = 0; i < samplesParams; i++)
    {
        double w1 = i / 100.0;
        arma::vec w = { w1, 1.0 - w1 };

        // compute gradient and hessian
        arma::vec g = gradientCalculator->computeGradient(w);
        arma::mat H = hessianCalculator->computeHessian(w);

        // compute signed hessian
        arma::mat V;
        arma::vec Lambda;
        arma::eig_sym(Lambda, V, H);

        arma::mat Hs = V*arma::diagmat(arma::abs(Lambda))*V.i();

        // compute J
        valuesJ.row(i) = rvec.t()*w;

        // compute gradient norm
        valuesG.row(i) = g.t()*g;

        //compute trace of the hessian
        valuesT.row(i) = arma::trace(H);

        //compute expectedDeltaIRL function
        valuesF.row(i) = -0.5*g.t()*H.i()*g+0.5*arma::trace(H);

        //compute the signed expectedDeltaIRL function
        valuesFs.row(i) = -g.t()*Hs*g + 0.5*g.t()*H.i()*g + 0.5*arma::trace(H);

    }

    valuesJ.save("/tmp/ReLe/J.txt", arma::raw_ascii);
    valuesG.save("/tmp/ReLe/G.txt", arma::raw_ascii);
    valuesT.save("/tmp/ReLe/T.txt", arma::raw_ascii);
    valuesF.save("/tmp/ReLe/F.txt", arma::raw_ascii);
    valuesFs.save("/tmp/ReLe/Fs.txt", arma::raw_ascii);

    return 0;
}