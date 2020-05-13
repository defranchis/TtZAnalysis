/*
 * analysisPlotsJan.h
 *
 *  Created on: Feb 12, 2014
 *      Author: kiesej
 */

#ifndef ANALYSISPLOTSTTXSECFIT_H_
#define ANALYSISPLOTSTTXSECFIT_H_

#include "analysisPlots.h"

namespace ztop{
/**
 * This is a misuse of the histo1DUnfold class
 */
class analysisPlotsTtbarXsecFit : public analysisPlots{
public:
    analysisPlotsTtbarXsecFit(size_t step):analysisPlots(step),jetcategory(cat_0bjet0jet),bjetcategory(cat_0bjet),totevts_(0),vispsevts_(0),totevtsw_(0),vispsevtsw_(0),total_0bjets(0),
            leadFwdJetPt(0),FwdJetMulti(0)
{
	        leadjetpt_plots.resize(cat_bjetjetmax,0);
		secondjetpt_plots.resize(cat_bjetjetmax,0);
		thirdjetpt_plots.resize(cat_bjetjetmax,0);
		total_plots.resize(cat_bjetjetmax,0);
		htalljets_plots.resize(cat_bjetmax,0); 
		jetmulti_plots.resize(cat_bjetmax,0);
                mll_plots.resize(cat_bjetjetmax,0);
                dxi_plots.resize(cat_bjetjetmax,0);

}

	~analysisPlotsTtbarXsecFit(){/* destruction is handled in base class! */
		if(totevts_){
			std::cout << "fraction of visible phase space events:" << (double)vispsevts_/(double)totevts_<<std::endl;
			std::cout << "fraction of visible phase space events (weighted):" << vispsevtsw_/totevtsw_<<std::endl;
		}
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

	enum bjetcategories{
	    cat_0bjet,cat_1bjet,cat_2bjet,cat_bjetmax}
	bjetcategory;

	void setJetCategory(size_t nbjets,size_t njets);

	size_t totevts_,vispsevts_;
	double totevtsw_,vispsevtsw_;


	std::vector<histo1DUnfold*>
	leadjetpt_plots, secondjetpt_plots, thirdjetpt_plots, total_plots, htalljets_plots,mll_plots,dxi_plots, jetmulti_plots;

        histo1DUnfold *total_0bjets;
        histo1DUnfold *leadFwdJetPt;
        histo1DUnfold *FwdJetMulti;
};

}
#endif /* analysisPlotsTtbarXsecFit_H_ */
