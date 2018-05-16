/*
 * scalefactors.cc
 *
 *  Created on: Aug 1, 2013
 *      Author: kiesej
 */
#include "../interface/scalefactors.h"
#include <cstdlib>
#include <stdexcept>
#include "TGraphAsymmErrors.h"
#include "TGraphErrors.h"
#include "TH1F.h"
#include "TFile.h"
#include "TopAnalysis/ZTopUtils/interface/miscUtils.h"

namespace ztop {


/**
 * decide for 1d, 2d before
 */



/**
 * sets input with a lot of error checks
 * return codes < 0 --> error
 * -1: wrong filename
 * -2: wrong histoname
 * -3: wrong histoclass
 *
 * no error returns 0;
 */
int scalefactors::setInput(TString filename, TString histname, TString histnameerrup, TString histnameerrdown) {
	TFile * f = new TFile(filename, "READ");
	isglobal_ = false;
	if (f->IsZombie()) {
		std::cout << "scalefactors::setInput: file " << filename
				<< " not found.!" << std::endl;
		throw std::runtime_error("scalefactors::setInput: file not found");
	}

	TObject * ph = tryToGet<TObject>(f,histname);
	TString classname = ph->ClassName();
	if (classname.Contains("TH2")) {
		th2d_ = *(TH2D*) ph;
		h = &th2d_;
		isth2d_ = true;
		isglobal_ = false;
		//fill errors
		th2derrup_=th2d_;
		th2derrdown_=th2d_;
		if(histnameerrup.Length()<1 || histnameerrdown.Length()<1){
			for(int binx=0;binx<=th2d_.GetNbinsX();binx++){
				for(int biny=0;biny<=th2d_.GetNbinsY();biny++){
					//std::cout << "setting errors: " << th2d_.GetBinContent(binx,biny) + th2d_.GetBinError(binx,biny) <<std::endl;
					th2derrup_.SetBinContent(binx,biny, th2d_.GetBinContent(binx,biny) + th2d_.GetBinError(binx,biny) );
					th2derrdown_.SetBinContent(binx,biny, th2d_.GetBinContent(binx,biny) - th2d_.GetBinError(binx,biny) );
				}
			}
		}
		else{
			th2derrup_=*tryToGet<TH2D>(f, histnameerrup);
			th2derrdown_=*tryToGet<TH2D>(f, histnameerrdown);
		}

		return 0;
	} else if (classname.Contains("TH1D")) {
		th1d_ = *(TH1D*) ph;
		h = &th1d_;
		isth2d_ = false;
		isglobal_ = false;

		th1derrup_=th1d_;
		th1derrdown_=th1d_;
		if(histnameerrup.Length()<1 || histnameerrup.Length()<1){
			for(int binx=0;binx<=th1d_.GetNbinsX();binx++){
				th1derrup_.SetBinContent(binx, th1d_.GetBinContent(binx) + th1d_.GetBinError(binx) );
				th1derrdown_.SetBinContent(binx, th1d_.GetBinContent(binx) - th1d_.GetBinError(binx) );
			}

		}
		else{
			th1derrup_=*tryToGet<TH1D>(f, histnameerrup);
			th1derrdown_=*tryToGet<TH1D>(f, histnameerrdown);
		}

		return 0;
	} else if(classname.Contains("TGraphAsymmErrors")){
		TGraphAsymmErrors *g = (TGraphAsymmErrors*)ph;
		//create bins
		std::vector<double> bins;
		//sort points
		std::vector<double> xpoints;//(g->GetX(),(size_t)g->GetN());
		for(int i=0;i<g->GetN();i++)
			xpoints.push_back(g->GetX()[i]);

		std::vector<size_t> sortlist=retsort(xpoints.begin(),xpoints.end());
		for(size_t i=0;i<sortlist.size();i++){
			size_t realit=sortlist.at(i);
			//std::cout << "getting: "<< i << "->" << realit << std::endl;
			double x=0;
			double xnext=0;
			double y=0;
			g->GetPoint(realit,x,y);
			if(realit<(size_t)g->GetN()-1){
				g->GetPoint(realit+1,xnext,y);
				g->GetErrorXhigh(realit);
				g->GetErrorXlow(realit);
				g->GetErrorXlow(realit+1);
				if(fabs( x+ g->GetErrorXhigh(realit)-(xnext-g->GetErrorXlow(realit+1))) > 0.01*(xnext-x)){
					throw std::runtime_error("scalefactors::setInput: cannot find valid bin indications in input graph");
				}
				bins.push_back(x-g->GetErrorXlow(realit));
			}
			else{//last boundary
				bins.push_back(x-g->GetErrorXlow(realit));
				bins.push_back(x+g->GetErrorXhigh(realit));
			}
			//g->GetErrorYhigh(i);
			//g->GetErrorYlow(i);


		}
		//for (size_t i=0;i<bins.size();i++)
		//	std::cout << bins.at(i) << std::endl;

		th1d_=TH1D("","",bins.size()-1,&(bins.at(0)));
		th1derrdown_=th1d_;
		th1derrup_=th1d_;
		for(size_t i=0;i<sortlist.size();i++){
			size_t realit=sortlist.at(i);
			double x=0;
			double y=0;
			g->GetPoint(i,x,y);
			th1d_.SetBinContent(realit+1,y);
			th1derrdown_.SetBinContent(realit+1,y-g->GetErrorYlow(i));
			th1derrup_.SetBinContent(realit+1,y+g->GetErrorYhigh(i));
		}
		h = &th1d_;
		TH1D* htemp=(TH1D*)h->Clone();
		htemp->Draw();
		isth2d_ = false;
		isglobal_ = false;
		return 0;

	}else {
		std::cout
		<< "scalefactors::setInput: class of input histo must be TH1D, TGraphAsymmErrors or TH2*"
		<< std::endl;
		throw std::runtime_error("scalefactors::setInput: histo wrong format");
	}

}
/**
 * errors in percent!
 */
void scalefactors::setGlobal(double sf, double errup, double errdown) {
	glsf_ = sf;
	glsfup_ = glsf_ + errup / 100;
	glsfd_ = glsf_ - errdown / 100;
	isglobal_ = true;
}
//priv
void scalefactors::setHistPointer(){
	if(syst_==sys_nominal){
		if(isth2d_) h=&th2d_;
		else h=&th1d_;
	}
	else if(syst_==sys_up){
		if(isth2d_) h=&th2derrup_;
		else h=&th1derrup_;
	}
	else if(syst_==sys_down){
		if(isth2d_) h=&th2derrdown_;
		else h=&th1derrdown_;
	}
}

/**
 * sets systematic variation.
 * returns -1 in case of no success and sets to nominal
 * inputstring should contain only one option of:
 * -nom
 * -down
 * -up
 * if more than one option is provided, the first found is set according to the
 * ordering given above
 */
int scalefactors::setSystematics(TString updownnom) {
	if (updownnom.Contains("nom")) {
		syst_ = sys_nominal;
		setHistPointer();
		std::cout << "scalefactors::setSystematics: set to " << updownnom
				<< std::endl;
		return 0;
	} else if (updownnom.Contains("down")) {
		syst_ = sys_down;
		setHistPointer();
		std::cout << "scalefactors::setSystematics: set to " << updownnom
				<< std::endl;
		return 0;
	} else if (updownnom.Contains("up")) {
		syst_ = sys_up;
		setHistPointer();
		std::cout << "scalefactors::setSystematics: set to " << updownnom
				<< std::endl;
		return 0;
	} else {
		std::cout
		<< "scalefactors::setSystematics: input must contain up, down or nom"
		<< std::endl;
		h=0;
		return -1;
	}
}
scalefactors::scalefactors(const ztop::scalefactors & rhs) {
	if(&rhs != this)
		copyFrom(rhs);
}

scalefactors& scalefactors::operator = (const scalefactors& rhs){
	if(&rhs!=this)
		copyFrom(rhs);
	return *this;
}

void scalefactors::copyFrom(const scalefactors& rhs){
	th1d_ = rhs.th1d_;
	th2d_ = rhs.th2d_;
	th1derrup_ = rhs.th1derrup_;
	th2derrup_ = rhs.th2derrup_;
	th1derrdown_ = rhs.th1derrdown_;
	th2derrdown_ = rhs.th2derrdown_;
	isth2d_ = rhs.isth2d_;

	isMC_ = rhs.isMC_;
	isglobal_ = rhs.isglobal_;
	glsf_=rhs.glsf_;
	glsfup_=rhs.glsfup_;
	glsfd_=rhs.glsfd_;
	rangecheck_ = rhs.rangecheck_;
	switchedoff_=rhs.switchedoff_;

	syst_ = rhs.syst_;
	setHistPointer();
}

float scalefactors::getElectronESFactor( NTElectron* ele )const{
    if(switchedoff_ || !isMC_ || syst_==sys_nominal ) return 1.;
    else if (syst_==sys_up){
        return 1./ele->getMember(3);
    }
    else if (syst_==sys_down){
        return 1./ele->getMember(4);
    }
    else return -99999.;
}

float scalefactors::getElectronESFactorUp( NTElectron* ele )const{
    return 1./ele->getMember(3);
}
float scalefactors::getElectronESFactorDown( NTElectron* ele )const{
    return 1./ele->getMember(4);
}


float scalefactors::getElectronERFactor_phi( NTElectron* ele )const{
    if(switchedoff_ || !isMC_ || syst_==sys_nominal ) return 1.;
    else if (syst_==sys_up){
        return 1./ele->getMember(1);
    }
    else if (syst_==sys_down){
        return 1./ele->getMember(2); // same as member 1
    }
    else return -99999.;
}

float scalefactors::getElectronERFactor_rho( NTElectron* ele )const{
    if(switchedoff_ || !isMC_ || syst_==sys_nominal ) return 1.;
    else if (syst_==sys_up){
        return 1./ele->getMember(5);
    }
    else if (syst_==sys_down){
        return 1./ele->getMember(6);
    }
    else return -99999.;
}


float scalefactors::getElectronERFactorUp( NTElectron* ele )const{
    float temp_sf = 1.;
    if (ele->getMember(1) < temp_sf) temp_sf = ele->getMember(1);
    if (ele->getMember(2) < temp_sf) temp_sf = ele->getMember(2);
    if (ele->getMember(5) < temp_sf) temp_sf = ele->getMember(5);
    if (ele->getMember(6) < temp_sf) temp_sf = ele->getMember(6);
    return 1./temp_sf;
}

float scalefactors::getElectronERFactorDown( NTElectron* ele )const{
    float temp_sf = 1.;
    if (ele->getMember(1) > temp_sf) temp_sf = ele->getMember(1);
    if (ele->getMember(2) > temp_sf) temp_sf = ele->getMember(2);
    if (ele->getMember(5) > temp_sf) temp_sf = ele->getMember(5);
    if (ele->getMember(6) > temp_sf) temp_sf = ele->getMember(6);
    return 1./temp_sf;
}



float scalefactors::getElectronESERFactorFromEnvelope( NTElectron* ele )const{
    if(switchedoff_ || !isMC_ || syst_==sys_nominal ) return 1.;
    else if (syst_==sys_up){
        float temp_sf = 1.;
        if (ele->getMember(1) > temp_sf) temp_sf = ele->getMember(1);
        if (ele->getMember(2) > temp_sf) temp_sf = ele->getMember(2);
        if (ele->getMember(3) > temp_sf) temp_sf = ele->getMember(3);
        if (ele->getMember(4) > temp_sf) temp_sf = ele->getMember(4);
        if (ele->getMember(1)*ele->getMember(3) > temp_sf) temp_sf = ele->getMember(1)*ele->getMember(3);
        if (ele->getMember(1)*ele->getMember(4) > temp_sf) temp_sf = ele->getMember(1)*ele->getMember(4);
        if (ele->getMember(2)*ele->getMember(3) > temp_sf) temp_sf = ele->getMember(2)*ele->getMember(3);
        if (ele->getMember(2)*ele->getMember(4) > temp_sf) temp_sf = ele->getMember(2)*ele->getMember(4);
        return temp_sf;
    }
    else if (syst_==sys_down){
        float temp_sf = 1.;
        if (ele->getMember(1) < temp_sf) temp_sf = ele->getMember(1);
        if (ele->getMember(2) < temp_sf) temp_sf = ele->getMember(2);
        if (ele->getMember(3) < temp_sf) temp_sf = ele->getMember(3);
        if (ele->getMember(4) < temp_sf) temp_sf = ele->getMember(4);
        if (ele->getMember(1)*ele->getMember(3) < temp_sf) temp_sf = ele->getMember(1)*ele->getMember(3);
        if (ele->getMember(1)*ele->getMember(4) < temp_sf) temp_sf = ele->getMember(1)*ele->getMember(4);
        if (ele->getMember(2)*ele->getMember(3) < temp_sf) temp_sf = ele->getMember(2)*ele->getMember(3);
        if (ele->getMember(2)*ele->getMember(4) < temp_sf) temp_sf = ele->getMember(2)*ele->getMember(4);
        return temp_sf;
    }
    else return -99999.;
}

float scalefactors::getAdditionalJECFactor( NTJet* jet )const{
    if(switchedoff_ || !isMC_ || !isTtbar_ || syst_==sys_nominal ) return 1.;
    else return readJECFactorFromFile(jet);
}

float scalefactors::readJECFactorFromFile( NTJet* jet )const{

    if (jet->pt()<30) return 1.;

    TFile * f = new TFile ("data/analyse/jetrescale.root","READ");
    if (!f) std::runtime_error("scalefactors::readJECFactorFromFile: file not found");
    
    TString graphname ;
    if (variation_=="TT_FSRSCALE") graphname = "fsr";
    if (variation_=="TT_FRAG") graphname = "bfrag";
    else {
        std::cout<<"variation "<<variation_<<" not supported"<<std::endl;
        std::runtime_error("scalefactors::readJECFactorFromFile: variation not supported");
    }
    if (syst_==sys_up) graphname += "up_";
    else if (syst_==sys_down) graphname += "down_";

    if (abs(jet->getMember(1)) == 5) graphname += "b_"; // jet hadron flav
    else graphname += "l_";

    if (fabs(jet->eta())<1.5) graphname += "B";
    else graphname += "E";

    TGraphErrors * ge = (TGraphErrors*) f->GetObjectChecked(graphname,"TGraphErrors");
    if (!ge) {
        std::cout<<"graph "<<graphname<<" not found"<<std::endl;
        std::runtime_error("scalefactors::readJECFactorFromFile: graph  not found");
    }

    float factor = ge->Eval(jet->pt());
    if (variation_=="TT_FRAG" && abs(jet->getMember(1)) != 5) factor = 1.;

    delete ge;
    f->Close();
    delete f;

    return factor;
}

float scalefactors::getMuonRochesterFactorFromEnvelope( NTMuon* muon ) const{
    if(switchedoff_ || !isMC_ || syst_==sys_nominal ) return 1.;
    else if (syst_==sys_up){
        float temp_sf = 1.;
        for (int iVar = 2; iVar < 7; ++iVar){
            if ( muon->getMember(iVar) > temp_sf ) temp_sf = muon->getMember(iVar);
        }
        temp_sf = sqrt( (1-temp_sf)*(1-temp_sf) + (1-muon->getMember(0))*(1-muon->getMember(0)) );
        temp_sf += 1.;
        return temp_sf;
    }
    else if (syst_==sys_down){
        float temp_sf = 1.;
        for (int iVar = 2; iVar < 7; ++iVar){
            if ( muon->getMember(iVar) < temp_sf ) temp_sf = muon->getMember(iVar);
        }
        temp_sf = sqrt( (1-temp_sf)*(1-temp_sf) + (1-muon->getMember(1))*(1-muon->getMember(1)) );
        temp_sf = 1-temp_sf;
        return temp_sf;
    }
    else return -99999.;
}

float scalefactors::getMuonRochesterFactorFromEnvelope_up( NTMuon* muon ) const{
    if(switchedoff_ || !isMC_ ) return 1.;
    float temp_sf = 1.;
    for (int iVar = 2; iVar < 7; ++iVar){
        if ( muon->getMember(iVar) > temp_sf ) temp_sf = muon->getMember(iVar);
    }
    temp_sf = sqrt( (1-temp_sf)*(1-temp_sf) + (1-muon->getMember(0))*(1-muon->getMember(0)) );
    temp_sf += 1.;
    return temp_sf;
}

float scalefactors::getMuonRochesterFactorFromEnvelope_down( NTMuon* muon ) const{
    if(switchedoff_ || !isMC_) return 1.;
    float temp_sf = 1.;
    for (int iVar = 2; iVar < 7; ++iVar){
        if ( muon->getMember(iVar) < temp_sf ) temp_sf = muon->getMember(iVar);
    }
    temp_sf = sqrt( (1-temp_sf)*(1-temp_sf) + (1-muon->getMember(1))*(1-muon->getMember(1)) );
    temp_sf = 1-temp_sf;
    return temp_sf;
}



}

