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

#include "rele/environments/MountainCar.h"
#include "rele/core/Core.h"
#include "rele/algorithms/td/LinearSARSA.h"
#include "rele/approximators/basis/PolynomialFunction.h"
#include "rele/approximators/features/DenseFeatures.h"
#include "rele/policy/q_policy/e_Greedy.h"

using namespace std;
using namespace ReLe;

int main(int argc, char *argv[])
{
    int episodes = 40;
    MountainCar mdp;

    BasisFunctions basis = PolynomialFunction::generate(1, mdp.getSettings().continuosStateDim + 1);
    DenseFeatures phi(basis);

    e_GreedyApproximate policy;
    ConstantLearningRateDense alpha(0.2);
    LinearGradientSARSA agent(phi, policy, alpha);

    auto&& core = buildCore(mdp, agent);

    for (int i = 0; i < episodes; i++)
    {
        core.getSettings().episodeLength = 10000;
        cout << "starting episode" << endl;
        core.runEpisode();
    }

}
