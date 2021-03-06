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

#ifndef INCLUDE_RELE_CORE_CALLBACKS_CORECALLBACK_H_
#define INCLUDE_RELE_CORE_CALLBACKS_CORECALLBACK_H_

namespace ReLe
{

class CoreCallback
{
public:
    virtual void run(unsigned int step) = 0;
    virtual void runEnd() = 0;

    inline void setStepNumbers(unsigned int stepNumbers)
    {
        this->stepNumbers = stepNumbers;
    }

    virtual ~CoreCallback();

protected:
    unsigned int stepNumbers;

};

class CoreProgressBar : public CoreCallback
{
public:
    virtual void run(unsigned int step) override;
    virtual void runEnd() override;

};

}

#endif /* INCLUDE_RELE_CORE_CALLBACKS_CORECALLBACK_H_ */
