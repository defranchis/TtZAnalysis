/*
 * analyzer_run1.h
 *
 *  Created on: Mar 31, 2015
 *      Author: kiesej
 */

#ifndef ANALYZER_RUN2_H_
#define ANALYZER_RUN2_H_

#include "../interface/topAnalyzer.h"
#include "../interface/analysisUtils.h"
#include "../interface/analysisPlotsTtbarXsecFitSingleLep.h"
#include "../interface/analysisPlotsKinReco.h"

class top_analyzer_run2 : public ztop::topAnalyzer{
public:

        top_analyzer_run2():topAnalyzer(),
        bin_1_(420),
        bin_2_(550),
        bin_3_(810)
        {}


	void analyze(size_t i);
        bool checkTrigger(std::vector<bool> * ,ztop::NTEvent * , bool , size_t);

        bool isDilep;
        bool isSingleEle;
        bool isSingleMu;
        bool isRunBtoD ;
        bool isRunEtoF ;
        bool isRunFtoG ;
        bool isRunH ;
	
	void setMttCategories(float gen_mtt, TString legendname);

	enum leg_mttcategories{
            cat_mtt1_leg, cat_mtt2_leg, cat_mtt3_leg, cat_mttmax_leg}
	leg_mttcategory;
	enum gen_mttcategories{
            cat_mtt1_gen, cat_mtt2_gen, cat_mtt3_gen, cat_mttmax_gen}
	gen_mttcategory;


        //no copy
private:
	top_analyzer_run2(const top_analyzer_run2 &);
	top_analyzer_run2& operator = (const top_analyzer_run2 &);

        float bin_1_; // 420 (mtt> 345 by construction)
        float bin_2_; // 550
        float bin_3_; // 810


};



#endif /* ANALYZER_RUN2_H_ */
