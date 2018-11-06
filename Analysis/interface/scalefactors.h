
/*
 * scalefactors.h
 *
 *  Created on: Aug 1, 2013
 *      Author: kiesej
 */

#ifndef SCALEFACTORS_H_
#define SCALEFACTORS_H_

#include "TH1D.h"
#include "TH2D.h"
#include "TString.h"
#include <iostream>
#include <stdexcept>
#include "TFile.h"

#include "TtZAnalysis/DataFormats/interface/NTJet.h"
#include "TtZAnalysis/DataFormats/interface/NTElectron.h"
#include "TtZAnalysis/DataFormats/interface/NTMuon.h"

namespace ztop{

class scalefactors{
public:

	enum systematics{sys_up,sys_down,sys_nominal};

scalefactors(): h(0), isth2d_(false), isMC_(false), syst_(sys_nominal), variation_(""), rangecheck_(true),isglobal_(false),glsf_(0),glsfup_(0),glsfd_(0),switchedoff_(false){}
	scalefactors(const scalefactors&);
	scalefactors& operator = (const scalefactors& );
	~scalefactors(){}

	int setInput(TString filename, TString histname, TString histnameerrup="", TString histnameerrdown="");
	void setGlobal(double sf,double errup=0,double errdown=0);
	int setSystematics(TString updownnom);

	void setVariation(TString variation){variation_=variation;}

	void setIsMC(bool is){isMC_=is;}
	void setIsTtbar(bool istt){isTtbar_=istt;}
	void setIsTtbarLike(bool isttlike){isTtbarLike_=isttlike;}

	void setRangeCheck(bool check){rangecheck_=check;}

	void switchOff(bool switchoff){switchedoff_=switchoff;}

	bool isSystematicsUp(){return (syst_ == sys_up);}
	bool isSystematicsDown(){return ( syst_ == sys_down);}

	double getScalefactor(double xval,double yval=0)const;

        float getElectronESFactor( NTElectron* ele ) const;
        float getElectronERFactor_phi( NTElectron* ele ) const;
        float getElectronERFactor_rho( NTElectron* ele ) const;
        float getElectronESERFactorFromEnvelope( NTElectron* ele ) const;

        float getAdditionalJECFactor( NTJet* jet ) const;
        float readJECFactorFromFile( NTJet* jet ) const;

        float getMuonRochesterFactorFromEnvelope( NTMuon* muon ) const;
        float getMuonRochesterFactorFromEnvelope_up( NTMuon* muon ) const;
        float getMuonRochesterFactorFromEnvelope_down( NTMuon* muon ) const;

        float getElectronESFactorUp( NTElectron* ele ) const;
        float getElectronESFactorDown( NTElectron* ele ) const;
        float getElectronERFactorUp( NTElectron* ele ) const;
        float getElectronERFactorDown( NTElectron* ele ) const;

private:
	TH1D th1d_,th1derrup_,th1derrdown_;
	TH2D th2d_,th2derrup_,th2derrdown_;
	TH1 * h;
	bool isth2d_,isMC_,isTtbar_,isTtbarLike_;
	systematics syst_;
        TString variation_;
	bool rangecheck_;
	bool isglobal_;
	double glsf_,glsfup_,glsfd_;
	bool switchedoff_;

	void setHistPointer();

	template<class T>
	T* tryToGet(TFile * f, TString name)const{
	    T  * h= (T  *)f->Get(name);
	    if(!h || h->IsZombie()){
	        std::cout << "scalefactors::tryToGet " << name << " - not found in "<< f->GetName() <<std::endl;
	        throw std::runtime_error("scalefactors::tryToGet: Histogram not found " );
	    }
	    return h;
	}

	void copyFrom(const scalefactors& rhs);

};

/**
 * gets scale factor for either 2D or 1D input
 * returns -999999 if no histo input has been set
 */

inline double scalefactors::getScalefactor(double xval,double yval)const{
	if(switchedoff_ || !isMC_)
		return 1;

	if(!isglobal_ && !h){
		std::cout << "scalefactors::getScalefactor: input has not been set! returning -999999" << std::endl;
		return -999999;
	}
	if(!isglobal_ && !isth2d_ && yval!=0){
		throw std::logic_error("scalefactors::getScalefactor: trying to give a y value to 1D scale factor!");
	}

	if(!isglobal_){
		double content=0;
		Int_t bin=h->FindFixBin(xval,yval);
		content=h->GetBinContent(bin);
		if(content<0.001 && rangecheck_){
			std::cout << "scalefactors::getScalefactor: scale factor very small ~0. warning\nmaybe ut of range? input was: " << xval<< ","<< yval<< std::endl;
			throw std::runtime_error("scalefactors::getScalefactor: scale factor very small ~0. maybe ut of range");
		}
	       if(content<0.001){
                        if (xval < h->GetXaxis()->GetXmin()) this->getScalefactor(h->GetXaxis()->GetXmin()*1.000000001,yval);
                        else if (xval > h->GetXaxis()->GetXmax()) this->getScalefactor(h->GetXaxis()->GetXmin()*0.9999999999,yval);
                        else if (yval < h->GetYaxis()->GetXmin()) this->getScalefactor(xval,h->GetYaxis()->GetXmin()*1.000000001);
                        else if (yval > h->GetYaxis()->GetXmax()) this->getScalefactor(xval,h->GetYaxis()->GetXmin()*0.9999999999);
			return 1;
		}
		return content;
	}
	else{ //global sf
		if(syst_==sys_nominal)
			return glsf_;
		else if(syst_==sys_down) //down
			return glsfd_;
		else if(syst_==sys_up) //up
			return glsfup_;
		else return 0; //never reached
	}
}




}//namespace
#endif /* SCALEFACTORS_H_ */
