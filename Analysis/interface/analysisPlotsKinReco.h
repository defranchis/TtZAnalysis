/*
 * analysisPlotsJan.h
 *
 *  Created on: Feb 12, 2014
 *      Author: kiesej
 */

#ifndef ANALYSISPLOTSKINRECO_H_
#define ANALYSISPLOTSKINRECO_H_

#include "analysisPlots.h"

namespace ztop{

    class analysisPlotsKinReco : public analysisPlots{
    public:
    analysisPlotsKinReco(size_t step):analysisPlots(step),
            mtt(0),
            pt_top(0),
            eta_top(0),
            pt_antitop(0),
            eta_antitop(0),
            pt_ttbar(0),
            eta_ttbar(0){}
  	~analysisPlotsKinReco(){ /* destruction is handled in base class! */
	}

	void bookPlots();
	void fillPlotsReco();
	void fillPlotsGen();

    private:

	enum bjetnjetscategories{
            cat_0bjet0jet,cat_0bjet1jet,cat_0bjet2jet,cat_0bjet3jet,
            cat_1bjet0jet,cat_1bjet1jet,cat_1bjet2jet,cat_1bjet3jet,
            cat_2bjet0jet,cat_2bjet1jet,cat_2bjet2jet,cat_2bjet3jet,cat_bjetjetmax}
	jetcategory;

	void setJetCategory(size_t nbjets,size_t njets);

        histo1DUnfold  
            * mtt,
            * pt_top,
            * eta_top,
            * pt_antitop,
            * eta_antitop,
            * pt_ttbar,
            * eta_ttbar;
    };

}
#endif /* ANALYSISPLOTSMLBMT_H_ */
