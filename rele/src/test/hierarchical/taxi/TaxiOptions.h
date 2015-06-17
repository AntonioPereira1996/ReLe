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

#ifndef SRC_TEST_HIERARCHICAL_TAXI_TAXIOPTIONS_H_
#define SRC_TEST_HIERARCHICAL_TAXI_TAXIOPTIONS_H_

#include "Options.h"

namespace ReLe
{

class TaxiLocationOption : public FixedOption<FiniteAction, DenseState>
{
public:
	TaxiLocationOption(arma::vec location);
	virtual bool canStart(const arma::vec& state);
	virtual double terminationProbability(const DenseState& state);
	virtual void operator ()(const DenseState& state, FiniteAction& action);

protected:
	arma::vec location;
};


/*class TaxiPickupOption : public TaxiLocationOption
{
public:
	virtual bool canStart(const arma::vec& state);
	virtual double terminationProbability(const DenseState& state);
	virtual void operator ()(const DenseState& state, FiniteAction& action);
	TaxiPickupOption(arma::vec location);



};

class TaxiDropOffOption : public TaxiLocationOption
{
public:
	virtual bool canStart(const arma::vec& state);
	virtual double terminationProbability(const DenseState& state);
	virtual void operator ()(const DenseState& state, FiniteAction& action);
	TaxiDropOffOption(arma::vec location);


private:
	arma::vec location;
};*/

class TaxiFillUpOption : public FixedOption<FiniteAction, DenseState>
{
public:
	virtual bool canStart(const arma::vec& state);
	virtual double terminationProbability(const DenseState& state);
	virtual void operator ()(const DenseState& state, FiniteAction& action);
	//TaxiFillUpOption(arma::vec location);


private:
	arma::vec location;
};


}

#endif /* SRC_TEST_HIERARCHICAL_TAXI_TAXIOPTIONS_H_ */