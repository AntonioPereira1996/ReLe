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

#include "TaxiFuel.h"

#include "Core.h"

#include "policy_search/gradient/hierarchical/HierarchicalGPOMDP.h"

#include "parametric/differentiable/GibbsPolicy.h"
#include "parametric/differentiable/NewGibbsPolicy.h"
#include "features/DenseFeatures.h"
#include "basis/IdentityBasis.h"
#include "basis/PolynomialFunction.h"
#include "basis/GaussianRbf.h"
#include "basis/ConditionBasedFunction.h"

#include "FileManager.h"
#include "ConsoleManager.h"

#include "TaxiOptions.h"

#include <iostream>

using namespace std;
using namespace ReLe;
using namespace arma;


int main(int argc, char *argv[])
{
    FileManager fm("TaxiFuel", "HPG");
    fm.createDir();
    fm.cleanDir();

    TaxiFuel taxiMDP;


    auto locations = taxiMDP.getLocations();

    //-- options


    vector<Option<FiniteAction, DenseState>*> options;

    options.push_back(new TaxiComplexFillupOption(locations));
    options.push_back(new TaxiComplexDropOffOption(locations));
    options.push_back(new TaxiComplexPickupOption(locations));
    /*for(auto& location : locations)
    {
        options.push_back(new TaxiLocationOption(location));
    }*/

    vector<FiniteAction> actions = FiniteAction::generate(options.size());

    //-- Features
    //BasisFunctions basis = GaussianRbf::generate({5, 5, 3, 3}, {0, 5, 0, 5, 0, 12, -1, 1});
    //BasisFunctions basis = PolynomialFunction::generate(1, TaxiFuel::STATESIZE);
    //BasisFunctions basis = IdentityBasis::generate(TaxiFuel::STATESIZE);
    BasisFunctions basisSpace = VectorFiniteIdentityBasis::generate(1, 5);

    vector<unsigned int> indexes;
    vector<unsigned int> values;
    indexes.push_back(TaxiFuel::y);
    values.push_back(5);
    indexes.push_back(TaxiFuel::onBoard);
    values.push_back(3);
    indexes.push_back(TaxiFuel::location);
    values.push_back(4);
    indexes.push_back(TaxiFuel::destination);
    values.push_back(4);
    indexes.push_back(TaxiFuel::fuel);
    values.push_back(12);

    BasisFunctions basis = AndConditionBasisFunction::generate(basisSpace, indexes, values);
    //basis.push_back(new IdentityBasis(TaxiFuel::fuel));

    BasisFunctions basisGibbs = AndConditionBasisFunction::generate(basis, TaxiFuel::STATESIZE, actions.size() - 1);
    DenseFeatures phi(basisGibbs);
    cout << phi.rows() << endl;

    double temperature = 50;
    ParametricGibbsPolicy<DenseState> rootPolicyOption(actions, phi, temperature);
    DifferentiableOption<FiniteAction, DenseState> rootOption(rootPolicyOption, options);
    //--

    //-- agent
    int nbepperpol = 100, nbstep = 100;
    //AdaptiveStep stepRule(0.01);
    ConstantStep stepRule(0.01);
    HierarchicalGPOMDPAlgorithm<FiniteAction, DenseState> agent(rootOption, nbepperpol, nbstep, stepRule,
            HierarchicalGPOMDPAlgorithm<FiniteAction, DenseState>::BaseLineType::MULTI);

    Core<FiniteAction, DenseState> core(taxiMDP, agent);
    //--


    int episodes = 50000;
    core.getSettings().episodeLenght = 100;
    core.getSettings().loggerStrategy = new WriteStrategy<FiniteAction, DenseState>(fm.addPath("TaxiFuel.log"),
            WriteStrategy<FiniteAction, DenseState>::AGENT);


    ConsoleManager console(episodes, 1);
    console.printInfo("starting learning");
    for (int i = 0; i < episodes; i++)
    {
        console.printProgress(i);
        core.runEpisode();
        cout << "temperature: " << temperature << endl;
        double tNew = temperature * (static_cast<double>(episodes) - static_cast<double>(i))/static_cast<double>(episodes);
        tNew = max(tNew, 0.1);
        cout << "temperature: " << tNew << endl;
        rootPolicyOption.setTemperature(tNew);
    }

    delete core.getSettings().loggerStrategy;

    console.printInfo("Starting evaluation episode");
    core.getSettings().loggerStrategy = new WriteStrategy<FiniteAction, DenseState>(fm.addPath("TaxiFuel.log"),
            WriteStrategy<FiniteAction, DenseState>::TRANS);

    for(int i = 0; i < 10; i++)
    {
        cout << "# " << i + 1 << "/10" << endl;
        core.runTestEpisode();
    }

    delete core.getSettings().loggerStrategy;

    //cout << "last option: " << rootOption.getLastChoice() << endl;

    //cout << "p" << rootPolicyOption.getParameters().t();
    return 0;

}
