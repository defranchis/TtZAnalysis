/*
 * ttbarGenPlots.h
 *
 *  Created on: Nov 1, 2013
 *      Author: kiesej
 */

#ifndef TTBARGENPLOTS_H_
#define TTBARGENPLOTS_H_

#include "controlPlotBasket.h"

namespace ztop{

class ttbarGenPlots : public controlPlotBasket{
public:
	ttbarGenPlots():controlPlotBasket(){}
	~ttbarGenPlots(){}

protected:
	void makeControlPlotsPriv();

};
}//namespace

#endif /* TTBARGENPLOTS_H_ */
