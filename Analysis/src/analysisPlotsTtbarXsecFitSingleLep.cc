/*
 * analysisPlotsTtbarXsecFit.cc
 *
 *  Created on: Feb 12, 2014
 *      Author: kiesej
 */


#include "../interface/analysisPlotsTtbarXsecFitSingleLep.h"
#include "../interface/AnalysisUtils.h"
#include "TopAnalysis/ZTopUtils/interface/miscUtils.h"
#include "TtZAnalysis/DataFormats/src/classes.h"
#include "TtZAnalysis/DataFormats/interface/helpers.h"

namespace ztop{

void analysisPlotsTtbarXsecFitSingleLep::bookPlots(){
	if(!use()) return;
	using namespace std;
	using namespace ztop;


	std::vector<float> bins=histo1D::createBinning(1,0,1);
	//leadjetpt_plots, secjetpt_plots, thirdjetpt_plots, total_plots;
	total_plots.at(cat_0bjet0jet) = addPlot(bins,bins,"total 0,0 b-jets","","Events");
	total_plots.at(cat_0bjet1jet) = addPlot(bins,bins,"total 0,1 b-jets","","Events");
	total_plots.at(cat_0bjet2jet) = addPlot(bins,bins,"total 0,2 b-jets","","Events");
	total_plots.at(cat_0bjet3jet) = addPlot(bins,bins,"total 0,3 b-jets","","Events");
	total_plots.at(cat_1bjet0jet) = addPlot(bins,bins,"total 1,0 b-jets","","Events");
	total_plots.at(cat_1bjet1jet) = addPlot(bins,bins,"total 1,1 b-jets","","Events");
	total_plots.at(cat_1bjet2jet) = addPlot(bins,bins,"total 1,2 b-jets","","Events");
	total_plots.at(cat_1bjet3jet) = addPlot(bins,bins,"total 1,3 b-jets","","Events");
	total_plots.at(cat_2bjet0jet) = addPlot(bins,bins,"total 2,0 b-jets","","Events");
	total_plots.at(cat_2bjet1jet) = addPlot(bins,bins,"total 2,1 b-jets","","Events");
	total_plots.at(cat_2bjet2jet) = addPlot(bins,bins,"total 2,2 b-jets","","Events");
	total_plots.at(cat_2bjet3jet) = addPlot(bins,bins,"total 2,3 b-jets","","Events");

	bins.clear();
        bins << 30 << 35  << 40<<45 << 50<<55 << 60 << 70 << 80 << 90 << 100 << 120 << 140  << 200;
        leadjetpt_plots.at(cat_0bjet0jet) = addPlot(bins,bins,"lead jet pt 0,0 b-jets","p_{T} [GeV]","Events/GeV");
        leadjetpt_plots.at(cat_0bjet1jet) = addPlot(bins,bins,"lead jet pt 0,1 b-jets","p_{T} [GeV]","Events/GeV");
        leadjetpt_plots.at(cat_0bjet2jet) = addPlot(bins,bins,"lead jet pt 0,2 b-jets","p_{T} [GeV]","Events/GeV");
        leadjetpt_plots.at(cat_0bjet3jet) = addPlot(bins,bins,"lead jet pt 0,3 b-jets","p_{T} [GeV]","Events/GeV");
        bins.clear();
	bins << 30  << 40 << 50 << 60 << 70 << 80 << 90 << 100 << 120 << 140 << 160 << 200;
	leadjetpt_plots.at(cat_1bjet0jet) = addPlot(bins,bins,"lead jet pt 1,0 b-jets","p_{T} [GeV]","Events/GeV");
	leadjetpt_plots.at(cat_1bjet1jet) = addPlot(bins,bins,"lead jet pt 1,1 b-jets","p_{T} [GeV]","Events/GeV");
	leadjetpt_plots.at(cat_1bjet2jet) = addPlot(bins,bins,"lead jet pt 1,2 b-jets","p_{T} [GeV]","Events/GeV");
	leadjetpt_plots.at(cat_1bjet3jet) = addPlot(bins,bins,"lead jet pt 1,3 b-jets","p_{T} [GeV]","Events/GeV");
	bins.clear();
	bins << 30 << 50 << 60 << 70 << 80 << 90 << 120 << 160;
	leadjetpt_plots.at(cat_2bjet0jet) = addPlot(bins,bins,"lead jet pt 2,0 b-jets","p_{T} [GeV]","Events/GeV");
	leadjetpt_plots.at(cat_2bjet1jet) = addPlot(bins,bins,"lead jet pt 2,1 b-jets","p_{T} [GeV]","Events/GeV");
	leadjetpt_plots.at(cat_2bjet2jet) = addPlot(bins,bins,"lead jet pt 2,2 b-jets","p_{T} [GeV]","Events/GeV");
	leadjetpt_plots.at(cat_2bjet3jet) = addPlot(bins,bins,"lead jet pt 2,3 b-jets","p_{T} [GeV]","Events/GeV");


	bins.clear();
	bins << 30 << 35  << 40 << 50 << 60 << 70 << 80 << 90 << 100 << 120  << 200;
	secondjetpt_plots.at(cat_0bjet0jet) = addPlot(bins,bins,"second jet pt 0,0 b-jets","p_{T} [GeV]","Events/GeV");
	secondjetpt_plots.at(cat_0bjet1jet) = addPlot(bins,bins,"second jet pt 0,1 b-jets","p_{T} [GeV]","Events/GeV");
	secondjetpt_plots.at(cat_0bjet2jet) = addPlot(bins,bins,"second jet pt 0,2 b-jets","p_{T} [GeV]","Events/GeV");
	secondjetpt_plots.at(cat_0bjet3jet) = addPlot(bins,bins,"second jet pt 0,3 b-jets","p_{T} [GeV]","Events/GeV");
	bins.clear();
	bins      << 30 << 40 << 50 << 60 << 70 << 80 << 90 << 100 << 120 << 140  << 200;
	secondjetpt_plots.at(cat_1bjet0jet) = addPlot(bins,bins,"second jet pt 1,0 b-jets","p_{T} [GeV]","Events/GeV");
	secondjetpt_plots.at(cat_1bjet1jet) = addPlot(bins,bins,"second jet pt 1,1 b-jets","p_{T} [GeV]","Events/GeV");
	secondjetpt_plots.at(cat_1bjet2jet) = addPlot(bins,bins,"second jet pt 1,2 b-jets","p_{T} [GeV]","Events/GeV");
	bins.clear();
	bins      << 30 << 35  << 40 << 50 << 60 << 70 << 80 << 90 << 100 << 120 << 200;
	secondjetpt_plots.at(cat_1bjet3jet) = addPlot(bins,bins,"second jet pt 1,3 b-jets","p_{T} [GeV]","Events/GeV");
	bins.clear();
	bins  << 30  << 45 <<  65  << 80 << 100 << 120 << 160 << 300;
	secondjetpt_plots.at(cat_2bjet0jet) = addPlot(bins,bins,"second jet pt 2,0 b-jets","p_{T} [GeV]","Events/GeV");
	secondjetpt_plots.at(cat_2bjet1jet) = addPlot(bins,bins,"second jet pt 2,1 b-jets","p_{T} [GeV]","Events/GeV");
	bins.clear();
	bins  << 30  << 45 <<  65  << 80 << 100 << 120  << 300;
	secondjetpt_plots.at(cat_2bjet2jet) = addPlot(bins,bins,"second jet pt 2,2 b-jets","p_{T} [GeV]","Events/GeV");
	secondjetpt_plots.at(cat_2bjet3jet) = addPlot(bins,bins,"second jet pt 2,3 b-jets","p_{T} [GeV]","Events/GeV");

	bins.clear();
	bins<< 30 << 35  << 40 << 50 << 60 << 70 << 80 << 90 << 100 << 120 << 140 << 200;
	thirdjetpt_plots.at(cat_0bjet0jet) = addPlot(bins,bins,"third jet pt 0,0 b-jets","p_{T} [GeV]","Events/GeV");
	thirdjetpt_plots.at(cat_0bjet1jet) = addPlot(bins,bins,"third jet pt 0,1 b-jets","p_{T} [GeV]","Events/GeV");
	thirdjetpt_plots.at(cat_0bjet2jet) = addPlot(bins,bins,"third jet pt 0,2 b-jets","p_{T} [GeV]","Events/GeV");
	bins.clear();
	bins << 30 << 35  << 40 << 50 << 60 << 70 << 80 << 200;
	thirdjetpt_plots.at(cat_0bjet3jet) = addPlot(bins,bins,"third jet pt 0,3 b-jets","p_{T} [GeV]","Events/GeV");
	bins.clear();
	bins << 30 << 35  << 40 << 50 << 60 << 70 << 80 << 90 << 100 << 200;
	thirdjetpt_plots.at(cat_1bjet0jet) = addPlot(bins,bins,"third jet pt 1,0 b-jets","p_{T} [GeV]","Events/GeV");
	thirdjetpt_plots.at(cat_1bjet1jet) = addPlot(bins,bins,"third jet pt 1,1 b-jets","p_{T} [GeV]","Events/GeV");
	thirdjetpt_plots.at(cat_1bjet2jet) = addPlot(bins,bins,"third jet pt 1,2 b-jets","p_{T} [GeV]","Events/GeV");
	bins.clear();
	bins << 30 << 35  << 40 << 50 << 60 << 200;
	thirdjetpt_plots.at(cat_1bjet3jet) = addPlot(bins,bins,"third jet pt 1,3 b-jets","p_{T} [GeV]","Events/GeV");
	bins.clear();
	bins << 30    << 55  << 70  << 90  << 200;
	thirdjetpt_plots.at(cat_2bjet0jet) = addPlot(bins,bins,"third jet pt 2,0 b-jets","p_{T} [GeV]","Events/GeV");
	thirdjetpt_plots.at(cat_2bjet1jet) = addPlot(bins,bins,"third jet pt 2,1 b-jets","p_{T} [GeV]","Events/GeV");
	bins.clear();
	bins << 30    << 55  << 70  <<  200;
	thirdjetpt_plots.at(cat_2bjet2jet) = addPlot(bins,bins,"third jet pt 2,2 b-jets","p_{T} [GeV]","Events/GeV");
	bins.clear();
	bins << 30    << 50  <<  200;
	thirdjetpt_plots.at(cat_2bjet3jet) = addPlot(bins,bins,"third jet pt 2,3 b-jets","p_{T} [GeV]","Events/GeV");

	//agrohsje combined jet multi 
	bins.clear();
	bins=histo1D::createBinning(4,-0.5,3.5);
	jetmulti_plots.at(cat_0bjet) = addPlot(bins,bins,"jet multi 0x b-jets","","Events");
	jetmulti_plots.at(cat_1bjet) = addPlot(bins,bins,"jet multi 1x b-jets","","Events");
	bins.clear();
	//agrohsje
	//bins=histo1D::createBinning(4,-0.5,3.5);
	bins=histo1D::createBinning(3,-0.5,2.5);
	jetmulti_plots.at(cat_2bjet) = addPlot(bins,bins,"jet multi 2x b-jets","","Events");
	
	//agrohsje ht 
	bins.clear();
	bins<< 30 << 70 << 110 << 150 << 200 <<250;
	htalljets_plots.at(cat_0bjet) = addPlot(bins,bins,"ht all jets 0x b-jets","h_{T} [GeV]","Events/GeV");
	htalljets_plots.at(cat_1bjet) = addPlot(bins,bins,"ht all jets 1x b-jets","h_{T} [GeV]","Events/GeV");
	htalljets_plots.at(cat_2bjet) = addPlot(bins,bins,"ht all jets 2x b-jets","h_{T} [GeV]","Events/GeV");

}


void analysisPlotsTtbarXsecFitSingleLep::fillPlotsGen(){
	if(!use()) return;
	if(!event()) return;
	using namespace std;
	using namespace ztop;
	//define vis PS here
	/*
	 * Following the ATLAS definition, taking into account leptons after FSR/PS (Pythia stat 1)
	 * Including intermediate tau decays (or not)
	 */
	std::vector<NTGenParticle*> genvisleptons1=*(event()->genleptons1);
        std::vector<NTGenParticle*> genbs=*(event()->genbs);
	totevts_++;
	totevtsw_+=puweight();
        if (!event()->genleptons1) return;
        if (!event()->genbs) return;
	//only fill one bin in some visible part of the histogram to get the total
	// n_gen and a nice display of PS migrations
	if (!genvisleptons1.size()) return;
	if(genvisleptons1.size()>0 && genvisleptons1.at(0)->pt() > 30 && genvisleptons1.at(0)->eta() < 2.4 && genbs.size() > 1 && genbs.at(0)->pt() > 30 && genbs.at(1)->pt()> 30 && genbs.at(0)->eta()<2.4 && genbs.at(1)->eta()<2.4){  
        if(genvisleptons1.size()>1 && genvisleptons1.at(1)->pt() > 20 && genvisleptons1.at(1)->eta() <2.4  ) return;
                 for(size_t i=0;i<total_plots.size();i++){
			total_plots.at(i)->fillGen(0.5,puweight());
			//agrohsje changed from 20.5 to 30.5 checked with Jan 
			leadjetpt_plots.at(i)->fillGen(30.5,puweight());
			secondjetpt_plots.at(i)->fillGen(30.5,puweight());
			thirdjetpt_plots.at(i)->fillGen(30.5,puweight());
		}
		for(size_t i=0;i<jetmulti_plots.size();i++){
		    jetmulti_plots.at(i)->fillGen(0,puweight());
		    htalljets_plots.at(i)->fillGen(30.5,puweight()); 
		}
		vispsevts_++;
		vispsevtsw_+=puweight();
	}


}

void analysisPlotsTtbarXsecFitSingleLep::fillPlotsReco(){
	if(!use()) return;
	if(!event()) return;
	using namespace std;
	using namespace ztop;
	size_t nbjets=0;
	size_t naddjets=0;
	if(event()->selectedbjets && event()->selectedjets){
		nbjets=event()->selectedbjets->size();
		naddjets=event()->selectedjets->size() - nbjets;

		setJetCategory(nbjets,naddjets);
		//agrohsje 
		//if(naddjets==0)
		total_plots.at(jetcategory)->fillReco(0.5,puweight());
		if(naddjets==4)
			leadjetpt_plots.at(jetcategory)->fillReco(event()->selectedjets->at(0)->pt(),puweight());
		else if(naddjets==5)
			secondjetpt_plots.at(jetcategory)->fillReco(event()->selectedjets->at(1)->pt(),puweight());
		else if(naddjets>5){
			thirdjetpt_plots.at(jetcategory)->fillReco(event()->selectedjets->at(event()->selectedjets->size()-1)->pt(),puweight());
		}
		//agrohsje fill jet multi in one plot, limit to 2 jets at most 
	        if (bjetcategory==cat_2bjet)
		    if(naddjets>=2) jetmulti_plots.at(bjetcategory)->fillReco(2,puweight());
		    //if(naddjets>=3) jetmulti_plots.at(bjetcategory)->fillReco(3,puweight());
		    else jetmulti_plots.at(bjetcategory)->fillReco(naddjets,puweight());
		else
		    if(naddjets>=3) jetmulti_plots.at(bjetcategory)->fillReco(3,puweight());
		    else jetmulti_plots.at(bjetcategory)->fillReco(naddjets,puweight());
		//agrohsje fill ht including all light and b quark jets 
		double ht(0.);
		for(size_t i=0;i<event()->selectedjets->size();i++) ht+=event()->selectedjets->at(i)->pt();
		htalljets_plots.at(bjetcategory)->fillReco(ht,puweight());

       }
}


void analysisPlotsTtbarXsecFitSingleLep::setJetCategory(size_t nbjets,size_t njets){
	if(nbjets < 1 || nbjets>2){
	        bjetcategory=cat_0bjet; 
	        if(njets<4)
			jetcategory=cat_0bjet0jet;
		else if(njets<5)
			jetcategory=cat_0bjet1jet;
		else if(njets<6)
			jetcategory=cat_0bjet2jet;
		else if(njets>5)
			jetcategory=cat_0bjet3jet;
	}
	else if(nbjets < 2){
	        bjetcategory=cat_1bjet; 
		if(njets<4)
			jetcategory=cat_1bjet0jet;
		else if(njets<5)
			jetcategory=cat_1bjet1jet;
		else if(njets<6)
			jetcategory=cat_1bjet2jet;
		else if(njets>5)
			jetcategory=cat_1bjet3jet;
	}
	else if(nbjets < 3){
	        bjetcategory=cat_2bjet; 
		if(njets<4)
			jetcategory=cat_2bjet0jet;
		else if(njets<5)
			jetcategory=cat_2bjet1jet;
		else if(njets<6)
			jetcategory=cat_2bjet2jet;
		else if(njets>5)
			jetcategory=cat_2bjet3jet;
	}
}


}//ns
