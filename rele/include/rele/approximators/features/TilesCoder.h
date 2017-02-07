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

#ifndef INCLUDE_RELE_APPROXIMATORS_FEATURES_TILESCODER_H_
#define INCLUDE_RELE_APPROXIMATORS_FEATURES_TILESCODER_H_

#include "rele/approximators/Features.h"
#include "rele/approximators/Tiles.h"

namespace ReLe
{

/*!
 * A tile coder can be used to transform tiles into a set of features.
 * A tile coder evaluate each tile set provided and transform it in a coherent set of
 * binary features sparse vector.
 */
template<class InputC, bool denseOutput = false>
class TilesCoder_: public Features_<InputC, denseOutput>
{
    using return_type = typename feature_traits<denseOutput>::type;

public:
    /*!
     * Constructor.
     * \param tiles a set of tiles to be used
     * \param outputs the number of output features vectors
     */
    TilesCoder_(Tiles_<InputC>* tiles, bool destroy = true)
		: destroy(destroy)
    {
        tilesVector.push_back(tiles);
        rowsN = tiles->size();
    }

    /*!
     * Constructor.
     * \param tilesVector a vector of multiple tilings to be used
     * \param outputs the number of output features vectors
     */
    TilesCoder_(TilesVector_<InputC>& tilesVector, bool destroy = true) :
        tilesVector(tilesVector), rowsN(0), destroy(destroy)
    {
        computeRows();
    }

    virtual return_type operator()(const InputC& input) const override
    {
        return_type output(rowsN);
        output.zeros();

        unsigned int offset = 0;
        for(auto t : tilesVector)
        {
             auto& tiles = *t;
             computeTile(input, offset, tiles, output);
             offset += tiles.size();
        }


        return output;
    }

    virtual size_t size() const override
    {
        return rowsN;
    }


    /*!
     * Destructor.
     * Destroys also all the tiles passed to the coder.
     */
    virtual ~TilesCoder_()
    {
    	if(destroy)
    	{
    		for(auto tile : tilesVector)
    			delete tile;
    	}
    }

protected:
    void computeTile(const InputC& input, unsigned int offset,
                     Tiles_<InputC>& tiles,
                     return_type& output) const
    {
        try
        {
            unsigned int index = tiles(input) + offset;
            output(index) = 1.0;
        }
        catch (out_of_bounds& e) {}

    }

private:
    void computeRows()
    {
        rowsN = 0;

        for (auto& tiles : tilesVector)
        {
            rowsN += tiles->size();
        }
    }

private:
    TilesVector_<InputC> tilesVector;
    size_t rowsN;
    bool destroy;
};

//! Template alias.
typedef TilesCoder_<arma::vec> TilesCoder;

//! Template Alias.
typedef TilesCoder_<arma::vec, true> DenseTilesCoder;

}

#endif /* INCLUDE_RELE_APPROXIMATORS_FEATURES_TILESCODER_H_ */
