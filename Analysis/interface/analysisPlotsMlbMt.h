/*
 * analysisPlotsJan.h
 *
 *  Created on: Feb 12, 2014
 *      Author: kiesej
 */

#ifndef ANALYSISPLOTSMLBMT_H_
#define ANALYSISPLOTSMLBMT_H_

#include "analysisPlots.h"

namespace ztop{

class analysisPlotsMlbMt : public analysisPlots{
public:
	analysisPlotsMlbMt(size_t step):analysisPlots(step),finemlbbinning(false),
	Mlb(0),
	mlb(0),
	mlbmin(0),
	mlbmin_test(0),
	mlbivansbins(0),
	mlbminbsrad(0),
	leadleppt(0),
	mll(0),
	total(0),
	vistotal(0),
	mlbcombthresh_(165),
	rightassocounter_(0),totalcounter_(0)
{extraplots_.resize(cat_bjetjetmax,0);extraplots2_.resize(cat_bjetjetmax,0);extraplots3_.resize(cat_bjetjetmax,0);
    mlb_3_.resize(cat_bjetmax,0); mlb_5_.resize(cat_bjetmax,0); mlb_7_.resize(cat_bjetmax,0); mlb_10_.resize(cat_bjetmax,0);
}

	~analysisPlotsMlbMt(){/* destruction is handled in base class! */
		if(totalcounter_)
			std::cout << "fraction of properly associated combinations for mlbmin: " << rightassocounter_/totalcounter_
			<< std::endl;

	}

	void bookPlots();
	void fillPlotsReco();
	void fillPlotsGen();

	bool finemlbbinning;

private:

	enum bjetnjetscategories{
		cat_0bjet0jet,cat_0bjet1jet,cat_0bjet2jet,cat_0bjet3jet,
		cat_1bjet0jet,cat_1bjet1jet,cat_1bjet2jet,cat_1bjet3jet,
		cat_2bjet0jet,cat_2bjet1jet,cat_2bjet2jet,cat_2bjet3jet,cat_bjetjetmax}
	jetcategory;

	enum bjetcategories{
            cat_0bjet, cat_1bjet, cat_2bjet, cat_bjetmax}
	bjetcategory;

	void setJetCategory(size_t nbjets,size_t njets);

	std::vector<histo1DUnfold*> extraplots_;
	std::vector<histo1DUnfold*> extraplots2_;
	std::vector<histo1DUnfold*> extraplots3_;

        std::vector<histo1DUnfold*> mlb_3_, mlb_5_, mlb_7_, mlb_10_;

	histo1DUnfold
	*Mlb,
	*mlb,
	*mlbmin,
	*mlbmin_test,
	*mlbivansbins,
	*mlbminbsrad,            

	*leadleppt,
	*mll,

	*total,
	*vistotal,

	*mll0b;

	float mlbcombthresh_;
	float rightassocounter_,totalcounter_;

};

}
#endif /* ANALYSISPLOTSJAN_H_ */
