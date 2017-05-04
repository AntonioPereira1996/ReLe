/*
 * rele,
 *
 *
 * Copyright (C) 2017 Davide Tateo
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

#include "rele/approximators/basis/HaarWavelets.h"
#include <cassert>

using namespace arma;

namespace ReLe
{


HaarWavelets::~HaarWavelets()
{
}

double HaarWavelets::scaling(double value)
{
    if(value >= 0 && value <= 1)
    {
        return 1;
    }

    return 0;
}

double HaarWavelets::mother(double value)
{
    if(value >= 0 && value < 0.5)
    {
        return 1;
    }
    else if(value > 0.5 && value <= 1)
    {
        return -1;
    }

    return 0;
}


}
