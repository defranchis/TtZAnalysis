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

class top_analyzer_run2 : public ztop::topAnalyzer{
public:


	top_analyzer_run2():topAnalyzer(){}


	void analyze(size_t i);
        bool checkTrigger(std::vector<bool> * ,ztop::NTEvent * , bool , size_t);

        bool isDilep;
        bool isSingleEle;
        bool isSingleMu;
        bool isRunBtoD ;
        bool isRunEtoF ;
        bool isRunFtoG ;
        bool isRunH ;
	
        //no copy
private:
	top_analyzer_run2(const top_analyzer_run2 &);
	top_analyzer_run2& operator = (const top_analyzer_run2 &);



};



#endif /* ANALYZER_RUN2_H_ */
