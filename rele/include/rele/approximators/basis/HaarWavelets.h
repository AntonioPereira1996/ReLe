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

#ifndef INCLUDE_RELE_APPROXIMATORS_BASIS_HAARWAVELETS_H_
#define INCLUDE_RELE_APPROXIMATORS_BASIS_HAARWAVELETS_H_

#include "rele/approximators/basis/Wavelets.h"
#include <armadillo>

namespace ReLe
{

/*!
 * This class implements a haar wavelets
 */
class HaarWavelets : public WaveletType
{
public:
    virtual ~HaarWavelets();
    virtual double scaling(double value) override;
    virtual double mother(double value) override;

};

}//end namespace


#endif /* INCLUDE_RELE_APPROXIMATORS_BASIS_HAARWAVELETS_H_ */
