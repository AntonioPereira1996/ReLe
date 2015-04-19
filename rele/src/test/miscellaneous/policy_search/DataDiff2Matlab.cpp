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

#include "Core.h"
#include "parametric/differentiable/GibbsPolicy.h"
#include "BasisFunctions.h"
#include "parametric/differentiable/NormalPolicy.h"
#include "basis/PolynomialFunction.h"
#include "basis/GaussianRBF.h"
#include "basis/ConditionBasedFunction.h"

#include "Dam.h"
#include "PolicyEvalAgent.h"
#include "algorithms/GIRL.h"
#include "policy_search/PGPE/PGPE.h"
#include "ParametricRewardMDP.h"
#include "RandomGenerator.h"
#include "FileManager.h"

#include "policy_search/onpolicy/FunctionGradient.h"
#include "policy_search/onpolicy/FunctionHessian.h"
#include "policy_search/onpolicy/PolicyGradientAlgorithm.h"

using namespace std;
using namespace ReLe;
using namespace arma;

void help()
{
    cout << "datadiff2mat [algorithm] parameters_file" << endl;
    cout << " - algorithm: r, rb, g, gb (default)" << endl;
}


int main(int argc, char *argv[])
{
//    RandomGenerator::seed(418932850);

    /*** check inputs ***/
    char alg[10];
    GIRL<DenseAction,DenseState>::AlgType atype;
    vec params;
    if (argc == 3)
    {
        if (strcmp(argv[1], "r") == 0)
        {
            cout << "REINFORCE" << endl;
            atype = GIRL<DenseAction,DenseState>::AlgType::R;
        }
        else if (strcmp(argv[1], "rb") == 0)
        {
            cout << "REINFORCE BASE" << endl;
            atype = GIRL<DenseAction,DenseState>::AlgType::RB;
        }
        else if (strcmp(argv[1], "g") == 0)
        {
            cout << "GPOMDP" << endl;
            atype = GIRL<DenseAction,DenseState>::AlgType::G;
        }
        else if (strcmp(argv[1], "gb") == 0)
        {
            cout << "GPOMDP BASE" << endl;
            atype = GIRL<DenseAction,DenseState>::AlgType::GB;
        }
        else
        {
            std::cout << "Error unknown argument " << argv[1] << std::endl;
            help();
            exit(1);
        }
        strcpy(alg, argv[1]);

        params.load(argv[2], raw_ascii);
    }
    else
    {
        help();
        return 1;
    }
    /******/

    FileManager fm("datadiff2mat", "test");
    fm.createDir();
    //    fm.cleanDir();
    std::cout << std::setprecision(OS_PRECISION);

    /*** Set up MDP ***/
    Dam mdp;

    PolynomialFunction *pf = new PolynomialFunction(1,0);
    GaussianRbf* gf1 = new GaussianRbf(0,50);
    GaussianRbf* gf2 = new GaussianRbf(50,20);
    GaussianRbf* gf3 = new GaussianRbf(120,40);
    GaussianRbf* gf4 = new GaussianRbf(160,50);
    BasisFunctions basis;
    basis.push_back(pf);
    basis.push_back(gf1);
    basis.push_back(gf2);
    basis.push_back(gf3);
    basis.push_back(gf4);

    DenseFeatures phi(basis);
    LinearApproximator regressor(mdp.getSettings().continuosStateDim, phi);
//    vec p(5);
//    p(0) = 50;
//    p(1) = -50;
//    p(2) = 0;
//    p(3) = 0;
//    p(4) = 50;
//    regressor.setParameters(p);
    NormalPolicy policy(0.1, &regressor);
    policy.setParameters(params);
    //---

    cout << policy.getParameters().t();


    PolicyEvalAgent<DenseAction, DenseState> expert(policy);

    /* Generate DAM expert dataset */
    Core<DenseAction, DenseState> expertCore(mdp, expert);
    CollectorStrategy<DenseAction, DenseState> collection;
    expertCore.getSettings().loggerStrategy = &collection;
    expertCore.getSettings().episodeLenght = mdp.getSettings().horizon;
    expertCore.getSettings().testEpisodeN = 200;
    expertCore.runTestEpisodes();

    Dataset<DenseAction,DenseState>& data = collection.data;
    ofstream datasetfile(fm.addPath("dataset.dat"));
    if (datasetfile.is_open())
    {
        data.writeToStream(datasetfile);
        datasetfile.close();
    }


    vec gradient;
    mat hessian;
    GradientFromDataWorker<DenseAction, DenseState> gdw(data, policy, mdp.getSettings().gamma, 0);
    HessianFromDataWorker<DenseAction, DenseState, NormalPolicy> hdw(data, policy, mdp.getSettings().gamma, 0);
    if (atype == GIRL<DenseAction,DenseState>::AlgType::R)
    {
        cout << "PG REINFORCE" << endl;
        gradient = gdw.ReinforceGradient();
        hessian = hdw.ReinforceHessian();
    }
    else if (atype == GIRL<DenseAction,DenseState>::AlgType::RB)
    {
        cout << "PG REINFORCE BASE" << endl;
        gradient = gdw.ReinforceBaseGradient();
    }
    else if (atype == GIRL<DenseAction,DenseState>::AlgType::G)
    {
        cout << "PG GPOMDP" << endl;
        gradient = gdw.GpomdpGradient();
    }
    else if (atype == GIRL<DenseAction,DenseState>::AlgType::GB)
    {
        cout << "PG GPOMDP BASE" << endl;
        gradient = gdw.GpomdpBaseGradient();
    }

    gradient.save(fm.addPath("gradient.dat"), raw_ascii);
    hessian.save(fm.addPath("hessian.dat"), raw_ascii);

    return 0;
}