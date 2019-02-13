/*
 * ttbarXsecFitter.cc
 *
 *  Created on: Oct 9, 2014
 *      Author: kiesej
 */


#include <TtZAnalysis/Tools/interface/plotterMultiStack.h>
#include <TtZAnalysis/Tools/interface/plotterMultiCompare.h>
#include <TtZAnalysis/Tools/interface/plotterMultiplePlots.h>
#include "../interface/ttbarXsecFitter.h"
#include "TopAnalysis/ZTopUtils/interface/miscUtils.h"
#include "TtZAnalysis/Tools/interface/histoStack.h"
#include "TtZAnalysis/Tools/interface/histoStackVector.h"
#include "TtZAnalysis/Tools/interface/fileReader.h"
#include "Math/Functor.h"
#include "TtZAnalysis/Tools/interface/plotterControlPlot.h"
#include "TtZAnalysis/Tools/interface/texTabler.h"
#include "limits.h"

size_t n_signals = 4; //hardcoded

namespace ztop{

bool ttbarXsecFitter::debug=false;
TRandom3 * ttbarXsecFitter::random_=0;

bool ttbarXsecFitter::includeUFOF=true;

void ttbarXsecFitter::readInput(const std::string & configfilename){
	if(debug)
		std::cout << "ttbarXsecFitter::readInput" <<std::endl;
	fitsucc_=false;

	fileReader fr;
	fr.setComment("$");
	fr.setDelimiter(",");

	fr.setStartMarker("[ general ]");
	fr.setEndMarker("[ end - general ]");
	fr.readFile(configfilename);
	fr.setRequireValues(false);
	wjetsrescalefactor_=fr.getValue<float>("rescaleWjets",1.);
	fr.setRequireValues(true);
	mergesystfile_="/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/"+fr.getValue<std::string>("mergeSystFile");
	if(!fileExists((getenv("CMSSW_BASE")+mergesystfile_).data()))
		throw std::runtime_error("mergeSystFile does not exists");
	std::cout << "reading syst merging from " << mergesystfile_ << std::endl;

	if(wjetsrescalefactor_!=1.)
		std::cout << "Warning! Will rescale Wjets contribution with " << wjetsrescalefactor_ << std::endl;
	fr.clear();
	fr.setStartMarker("[ datasets ]");
	fr.setEndMarker("[ end - datasets ]");
	fr.readFile(configfilename);
	std::vector<std::string> datasetmarkers=fr.getMarkerValues("dataset");
	for(size_t i=0;i<datasetmarkers.size();i++){
		std::cout << datasetmarkers.at(i) <<std::endl;
		fileReader frd;
		frd.setComment("$");
		frd.setDelimiter(",");
		//8 TeV
		frd.setStartMarker("[ " +datasetmarkers.at(i) +  " ]");
		frd.setEndMarker("[ end " +datasetmarkers.at(i) + " ]");
		frd.readFile(configfilename);

		double lumi = frd.getValue<double>("lumi");
		double lumiunc = frd.getValue<double>("lumiunc");
		double energy = frd.getValue<double>("energy");
		double topmass = frd.getValue<double>("topmass");
		if(topmassrepl_>0){
			topmass=topmassrepl_;
			TString mtrepl="_"+toTString(172.5)+"_";TString mtrplwith="_"+toTString(topmass)+"_";
			setReplaceInInfile(mtrepl,mtrplwith);
			std::cout << "Replacing top mass with " <<topmass <<std::endl;
		}
		TString name = datasetmarkers.at(i);
		name.ReplaceAll(" ","");
		name.ReplaceAll("dataset-","");
		double xsec=getTtbarXsec(topmass,energy);

		datasets_.push_back(dataset(lumi,lumiunc,xsec,name,this));
		//DEBUG
		std::cout << "ttbarXsecFitter::readInput: created dataset: "
				<< lumi << ", "<< lumiunc <<", " << xsec << ", "<<name <<std::endl;
	}
	if(datasets_.size()<1)
		throw std::logic_error("ttbarXsecFitter::readInput: no dataset description found");

	if(datasets_.size()>1){
		//read correlations
		//more than two not implemented..but could be!
		if(datasets_.size()>2)
			throw std::out_of_range("ttbarXsecFitter: at the moment only max two datasets implemented\nIn case more datasets are required, the part that handles correlations between systematics has to be adjusted. The rest is already compatible");
		for(size_t i=0;i<datasets_.size();i++){
			for(size_t j=0;j<datasets_.size();j++){
				if(i==j)continue;
				fr.setStartMarker(("[ correlations "+ datasets_.at(i).getName() +" - " + datasets_.at(j).getName() + " ]").Data());
				fr.setEndMarker("[ end - correlations ]");
				fr.setDelimiter("=");
				//	fileReader::debug=true;
				fr.readFile(configfilename);

				for(size_t i=0;i<fr.nLines();i++){
					if(fr.nEntries(i)<2)
						continue;
					std::pair<TString, double> ptmp(fr.getData<TString>(i,0),fr.getData<double>(i,1));
					priorcorrcoeff_.push_back(ptmp);
				}
				//	fileReader::debug=false;
			}
		}
	}
	for(size_t i=0;i<datasets_.size();i++){
		datasets_.at(i).readStacks(configfilename,replaceininfiles_,removesyst_,priorcorrcoeff_);
	}
	for(size_t i=0;i<datasets_.size();i++){
		for(size_t j=0;j<datasets_.size();j++){
			if(i==j)
				continue;
			datasets_.at(i).equalizeIndices(datasets_.at(j));
		}
	}
	//now idices are fixed
	for(size_t i=0;i<datasets_.size();i++){
            if (!mttfit_) datasets_.at(i).createXsecIdx();
            else datasets_.at(i).createXsecIdxs();
            datasets_.at(i).createMassIdx();

	}
	createContinuousDependencies();
	//fileReader::debug=true;
	///////////// READ PRIORS ////////////


	fr.setStartMarker("[ priors ]");
	fr.setEndMarker("[ end - priors ]");
	fr.setDelimiter("=");
	fr.readFile(configfilename);
	for(size_t i=0;i<fr.nLines();i++){
		if(fr.nEntries(i)<2)
			continue;
		TString sysname= fr.getData<TString>(i,0);
		TString priorstr=fr.getData<TString>(i,1);
		if(!nopriors_ || sysname=="TOPMASS"){
			if(priorstr=="box")
				setPrior(sysname, prior_box);
                        else if(priorstr=="fsrbox")
                                setPrior(sysname, prior_narrowboxfsr);
			else if(priorstr=="boxright")
				setPrior(sysname, prior_narrowboxright);
			else if(priorstr=="boxleft")
				setPrior(sysname, prior_narrowboxleft);
			else if(priorstr=="float")
				setPrior(sysname, prior_float);
			else if(priorstr=="gauss")
				setPrior(sysname, prior_gauss);
                        else if(priorstr=="gaussbroad")
                                setPrior(sysname, prior_gaussbroad);
                        else if(priorstr=="gaussmass")
                                setPrior(sysname, prior_gaussmass);
			else if(priorstr=="fixed")
				setPrior(sysname, prior_parameterfixed);
			else if(priorstr=="fixed_up")
				setPrior(sysname, prior_parameterfixed_up);
			else if(priorstr=="fixed_down")
				setPrior(sysname, prior_parameterfixed_down);
			else{
				std::string errstr=("Did not recognize prior " +priorstr + " for "+ sysname).Data();
				throw std::runtime_error(errstr);
			}
		}
                            
	}


	if(!removesyst_){
		fr.setStartMarker("[ full extrapolation ]");
		fr.setEndMarker("[ end - full extrapolation ]");
		fr.setDelimiter(",");
	fr.readFile(configfilename);
		for(size_t i=0;i<fr.nLines();i++){
			if(fr.nEntries(i)<1)
				continue;
			if(fr.nEntries(i)<2)
				addFullExtrapolError(fr.getData<TString>(i,0));
			else if(fr.nEntries(i)==3)
				addFullExtrapolError(fr.getData<TString>(i,0),fr.getData<float>(i,1),fr.getData<float>(i,2));
			else{
				std::string errstr="ttbarXsecFitter::readInput: extrapolation uncertainty line not recognized ";
				errstr+=fr.getReJoinedLine(i);
				throw std::runtime_error(errstr);
			}
		}

	}

	std::cout << "Created " << ndependencies_ << " nuisance parameters"<<std::endl;
	if(debug){
		std::cout << "ttbarXsecFitter::readInput: done. " <<std::endl;
	}
}

void ttbarXsecFitter::createToysFromSyst(histo1D::pseudodatamodes mode){
	for(size_t i=0;i<datasets_.size();i++)
		datasets_.at(i).createToysFromSyst(mode);
}

void ttbarXsecFitter::dataset::createToysFromSyst(histo1D::pseudodatamodes mode){
    if (firstToy_){
        if(!random_){
            if (parent_->seed_==0) random_ = new TRandom3();
            else random_ = new TRandom3(parent_->seed_);
            if(debug) std::cout << "invoked new random" <<std::endl;

        }
        // careful: hardcoded!!! always check
        parent_->var_for_toys_ = {"TT_ISRSCALE_down","TT_ISRSCALE_up","TT_FSRSCALE_down","TT_FSRSCALE_up",
                                  "ST_ISRSCALE_down","ST_ISRSCALE_up","ST_FSRSCALE_down","ST_FSRSCALE_up",
                                  "TT_MATCH_down","TT_MATCH_up","TT_TTTUNE_down","TT_TTTUNE_up","TT_FRAG_PETERSON_up",
                                  "TT_CRGLUON_up","TT_CRQCD_up","TT_CRERD_up","TT_GENMCATNLO_up",
                                  "TOPMASS_up","TOPMASS_down"};

        std::cout<<"variations for toys:"<<std::endl;
        for(auto var: parent_->var_for_toys_) std::cout<<var<<std::endl;
        std::cout<<std::endl;

        datacontsorig_nbjets_=dataconts_nbjets_; //safe originals
        backgroundcontsorig_nbjets_=backgroundconts_nbjets_; //safe originals
        if (!parent_->mttfit_){
            signalcontsorig_nbjets_=signalconts_nbjets_; //safe originals
            signalpsmigcontsorig_nbjets_=signalpsmigconts_nbjets_; //safe originals
            signalvisgencontsorig_nbjets_=signalvisgenconts_nbjets_; //safe originals
        }
        else{
            signalcontsorig_nbjets_v_=signalconts_nbjets_v_; //safe originals
            signalpsmigcontsorig_nbjets_v_=signalpsmigconts_nbjets_v_; //safe originals
            signalvisgencontsorig_nbjets_v_=signalvisgenconts_nbjets_v_; //safe originals
        }

        std::cout << "Preparing toy syst experiments mode for dataset " << name_  <<std::endl;
        if(!debug)simpleFitter::printlevel=-1;

        firstToy_ = false;
    }


    for(size_t i=0;i<dataconts_nbjets_.size();i++){
        dataconts_nbjets_.at(i) = datacontsorig_nbjets_.at(i);
        // histo1D tmpmc=backgroundcontsorig_nbjets_.at(i) + signalcontsorig_nbjets_.at(i);
        // tmpmc.setAllErrorsZero(false); tmpmc.createStatFromContent();
        // dataconts_nbjets_.at(i) = tmpmc.createPseudoExperiment(random_,0,mode,-1);


        // // performing toys on nominal
        if (!parent_->mttfit_){
            signalconts_nbjets_.at(i)=signalcontsorig_nbjets_.at(i).createPseudoExperiment(random_,0,mode,-1);
            signalpsmigconts_nbjets_.at(i)=signalpsmigcontsorig_nbjets_.at(i).createPseudoExperiment(random_,0,mode,-1);
            signalvisgenconts_nbjets_.at(i)=signalvisgencontsorig_nbjets_.at(i).createPseudoExperiment(random_,0,mode,-1);
            // backgroundconts_nbjets_.at(i)=backgroundcontsorig_nbjets_.at(i).createPseudoExperiment(random_,0,mode,-1);
        }
        else{
            for (size_t s=0; s<signalconts_nbjets_v_.size(); ++s){
                signalconts_nbjets_v_.at(s).at(i)=signalcontsorig_nbjets_v_.at(s).at(i).createPseudoExperiment(random_,0,mode,-1);
                signalpsmigconts_nbjets_v_.at(s).at(i)=signalpsmigcontsorig_nbjets_v_.at(s).at(i).createPseudoExperiment(random_,0,mode,-1);
                signalvisgenconts_nbjets_v_.at(s).at(i)=signalvisgencontsorig_nbjets_v_.at(s).at(i).createPseudoExperiment(random_,0,mode,-1);
            }
        }

        std::vector<histo1D> bgs = backgroundconts_split_nbjets_.at(i);

        for (size_t t=0; t<bgs.size(); ++t){
            bgs.at(t)=backgroundconts_split_nbjets_.at(i).at(t).createPseudoExperiment(random_,0,mode,-1);
            bgs.at(t).getRelSystematicsFrom(backgroundconts_split_nbjets_.at(i).at(t));
        }

        // // getting relative variations from originals
        if (!parent_->mttfit_){
            signalconts_nbjets_.at(i).getRelSystematicsFrom(signalcontsorig_nbjets_.at(i));
            signalpsmigconts_nbjets_.at(i).getRelSystematicsFrom(signalpsmigcontsorig_nbjets_.at(i));
            signalvisgenconts_nbjets_.at(i).getRelSystematicsFrom(signalvisgencontsorig_nbjets_.at(i));
            // backgroundconts_nbjets_.at(i).getRelSystematicsFrom(backgroundcontsorig_nbjets_.at(i));
        }
        else{
            for (size_t s=0; s<signalconts_nbjets_v_.size(); ++s){
                signalconts_nbjets_v_.at(s).at(i).getRelSystematicsFrom(signalcontsorig_nbjets_v_.at(s).at(i));
                signalpsmigconts_nbjets_v_.at(s).at(i).getRelSystematicsFrom(signalpsmigcontsorig_nbjets_v_.at(s).at(i));
                signalvisgenconts_nbjets_v_.at(s).at(i).getRelSystematicsFrom(signalvisgencontsorig_nbjets_v_.at(s).at(i));
            }
        }

        for (auto var : parent_->var_for_toys_){
            if (var.BeginsWith("TT_") || var.Contains("TOPMASS")){

                if (!parent_->mttfit_) {
                    size_t index = signalcontsorig_nbjets_.at(i).getSystErrorIndex(var);
                    histo1D toy = signalcontsorig_nbjets_.at(i).createPseudoExperiment(random_,0,mode,index);
                    signalconts_nbjets_.at(i).removeError(var);
                    signalconts_nbjets_.at(i).addErrorContainer(var,toy);

                }
                else {
                    for (size_t s=0; s<signalconts_nbjets_v_.size(); ++s){
                        size_t index = signalcontsorig_nbjets_v_.at(s).at(i).getSystErrorIndex(var);
                        histo1D toy = signalcontsorig_nbjets_v_.at(s).at(i).createPseudoExperiment(random_,0,mode,index);
                        signalconts_nbjets_v_.at(s).at(i).removeError(var);
                        signalconts_nbjets_v_.at(s).at(i).addErrorContainer(var,toy);
                    }
                }

                for (size_t t=0; t<bgs.size(); ++t){
                    if (!backgroundlegends_.at(t).Contains("bg")) continue;

                    size_t index = backgroundconts_split_nbjets_.at(i).at(t).getSystErrorIndex(var);
                    histo1D toy  = backgroundconts_split_nbjets_.at(i).at(t).createPseudoExperiment(random_,0,mode,index);
                    
                    bgs.at(t).removeError(var);
                    bgs.at(t).addErrorContainer(var,toy);
                    bgs.at(t).equalizeSystematicsIdxs(backgroundconts_split_nbjets_.at(i).at(t));
                }
            }

            if (var.BeginsWith("ST_") || var.Contains("TOPMASS")){
                for (size_t t=0; t<bgs.size(); ++t){
                    if (!backgroundlegends_.at(t).Contains("tW")) continue;

                    size_t index = backgroundconts_split_nbjets_.at(i).at(t).getSystErrorIndex(var);
                    histo1D toy  = backgroundconts_split_nbjets_.at(i).at(t).createPseudoExperiment(random_,0,mode,index);
                    
                    bgs.at(t).removeError(var);
                    bgs.at(t).addErrorContainer(var,toy);
                    bgs.at(t).equalizeSystematicsIdxs(backgroundconts_split_nbjets_.at(i).at(t));
                }
            }
        }

        backgroundconts_nbjets_.at(i) = backgroundcontsorig_nbjets_.at(i);
        backgroundconts_nbjets_.at(i).clear();
        for (auto bg : bgs){
            backgroundconts_nbjets_.at(i) += bg;
        }

        if (!parent_->mttfit_) signalconts_nbjets_.at(i).equalizeSystematicsIdxs(signalcontsorig_nbjets_.at(i));
        else{
            for (size_t s=0; s<signalconts_nbjets_v_.size(); ++s)
                signalconts_nbjets_v_.at(s).at(i).equalizeSystematicsIdxs(signalcontsorig_nbjets_v_.at(s).at(i));
        }
        backgroundconts_nbjets_.at(i).equalizeSystematicsIdxs(backgroundcontsorig_nbjets_.at(i));

    }
    if (!parent_->mttfit_) signalshape_nbjet_.clear();
    else{
        for (size_t s=0; s<signalshape_nbjet_v_.size(); ++s)
            signalshape_nbjet_v_.at(s).clear();
    }
    data_nbjet_.clear();
    background_nbjet_.clear();

    if(debug)
        std::cout << "ttbarXsecFitter::dataset::createToysFromSyst: new toy created" <<std::endl;


}


void ttbarXsecFitter::createPseudoDataFromMC(histo1D::pseudodatamodes mode){
	pseudodatarun_=true;
	std::vector<size_t> excludefromsyspseudo;
	for(size_t i=0;i<datasets_.size();i++){
                if (!mttfit_) excludefromsyspseudo.push_back(datasets_.at(i).xsecIdx());
                else excludefromsyspseudo = datasets_.at(i).xsecIdxs();
                excludefromsyspseudo.push_back(datasets_.at(i).massIdx());
        }
	for(size_t i=0;i<datasets_.size();i++)
		datasets_.at(i).createPseudoDataFromMC(mode,excludefromsyspseudo);
}

void ttbarXsecFitter::dataset::createPseudoDataFromMC(histo1D::pseudodatamodes mode, const std::vector<size_t> & excludefromvar){
	if(firstpseudoexp_){ //first call
		if(!random_){
			random_ = new TRandom3();
			if(debug) std::cout << "invoked new random" <<std::endl;
		}
		datacontsorig_nbjets_=dataconts_nbjets_; //safe originals
		backgroundcontsorig_nbjets_=backgroundconts_nbjets_; //safe originals
		signalcontsorig_nbjets_=signalconts_nbjets_; //safe originals
		signalpsmigcontsorig_nbjets_=signalpsmigconts_nbjets_; //safe originals
		signalvisgencontsorig_nbjets_=signalvisgenconts_nbjets_; //safe originals

		//there are no syst anymore (only dummies)
		/*for(size_t i=0;i<priors_.size();i++){
			if(i!=xsecidx7_ && i != xsecidx8_)
				setPrior(parameternames_.at(i),prior_parameterfixed);
		}
		 */
		//minimize output
		std::cout << "Preparing pseudo-experiments mode for dataset " << name_  <<std::endl;
		if(!debug)simpleFitter::printlevel=-1;
		firstpseudoexp_=false;
		//debug=false;
	}


	for(size_t i=0;i<dataconts_nbjets_.size();i++){
		histo1D tmpmc=backgroundcontsorig_nbjets_.at(i) + signalcontsorig_nbjets_.at(i);
		dataconts_nbjets_.at(i)=tmpmc.createPseudoExperiment(random_,&datacontsorig_nbjets_.at(i),mode); //no variations
		backgroundconts_nbjets_.at(i)=backgroundcontsorig_nbjets_.at(i).createPseudoExperiments(random_,0,mode,excludefromvar);
		signalconts_nbjets_.at(i)=signalcontsorig_nbjets_.at(i).createPseudoExperiments(random_,0,mode,excludefromvar);
		signalpsmigconts_nbjets_.at(i)=signalpsmigcontsorig_nbjets_.at(i).createPseudoExperiments(random_,0,mode,excludefromvar);
		signalvisgenconts_nbjets_.at(i)=signalvisgencontsorig_nbjets_.at(i).createPseudoExperiments(random_,0,mode,excludefromvar);
	}
	signalshape_nbjet_.clear();
	data_nbjet_.clear();
	background_nbjet_.clear();
        for (size_t s=0; s<signalshape_nbjet_v_.size(); ++s)
            signalshape_nbjet_v_.at(s).clear();

	if(debug)
		std::cout << "ttbarXsecFitter::dataset::createPseudoDataFromMC: new pseudoexp created" <<std::endl;

}

void ttbarXsecFitter::createContinuousDependencies(){
	if(debug)
		std::cout << "ttbarXsecFitter::createContinuousDependencies" << std::endl;
	for(size_t i=0;i<datasets_.size();i++)
		datasets_.at(i).createContinuousDependencies();

	if (!mttfit_) ndependencies_=datasets_.at(0).signalshape(0).getNDependencies();
        else ndependencies_=datasets_.at(0).signalshape(0,0).getNDependencies();
	if(debug)
		std::cout << "ttbarXsecFitter::createContinuousDependencies: created dep: "
		<< ndependencies_<<std::endl;
	fittedparas_.resize(ndependencies_,0);
	priors_.resize(ndependencies_,prior_gauss);
	if (!mttfit_) parameternames_=datasets_.at(0).signalshape(0).getSystNames();
        else parameternames_=datasets_.at(0).signalshape(0,0).getSystNames();

	if(debug){
		for(size_t i=0;i<parameternames_.size();i++){
			std::cout << "\t"<< parameternames_.at(i) <<std::endl;
		}
                std::cout << "ttbarXsecFitter::createContinuousDependencies: done" <<std::endl;
        }
}
void ttbarXsecFitter::dataset::createContinuousDependencies(){
	if(debug)
		std::cout << "ttbarXsecFitter::dataset::createContinuousDependencies" <<std::endl;

	size_t bjetcount=0;
	//bool eighttev=true;
	signalshape_nbjet_.clear();
	data_nbjet_.clear();
	background_nbjet_.clear();
        for (size_t s=0; s<signalshape_nbjet_v_.size(); ++s)
            signalshape_nbjet_v_.at(s).clear();
	bool useMConly=parent_->useMConly_;
        size_t size;
        if (!parent_->mttfit_) size=signalconts_nbjets_.size();
        else {
            size=signalconts_nbjets_v_.at(0).size();
            signalshape_nbjet_v_.resize(signalconts_nbjets_v_.size());
        }
	for(size_t it=0;it<size;it++){
                if (!parent_->mttfit_)
                    signalshape_nbjet_.push_back(createLeptonJetAcceptance(signalconts_nbjets_,signalpsmigconts_nbjets_,signalvisgenconts_nbjets_, bjetcount));
                else{
                        signalintegrals_.resize(size);
                        acceptance_v_.resize(size);
                        acceptance_extr_v_.resize(size);
                        eps_emu_v_.resize(size);
                        container_c_b_v_.resize(size);
                        container_eps_b_v_.resize(size);
                        omega_nbjet_v_.resize(size);

                    for (size_t s=0; s<signalconts_nbjets_v_.size(); ++s){
                        signalshape_nbjet_v_.at(s).push_back(createLeptonJetAcceptance(signalconts_nbjets_v_.at(s),signalpsmigconts_nbjets_v_.at(s),signalvisgenconts_nbjets_v_.at(s), bjetcount, s));
                    }
                }
		histo1D temp=dataconts_nbjets_.at(it);
		if(useMConly){
                    if (!parent_->mttfit_) temp=signalconts_nbjets_.at(it) + backgroundconts_nbjets_.at(it);
                    else {
                        temp=backgroundconts_nbjets_.at(it);
                        for (size_t s=0; s<signalconts_nbjets_v_.size();++s)
                            temp += signalconts_nbjets_v_.at(s).at(it);
                    }
                    if (parent_->variationToFit_!=""){
                        size_t var_index = temp.getSystErrorIndex(parent_->variationToFit_);
                        temp = temp.getSystContainer(var_index);
                        if (!parent_->variationToFit_.Contains("TOPMASS_1GeV")){
                            temp.createStatFromContent();
                        }
                        temp.mergeLayers(dataconts_nbjets_.at(it));
                    }
                    else temp.setAllErrorsZero(false);
		}

		variateHisto1D tmpvarc;
		tmpvarc.import(temp);
		data_nbjet_.push_back(tmpvarc);

		tmpvarc.import(backgroundconts_nbjets_.at(it));
		background_nbjet_.push_back(tmpvarc);

		bjetcount++;
	}
}

size_t ttbarXsecFitter::nDatasets()const{
	return datasets_.size();
}
const TString& ttbarXsecFitter::datasetName(size_t datasetidx)const{
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::datasetName: index out of range");
	return datasets_.at(datasetidx).getName();
}
void ttbarXsecFitter::setPrior(const TString& sysname, priors prior){
	if(debug)
		std::cout << "ttbarXsecFitter::setPrior" <<std::endl;

	if(datasets_.size()<1){
		throw std::logic_error("ttbarXsecFitter::setPrior: first read in plots");
	}
	std::vector<TString> systs=datasets_.at(0).getSystNames();
	size_t idx=std::find(systs.begin(),systs.end(),sysname) - systs.begin();
	if(idx == systs.size()){
		std::string errstr=((TString)"ttbarXsecFitter::setPrior: requested variation \""+ sysname+"\" not found").Data();
		throw std::out_of_range(errstr);
	}
	priors_.at(idx) = prior;
}

void ttbarXsecFitter::addUncertainties(histoStack * stack,size_t datasetidx,size_t nbjets)const{
	if(datasets_.size()<=datasetidx){
		throw std::out_of_range("ttbarXsecFitter::addUncertainties: datasetidx too large");
	}
	datasets_.at(datasetidx).addUncertainties(stack,nbjets,false,std::vector<std::pair<TString, double> >());
}

void ttbarXsecFitter::addFullExtrapolError(const TString& sysname, const float & restmin, const float& restmax){
	if(debug)
		std::cout << "ttbarXsecFitter::addFullError" <<std::endl;

	if(datasets_.size()<1){
		throw std::logic_error("ttbarXsecFitter::addFullExtrapolError: first read in plots");
	}
	std::vector<TString> systs=datasets_.at(0).getSystNames();
	if(!sysname.EndsWith("*")){
		size_t idx=std::find(systs.begin(),systs.end(),sysname) - systs.begin();
		if(idx == systs.size()){
			std::string errstr=(std::string)"ttbarXsecFitter::addFullError: requested variation not found: "+sysname.Data();
			throw std::runtime_error (errstr);
		}
		addfullerrors_.push_back(extrapolationError(sysname, std::vector<size_t>(1, idx),
				std::vector<float>(1, restmin),std::vector<float>(1, restmax)));

	}
	else{
		size_t oldsize=addfullerrors_.size();
		TString choppedname=sysname;
		choppedname.ReplaceAll("*","");
		std::vector<float> maxes,mins;
		std::vector<size_t> idxs;
		for(size_t i=0;i<systs.size();i++){
			if(systs.at(i).BeginsWith(choppedname)){
				idxs.push_back(i);
				maxes.push_back(restmax);
				mins.push_back(restmin);
			}
		}
		if(idxs.size()>0)
			addfullerrors_.push_back(extrapolationError(choppedname, idxs, mins,maxes));
		if(oldsize==addfullerrors_.size())
			throw std::runtime_error ("ttbarXsecFitter::addFullError: requested variations not found");
	}

}

void  ttbarXsecFitter::printControlStack(bool fittedvalues,size_t bjetcat,size_t datasetidx,
		const std::string& prependToOutput)const{
        if (mttfit_) return;
	if(debug)
		std::cout << "ttbarXsecFitter::printControlStack" << std::endl;
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::printControlStack: dataset index.");
	if(bjetcat >= datasets_.at(0).nBjetCat()){
		throw std::out_of_range("ttbarXsecFitter::printControlStack: bjetcat out of range or no input read, yet.");
	}

	if(fittedvalues && !fitsucc_){
		std::cout << "ttbarXsecFitter::printControlStack: fit not successful, skipping post-fit plots" <<std::endl;
		return;
	}
        if (mlbCrossCheck_ && bjetcat!=1) return;
	plotterMultiStack plm;
	if (emuOnly_ || bjetcat ==0) {
            if (!mlbCrossCheck_) plm.readStyleFromFileInCMSSW("/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/plotterMultiStack_emu.txt");
            else plm.readStyleFromFileInCMSSW("/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/plotterMultiStack_mlb.txt");
        }
        else plm.readStyleFromFileInCMSSW("/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/plotterMultiStack_standard.txt");
	if(!fittedvalues)
		plm.addStyleForAllPlots(getenv("CMSSW_BASE")
				+(std::string)"/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/controlPlots_mergeleg.txt",
				"[merge for pre-fit plots]",
				"[end - merge for pre-fit plots]");

        if (emuOnly_ || bjetcat==0){
            if (!mlbCrossCheck_) plm.readTextBoxesInCMSSW("/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/plotterMultiStack_emu.txt", 
                                                          toString(bjetcat)+"btag"+toString( datasets_.at(datasetidx).getName()));
            else plm.readTextBoxesInCMSSW("/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/plotterMultiStack_mlb.txt", 
                                                          toString(bjetcat)+"btag"+toString( datasets_.at(datasetidx).getName()));
        }

	else plm.readTextBoxesInCMSSW("/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/plotterMultiStack_standard.txt",
                                      toString(bjetcat)+"btag"+toString( datasets_.at(datasetidx).getName()));

	std::vector<histoStack> stacks=datasets_.at(datasetidx).getOriginalInputStacks(bjetcat);

	corrMatrix corrmatr,corrm1;

	if(fittedvalues || pseudodatarun_){
		for(size_t i=0;i<stacks.size();i++){
			if(fittedvalues)
				stacks.at(i)=applyParametersToStack(stacks.at(i),bjetcat,datasetidx,fittedvalues,corrmatr);
			else
				stacks.at(i)=applyParametersToStack(stacks.at(i),bjetcat,datasetidx,false,corrmatr);
		}
	}
	else{
		//add background uncertainties here
		for(size_t i=0;i<stacks.size();i++){
			datasets_.at(datasetidx).addUncertainties(&stacks.at(i),bjetcat,false,std::vector<std::pair<TString, double> > ());
		}
	}
	if(fittedvalues)
		plm.associateCorrelationMatrix(corrmatr);

	float chi2=0;
	for(size_t i=0;i<stacks.size();i++){
		plm.addStack(&stacks.at(i));
		histoStack cp=stacks.at(i);
		cp.removeAllSystematics();
		chi2+=cp.chi2();
	}


	TString add="";
	if(fittedvalues)
		add="_fitted";
	if(prependToOutput.length()>0)
		add=(prependToOutput+"_fitdistr_").data()+toTString(bjetcat) +"bjets_"+  datasets_.at(datasetidx).getName()+add;
	else
		add=("fitdistr_")+toTString(bjetcat) +"bjets_"+  datasets_.at(datasetidx).getName()+add;

	formatter fmt;

	//plm.addTextBox(0.144,0.24,"#chi^{2}="+toTString(fmt.round(chi2,0.1)));
	// not include for now tb.drawToPad(&cv);


	plm.printToPdf(add.Data());
	plm.printToPng(add.Data());

	//TCanvas cv;
	//plm.usePad(&cv);
	//plm.draw();
	//cv.Print(add+".png"); //add pdf afterwards otherwise it will be deleted



}

void ttbarXsecFitter::printVariations(size_t bjetcat,size_t datasetidx,const std::string& prependToOutput, bool postfitconstraints)const{
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::printControlStack: dataset index.");
	if(bjetcat >= datasets_.at(0).nBjetCat()){
		throw std::out_of_range("ttbarXsecFitter::printControlStack: bjetcat out of range or no input read, yet.");
	}
	if(debug)
		std::cout << "ttbarXsecFitter::printVariations " << bjetcat << " b-jets, " << datasets_.at(datasetidx).getName() <<std::endl;

	std::vector<histoStack> stacks=datasets_.at(datasetidx).getOriginalInputStacks(bjetcat);
	for(size_t i=0;i<stacks.size();i++){
		datasets_.at(datasetidx).addUncertainties(&stacks.at(i),bjetcat,false,std::vector<std::pair<TString, double> > ());
	}
	TString add="";
	if(prependToOutput.length()>0 && prependToOutput.find_last_of('/') != prependToOutput.size()-1)
		add=(prependToOutput+"_var_").data()+toTString(bjetcat) +"bjets_"+  datasets_.at(datasetidx).getName()+add;
	else if(prependToOutput.find_last_of('/') == prependToOutput.size()-1)
		add=(prependToOutput+"var_").data()+toTString(bjetcat) +"bjets_"+  datasets_.at(datasetidx).getName()+add;
	else
		add=("var_")+toTString(bjetcat) +"bjets_"+  datasets_.at(datasetidx).getName()+add;

        add.ReplaceAll("_vars/var_","_vars_var_");

	/*
    if(! postfitconstraints)
	  for(size_t i=0;i<stacks.size();i++){
		stacks.at(i).mergeVariationsFromFileInCMSSW(mergesystfile_);
		stacks.at(i).mergeVariationsFromFileInCMSSW(mergesystfile_,"merge for plots");
	}*/
	std::vector<TString> sys=stacks.at(0).getSystNameList();
	std::vector<variateHisto1D> fittedwithsys;
	std::cout << "producing variation plots for "<< bjetcat<< " bjets, "<< datasets_.at(datasetidx).getName() ;
	if(postfitconstraints)
		std::cout << " (postfit)";
	std::cout << std::endl;

	if(postfitconstraints){
		for(size_t i=0;i<stacks.size();i++){
			displayStatusBar(i,stacks.size()+sys.size());
			corrMatrix dummy;
			variateHisto1D tmpvh;
			histoStack tmpcp=applyParametersToStack(datasets_.at(datasetidx).getOriginalInputStacks(bjetcat).at(i),
					bjetcat,datasetidx,true,dummy,&tmpvh);
			if(!i)
				sys=tmpcp.getSystNameList();
			tmpvh.setName("fitted");
			fittedwithsys.push_back(tmpvh);
		}
	}
	else{
		displayStatusBar(0,stacks.size()+sys.size());
	}
	for(size_t j=0;j<sys.size();j++){
		displayStatusBar(stacks.size()+j,stacks.size()+sys.size());
		const TString& sysname=sys.at(j);

		//if(sysname != "TOPMASS")continue; //DEBUGHERE

		std::vector<histo1D> nominals;
		std::vector<histo1D> ups;
		std::vector<histo1D> downs;
		for(size_t i=0;i<stacks.size();i++){
			std::vector<histo1D> tmp;
			if(postfitconstraints){


				histo1D nomtmp=fittedwithsys.at(i).exportContainer(fittedparas_);
				nomtmp.removeAllSystematics();
				nomtmp.setAllErrorsZero(true);
				size_t paraindex=std::find(parameternames_.begin(),parameternames_.end(),sysname)-parameternames_.begin();
				if(paraindex<parameternames_.size()){

					std::vector<double> fitparacp=fittedparas_;
					fitparacp.at(paraindex)+=fitter_.getParameterErrUp()->at(paraindex);
					nomtmp.addErrorContainer(sysname+"_up",fittedwithsys.at(i).exportContainer(fitparacp));
					fitparacp.at(paraindex)-=fitter_.getParameterErrUp()->at(paraindex);
					fitparacp.at(paraindex)+=fitter_.getParameterErrDown()->at(paraindex);
					nomtmp.addErrorContainer(sysname+"_down",fittedwithsys.at(i).exportContainer(fitparacp));
					nomtmp.setName("fitted");
					nominals.push_back(nomtmp);
					fitparacp.at(paraindex)=1;
					nomtmp=fittedwithsys.at(i).exportContainer(fitparacp);
					nomtmp.setName(sysname +" up");
					nomtmp.removeAllSystematics();
					ups.push_back(nomtmp);
					fitparacp.at(paraindex)=-1;
					nomtmp=fittedwithsys.at(i).exportContainer(fitparacp);
					nomtmp.removeAllSystematics();
					nomtmp.setName(sysname +" down");
					downs.push_back(nomtmp);
				}
				else{
					std::cout << "did not find " << sysname << std::endl;
				}

			}
			else{
				if(sysname != "TOPMASS")
					tmp=stacks.at(i).getFullMCContainer().produceVariations(sysname);
				else
					tmp=stacks.at(i).getFullMCContainer().produceVariations(sysname,
							"m_{t}^{MC}=172.5 GeV",
							"m_{t}^{MC}=175.5 GeV",
							"m_{t}^{MC}=169.5 GeV");
				nominals.push_back(tmp.at(0));
				ups.push_back(tmp.at(1));
				downs.push_back(tmp.at(2));
			}
		}
		plotterMultiCompare pl;
                if (emuOnly_ || bjetcat==0){
                    pl.readStyleFromFileInCMSSW("/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/plotterMultiCompare_emu.txt");
                    pl.readTextBoxesInCMSSW("/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/plotterMultiStack_emu.txt",
                                            toString(bjetcat)+"btag"+toString( datasets_.at(datasetidx).getName()));
                }
                else{
                    pl.readStyleFromFileInCMSSW("/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/plotterMultiCompare_standard.txt");
                    pl.readTextBoxesInCMSSW("/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/plotterMultiStack_standard.txt",
                                            toString(bjetcat)+"btag"+toString( datasets_.at(datasetidx).getName()));
                }

		pl.setNominalPlots(&nominals);
		pl.setComparePlots(&ups,0);
		pl.setComparePlots(&downs,1);

		TString outsysname=textFormatter::makeCompatibleFileName(sysname.Data());
		if(postfitconstraints)
			outsysname+="_postfit";
		pl.printToPdf(add.Data() + (std::string)"_"+ outsysname.Data());
		pl.saveToTFile(add+"_"+ outsysname+".root");
	}
	std::cout << std::endl;

}


int ttbarXsecFitter::fit(std::vector<float>& xsecs, std::vector<float>& errup ,std::vector<float>& errdown){

	bool onlychecks=false;

	if(debug)
		std::cout << "ttbarXsecFitter::fit" <<std::endl;
	if(datasets_.size()<1){
		throw std::logic_error("ttbarXsecFitter::fit: First read in datasets");
	}
	if(!datasets_.at(0).readyForFit())
		createContinuousDependencies();
	checkSizes();
	if(debug){
		std::cout << "fitting parameters: "<<std::endl;
		for(size_t i=0;i<parameternames_.size();i++)
			std::cout << parameternames_.at(i) << std::endl;
		std::cout << "\nprior correlations:" << std::endl;
		for(size_t i=0;i<priorcorrcoeff_.size();i++)
			std::cout << priorcorrcoeff_.at(i).first << " " << priorcorrcoeff_.at(i).second << std::endl;
	}
	//no except: sizes ok
	//	fittedparas_.at(xsecidx8_)=-8.; //other starting value closer to measured value TEST

	fitter_.setParameterNames(parameternames_);
	fitter_.setRequireFitFunction(false);
	std::vector<double> stepwidths;
	stepwidths.resize(fittedparas_.size(),0.1);

	for(size_t i=0;i<datasets_.size();i++){
            if (!mttfit_) stepwidths.at(datasets_.at(i).xsecIdx())=0.5;
            else{
                for (size_t s=0; s<datasets_.at(i).xsecIdxs().size(); s++)
                    stepwidths.at(datasets_.at(i).xsecIdxs().at(s))=0.5;
            }
	}

	fitter_.setParameters(fittedparas_,stepwidths);
	//set fixed paras
	for(size_t i=0;i<priors_.size();i++){
		if(priors_.at(i) == prior_parameterfixed)
			fitter_.setParameterFixed(i,true);
                else if(priors_.at(i) == prior_parameterfixed_up)
                        fitter_.setParameterFixed(i,true,1);
                else if(priors_.at(i) == prior_parameterfixed_down)
                        fitter_.setParameterFixed(i,true,-1);
		else
			fitter_.setParameterFixed(i,false);
		if(priors_.at(i) == prior_box){
			fitter_.setParameterLowerLimit(i,-1);
			fitter_.setParameterUpperLimit(i,1);
		}
                else if(priors_.at(i) == prior_narrowboxfsr){
                        fitter_.setParameterLowerLimit(i,-1./1.414);
                        fitter_.setParameterUpperLimit(i,1./1.414);
                }
		else if(priors_.at(i) == prior_narrowboxleft){
			fitter_.setParameterLowerLimit(i,-1);
			fitter_.setParameterUpperLimit(i,0);
		}
		else if(priors_.at(i) == prior_narrowboxright){
			fitter_.setParameterLowerLimit(i,0);
			fitter_.setParameterUpperLimit(i,1);
		}
	}

	//set lower limit for xsecs (=> 0)
	for(size_t i=0;i<datasets_.size();i++){
            if (!mttfit_) fitter_.setParameterLowerLimit(datasets_.at(i).xsecIdx(),-datasets_.at(i).xsecOffset());
            else{
                for (size_t s=0; s<datasets_.at(i).xsecIdxs().size(); ++s)
                    fitter_.setParameterLowerLimit(datasets_.at(i).xsecIdxs().at(s),-datasets_.at(i).xsecOffset(s));
            }
	}

	fitter_.setMinimizer(simpleFitter::mm_minuit2);
	if(fittedparas_.size() != ndependencies_)
		throw std::logic_error("ttbarXsecFitter::fit: fittedparas.size() != ndep");
	functor_ = ROOT::Math::Functor(this,&ttbarXsecFitter::toBeMinimized,ndependencies_);//ROOT::Math::Functor f(this,&ttbarXsecFitter::toBeMinimized,ndependencies_);
	fitter_.setMinFunction(&functor_);

	//pre fit to get rough estimate
	fitter_.setStrategy(0);
	fitter_.setTolerance(1);

	if(!onlychecks)
		fitter_.fit();
	else
		std::cout << "Warning fitting in sloppy mode for checks!" <<std::endl;
	if(fitter_.wasSuccess())
		fitter_.feedErrorsToSteps();

	if(!silent_)
		std::cout << "First rough fit done" <<std::endl;
	if(!onlychecks){
		if (!mttfit_){
                    fitter_.setStrategy(2);
                    fitter_.setTolerance(0.1);
                }
                else {
                    fitter_.setStrategy(1);
                    fitter_.setTolerance(0.3);
                }
	}

	size_t masspos=std::find(parameternames_.begin(),parameternames_.end(),"TOPMASS")-parameternames_.begin();
	if(!nominos_){
                for(size_t i=0;i<datasets_.size();i++){
                    if (!mttfit_) fitter_.setAsMinosParameter(datasets_.at(i).xsecIdx());
                    else{
                        for (size_t s=0; s<datasets_.at(i).xsecIdxs().size(); ++s)
                            fitter_.setAsMinosParameter(datasets_.at(i).xsecIdxs().at(s));
                    }
                }
                if(masspos < parameternames_.size() && priors_.at(masspos)!=prior_parameterfixed ){
                    fitter_.setAsMinosParameter(masspos);
                }
		else{
			masspos=parameternames_.size();
		}
	}

	if(debug)
		simpleFitter::printlevel=simpleFitter::pl_verb;
	fitter_.fit();
	if(debug)
		simpleFitter::printlevel=simpleFitter::pl_silent;

	//feed back

	if(!silent_){
		for(size_t i=0;i<datasets_.size();i++){
                    if (!mttfit_){
			size_t xsecidx=datasets_.at(i).xsecIdx();
			double xsecoff=datasets_.at(i).xsecOffset();
			std::cout << "fitted xsecs: "<< datasets_.at(i).getName()<< ": "<<fitter_.getParameters()->at(xsecidx)+xsecoff <<
                            "+" << fitter_.getParameterErrUp()->at(xsecidx) *100/(fitter_.getParameters()->at(xsecidx)+xsecoff) <<
                            "-" << fitter_.getParameterErrDown()->at(xsecidx) *100/(fitter_.getParameters()->at(xsecidx)+xsecoff) <<
                            std::endl;
			for(size_t j=0;j<datasets_.size();j++){
                            if (i==j) continue;
                            std::cout << "correlation between " << datasets_.at(i).getName() << " and " <<
                                datasets_.at(j).getName() << " : " <<
                                fitter_.getCorrelationCoefficient(xsecidx,datasets_.at(j).xsecIdx()) <<std::endl;
			}
                    }
                    else{
                        std::vector<size_t> xsecidxs=datasets_.at(i).xsecIdxs();
                        for (size_t s=0; s<datasets_.at(i).xsecIdxs().size(); ++s){
                            double xsecoff=datasets_.at(i).xsecOffset(s);
                            std::cout << "fitted xsecs: "<< datasets_.at(i).getName()<<" mttbin n."<<toString(s+1)<< ": "<<fitter_.getParameters()->at(xsecidxs.at(s))+xsecoff <<
                                "+" << fitter_.getParameterErrUp()->at(xsecidxs.at(s)) *100/(fitter_.getParameters()->at(xsecidxs.at(s))+xsecoff) <<
                                "-" << fitter_.getParameterErrDown()->at(xsecidxs.at(s)) *100/(fitter_.getParameters()->at(xsecidxs.at(s))+xsecoff) <<
                                std::endl;
                        
                        }
                    }
		}
		if(masspos < parameternames_.size()){
			std::cout << "fitted mass: "<<fitter_.getParameters()->at(masspos) <<
					"+" << fitter_.getParameterErrUp()->at(masspos) <<
					"-" << fitter_.getParameterErrDown()->at(masspos)<<
					std::endl;
			for(size_t i=0;i<datasets_.size();i++){
                            if (mttfit_) break;
				std::cout << "Corr coeff to xsec " << datasets_.at(i).getName() << " " <<
						fitter_.getCorrelationCoefficient(datasets_.at(i).xsecIdx(),masspos) << std::endl;
			}
		}
	}
	else if(fitter_.wasSuccess() && masspos < parameternames_.size()){
		tempdata.push_back(fitter_.getParameters()->at(masspos));
		tempdata.push_back(fitter_.getParameterErrUp()->at(masspos));
		tempdata.push_back(fitter_.getParameterErrDown()->at(masspos));

	}
	if(fitter_.wasSuccess()){
		fitsucc_=true;
		if(parameterwriteback_){
			fittedparas_ = *fitter_.getParameters();
		}
                if (!mttfit_){
                    xsecs.clear();
                    errup.clear();
                    errdown.clear();
                    for(size_t i=0;i<datasets_.size();i++){
			size_t xsecidx=datasets_.at(i).xsecIdx();
			double xsecoff=datasets_.at(i).xsecOffset();
			xsecs.push_back(fitter_.getParameters()->at(xsecidx)+xsecoff);
			errup.push_back(fitter_.getParameterErrUp()->at(xsecidx));
			errdown.push_back(fitter_.getParameterErrDown()->at(xsecidx));
                    }
                }
		if(!nominos_ && !fitter_.minosWasSuccess()){
			throw std::runtime_error("ttbarXsecFitter::fit(): Minos error failed!");

		}
		if(!silent_){
			for(size_t i=0;i<datasets_.size();i++){
                            if (!mttfit_){
				std::cout << "eps_b: " << getEps(false,i).getBin(1).getContent() << " postfit: "
                                          << getEps(true,i).getBin(1).getContent() << " " << datasets_.at(i).getName()<<std::endl;
				std::cout << "C_b:   " << getCb(false,i).getBin(1).getContent() << " postfit: "
                                          << getCb(true,i).getBin(1).getContent() << " " << datasets_.at(i).getName()<<std::endl;
				std::cout << "eps_emu:   " << getEps_emu(false,i)<< " postfit: "
                                          << getEps_emu(true,i)<< " " << datasets_.at(i).getName()<<std::endl;//acceptance()
				std::cout << "acceptance(extr):   " << datasets_.at(i).acceptance_extr().getNominal()<< " postfit: "
                                          << datasets_.at(i).acceptance_extr().getValue(fittedparas_)<< " " << datasets_.at(i).getName()<<std::endl;
                            }
                            else{
                                for (size_t s=0; s<datasets_.at(i).xsecIdxs().size(); ++s){
                                    std::cout << "eps_b: " << getEps(false,i,s).getBin(1).getContent() << " postfit: "
                                              << getEps(true,i,s).getBin(1).getContent() << " " << datasets_.at(i).getName() << "  n."<<toString(s)<<std::endl;
                                    std::cout << "C_b:   " << getCb(false,i,s).getBin(1).getContent() << " postfit: "
                                              << getCb(true,i,s).getBin(1).getContent() << " " << datasets_.at(i).getName() << "  n."<<toString(s)<<std::endl;
                                    std::cout << "eps_emu:   " << getEps_emu(false,i,s)<< " postfit: "
                                              << getEps_emu(true,i,s)<< " " << datasets_.at(i).getName() << "  n."<<toString(s)<<std::endl;
                                    std::cout << "acceptance(extr):   " << datasets_.at(i).acceptance_extr(s).getNominal()<< " postfit: "
                                              << datasets_.at(i).acceptance_extr(s).getValue(fittedparas_)<< " " << datasets_.at(i).getName() << "  n."<<toString(s)<<std::endl;                               
                                }
                            }

			}
		}

		return 0;
	}
	//else
	if(!silent_)
		std::cout << "ttbarXsecFitter::fit(): Fit failed" <<std::endl;
	throw std::runtime_error("ttbarXsecFitter::fit(): Fit failed");
	return -1;
}
int ttbarXsecFitter::fit(){
	std::vector<float> dummy;
	return fit(dummy,dummy,dummy);
}

    histo1D ttbarXsecFitter::getCb (bool fittedvalues,size_t datasetidx, size_t mttbin)const{
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::getCb: dataset index out of range");

        if (mttfit_ && mttbin==9999)
            throw std::logic_error("ttbarXsecFitter::getCb: choose a proper mtt bin");

	//if(eighttev) return container_c_b_.at(0); else return container_c_b_.at(1);
	variateHisto1D temp;
        if (!mttfit_) temp=datasets_.at(datasetidx).container_c_b();
        else temp=datasets_.at(datasetidx).container_c_b(mttbin);

	if(fittedvalues)
		return temp.exportContainer(fittedparas_);
	else
		return temp.exportContainer();

}
    histo1D ttbarXsecFitter::getEps(bool fittedvalues, size_t datasetidx, size_t mttbin)const{
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::getEps: dataset index out of range");

        if (mttfit_ && mttbin==9999)
            throw std::logic_error("ttbarXsecFitter::getEps: choose a proper mtt bin");

	//{if(eighttev) return container_eps_b_.at(0); else return container_eps_b_.at(1);}
	variateHisto1D temp;
        if (!mttfit_) temp=datasets_.at(datasetidx).container_eps_b();
        else temp=datasets_.at(datasetidx).container_eps_b(mttbin);

	if(fittedvalues)
		return temp.exportContainer(fittedparas_);
	else
		return temp.exportContainer();

}
    float ttbarXsecFitter::getEps_emu(bool fittedvalues,size_t datasetidx, size_t mttbin)const{
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::getEps: dataset index out of range");

        if (mttfit_ && mttbin==9999)
            throw std::logic_error("ttbarXsecFitter::getEps_emu: choose a proper mtt bin");

	//{if(eighttev) return container_eps_b_.at(0); else return container_eps_b_.at(1);}
	const extendedVariable* temp;
        if (!mttfit_) temp=&datasets_.at(datasetidx).eps_emu();
        else temp=&datasets_.at(datasetidx).eps_emu(mttbin);

	if(fittedvalues)
		return temp->getValue(fittedparas_);
	else
		return temp->getNominal();

}

histo2D  ttbarXsecFitter::getCorrelations()const{
	histo2D out;
	if(fitsucc_)
		fitter_.fillCorrelationCoefficients(&out);
	return out;
}


double ttbarXsecFitter::getXsec(size_t datasetidx)const{
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::getXsec: dataset index out of range");
	if(!fitsucc_)
		throw std::out_of_range("ttbarXsecFitter::getXsec: first perform successful fit");
	double fitted= fitter_.getParameters()->at(datasets_.at(datasetidx).xsecIdx())+datasets_.at(datasetidx).xsecOffset();
	double acceptancecorr=datasets_.at(datasetidx).acceptance().getValue(fittedparas_)/
			datasets_.at(datasetidx).acceptance_extr().getValue(fittedparas_);
	return fitted*acceptancecorr;

}
double ttbarXsecFitter::getVisXsec(size_t datasetidx)const{
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::getXsec: dataset index out of range");
	if(!fitsucc_)
		throw std::out_of_range("ttbarXsecFitter::getXsec: first perform successful fit");
	double fitted= fitter_.getParameters()->at(datasets_.at(datasetidx).xsecIdx())+datasets_.at(datasetidx).xsecOffset();
	double acceptance=datasets_.at(datasetidx).acceptance().getValue(fittedparas_);
	return fitted*acceptance;
}



double ttbarXsecFitter::getXsec(size_t datasetidx, size_t mttbin)const{
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::getXsec: dataset index out of range");
	if(mttbin>=datasets_.at(datasetidx).xsecIdxs().size())
		throw std::out_of_range("ttbarXsecFitter::getXsec: mttbin index out of range");
	if(!fitsucc_)
		throw std::out_of_range("ttbarXsecFitter::getXsec: first perform successful fit");
	double fitted= fitter_.getParameters()->at(datasets_.at(datasetidx).xsecIdxs().at(mttbin))+datasets_.at(datasetidx).xsecOffset(mttbin);
	double acceptancecorr=datasets_.at(datasetidx).acceptance(mttbin).getValue(fittedparas_)/
			datasets_.at(datasetidx).acceptance_extr(mttbin).getValue(fittedparas_);
	return fitted*acceptancecorr;

}
double ttbarXsecFitter::getVisXsec(size_t datasetidx, size_t mttbin)const{
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::getXsec: dataset index out of range");
	if(!fitsucc_)
		throw std::out_of_range("ttbarXsecFitter::getXsec: first perform successful fit");
	double fitted= fitter_.getParameters()->at(datasets_.at(datasetidx).xsecIdxs().at(mttbin))+datasets_.at(datasetidx).xsecOffset(mttbin);
	double acceptance=datasets_.at(datasetidx).acceptance(mttbin).getValue(fittedparas_);
	return fitted*acceptance;
}

void ttbarXsecFitter::cutAndCountSelfCheck(size_t datasetidx)const{
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::cutAndCountSelfCheck: dataset index out of range");
	datasets_.at(datasetidx).cutAndCountSelfCheck(std::vector<std::pair<TString, double> >());
}
void ttbarXsecFitter::dataset::cutAndCountSelfCheck(const std::vector<std::pair<TString, double> >& priorcorrcoeff)const{
	//very specific function TROUBLE (just a marker if something makes problems)
	if(debug)
		std::cout << "ttbarXsecFitter::dataset::cutAndCountSelfCheck" << std::endl;
	//get the input for >1 jet, >0 bjets
	if(inputstacks_.size()<2)
		throw std::logic_error("ttbarXsecFitter::dataset::cutAndCountSelfCheck(): first read in stacks (bjets)");
	if(inputstacks_.at(0).size()<4)
		throw std::logic_error("ttbarXsecFitter::dataset::cutAndCountSelfCheck(): first read in stacks (add jets)");

	std::vector< std::vector<histoStack> > incp=inputstacks_;
	for(size_t i=0;i<incp.size();i++)
		for(size_t j=0;j<incp.at(i).size();j++){
			addUncertainties(&(incp.at(i).at(j)),i,false,priorcorrcoeff);
			incp.at(i).at(j).mergeVariationsFromFileInCMSSW(parent_-> mergesystfile_,"merge for cutandcount");
		}

	histo1D signh,bghist,datah,vispsh;
	//get nominal value
	float data=0,signal=0,generated=0,background=0,
			alldils=0,alldild=0,alldilbg=0,
			alltwojetss=0,alltwojetsd=0,alltwojetsbg=0;
	float visps=0;
	generated=xsecoff_*lumi_;
	float nplots=0;
	for(size_t nbjet=0;nbjet<nBjetCat();nbjet++){
		size_t minjet=0;
		if(nbjet==1) minjet=1;
		for(size_t njet=0;njet<4;njet++){
			vispsh+=incp.at(nbjet).at(njet).getSignalContainer1DUnfold().getGenContainer().getIntegralBin(true);
			visps+=incp.at(nbjet).at(njet).getSignalContainer1DUnfold().getGenContainer().integral(true);
			nplots++;
			if(nbjet<1) continue;
			if(njet<minjet) continue;
			signal+=incp.at(nbjet).at(njet).getSignalContainer().integral(true);
			background+=incp.at(nbjet).at(njet).getBackgroundContainer().integral(true);
			data+=incp.at(nbjet).at(njet).getDataContainer().integral(true);

			signh += incp.at(nbjet).at(njet).getSignalContainer().getIntegralBin(true);
			bghist+=incp.at(nbjet).at(njet).getBackgroundContainer().getIntegralBin(true);
			datah+=incp.at(nbjet).at(njet).getDataContainer().getIntegralBin(true);
		}
	}
	for(size_t nbjet=0;nbjet<nBjetCat();nbjet++){
		size_t minjet=0;
		for(size_t njet=minjet;njet<4;njet++){
			alldils+=incp.at(nbjet).at(njet).getSignalContainer().integral(true);
			alldilbg+=incp.at(nbjet).at(njet).getBackgroundContainer().integral(true);
			alldild+=incp.at(nbjet).at(njet).getDataContainer().integral(true);
		}
	}
	for(size_t nbjet=0;nbjet<nBjetCat();nbjet++){
		size_t minjet=0;
		if(!nbjet) minjet=2;
		if(nbjet==1) minjet=1;
		if(nbjet==2) minjet=0;
		for(size_t njet=minjet;njet<4;njet++){
			alltwojetss+=incp.at(nbjet).at(njet).getSignalContainer().integral(true);
			alltwojetsbg+=incp.at(nbjet).at(njet).getBackgroundContainer().integral(true);
			alltwojetsd+=incp.at(nbjet).at(njet).getDataContainer().integral(true);
		}
	}


	visps*= (3 /(float)totalvisgencontsread_);
	float xsecnom = (data-background)/(signal/generated * lumi_);
	//float xsecvisnom= (data-background)/(signal/visps);
	std::cout << "nominal cross section C&C "<<getName()<<" " << xsecnom <<std::endl;
	float xsecvis=(data-background)/(signal/visps * lumi_);
	std::cout << "vis cross section C&C "<<getName()<<" " << xsecvis <<std::endl;


	vispsh*= (3/(float)totalvisgencontsread_);
	//ignore MC stat
	bghist.removeStatFromAll(true);
	signh.removeStatFromAll(true);
	vispsh.removeStatFromAll(true);
	datah.removeStatFromAll(false);
	histo1D xsec = (datah - bghist)/(signh * (lumi_/generated ));
	//xsec.removeStatFromAll();
	//std::cout << "xsec " << getName() <<std::endl;
	//xsec.coutBinContent(1,"pb");

	//std::cout << xsec.makeTableFromBinContent(1,true,true,0.1).getTable() << std::endl;

	vispsh.setErrorZeroContaining("Lumi");
	histo1D xsecvish = (datah - bghist)/((signh /vispsh)  *lumi_);
	//	xsecvish.removeStatFromAll(false);
	std::cout << "\n\nxsec vis " << getName() <<std::endl;
	//xsecvish.coutBinContent(1,"pb");
	std::cout << xsecvish.makeTableFromBinContent(1,true,true,0.1).getTable() << "\n"<< std::endl;
	std::cout << xsecvish.makeTableFromBinContent(1,false,true,0.01).getTable() << "\n"<<std::endl;
	std::cout << xsecvish.makeTableFromBinContent(1,false,false,0.01).getTable() << "\n"<<std::endl;

	std::cout << "extrapolate yourself, please" <<std::endl;

	std::cout
	<< "\n"       << "Yields data:    "
	<< data      <<"\n       bg:      "
	<< background<<"\n       sig:     "
	<< signal
	<<"\n all dil data:  "<< alldild
	<<"\n all dil sig:  "<< alldils
	<<"\n all dil bg:    "<< alldilbg
	<<"\n two jets data:  "<< alltwojetsd
	<<"\n two jets sig:  "<< alltwojetss
	<<"\n two jets bg:    "<< alltwojetsbg
	<<"\n" <<std::endl;

	//old impl
	/*
	 * float totalup2=0;
	float totaldown2=0;
	 *
	for(size_t i=0;i<sysnames.size();i++){
		TString nameup=*sysnames.at(i)+"_up";
		TString namedown=*sysnames.at(i)+"_down";
		int idxup=incp.at(0).at(0).getDataContainer().getSystErrorIndex(nameup);
		int idxdown=incp.at(0).at(0).getDataContainer().getSystErrorIndex(namedown);

		float signalup=0, signaldown=0, backgroundup=0, backgrounddown=0;
		float vispsup=0,vispsdown=0;
		for(size_t nbjet=1;nbjet<nBjetCat();nbjet++){
			size_t minjet=0;
			if(nbjet==1) minjet=1;
			for(size_t njet=minjet;njet<4;njet++){

				signalup+=incp.at(nbjet).at(njet).getSignalContainer().integral(false,idxup);
				signaldown+=incp.at(nbjet).at(njet).getSignalContainer().integral(false,idxdown);
				backgroundup+=incp.at(nbjet).at(njet).getBackgroundContainer().integral(false,idxup);
				backgrounddown+=incp.at(nbjet).at(njet).getBackgroundContainer().integral(false,idxdown);
			}
		}
		float xsecup = (data-backgroundup)/(signalup/generated * lumi_);
		float xsecdown = (data-backgrounddown)/(signaldown/generated * lumi_);
		float reluncup=(xsecup/xsecnom -1) * 100 ;
		float reluncdown= (xsecdown/xsecnom -1) * 100 ;
		//loop
		std::cout << nameup << " \t" << reluncup<<"%\n"
				<<  namedown << " \t" << reluncdown<<"%" <<std::endl;
		bool dummy;
		float up=getMaxVar(true,reluncup,reluncdown,dummy);
		float down=getMaxVar(false,reluncup,reluncdown,dummy);
		totalup2+=up*up;
		totaldown2+=down*down;
	}
	std::cout << "Xsec (C&C): "
			<< xsecnom << " +" << sqrt(totalup2)
			<< " -" << sqrt(totaldown2)
			<< "\n"       << "Yields data:    "
			<< data      <<"\n       bg:      "
			<< background<<"\n       sig:     "
			<< signal
			<<"\n all dil data:  "<< alldild
			<<"\n all dil sig:  "<< alldils
			<<"\n all dil bg:    "<< alldilbg
			<<"\n two jets data:  "<< alltwojetsd
			<<"\n two jets sig:  "<< alltwojetss
			<<"\n two jets bg:    "<< alltwojetsbg
			<<"\n" <<std::endl;
	 */
}

double ttbarXsecFitter::getXsecOffset(size_t datasetidx, size_t mttbin)const{
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::getXsecOffset: dataset index out of range");
	return datasets_.at(datasetidx).xsecOffset(mttbin);
}


double ttbarXsecFitter::getXsecRatio(size_t datasetidx1,size_t datasetidx2, double & errup, double& errdown)const{
	if(datasetidx1>=datasets_.size() || datasetidx2>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::getXsecRatio: dataset index out of range");
	if(!fitsucc_){
		throw std::out_of_range("ttbarXsecFitter::getXsecRatio: first perform successful fit");
	}
	const std::vector<dataset::systematic_unc> * unc1=&datasets_.at(datasetidx1).postFitSystematicsFullSimple();
	const std::vector<dataset::systematic_unc> * unc2=&datasets_.at(datasetidx2).postFitSystematicsFullSimple();

	if(unc1->size()<1|| unc2->size()<1)
		throw std::out_of_range("ttbarXsecFitter::getXsecRatio: first evaluate individual systematics contributions");

	double xsec1=getXsec(datasetidx1) ;
	double xsec2=getXsec(datasetidx2);

	//correlation coefficients without the extrapolation uncertainty:


	double ratio=getXsec(datasetidx1) / getXsec(datasetidx2);
	size_t xsecidx1=datasets_.at(datasetidx1).xsecIdx();
	size_t xsecidx2=datasets_.at(datasetidx2).xsecIdx();
	/*
	 * assume same unc for vis and full in fitted PS
	 */
	double errup1 = fitter_.getParameterErrUp()->at(xsecidx1);
	double errdown1 = fitter_.getParameterErrDown()->at(xsecidx1);
	double errup2 = fitter_.getParameterErrUp()->at(xsecidx2);
	double errdown2 = fitter_.getParameterErrDown()->at(xsecidx2);
	double corrcoeff= fitter_.getCorrelationCoefficient(xsecidx1,xsecidx2);
	//assumes positive correlation! (choice of up/down)
	double err2up =  square(errup1 / xsec1) + square(errup2 / xsec2) - 2* (corrcoeff * errup1*errup2)/(xsec1*xsec2);
	double err2down = square(errdown1 / xsec1) + square(errdown2 / xsec2) - 2* (corrcoeff * errdown1*errdown2)/(xsec1*xsec2);
	err2up=err2up * ratio*ratio;
	err2down=err2down * ratio*ratio;

	//add extrapolation uncertainties (inter-unc-correlation 1)
	//debug check
	if(unc1->size() != unc2->size())
		throw std::logic_error("ttbarXsecFitter::getXsecRatio: uncertity breakdown not same size");

	for(size_t i=0;i<unc1->size();i++){
		//debug check
		//get correlation to xsec
		if(! unc1->at(i).name.Contains("(extr)")) continue;
		if(unc1->at(i).name != unc2->at(i).name)
			throw std::logic_error("ttbarXsecFitter::getXsecRatio: uncertianty ordering broken");


		//fully correlated
		double applyup=(xsec1 + xsec1*unc1->at(i).errup) / (xsec2 + xsec2*unc2->at(i).errup);
		double applydown=(xsec1 + xsec1*unc1->at(i).errdown) / (xsec2 + xsec2*unc2->at(i).errdown);

		if(applyup - ratio >0){
			if(applydown - ratio < 0){
				err2up+= square(applyup - ratio);
				err2down+=square(ratio-applydown);
			}
			else if(applyup>applydown)
				err2up+= square(applyup - ratio);
			else
				err2up+= square(applydown - ratio);
		}
		else{
			if(applydown - ratio > 0){
				err2up+= square(applydown - ratio);
				err2down+=square(ratio-applyup);
			}
			else if(applyup<applydown)
				err2down+= square(ratio-applyup);
			else
				err2down+= square(ratio-applydown);
		}
	}

	errup=sqrt(err2up);
	errdown=sqrt(err2down);
	return ratio;
}

double ttbarXsecFitter::getParaError(size_t idx)const{
	if(idx>=fitter_.getParameters()->size())
		throw std::out_of_range("ttbarXsecFitter::getParaError");
	return fitter_.getParameterErr(idx);
}
TString ttbarXsecFitter::getParaName(size_t idx)const{
	if(idx>=fitter_.getParameterNames()->size())
		throw std::out_of_range("ttbarXsecFitter::getParaName");
	return fitter_.getParameterNames()->at(idx);
}
void ttbarXsecFitter::getParaErrorContributionToXsec(int idxin, size_t datasetidx,double&up,double&down,bool& anticorr){
	if(idxin>=(int)fitter_.getParameters()->size())
		throw std::out_of_range("ttbarXsecFitter::getParaErrorContributionToXsec: index out of range");
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::getParaErrorContributionToXsec: dataset index out of range");

	size_t xsecidx=datasets_.at(datasetidx).xsecIdx() ;
	double xsec=getXsec(datasetidx);
	if(!nosystbd_){
		if(idxin>=0){

			size_t idx=idxin;
			anticorr=fitter_.getCorrelationCoefficient(xsecidx,idx)<0;

			fitter_.getParameterErrorContribution(idx,xsecidx,up,down);
			up/=xsec;
			down/=xsec;
			return;
		}
		else{ //stat
			fitter_.getStatErrorContribution(xsecidx,up,down);
			up/=xsec;
			down/=xsec;
			return;
		}
	}
	else{
		up=1;
		down=1;
		return;
	}
}

histoStack ttbarXsecFitter::applyParametersToStack(const histoStack& stack, size_t bjetcat,
		size_t datasetidx, bool fitted, corrMatrix& corrmatr, variateHisto1D* fullmc)const{
	if(debug)
		std::cout << "ttbarXsecFitter::applyParametersToStack" << std::endl;
	if(datasetidx >= datasets_.size()){
		throw std::out_of_range("ttbarXsecFitter::applyParametersToStack: dataset idx out of range");
	}
	if(bjetcat >= dataset::nBjetCat()){
		throw std::out_of_range("ttbarXsecFitter::applyParametersToStack: b-jet cat>2, must be <2");
	}
	if(fitted && !fitsucc_){
		throw std::out_of_range("ttbarXsecFitter::applyParametersToStack: first perform fit successfully");
	}

	histoStack out=stack;
	std::vector<TString> sysnames=stack.getDataContainer().getSystNameList();
	if(std::find(sysnames.begin(),sysnames.end(),("BG_0_bjets_DY")) == sysnames.end())
		datasets_.at(datasetidx).addUncertainties(&out,bjetcat,removesyst_,priorcorrcoeff_);
	//otherwise uncertaintes are already added

	//map the indicies in a later step!!

	dataset cpdts = datasets_.at(datasetidx);
	cpdts.resetTotVisGenCount();
	cpdts.readStack(out,bjetcat);
	cpdts.adaptIndices(datasets_.at(datasetidx));

	cpdts.createContinuousDependencies();

	histo1D data,signal,background;
        std::vector<histo1D> signal_v;

	std::vector<double> fittedparas;
	std::vector<double> constr;
	std::vector<TString> names;//=*fitter_.getParameterNames();

	fittedparas.resize(fittedparas_.size(),0);
	names=*fitter_.getParameterNames();
	constr.resize(fittedparas_.size(),1);
	if(fitted){
		fittedparas=fittedparas_;
		constr=*fitter_.getParameterErrUp(); //anyway symm
	}

	//for(size_t i=0;i<datasets_.size();i++)
	//	constr.at(datasets_.at(i).xsecIdx())=0;




	if(debug)
		std::cout << "ttbarXsecFitter::applyParametersToStack: data: "<< datasets_.at(datasetidx).data(bjetcat).getNDependencies() << std::endl;
	data=cpdts.data(bjetcat).exportContainer( fittedparas,constr,names );

	if(debug)
		std::cout << "ttbarXsecFitter::applyParametersToStack: background" << std::endl;
	background=cpdts.background(bjetcat).exportContainer( fittedparas,constr,names );

	//shape
	variateHisto1D tmpvar;
        std::vector<variateHisto1D> tmpvar_v;
        size_t n_signal = datasets_.at(datasetidx).numberOfSignals();

        if (!mttfit_) tmpvar=cpdts.signalshape(bjetcat);
        else {
            tmpvar_v.resize(n_signal);
            for (size_t s=0; s<n_signal; ++s){
                tmpvar_v.at(s)=cpdts.signalshape(s,bjetcat);
            }
        }
        
	extendedVariable integral;
        std::vector<extendedVariable> integral_v;
        
        if (!mttfit_){
            integral=datasets_.at(datasetidx).signalshape(bjetcat).getIntegral();
            tmpvar/=integral;
        }
        else{
            integral_v.resize(n_signal);
            for (size_t s=0; s<n_signal; ++s){
                integral_v.at(s)=datasets_.at(datasetidx).signalshape(s,bjetcat).getIntegral();
                tmpvar_v.at(s) /= integral_v.at(s);
            }
        }

	if(debug)
		std::cout << "ttbarXsecFitter::applyParametersToStack: normalization" << std::endl;

	double multi = 1;
        std::vector<double> multi_v;

	if(fitted){
            if (!mttfit_) multi = cpdts.lumi() * getXsec(datasetidx);
            else{
                multi_v.resize(n_signal);
                for (size_t s=0; s<n_signal; ++s){
                    multi_v.at(s)=cpdts.lumi() * getXsec(datasetidx,s);
                }
            }
        }
	else{
            if (!mttfit_) multi = cpdts.lumi() * datasets_.at(datasetidx).xsecOffset();
            else{
                multi_v.resize(n_signal);
                for (size_t s=0; s<n_signal; ++s){
                    multi_v.at(s)=cpdts.lumi() * datasets_.at(datasetidx).xsecOffset(s);
                }
            }
        }
	//adapt for indicies consistency!

        if (!mttfit_){
            tmpvar *= multi ; //add xsec and lumi
            tmpvar *= datasets_.at(datasetidx).acceptance() ; //in case this wasnt a unfold histo
            tmpvar *= datasets_.at(datasetidx).eps_emu() ;//to get right reco
            if (mlbCrossCheck_) tmpvar *= integral;
            else tmpvar *= datasets_.at(datasetidx).omega_b(bjetcat); //add bjet norm
        }
        else{
            for (size_t s=0; s<n_signal; ++s){
                tmpvar_v.at(s) *= multi_v.at(s) ; //add xsec and lumi
                tmpvar_v.at(s) *= datasets_.at(datasetidx).acceptance(s) ; //in case this wasnt a unfold histo
                tmpvar_v.at(s) *= datasets_.at(datasetidx).eps_emu(s) ;//to get right reco
                tmpvar_v.at(s) *= datasets_.at(datasetidx).omega_b(s,bjetcat); //add bjet norm
            }
        }

	//get part of b-jet norm
	extendedVariable subpart;
        std::vector<extendedVariable> subpart_v;
        
	histo1D sigintstack, sigcontint;
        std::vector<histo1D> sigintstack_v, sigcontint_v;

        if (!mttfit_){
            sigintstack=cpdts.getSignalCont()->at(bjetcat).getIntegralBin(true);
            sigcontint=datasets_.at(datasetidx).getSignalCont()->at(bjetcat).getIntegralBin(true);
        }
        else{
            subpart_v.resize(n_signal);
            sigintstack_v.resize(n_signal);
            sigcontint_v.resize(n_signal);
            for (size_t s=0; s<n_signal; ++s){
                sigintstack_v.at(s)=cpdts.getSignalCont(s)->at(bjetcat).getIntegralBin(true);
                sigcontint_v.at(s)=datasets_.at(datasetidx).getSignalCont(s)->at(bjetcat).getIntegralBin(true);                
            }
        }

	histo1D subparth;
        std::vector<histo1D> subparth_v;

        if (!mttfit_){
            subparth=sigintstack/sigcontint;
            subparth.removeStatFromAll(true);
        }
        else{
            subparth_v.resize(n_signal);
            for (size_t s=0; s<n_signal; ++s){
                subparth_v.at(s)=sigintstack_v.at(s)/sigcontint_v.at(s);
                subparth_v.at(s).removeStatFromAll(true);
            }
        }

	variateHisto1D tmptmp;
        std::vector<variateHisto1D> tmptmp_v;

        if (!mttfit_){
            tmptmp.import(subparth);
            subpart=*tmptmp.getBin(1);
            tmpvar *= subpart;// stack.getSignalContainer().integral(true) /  datasets_.at(datasetidx).signalIntegral(bjetcat);
        }
        else{
            tmptmp_v.resize(n_signal);
            for (size_t s=0; s<n_signal; ++s){
                tmptmp_v.at(s).import(subparth_v.at(s));
                subpart_v.at(s)=*tmptmp_v.at(s).getBin(1);
                tmpvar_v.at(s) *= subpart_v.at(s);// stack.getSignalContainer().integral(true) /  datasets_.at(datasetidx).signalIntegral(bjetcat);
            }
        }

	if(debug){
            if (!mttfit_){
		float contr=subparth.getBinContent(1);
		std::cout << "ttbarXsecFitter::applyParametersToStack: <"<< stack.getName()<<"> contr in b-jet cat "<<bjetcat <<": "<<
				sigintstack.getBinContent(1) <<"/" << sigcontint.getBinContent(1) <<"="<<contr<<std::endl;
		std::cout << sigintstack.integral(true) <<std::endl;
		std::cout << cpdts.getSignalCont()->at(bjetcat).integral(true) <<std::endl;
		std::cout << stack.getSignalContainer().integral(true) <<std::endl;
		std::cout << stack.getSignalContainer().getIntegralBin(true).integral(true) <<std::endl;
            }
            else{
                for (size_t s=0; s<n_signal; ++s){
                    float contr=subparth_v.at(s).getBinContent(1);
                    std::cout << "ttbarXsecFitter::applyParametersToStack: <"<< stack.getName()<<"> contr in b-jet cat "<<bjetcat<< " (mttbin n."<<s<<"): "<<
                        sigintstack_v.at(s).getBinContent(1) <<"/" << sigcontint_v.at(s).getBinContent(1) <<"="<<contr<<std::endl;
                    std::cout << sigintstack_v.at(s).integral(true) <<std::endl;
                    std::cout << cpdts.getSignalCont(s)->at(bjetcat).integral(true) <<std::endl;
                    std::cout << stack.getSignalContainer(s).integral(true) <<std::endl;
                    std::cout << stack.getSignalContainer(s).getIntegralBin(true).integral(true) <<std::endl;               
                }
            }
	}

	if(fullmc){
            if (!mttfit_) *fullmc=cpdts.background(bjetcat) + tmpvar;
            else{
                *fullmc=cpdts.background(bjetcat);
                for (size_t s=0; s<n_signal; ++s){
                    *fullmc += tmpvar_v.at(s);
                }
            }
        }


        if (!mttfit_){
            signal=tmpvar.exportContainer(fittedparas,constr,names );
            signal.setYAxisName(stack.getSignalContainer().getYAxisName());
            signal.setXAxisName(stack.getSignalContainer().getXAxisName());
        }
        else{
            signal_v.resize(n_signal);
            for (size_t s=0; s<n_signal; ++s){
                signal_v.at(s)=tmpvar_v.at(s).exportContainer(fittedparas,constr,names );
                signal_v.at(s).setYAxisName(stack.getSignalContainer(s).getYAxisName());
                signal_v.at(s).setXAxisName(stack.getSignalContainer(s).getXAxisName());
            }            
        }
	data.setYAxisName(stack.getSignalContainer().getYAxisName());
	data.setXAxisName(stack.getSignalContainer().getXAxisName());
	background.setYAxisName(stack.getSignalContainer().getYAxisName());
	background.setXAxisName(stack.getSignalContainer().getXAxisName());
        
        //fromhere
	out=histoStack();
	out.setDataLegend("Data");
	out.push_back(data,"Data",1,1,99);

        if (mttfit_){
            out.push_back(background,"Background",kGreen-9,1,50);
            if(topontop_){
                for (size_t s=0; s<n_signal; ++s)
                    out.push_back(signal_v.at(s),"Signal"+toString(s+1),633+2*s,1,signal_v.size()-s+50);
            }
            else{
                for (size_t s=0; s<n_signal; ++s)
                    out.push_back(signal_v.at(s),"Signal"+toString(s+1),633+2*s,1,1+s);
            }
        }
        else{
            out.push_back(background,"Background",kGreen-9,1,2);
            if(topontop_)
		out.push_back(signal,"Signal",628,1,3);
            else
		out.push_back(signal,"Signal",628,1,1);
        }

	out.setName(stack.getName() + "_" + datasets_.at(datasetidx).getName());

	//return corr matrix
	corrmatr = fitter_.getCorrelationMatrix();


	if(debug)
		std::cout << "ttbarXsecFitter::applyParametersToStack: done" << std::endl;
	return out;
}


void ttbarXsecFitter::printAdditionalControlplots(const std::string& inputfile, const std::string & configfile,const std::string& prependToOutput)const{

	//read file
	fileReader fr;
	fr.setComment("#");
	fr.setDelimiter(":");
	//read plots
	std::vector< std::vector<std::string> > plotnames; //0: main, 1+ add
	std::vector<size_t> nbjets;
	std::vector<std::string> stylefiles;
	std::vector<TString> allsys=datasets_.at(0).getSystNames();
	bool hasmtvar=std::find(allsys.begin(),allsys.end(),"TOPMASS")!=allsys.end();
	fr.readFile(configfile);
	for(size_t line=0;line<fr.nLines();line++){
		if(fr.nEntries(line)>1){
			nbjets.push_back(fr.getData<size_t>(line,0));

			textFormatter fmt;
			fmt.setComment("#");
			fmt.setDelimiter("=");
			std::vector<std::string> subnames;
			std::vector<std::string> subplots=fmt.getFormatted(fr.getData<std::string>(line,1));
			if(subplots.size()<2)
				subnames=subplots;
			else{
				fmt.setDelimiter("+");
				subnames.push_back(subplots.at(0));
				subplots=fmt.getFormatted(subplots.at(1));
				subnames << subplots;
			}
			plotnames.push_back(subnames);
			if(fr.nEntries(line)>2)
				stylefiles.push_back(((std::string)getenv("CMSSW_BASE")+fr.getData<std::string>(line,2)));
			else
				stylefiles.push_back("");

		}
	}
	histoStackVector  hsv;hsv.readFromFile(inputfile);
	for(size_t i=0;i<nbjets.size();i++){
		if(plotnames.at(i).size()<1)continue;
		plotterControlPlot pl;
                pl.readStyleFromFileInCMSSW("src/TtZAnalysis/Analysis/configs/fitTtBarXsec/controlPlots_standard.txt");
                pl.addStyleFromFile(stylefiles.at(i),"[controlplot - "+ plotnames.at(i).at(0)+ "]","[end - controlplot]");                 
		if( inputfile.find("8TeV")!=std::string::npos)
			pl.readTextBoxesInCMSSW("/src/TtZAnalysis/Analysis/configs/general/noCMS_boxes.txt","CMSSplit03Left");
		else if(inputfile.find("7TeV")!=std::string::npos)
			pl.readTextBoxesInCMSSW("/src/TtZAnalysis/Analysis/configs/general/noCMS_boxes.txt","CMSSplit03Left7TeV");

		if(hasmtvar && massFit_)
			//pl.setSystLabel("#splitline{Syst}{+#Deltam_{t}^{MC}}");
			pl.setSystLabel("Syst+#Deltam_{t}^{MC}");
		histoStack stack,poststack;
		size_t bjets=nbjets.at(i);
		if(bjets>2)
			bjets=0;
		corrMatrix correlations;
		for(size_t j=0;j<plotnames.at(i).size();j++){
			std::cout << plotnames.at(i).at(j) << std::endl;
			if(plotnames.at(i).size()==1){
				const histoStack& tmp=hsv.getStack(plotnames.at(i).at(j).data());
				stack=applyParametersToStack(tmp,bjets,0,false, correlations);
				std::cout << "created single a" <<std::endl;
				poststack=applyParametersToStack(tmp,bjets,0,true, correlations);
				std::cout << "created single" <<std::endl;
			}
			else if(j==0){
				j++;
				const histoStack& stmp=hsv.getStack(plotnames.at(i).at(j).data());
				stack=applyParametersToStack(stmp,j-1+bjets,0,false, correlations);
				poststack=applyParametersToStack(stmp,j-1+bjets,0,true, correlations);
				std::cout << "created first "<< plotnames.at(i).at(j) <<std::endl;
			}
			else{
				const histoStack& stmp=hsv.getStack(plotnames.at(i).at(j).data());
				stack=stack+applyParametersToStack(stmp,j-1+bjets,0,false, correlations);
				poststack=poststack+applyParametersToStack(stmp,j-1+bjets,0,true, correlations);
				std::cout << "added  "<< plotnames.at(i).at(j) <<std::endl;
			}
		}
		//if(plotnames.at(i).size()!=1){
		stack.setName(plotnames.at(i).at(0).data());
		poststack.setName(stack.getName()+"_postfit");
		//}

                if (stack.getName() == "m_lb min 1,1 b-jets step 8") stack = stack.rebinXToBinning({20,48,76,104,132,160});
                else if (stack.getName() == "m_lb min 1,2 b-jets step 8") stack = stack.rebinXToBinning({20,48,76,104,132,160});
                else if (stack.getName() == "lead jet pt 0,1 b-jets step 8") stack = stack.rebinXToBinning({30,50,100,200}); 
                else if (stack.getName() == "second jet pt 0,2 b-jets step 8") stack = stack.rebinXToBinning({30,50,120,200}); 
                else if (stack.getName() == "third jet pt 0,3 b-jets step 8") stack = stack.rebinXToBinning({30,60,200});
                else if (stack.getName() == "m_lb min step 8") stack = stack.rebinXToBinning({20,50,75,105,130,160});
                else if (stack.getName() == "last jet pt 2 b-jets step 8") stack = stack.rebinXToBinning({30,40,50,60,70,80,90,100,120,140,200});                
                else if (stack.getName() == "last jet pt 2 b-jets mtt1 step 8") stack = stack.rebinXToBinning({30,40,60,200});
                else if (stack.getName() == "last jet pt 2 b-jets mtt2 step 8") stack = stack.rebinXToBinning({30,50,70,100,200});
                else if (stack.getName() == "last jet pt 2 b-jets mtt3 step 8") stack = stack.rebinXToBinning({30,50,70,120,200});


                if (poststack.getName() == "m_lb min 1,1 b-jets step 8_postfit") poststack = poststack.rebinXToBinning({20,48,76,104,132,160});
                else if (poststack.getName() == "m_lb min 1,2 b-jets step 8_postfit") poststack = poststack.rebinXToBinning({20,48,76,104,132,160});
                else if (poststack.getName() == "lead jet pt 0,1 b-jets step 8_postfit") poststack = poststack.rebinXToBinning({30,50,100,200});
                else if (poststack.getName() == "second jet pt 0,2 b-jets step 8_postfit") poststack = poststack.rebinXToBinning({30,50,120,200});
                else if (poststack.getName() == "third jet pt 0,3 b-jets step 8_postfit") poststack = poststack.rebinXToBinning({30,60,200});
                else if (poststack.getName() == "m_lb min step 8_postfit") poststack = poststack.rebinXToBinning({20,50,75,105,130,160});
                else if (poststack.getName() == "last jet pt 2 b-jets step 8_postfit") poststack = poststack.rebinXToBinning({30,40,50,60,70,80,90,100,120,140,200});
                else if (poststack.getName() == "last jet pt 2 b-jets mtt1 step 8_postfit") poststack = poststack.rebinXToBinning({30,40,60,200});
                else if (poststack.getName() == "last jet pt 2 b-jets mtt2 step 8_postfit") poststack = poststack.rebinXToBinning({30,50,70,100,200});
                else if (poststack.getName() == "last jet pt 2 b-jets mtt3 step 8_postfit") poststack = poststack.rebinXToBinning({30,50,70,120,200});


		TString name=stack.getName();
		name.ReplaceAll(" ","_");
		stack.setName(name);
		TFile f((TString)prependToOutput+textFormatter::makeCompatibleFileName(("prepostfit_"+stack.getName()+".root").Data()),"RECREATE");
		TCanvas cv(stack.getName());
		pl.setStack(&stack);
		pl.printToPdf(prependToOutput+textFormatter::makeCompatibleFileName( stack.getName().Data()));
		pl.usePad(&cv);
		pl.draw();
		cv.Write();
		if(fitsucc_){
			cv.SetName(poststack.getName());
			pl.setStack(&poststack);
			pl.associateCorrelationMatrix(correlations);
			pl.printToPdf(prependToOutput+textFormatter::makeCompatibleFileName(poststack.getName().Data()));
			pl.draw();
			cv.Write();
		}
		f.Close();
	}
}



void ttbarXsecFitter::printXsecScan(size_t datasetidx, const std::string & outname, const size_t mttbin){//const{
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::printXsecScan: dataset index out of range");

	//simpleFitter newfitter(fitter_);
	size_t xscidx=9999;
        if (!mttfit_) xscidx=datasets_.at(datasetidx).xsecIdx();
        else xscidx=datasets_.at(datasetidx).xsecIdxs().at(mttbin);

	double xsecerrdo=fitter_.getParameterErrDown()->at(xscidx);
	double xsecerrup=fitter_.getParameterErrUp()->at(xscidx);
	//double xsec=getXsec(datasetidx);
	double priorxsec=0;
        if (!mttfit_) priorxsec=datasets_.at(datasetidx).xsecOffset();
        else priorxsec=datasets_.at(datasetidx).xsecOffset(mttbin);

	double xsecparashift=fitter_.getParameter(xscidx);
	graph g=fitter_.scan( xscidx, xsecparashift + 3* xsecerrdo, xsecparashift+ 3*xsecerrup ,12);
	g.shiftAllXCoordinates(priorxsec);

	g.setXAxisName("#sigma_{t#bar{t}} ("+datasets_.at(datasetidx).getName()+") [pb]");
        if (mttfit_) g.setXAxisName("#sigma_{t#bar{t}} mtt"+toString(mttbin+1)+" ("+datasets_.at(datasetidx).getName()+") [pb]");
	g.setYAxisName("-2 ln(L)");

	std::string outfile=outname;
	outfile+="_LHScan_";
	outfile+=datasets_.at(datasetidx).getName().Data();
        if (mttfit_) outfile+="_mtt"+toString(mttbin+1);

	TFile f((TString)outfile.data()+".root","RECREATE");
	TCanvas cv;
	plotterMultiplePlots pl;
	pl.usePad(&cv);
	pl.addPlot(&g);
	pl.setLastNoLegend();
	pl.draw();
	cv.Write();

	pl.printToPdf(outfile);

}


void ttbarXsecFitter::createSystematicsBreakdowns(const TString& paraname, const size_t mttbin){
        if (mttfit_ && mttbin==9999) 
            throw std::logic_error("ttbarXsecFitter::createSystematicsBreakdowns: please select valid mtt bin");
	for(size_t i=0;i<datasets_.size();i++)
            createSystematicsBreakdown(i,paraname, mttbin);
}

void ttbarXsecFitter::createSystematicsBreakdown(size_t datasetidx, const TString& paraname, const size_t mttbin){
        if (mttfit_ && mttbin==9999 && paraname.Length()==0) 
            throw std::logic_error("ttbarXsecFitter::createSystematicsBreakdowns: please select valid mtt bin");
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::createSystematicsBreakdown: dataset index out of range");
	//size_t xsecidx=datasets_.at(datasetidx).xsecIdx();
	std::cout << "creating systematics breakdowns for " << datasets_.at(datasetidx).getName();
        if (mttbin!=9999) std::cout << " (mttbin n. " << mttbin<<")";
        std::cout<<std::endl;

	formatter fmt;
	TString cmsswbase=getenv("CMSSW_BASE");
	fmt.readInNameTranslateFile((cmsswbase+"/src/TtZAnalysis/Analysis/configs/general/SystNames.txt").Data());

	//bool completetable=false; //make configureable later

	size_t paraindex;
	if(paraname.Length()>0){
		paraindex = std::find(fitter_.getParameterNames()->begin(),fitter_.getParameterNames()->end(),paraname) -fitter_.getParameterNames()->begin();
		if(paraindex==fitter_.getParameterNames()->size())
			throw std::runtime_error(((std::string)"ttbarXsecFitter::createSystematicsBreakdown: for para "+paraname.Data()+" failed. parameter not found."  ));
	}
        else if (!mttfit_) paraindex=datasets_.at(datasetidx).xsecIdx();
        else paraindex=datasets_.at(datasetidx).xsecIdxs().at(mttbin);

	datasets_.at(datasetidx).postFitSystematicsFull().clear();
	datasets_.at(datasetidx).postFitSystematicsFullSimple().clear();

	float xsec;
	if(paraname.Length()>0) xsec=fitter_.getParameter(paraindex);
        else if (!mttfit_) xsec=getXsec(datasetidx);
        else xsec=getXsec(datasetidx, mttbin);

	float parfullvarmulti=1;
	if(paraname=="TOPMASS"){
                xsec*=3;
		xsec+=172.5;
		parfullvarmulti=3;
	}

	//read in merge stuff
	fileReader fr;
	fr.setDelimiter("=");
	fr.setComment("%");
	fr.setStartMarker("[merge in table]");
	fr.setEndMarker("[end - merge in table]");
	fr.readFile(cmsswbase.Data()+ mergesystfile_);

	std::vector<std::pair< TString , std::vector<size_t> > > mergeasso;
	//std::vector<size_t> allcrosscorr;

	//first merge dataset dependend contributions
	std::vector< std::pair <TString , std::vector<TString> > > plannedmerges;
	for(size_t i=0;i<fr.nLines();i++){
		TString merged=fr.getData<TString>(i,0);
		textFormatter fmt2;
		fmt2.setDelimiter("+");
                if (!removesyst_){
                    std::vector<TString> asso=fmt2.getFormatted<TString>(fr.getData<std::string>(i,1));
                    plannedmerges.push_back(std::pair <TString , std::vector<TString> > (merged ,asso));
                }
	}

	//create dataset dependent parts
	std::vector< std::pair <TString , std::vector<size_t> > > datasetdepmerges;
	for(size_t i=0;i<parameternames_.size();i++){
		if((paraname.Length()<1 && isXSecIdx(i)) || (paraname.Length()>0 && paraindex==i)) continue;
		if(parameternames_.at(i).EndsWith(datasets_.at(0).getName())){
			std::vector<size_t>  merges;
			merges.push_back(i);
			TString paraname=parameternames_.at(i);
			paraname.ReplaceAll("_"+datasets_.at(0).getName(),"");
			size_t mergeidx = std::find(parameternames_.begin(),parameternames_.end(),paraname) - parameternames_.begin();
			if(mergeidx<parameternames_.size()){
				merges.push_back(mergeidx);
			}
			for(size_t j=1;j<datasets_.size();j++){
				mergeidx = std::find(parameternames_.begin(),parameternames_.end(),
						paraname+"_"+datasets_.at(j).getName()) - parameternames_.begin();
				if(mergeidx<parameternames_.size()){
					merges.push_back(mergeidx);
				}
			}
			datasetdepmerges.push_back(std::pair<TString , std::vector<size_t> >(paraname ,merges) );
		}
	}

	//create index equivalents
	std::vector< std::pair <TString , std::vector<size_t> > > plannedmergesindx;
	for(size_t i=0;i<plannedmerges.size();i++){
		const TString & mergedname = plannedmerges.at(i).first;
		const std::vector<TString> & tomergenames=plannedmerges.at(i).second;
		//search in datasetdepmerges
		std::vector<size_t> indicies;
		for(size_t j=0;j<tomergenames.size();j++){
			bool foundindatasetmerges=false;
			for(size_t k=0;k<datasetdepmerges.size();k++){
				const TString & dtsmergename=datasetdepmerges.at(k).first;
				if(tomergenames.at(j) == dtsmergename){
					foundindatasetmerges=true;
					indicies<<datasetdepmerges.at(k).second; //add full vector
					//delete from the other vector
					datasetdepmerges.erase(datasetdepmerges.begin()+k);
				}
			}
			if(!foundindatasetmerges){
				size_t newidx=std::find(parameternames_.begin(),parameternames_.end(), tomergenames.at(j))-parameternames_.begin();
				if(newidx<parameternames_.size()){
					indicies<<newidx;
				}
				else{
					std::string errstr="ttbarXsecFitter::createSystematicsBreakdown: index for ";
					errstr+=tomergenames.at(j).Data();
					errstr+=" not found. E.g. check for duplicates";
					throw std::out_of_range(errstr);
				}
			}
		}
		plannedmergesindx.push_back(std::pair<TString , std::vector<size_t> > (mergedname, indicies ));
	}

	//for consistency, add remaining ones
	plannedmergesindx<<datasetdepmerges;
	std::vector<size_t> alltobemerged;
	for(size_t i=0;i<plannedmergesindx.size();i++)
		alltobemerged << plannedmergesindx.at(i).second;

	for(size_t i=0;i<parameternames_.size();i++){
		if((paraname.Length()<1 && isXSecIdx(i)) || (paraname.Length()>0 && paraindex==i))continue;
		if(std::find(alltobemerged.begin(),alltobemerged.end(),i) == alltobemerged.end()){
			plannedmergesindx.push_back(std::pair<TString,std::vector<size_t> >(parameternames_.at(i),std::vector<size_t> (1,i)  )  );
		}
	}

	if(debug){
		std::cout << "merge configuration: "<<std::endl;
		for(size_t i=0;i<plannedmergesindx.size();i++){
			std::cout <<  plannedmergesindx.at(i).first << ": " <<std::endl;
			for(size_t j=0;j<plannedmergesindx.at(i).second.size();j++){
				std::cout <<  parameternames_.at(plannedmergesindx.at(i).second.at(j)) <<" ";
			}
			std::cout << '\n'<< std::endl;
		}
	}
	std::cout << "creating simplified breakdown table for " << datasets_.at(datasetidx).getName()<<" "<< paraname<<std::endl;

	for(size_t i=0;i<plannedmergesindx.size();i++){
		if(!debug)
			displayStatusBar(i,plannedmergesindx.size());
		TString name=translatePartName(fmt, plannedmergesindx.at(i).first);

		double errup=999,errdown=999;

		//redo!
		dataset::systematic_unc tmpunc;
		//	if(std::find(addfullerrors_.begin(),addfullerrors_.end(),i)  != addfullerrors_.end()){
		//		name+= " (vis)";
		//	}
		if(!nosystbd_){
			fitter_.getParameterErrorContributions(plannedmergesindx.at(i).second ,paraindex,errup,errdown);
			errup/=xsec;
			errdown/=xsec;
			errup*=parfullvarmulti;
			errdown*=parfullvarmulti;
			if(debug)
				std::cout << name << ": " << errup << " " << errdown<< std::endl;
		}
		tmpunc.name=name;
		tmpunc.errdown=errdown;
		tmpunc.errup=errup;
		tmpunc.constr= 999; //fitter_.getParameterErr(i);
		tmpunc.pull= 999;//fitter_.getParameters()->at(i);
		datasets_.at(datasetidx).postFitSystematicsFullSimple().push_back(tmpunc);
	}

	//make fast individual contributions table

	for(size_t i=0;i<getNParameters();i++){

		double errup,errdown;
		double corr=fitter_.getCorrelationCoefficient(i, paraindex);
		double xsecerr=fitter_.getParameterErr(paraindex);
		errup=corr*xsecerr/xsec;
		errup*=parfullvarmulti;
		errdown=-errup;
		TString name=translatePartName(fmt, parameternames_.at(i));
		dataset::systematic_unc tmpunc;
		tmpunc.name=name;
		tmpunc.errdown=errdown;
		tmpunc.errup=errup;
		tmpunc.constr= fitter_.getParameterErr(i) ; //fitter_.getParameterErr(i);
		tmpunc.pull= fittedparas_.at(i);//fitter_.getParameters()->at(i);
		datasets_.at(datasetidx).postFitSystematicsFull().push_back(tmpunc);
	}

	dataset::systematic_unc tmpunc;

	//statistics
	double errupstat,errdownstat;
	//bool anticorr=false;//dummy
	fitter_.getStatErrorContribution(paraindex,errupstat,errdownstat);
	errupstat/=xsec;
	errdownstat/=xsec;
	errupstat*=parfullvarmulti;
	errdownstat*=parfullvarmulti;

	tmpunc.name="Stat";
	tmpunc.errup=errupstat;
	tmpunc.errdown=errdownstat;
	tmpunc.pull=999;
	tmpunc.constr=999;

	datasets_.at(datasetidx).postFitSystematicsFull().push_back(tmpunc);
	datasets_.at(datasetidx).postFitSystematicsFullSimple().push_back(tmpunc);


	double fullpsxsec=xsec;//getXsec(datasetidx);

	double fullrelxsecerrup=fitter_.getParameterErrUp()->at(paraindex)/fullpsxsec * parfullvarmulti;
	double fullrelxsecerrdown=fitter_.getParameterErrDown()->at(paraindex)/fullpsxsec * parfullvarmulti;

	tmpunc.name="Total vis";
	if(paraname.Length()>0)
		tmpunc.name="Total "+paraname;
	tmpunc.errup=fullrelxsecerrup;
	tmpunc.errdown=fullrelxsecerrdown;
	tmpunc.pull=999;
	tmpunc.constr=999;

	datasets_.at(datasetidx).postFitSystematicsFull().push_back(tmpunc);
	datasets_.at(datasetidx).postFitSystematicsFullSimple().push_back(tmpunc);

	std::vector<double> fulladdtoerrup2,fulladdtoerrdown2;


	if(paraname.Length()<1){

		std::cout << "\nadding extrapolation uncertainties..." <<std::endl;

		for(size_t i=0;i<addfullerrors_.size();i++){
			double errup2=0,errdown2=0;
			TString name=translatePartName(fmt,addfullerrors_.at(i).name );
			name+=" (extr)";
			float exup=100,exdown=100;
			for(size_t j=0;j<addfullerrors_.at(i).indices.size();j++){
				size_t idx=addfullerrors_.at(i).indices.at(j);
				float min=addfullerrors_.at(i).restmin.at(j);
				float max=addfullerrors_.at(i).restmax.at(j);

				if (!mttfit_){
                                    exup=getExtrapolationError(datasetidx,idx,true,min,max);
                                    exdown=getExtrapolationError(datasetidx,idx,false,min,max);
                                }
                                else{
                                    exup=getExtrapolationError(datasetidx,idx,true,min,max,mttbin);
                                    exdown=getExtrapolationError(datasetidx,idx,false,min,max,mttbin);
                                }
				bool corranti;
				double tmp=getMaxVar(true,exup,exdown,corranti);
				fulladdtoerrup2.push_back(tmp*tmp);
				errup2+=tmp*tmp;
				tmp=getMaxVar(false,exup,exdown,corranti);
				fulladdtoerrdown2.push_back(tmp*tmp);
				errdown2+=tmp*tmp;
			}

			tmpunc.name=name;
			if(addfullerrors_.at(i).indices.size()>1){
				tmpunc.errdown=-sqrt(errdown2);
				tmpunc.errup=sqrt(errup2);
			}
			else{
				tmpunc.errdown=exdown;
				tmpunc.errup=exup;
			}
			tmpunc.pull=999;
			tmpunc.constr=999;

			datasets_.at(datasetidx).postFitSystematicsFull().push_back(tmpunc);
			datasets_.at(datasetidx).postFitSystematicsFullSimple().push_back(tmpunc);

		}

	}

	for(size_t i=0;i<fulladdtoerrup2.size();i++)
		fullrelxsecerrup=sqrt(fullrelxsecerrup*fullrelxsecerrup+fulladdtoerrup2.at(i));
	for(size_t i=0;i<fulladdtoerrdown2.size();i++)
		fullrelxsecerrdown=sqrt(fullrelxsecerrdown*fullrelxsecerrdown+fulladdtoerrdown2.at(i));

	tmpunc.name="Total";
	tmpunc.errup=fullrelxsecerrup;
	tmpunc.errdown=fullrelxsecerrdown;

	datasets_.at(datasetidx).postFitSystematicsFull().push_back(tmpunc);
	datasets_.at(datasetidx).postFitSystematicsFullSimple().push_back(tmpunc);

}

    texTabler ttbarXsecFitter::makeSystBreakDownTable(size_t datasetidx,bool detailed,const TString& paraname, const size_t mttbin){

	/*
	 * Split this function in a part where the systematic contributions are
	 * calculated and saved per dataset (for extrapol unc minos errors up/down)
	 * and then a function to print them to a table
	 */

	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::makeSystBreakdown: dataset index out of range");


        if (!mttfit_ || paraname.Length()>0) {
            if((datasets_.at(datasetidx).postFitSystematicsFull().size()<1) ) createSystematicsBreakdown(datasetidx,paraname);                
        }
        else createSystematicsBreakdown(datasetidx,paraname,mttbin);



	size_t paraindex;
	if(paraname.Length()>0){
		paraindex = std::find(fitter_.getParameterNames()->begin(),fitter_.getParameterNames()->end(),paraname) -fitter_.getParameterNames()->begin();
		if(paraindex==fitter_.getParameterNames()->size())
			throw std::runtime_error(((std::string)"ttbarXsecFitter::createSystematicsBreakdown: for para "+paraname.Data()+" failed. parameter not found."  ));
	}
        else if (!mttfit_) paraindex=datasets_.at(datasetidx).xsecIdx();
        else paraindex=datasets_.at(datasetidx).xsecIdxs().at(mttbin);


	float xsec;
	if(paraname.Length()>0) xsec=fitter_.getParameter(paraindex);
        else if (!mttfit_) xsec=getXsec(datasetidx);
        else xsec=getXsec(datasetidx,mttbin);
	float tableprecision=0.1;
        float norm = 1./xsec;
	if(paraname=="TOPMASS"){
		xsec*=3;
		xsec+=172.5;
		tableprecision=0.01;
                norm=.01;
	}

	bool symmetrize=false;

	formatter fmt;
	TString cmsswbase=getenv("CMSSW_BASE");
	fmt.readInNameTranslateFile((cmsswbase+"/src/TtZAnalysis/Analysis/configs/general/SystNames.txt").Data());
	texTabler table;
        if (detailed)table = texTabler(" l | c | c | c ");
        else table = texTabler(" l |  c ");
	if(!fitsucc_)
		return table;

        if (paraname=="TOPMASS"){
            if(detailed) table << "Name" << "Pull" << "Constr / $\\sigma$" << "Contribution [GeV]";
            else table << "Name"  << "Contribution [GeV]";
        }
        else{
            if(detailed) table << "Name" << "Pull" << "Constr / $\\sigma$" << "Contribution [\\%]";
            else table << "Name"  << "Contribution [\\%]";
        }
	table << texLine();
	std::vector<dataset::systematic_unc> * unc=&datasets_.at(datasetidx).postFitSystematicsFull();

	if(!detailed){
		unc=&datasets_.at(datasetidx).postFitSystematicsFullSimple();
		symmetrize=true;
	}
	else
		tableprecision=0.001;

        size_t xsecidx;
        if (!mttfit_) xsecidx=datasets_.at(datasetidx).xsecIdx();
        else if (mttbin!=9999) xsecidx=datasets_.at(datasetidx).xsecIdxs().at(mttbin);
        else xsecidx=0;
        double visxsec;
	if (!mttfit_) visxsec=getVisXsec(datasetidx);
        else if (mttbin!=9999) visxsec=getVisXsec(datasetidx,mttbin);
        else visxsec=0;

        for(size_t i=0;i<unc->size();i++){
		dataset::systematic_unc * sys=&unc->at(i);

		if(symmetrize && !(sys->name == "Total" || sys->name == "Total fitted")
				&& !(sys->name.Contains("(extr)")))
			sys->symmetrize(paraname=="TOPMASS");

		bool anticorr=false;
		getMaxVar(true,sys->errup,sys->errdown,anticorr);
		TString errstr;
		if(sys->errdown>900 && sys->errup > 900){
			errstr= " ";
		}
		else if(fabs(sys->errdown) == fabs(sys->errup)){
			errstr="${"+fmt.toFixedCommaTString(100*xsec*norm*sys->errup,tableprecision)+ "}$";
		}
		else{
			if(anticorr)
				errstr="$\\mp^{"+fmt.toFixedCommaTString(100*xsec*norm*fabs(sys->errdown),tableprecision)+"}_{"+fmt.toFixedCommaTString(100*xsec*norm*fabs(sys->errup),tableprecision)+ "}$";
			else
				errstr="$\\pm^{"+fmt.toFixedCommaTString(100*xsec*norm*fabs(sys->errup),tableprecision)+ "}_{"+fmt.toFixedCommaTString(100*xsec*norm*fabs(sys->errdown),tableprecision)+"}$";;
		}
		if(sys->name == "Total" || sys->name == "Total fitted")
			table <<texLine(1);
		if(sys->pull < 900)
			table << sys->name << fmt.round( sys->pull, tableprecision)<<
			fmt.round( sys->constr, 0.01)	<< errstr;
		else
			if (detailed) table << sys->name<< ""<<""      << errstr;
                        else table << sys->name<<  errstr;
		if(sys->name == "Total vis"){
			table <<texLine(1);
			if (detailed) table <<fmt.translateName( getParaName(xsecidx) ) +" vis"                                                                                                                                     << " " << " "<< fmt.toTString(fmt.round(visxsec,tableprecision/10))+" pb";
                        else table <<fmt.translateName( getParaName(xsecidx) ) +" vis"
                                        << fmt.toTString(fmt.round(visxsec,tableprecision/10))+" pb";
			table <<texLine(1);
		}
	}
	table <<texLine(1);
	//float fullxsec=getXsec(datasetidx);
	/*if(!visibleps_){
		table <<fmt.translateName( getParaName(xsecidx) )<<
				" " << " "
				<< fmt.toTString(fmt.round(fullxsec,0.1))+" pb";
	}
	else{*/
	if((paraname.Length()>0)){
		if(detailed) table <<fmt.translateName( getParaName(paraindex ))<<
				" " << " "
				<< fmt.toTString(fmt.round(xsec,tableprecision))+" ";
                else table <<fmt.translateName( getParaName(paraindex ))
                                << fmt.toTString(fmt.round(xsec,tableprecision))+" ";

	}
	else{
		if(detailed)table <<fmt.translateName( getParaName(xsecidx) )<<
				" " << " "
				<< fmt.toTString(fmt.round(xsec,tableprecision*10))+" pb";
                else table <<fmt.translateName( getParaName(xsecidx) )
                                << fmt.toTString(fmt.round(xsec,tableprecision*10))+" pb";

		//}
	}
	table << texLine();
	return table;



}

void ttbarXsecFitter::writeCorrelationMatrix(const std::string& filename, bool addconstraints)const{

	corrMatrix m=fitter_.getCorrelationMatrix();
	std::ofstream outfile(filename);
	size_t maxlength=0;
	for(size_t i=0;i<m.size();i++){
		size_t length=textFormatter::makeCompatibleFileName(m.getEntryName(i).Data()).length();
		if(length>maxlength) maxlength=length;
	}

	for(size_t i=0;i<m.size();i++){
		outfile <<  textFormatter::fixLength(textFormatter::makeCompatibleFileName(m.getEntryName(i).Data()),maxlength) <<" ";

		if(addconstraints){
			outfile <<  " (";
			outfile.precision(2);
			outfile << fitter_.getParameterErr(i);
			outfile <<  ") ";
		}
		for(size_t j=0;j<=i;j++){
			outfile.precision(10);
			outfile << m.getEntry(i,j) ;
			outfile << " ";
		}
		outfile<<std::endl;
	}
	outfile<<std::endl;
	outfile.close();
}


texTabler ttbarXsecFitter::makeCorrTable() const{

	TString format=" l ";
	histo2D corr=getCorrelations();
	for(size_t i=1;i<corr.getBinsX().size()-1;i++)
		format+=" | c ";
	texTabler tab(format);
	if(!fitsucc_)
		return tab;

	formatter Formatter;
	TString cmsswbase=getenv("CMSSW_BASE");
	Formatter.readInNameTranslateFile((cmsswbase+"/src/TtZAnalysis/Analysis/configs/general/SystNames.txt").Data());
	for(size_t j=0;j<corr.getBinsY().size()-1;j++){
		for(size_t i=0;i<corr.getBinsX().size()-1;i++){
			//names
			if(i && !j)
				tab << "";//Formatter.translateName(parameternames_.at(i-1));
			else if(j && !i)
				tab << Formatter.translateName(parameternames_.at(j-1));
			else if (!j && !i)
				tab << "";
			else if(i<=j){
				float content=corr.getBinContent(i,j);
				if(fabs(content)<0.3)
					tab << Formatter.toFixedCommaTString(corr.getBinContent(i,j),0.01);
				else
					tab <<  "\\textbf{" +Formatter.toFixedCommaTString(corr.getBinContent(i,j),0.01) +"}";
			}
			else
				tab << "";
		}
		tab<<texLine(); //row done
	}

	return tab;
}

graph ttbarXsecFitter::getResultsGraph(size_t datasetidx,const float x_coord)const{
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::getResultsGraph: dataset index out of range");


	if((datasets_.at(datasetidx).postFitSystematicsFull().size()<1) )
		throw std::out_of_range("ttbarXsecFitter::getResultsGraph: first create syst breakdown");

	graph g;
	if(!fitsucc_)
		return g;

	float xsec=getXsec(datasetidx);
	float fiterrup=fitter_.getParameterErrUp()->at(datasets_.at(datasetidx).xsecIdx());
	float fiterrdown=fitter_.getParameterErrDown()->at(datasets_.at(datasetidx).xsecIdx());

	g.addPoint(x_coord,xsec);
	graph gerr=g;
	gerr.setPointYContent(0,xsec+fiterrup);
	g.addErrorGraph("fit_up",gerr);
	gerr.setPointYContent(0,xsec+fiterrdown);
	g.addErrorGraph("fit_down",gerr);

	const std::vector<dataset::systematic_unc> * unc=&datasets_.at(datasetidx).postFitSystematicsFullSimple();

	for(size_t i=0;i<unc->size();i++){
		const dataset::systematic_unc * sys=&unc->at(i);
		if(! sys->name.Contains("(extr)")) continue;

		gerr.setPointYContent(0,xsec+ xsec*sys->errup);
		g.addErrorGraph(sys->name+"_up",gerr);
		gerr.setPointYContent(0,xsec- xsec*sys->errdown);
		g.addErrorGraph(sys->name+"_down",gerr);
	}
	return g;
}

// can not be const due to root limitations
// do size checks before!
double ttbarXsecFitter::toBeMinimized(const double * variations){
	static size_t ncalls=0;
	static double lastout=10000;
	ncalls++;
	if(variations[0]!=variations[0]){
		std::cout << "Exception in call "<<ncalls<<std::endl;
		throw std::runtime_error("ttbarXsecFitter::toBeMinimized: at least one nan input from minuit");
	}
	double out=0;
	for(size_t ndataset=0;ndataset<datasets_.size();ndataset++){
		dataset * set=&datasets_.at(ndataset);

		for(size_t nbjet=0;nbjet<dataset::nBjetCat();nbjet++){
			if(exclude0bjetbin_){
				if(nbjet==0) continue;
			}
			//make sure its normalized
			//the influence usually is <0.001%, but still more correct that way
			double shapeintegral=0;
                        double omega_b=0; 
                        double acceptance=0; 
                        double eps_emu=0;

                        std::vector<double> shapeintegral_v, omega_b_v, acceptance_v, eps_emu_v;
                        if (!mttfit_){
                            shapeintegral = set->signalshape(nbjet).getIntegral(variations);  //includes UFOF
                            omega_b = set->omega_b(nbjet).getValue(variations);
                            acceptance= set->acceptance().getValue(variations);
                            eps_emu=set->eps_emu().getValue(variations);
                        }
                        else{
                            for (size_t s=0; s<n_signals; ++s){ //hardcoded
                                shapeintegral_v.push_back(set->signalshape(s,nbjet).getIntegral(variations));
                                omega_b_v.push_back(set->omega_b(s,nbjet).getValue(variations));
                                acceptance_v.push_back(set->acceptance(s).getValue(variations));
                                eps_emu_v.push_back(set->eps_emu(s).getValue(variations));
                            }
                        }

                        size_t nbins;
                        if (!mttfit_) nbins = set->signalshape(nbjet).getNBins();
                        else nbins = set->signalshape(0,nbjet).getNBins();

			for(size_t bin=0;bin<nbins;bin++){
				//std::cout << bin << "/"<< signalshape_nbjet_.at(nbjet).getNBins()<<" " << nbjet << " " << std::endl;
				
                                double signal = 0;
                                
                                if (!mttfit_){
                                    double lumi_xsec    = set->lumi() * (variations[set->xsecIdx()] +set->xsecOffset()) ;
                                    signal=lumi_xsec* acceptance * eps_emu* omega_b * set->signalshape(nbjet).getBin(bin)->getValue(variations) / shapeintegral;
                                    if (mlbCrossCheck_){
                                        signal=lumi_xsec* acceptance * eps_emu* set->signalshape(nbjet).getBin(bin)->getValue(variations);
                                    }
                                }
                                else{
                                    for (size_t s=0; s<shapeintegral_v.size(); ++s){
                                        double lumi_xsec = set->lumi() * (variations[set->xsecIdxs().at(s)] +set->xsecOffset(s));
                                        signal+=lumi_xsec* acceptance_v.at(s) * eps_emu_v.at(s)* omega_b_v.at(s) * set->signalshape(s,nbjet).getBin(bin)->getValue(variations) / shapeintegral_v.at(s);
                                    }
                                }
                                

				double nbackground = set->background(nbjet).getBin(bin)->getValue(variations);

				double backgroundstat=set->background(nbjet).getBinErr(bin);

				double signalstat2 = 0;
                                if (!mttfit_){
                                    signalstat2 = set->signalshape(nbjet).getBinErr(bin);
                                    signalstat2 *= signalstat2;
                                }
                                else{
                                    for (size_t s=0; s<shapeintegral_v.size(); ++s){
                                        double tempstat = set->signalshape(s,nbjet).getBinErr(bin);
                                        signalstat2 += tempstat*tempstat;
                                    }
                                }

				double data = set->data(nbjet).getBin(bin)->getValue(variations);

				double datastat = set->data(nbjet).getBinErr(bin);

				//this  might happen for some variations, fix it to physics values
				if(signal<0)     signal=0;
				if(nbackground<0)nbackground=0;
				double predicted = signal+nbackground;

				if(lhmode_ == lhm_poissondatastat)   {
					if(data<0)data=0; //safety
					//if(data==0)
					if(predicted>0)
						out+=-2*logPoisson(data, predicted); //unstable...
					else if(data>0)
						out+=100;
				}

				else if(lhmode_ ==lhm_chi2datastat)   {
					if(datastat<=0) continue;
					out+= (data-predicted)*(data-predicted) / (datastat*datastat); //chi2 approach
				}
				else if(lhmode_ ==lhm_chi2datamcstat)	{
					if((datastat*datastat + backgroundstat*backgroundstat)<=0) continue;
					out+= (data-predicted)*(data-predicted) / (datastat*datastat + backgroundstat*backgroundstat ); //chi2 approach with bg errors
				}
				else if(lhmode_ ==lhm_chi2datafullmcstat)	{
					if((datastat*datastat + backgroundstat*backgroundstat)<=0) continue;
					out+= (data-predicted)*(data-predicted) / (datastat*datastat + backgroundstat*backgroundstat + signalstat2); //chi2 approach with bg errors
				}

				if(out != out){
					std::cout << "nan produced..." <<std::endl;
					ZTOP_COUTVAR(bin);
					ZTOP_COUTVAR(nbjet);
					ZTOP_COUTVAR(omega_b);
					ZTOP_COUTVAR(data);
					ZTOP_COUTVAR(datastat);
					ZTOP_COUTVAR(nbackground);
					ZTOP_COUTVAR(backgroundstat);
					ZTOP_COUTVAR(signal);
					ZTOP_COUTVAR(predicted);
					ZTOP_COUTVAR(out);
					for(size_t i=0;i<parameternames_.size();i++)
						std::cout << parameternames_.at(i) << ":\t"<<variations[i]<<std::endl;
				}
			}
		}
	}
	if(out==std::numeric_limits<double>::infinity()){
		out=lastout*10; //this is only supposed to happen for extreme variations during fit process->check pulls
	}
	lastout=out;
	//constraints
	for(size_t sys=0;sys<ndependencies_;sys++){
		bool isxsec=false;
		for(size_t i=0;i<datasets_.size();i++){
                    if (!mttfit_){
                        if(sys == datasets_.at(i).xsecIdx()){
                            isxsec=true;
                            break;
                        }
                    }
                    else{
                        if( std::find(datasets_.at(i).xsecIdxs().begin(), datasets_.at(i).xsecIdxs().end(), sys) != datasets_.at(i).xsecIdxs().end()){
                            isxsec=true;
                            break;
                        }
                    }
                }
		if(isxsec) continue;


		//put else here if lumi gets treated differently
		if(priors_[sys] == prior_gauss){
			out+= simpleFitter::nuisanceGaus(variations[sys]);}
                else if(priors_[sys] == prior_gaussbroad){
                        out+= simpleFitter::nuisanceGausBroad(variations[sys]);}
                else if(priors_[sys] == prior_gaussmass){
                        out+= simpleFitter::nuisanceGausMass(variations[sys]);}

		//else if(priors_[sys] == prior_box){
		//	out+= simpleFitter::nuisanceBox(variations[sys]);
		//}
		else if(priors_[sys] == prior_float){
			//don't do anything
		}
		//else if(priors_[sys] == prior_narrowboxleft){
		//	double var=variations[sys];
		//	if(var>=0) var+=1;
		//	out+= simpleFitter::nuisanceBox(var);
		//}
		//else if(priors_[sys] == prior_narrowboxright){
		//	double var=variations[sys];
		//	if(var<=0) var-=1;
		//	out+= simpleFitter::nuisanceBox(var);
		//}
		//the fitter will handle this case, delta should be zero in all cases
		//	else if(priors_[sys] == prior_parameterfixed){
		//		double deltafixed=variations[sys]-fitter_.getParameter(sys);
		//		out+=deltafixed*deltafixed*1e9; //make it a bit smooth
		//	}

	}


	if(out!=out){
		throw std::runtime_error("ttbarXsecFitter::toBeMinimized NAN");
	}
	if(out<0)
		out=0;
	if(out<DBL_EPSILON && out !=0){
		out=DBL_EPSILON;
		std::cout << "restricted out!" <<std::endl;
	}
	if(out!=out){
		throw std::runtime_error("ttbarXsecFitter::toBeMinimized NAN");
	}
	return out;
}

bool ttbarXsecFitter::dataset::readyForFit()const{
    if (!parent_->mttfit_) return signalshape_nbjet_.size()>0;
    else{
        for (size_t s=0; s<signalshape_nbjet_v_.size(); ++s){
            if (signalshape_nbjet_v_.at(s).size()<1){
                return false;
            }
        }
        return true;
    }
}

void ttbarXsecFitter::checkSizes()const{
	if(debug)
		std::cout << "ttbarXsecFitter::checkSizes" <<std::endl;
	std::vector<TString> chkv;
	for(size_t i=0;i<datasets_.size();i++){
		datasets_.at(i).checkSizes();
		//sizes ok, check for parameter consistency
		for(size_t b=0;b<dataset::nBjetCat();b++){
                    size_t s_max = n_signals; //hardcoded
                    if (!mttfit_) s_max=1;
                    for (size_t s=0;s<s_max;s++){
			if(chkv.size()>0){
                            std::vector<TString> tocompare;
                            if (!mttfit_) tocompare = datasets_.at(i).signalshape(b).getSystNames();
                            else tocompare = datasets_.at(i).signalshape(s,b).getSystNames();
                            if(chkv == tocompare)
                                continue;
                            else{
                                throw std::runtime_error("ttbarXsecFitter::checkSizes: parameters inconsistent");
                            }
			}
			else{
                            if (!mttfit_) chkv= datasets_.at(i).signalshape(b).getSystNames();
                            else chkv= datasets_.at(i).signalshape(s,b).getSystNames();
                        }
                    }
                }
	}
	if(debug)
		std::cout << "ttbarXsecFitter::checkSizes: done" <<std::endl;
}

//the whole function is obsolete once the class works as expected!
void ttbarXsecFitter::dataset::checkSizes()const{

	if(debug)
		std::cout << "ttbarXsecFitter::dataset::checkSizes: "<<name_ <<std::endl;
        if (!parent_->mttfit_){
            if(signalshape_nbjet_.size() < 1){
		throw std::logic_error("ttbarXsecFitter::checkSizes: no input");
            }
            if(signalshape_nbjet_.size() != background_nbjet_.size())
		throw std::logic_error("ttbarXsecFitter::checkSizes: leptonacceptance_nbjet_.size() != background_nbjet_.size()");
            if(signalshape_nbjet_.size() != data_nbjet_.size())
		throw std::logic_error("ttbarXsecFitter::checkSizes: leptonacceptance_nbjet_.size() != data_nbjet.size()");

            for(size_t i=0;i<signalshape_nbjet_.size() ;i++){
		if(signalshape_nbjet_.at(i).getBins() != background_nbjet_.at(i).getBins())
                    throw std::logic_error("leptonacceptance_nbjet_.at(i).getBins() != background_nbjet_.at(i).getBins()");
		if(signalshape_nbjet_.at(i).getBins() != data_nbjet_.at(i).getBins())
                    throw std::logic_error("leptonacceptance_nbjet_.at(i).getBins() != data_nbjet.at(i).getBins()");
            }
            if(signalshape_nbjet_.at(0).getNDependencies() != background_nbjet_.at(0).getNDependencies())
		throw  std::logic_error("ttbarXsecFitter::checkSizes: ndep broken");
            if(signalshape_nbjet_.at(0).getNDependencies() != data_nbjet_.at(0).getNDependencies())
		throw  std::logic_error("ttbarXsecFitter::checkSizes: ndep broken");
            if(signalshape_nbjet_.at(0).getNDependencies() != omega_nbjet_.at(0).getNDependencies())
		throw  std::logic_error("ttbarXsecFitter::checkSizes: ndep broken");

        }
        else{
            if(signalshape_nbjet_v_.size() < 1){
                throw std::logic_error("ttbarXsecFitter::checkSizes: no input");
            }
            for (size_t s=0; s<signalshape_nbjet_v_.size(); ++s){
                if(signalshape_nbjet_v_.at(s).size() < 1){
                    throw std::logic_error("ttbarXsecFitter::checkSizes: no input");
                }
                if(signalshape_nbjet_v_.at(s).size() != background_nbjet_.size())
                    throw std::logic_error("ttbarXsecFitter::checkSizes: leptonacceptance_nbjet_.size() != background_nbjet_.size()");
                if(signalshape_nbjet_v_.at(s).size() != data_nbjet_.size())
                    throw std::logic_error("ttbarXsecFitter::checkSizes: leptonacceptance_nbjet_.size() != data_nbjet.size()");
                for(size_t i=0;i<signalshape_nbjet_v_.at(s).size() ;i++){
                    if(signalshape_nbjet_v_.at(s).at(i).getBins() != background_nbjet_.at(i).getBins())
		        throw std::logic_error("leptonacceptance_nbjet_.at(i).getBins() != background_nbjet_.at(i).getBins()");
                    if(signalshape_nbjet_v_.at(s).at(i).getBins() != data_nbjet_.at(i).getBins())
		        throw std::logic_error("leptonacceptance_nbjet_.at(i).getBins() != data_nbjet.at(i).getBins()");
                }
                if(signalshape_nbjet_v_.at(s).at(0).getNDependencies() != background_nbjet_.at(0).getNDependencies())
                    throw  std::logic_error("ttbarXsecFitter::checkSizes: ndep broken");
                if(signalshape_nbjet_v_.at(s).at(0).getNDependencies() != data_nbjet_.at(0).getNDependencies())
                    throw  std::logic_error("ttbarXsecFitter::checkSizes: ndep broken");
                if(signalshape_nbjet_v_.at(s).at(0).getNDependencies() != omega_nbjet_v_.at(s).at(0).getNDependencies())
                    throw  std::logic_error("ttbarXsecFitter::checkSizes: ndep broken");
            }
        }


	if(debug)
		std::cout << "ttbarXsecFitter::dataset::checkSizes: "<<name_ << " done" <<std::endl;

}//ttbarXsecFitter::checkSizes()


double ttbarXsecFitter::getExtrapolationError( size_t datasetidx, size_t paraidx, bool up, const float& min,const float& max, const size_t mttbin){
	/*std::vector<double> paracopy=fittedparas_;
		float nom=var->getValue(paracopy);
		if(i==0)
			std::cout << "---->> Epsilon emu (%): " << nom*100 << std::endl;
		paracopy.at(idx)=1;
		float up=var->getValue(paracopy);

		paracopy.at(idx)=-1;
		float down=var->getValue(para */
	if(paraidx >= parameternames_.size()){
		throw std::out_of_range("ttbarXsecFitter::getExtrapolationError: parameter index.");
	}
	if(datasetidx>=datasets_.size()){
		throw std::out_of_range("ttbarXsecFitter::getExtrapolationError: dataset index.");
	}

        if (mttfit_ && mttbin==9999){
            throw std::logic_error("ttbarXsecFitter::getExtrapolationError: please select a suitable mttbin");
        }


	//new (17.3.)

	//in the new implementation everything that matters is the effect on acceptance_extr.

	extendedVariable extracp;
        if (!mttfit_) extracp=datasets_.at(datasetidx).acceptance_extr();
        else extracp=datasets_.at(datasetidx).acceptance_extr(mttbin);

	double optpara=fitter_.getParameter(paraidx);
	std::vector<double> paras=fittedparas_;

	if(up && optpara>max)
		return 0;
	if(!up && optpara<min)
		return 0;

	if(up)
		paras.at(paraidx)=max;
	else
		paras.at(paraidx)=min;

	double nominal=extracp.getValue(fittedparas_);
	double varied=extracp.getValue(paras);
	//double xsec=getXsec(datasetidx);
	double xsecerr = nominal / varied -1 ; // ((extrapolfactorF/extrapolfactorFull)*variedxsec - nominalxsec ) / nominalxsec;

	return xsecerr;

}

variateHisto1D ttbarXsecFitter::dataset::createLeptonJetAcceptance(const std::vector<histo1D>& signals,
		const std::vector<histo1D>& signalpsmig,
		const std::vector<histo1D>& signalvisPSgen,
                size_t bjetcategory, size_t mttbin)
{
	// this one can use histo1D operators!
	if(debug)
		std::cout << "ttbarXsecFitter::dataset::createLeptonJetAcceptance: "<< bjetcategory <<", "<< name_<<std::endl;

	if(signals.size()<1)
		throw std::logic_error("ttbarXsecFitter::createLeptonJetAcceptance  stack vector empty.");

        if (parent_->mttfit_ && mttbin==9999)
            throw std::logic_error("ttbarXsecFitter::createLeptonJetAcceptance: choose a proper mtt bin");

	size_t bjetbin=bjetcategory;
	size_t minbjetbin=0;
	size_t maxbjetbin=3;



	float gennorm = 1/(lumi_*xsecOffset(mttbin));

	//unused	size_t bjet0bin=minbjetbin;
	size_t bjet1bin=minbjetbin+1;
	size_t bjet2bin=minbjetbin+2;

	histo1D signal=signals.at(bjetbin);//.getSignalContainer();
	//if(norm_nbjet_global)
	signal=signal.getIntegralBin(includeUFOF);
	histo1D signalintegral=signals.at(minbjetbin); //stack->at(minbjetbin).getSignalContainer();
	//if(norm_nbjet_global)
	signalintegral=signalintegral.getIntegralBin(includeUFOF);
	for(size_t i=minbjetbin+1;i<maxbjetbin;i++){
		histo1D tmpsig=signals.at(i); //stack->at(i).getSignalContainer();
		//if(norm_nbjet_global)
		tmpsig=tmpsig.getIntegralBin(includeUFOF);
		signalintegral += tmpsig;
	}


        if (!parent_->mttfit_){
            signalintegral_.resize(nBjetCat(),0);
            signalintegral_.at(bjetbin)=signals.at(bjetbin).integral(true);
        }
        else {
            signalintegrals_.at(mttbin).resize(nBjetCat(),0);
            signalintegrals_.at(mttbin).at(bjetbin)=signals.at(bjetbin).integral(true);
        }

	histo1D psmigint;
	for(size_t i=minbjetbin;i<maxbjetbin;i++){
		histo1D tmp=signalpsmig.at(i);
		tmp=tmp.getIntegralBin(includeUFOF);
		psmigint+=tmp;
	}
	histo1D visgenint;
	for(size_t i=minbjetbin;i<maxbjetbin;i++){
		histo1D tmp=signalvisPSgen.at(i);
		tmp=tmp.getIntegralBin(includeUFOF);
		visgenint+=tmp;
	}
        if (parent_->emuOnly_) visgenint *= (1. /((double)totalvisgencontsread_));
        else visgenint *= (3 /((double)totalvisgencontsread_ + 2.));
	//this also scales with lumi due to technical reasons. remove this dependence
	visgenint.setErrorZeroContaining("Lumi");

	//need signal integral per bin -> same distributions for fixed add jet multi...
	bool tmpmultiplyStatCorrelated = histoContent::multiplyStatCorrelated;
	bool tmpaddStatCorrelated = histoContent::addStatCorrelated;
	bool tmpdivideStatCorrelated = histoContent::divideStatCorrelated;
	histoContent::multiplyStatCorrelated = true;
	histoContent::addStatCorrelated = true;
	histoContent::divideStatCorrelated = true;


	histo1D one=signal.createOne();
	histo1D acceptance,leptonreco;
	if(visgenint.integral(true)>0){
		acceptance =visgenint*gennorm;
		leptonreco =signalintegral/visgenint;
	}
	else{
		std::cout << "Warning no visible phase space defined. Will merge acceptance and dilepton efficiencies -> ok for control plots" <<std::endl;
		leptonreco=signalintegral.createOne();
		acceptance=signalintegral*gennorm;
	}
	variateHisto1D tmp;
	tmp.import(acceptance);
	if (!parent_->mttfit_) acceptance_extr_=*tmp.getBin(1);
        else acceptance_extr_v_.at(mttbin)=*tmp.getBin(1);
	for(size_t i=0;i<parent_->addfullerrors_.size();i++){
		for(size_t j=0;j<parent_->addfullerrors_.at(i).indices.size();j++)
			acceptance.setErrorZeroContaining(acceptance.getSystErrorName( parent_->addfullerrors_.at(i).indices.at(j)));
	}
	tmp.import(acceptance);
	if (!parent_->mttfit_) acceptance_=*tmp.getBin(1);
        else acceptance_v_.at(mttbin)=*tmp.getBin(1);
	tmp.import(leptonreco);
	if (!parent_->mttfit_) eps_emu_=*tmp.getBin(1);
        else eps_emu_v_.at(mttbin)=*tmp.getBin(1);

        
	histo1D onebjetsignal = signals.at(bjet1bin);
	onebjetsignal=onebjetsignal.getIntegralBin(includeUFOF);
	histo1D twobjetsignal = signals.at(bjet2bin);
	twobjetsignal=twobjetsignal.getIntegralBin(includeUFOF);

	histo1D correction_b =  ((signalintegral * twobjetsignal) * 4.)
																																																																						                																																																																																																																																																																																																																																																																																																																																																																																																																																								/ ( (onebjetsignal + (twobjetsignal * 2.)) * (onebjetsignal + (twobjetsignal * 2.)));

	correction_b.removeStatFromAll();

	if (!parent_->mttfit_) container_c_b_.import(correction_b);
        else container_c_b_v_.at(mttbin).import(correction_b);

	histo1D eps_b = twobjetsignal / (correction_b * signalintegral);
	eps_b.sqrt();
	eps_b.removeStatFromAll();

	if (!parent_->mttfit_) container_eps_b_.import(eps_b);
        else container_eps_b_v_.at(mttbin).import(eps_b);

	histo1D omega_1=    (eps_b * 2.  - (correction_b * (eps_b * eps_b)) * 2.);
	omega_1.removeStatFromAll();
	histo1D omega_2=    (correction_b * eps_b) * eps_b ;
	omega_2.removeStatFromAll();
	histo1D omega_0=    (one - omega_1 - omega_2);
	omega_0.removeStatFromAll();

	variateHisto1D tmpvarc;
	if(bjetcategory == 0){
		tmpvarc.import(omega_0);
	}
	else if(bjetcategory == 1){
		tmpvarc.import(omega_1);
	}
	else if(bjetcategory == 2){
		tmpvarc.import(omega_2);
	}

        if (!parent_->mttfit_){
            if(omega_nbjet_.size() <= bjetbin)
		omega_nbjet_.resize(bjetbin+1);
            omega_nbjet_.at(bjetbin) = *tmpvarc.getBin(1);
        }
        else{
            if(omega_nbjet_v_.at(mttbin).size() <= bjetbin)
		omega_nbjet_v_.at(mttbin).resize(bjetbin+1);
            omega_nbjet_v_.at(mttbin).at(bjetbin) = *tmpvarc.getBin(1);
        }

	histoContent::multiplyStatCorrelated=tmpmultiplyStatCorrelated;
	histoContent::addStatCorrelated=tmpaddStatCorrelated;
	histoContent::divideStatCorrelated=tmpdivideStatCorrelated;

	variateHisto1D out;
	histo1D normedsignal = signals.at(bjetbin);//stack->at(bjetbin).getSignalContainer(); //for shape
	normedsignal.normalize(true,true); //normalize to 1 incl syst
	out.import(normedsignal);


	if(debug)
		std::cout << "ttbarXsecFitter::dataset::createLeptonJetAcceptance: done " << std::endl;
	return out;
}

const size_t & ttbarXsecFitter::dataset::xsecIdx()const{
	if(xsecidx_==9999){
		throw std::logic_error("ttbarXsecFitter::datasets::xsecIdx: first create index");
	}
	return xsecidx_;
}
const std::vector<size_t> & ttbarXsecFitter::dataset::xsecIdxs()const{
        if(xsecidxs_.size()<1){
		throw std::logic_error("ttbarXsecFitter::datasets::xsecIdxs: first create indices");
	}
	return xsecidxs_;
}
const size_t & ttbarXsecFitter::dataset::massIdx()const{
	if(massidx_==9999){
		throw std::logic_error("ttbarXsecFitter::datasets::massIdx: first create index");
	}
	return massidx_;
}
void ttbarXsecFitter::dataset::createXsecIdx(){
	//search for it
	if(signalconts_nbjets_.size()<1){
		throw std::logic_error("ttbarXsecFitter::datasets::createXsecIdx: first read-in stacks");
	}
	std::vector<TString> sysnames=signalconts_nbjets_.at(0).getSystNameList();
	size_t idx=std::find(sysnames.begin(),sysnames.end(),"Xsec_"+name_)-sysnames.begin();
	if(idx == sysnames.size())
		throw std::logic_error("ttbarXsecFitter::datasets::createXsecIdx: index not found");
	xsecidx_=idx;
}
void ttbarXsecFitter::dataset::createXsecIdxs(){
	//search for it
	if(signalconts_nbjets_v_.size()<1){
		throw std::logic_error("ttbarXsecFitter::datasets::createXsecIdx: first read-in stacks");
	}
        for (size_t s=0; s<signalconts_nbjets_v_.size(); ++s){
            if(signalconts_nbjets_v_.at(s).size()<1){
		throw std::logic_error("ttbarXsecFitter::datasets::createXsecIdx: first read-in stacks");
            }
        }
	std::vector<TString> sysnames=signalconts_nbjets_v_.at(0).at(0).getSystNameList();

        std::vector<size_t> idxs;
        for (size_t s=0; s<signalconts_nbjets_v_.size(); ++s){
            size_t idx=std::find(sysnames.begin(),sysnames.end(),"Xsec_"+name_+toString(s+1))-sysnames.begin();
            if(idx == sysnames.size())
                throw std::logic_error("ttbarXsecFitter::datasets::createXsecIdx: index not found");
            idxs.push_back(idx);
        }

	xsecidxs_=idxs;
}
void ttbarXsecFitter::dataset::createMassIdx(){
	//search for it
        if (!parent_->mttfit_){
            if(signalconts_nbjets_.size()<1)
                throw std::logic_error("ttbarXsecFitter::datasets::createMassIdx: first read-in stacks");
        }
        else{
            for (size_t s=0; s<signalconts_nbjets_v_.size(); ++s){
                if(signalconts_nbjets_v_.at(s).size()<1)
                    throw std::logic_error("ttbarXsecFitter::datasets::createMassIdx: first read-in stacks");
            }
        }
	std::vector<TString> sysnames;
        if (!parent_->mttfit_) sysnames=signalconts_nbjets_.at(0).getSystNameList();
        else sysnames=signalconts_nbjets_v_.at(0).at(0).getSystNameList();
        

	size_t idx=std::find(sysnames.begin(),sysnames.end(),"TOPMASS")-sysnames.begin();
	if(idx == sysnames.size())
		throw std::logic_error("ttbarXsecFitter::datasets::createMassIdx: index not found");
	massidx_=idx;
}

void  ttbarXsecFitter::dataset::readStacks(const std::string configfilename,const std::pair<TString,
		TString>&replaceininfiles,bool removesyst, std::vector<std::pair<TString, double> >& priorcorr){
	if(debug)
		std::cout << "ttbarXsecFitter::dataset::readStacks" <<std::endl;

	fileReader fr;
	fr.setComment("$");
	fr.setDelimiter("|");
	//8 TeV
	fr.setStartMarker(("[ "+name_+ " ]").Data());
	fr.setEndMarker(("[ end - "+name_+ " ]").Data());
	fr.readFile(configfilename);
	std::vector<std::vector<histoStack> >  out;
	totalvisgencontsread_=0;

	std::cout << "reading and preparing " << name_ << " stacks..." <<std::endl;
	size_t bjetcount=0,jetcount=0,oldbjetcount=0;
	TString oldfilename="";
	histoStackVector csv;
	std::vector<histoStack> tmpstacks;
	for(size_t line=0;line<fr.nLines();line++){
		if(!debug)
			displayStatusBar(line,fr.nLines());
		if(fr.nEntries(line) < 1) continue;
		if(fr.getData<TString>(line,0) == "bjet++"){
			bjetcount++;
			jetcount=0;
			continue;
		}
		if(fr.nEntries(line) < 2 || ! fr.nEntries(line) % 2){
			std::cout << "ttbarXsecFitter::readInput: Line format wrong:\n" << fr.getReJoinedLine(line) << std::endl;
		}
		if(oldbjetcount!=bjetcount){
			out.push_back(tmpstacks);
			oldbjetcount=bjetcount;
			tmpstacks.clear();
		}

		inputstacks_.resize(nBjetCat(),std::vector<histoStack>());

		for(size_t entry=0;entry<fr.nEntries(line);entry++){
			TString filename=fr.getData<TString>(line,entry);
			if(replaceininfiles.first.Length()>0)
				filename.ReplaceAll(replaceininfiles.first,replaceininfiles.second);
			entry++;
			TString plotname=fr.getData<TString>(line,entry);
			bool newcsv=oldfilename!=filename;
			oldfilename=filename;
			if(newcsv)
				csv.readFromFile(filename.Data()); //new file

			//csv.listStacks();
			histoStack tmpstack;
			try{
				tmpstack=csv.getStack(plotname);
			}
			catch(std::exception &ex){
				csv.listStacks();
				ex.what();
				throw std::runtime_error("stack not found");
			}
			if(!tmpstack.is1DUnfold())
				throw std::runtime_error("ttbarXsecFitter::readStacks: Stack is has no unfolding information!");
			if(tmpstack.getContribution1DUnfold("data").isDummy())
				throw std::runtime_error("ttbarXsecFitter::readStacks: Stack has no data entry!");
			if(tmpstack.getSignalIdxs().size() <1)
				throw std::runtime_error("ttbarXsecFitter::readStacks: No signal defined!");

                        // std::cout<<plotname<<std::endl;

                        if (plotname == "m_lb min 1,1 b-jets step 8") tmpstack = tmpstack.rebinXToBinning({20,48,76,104,132,160});
                        else if (plotname == "m_lb min 1,2 b-jets step 8") tmpstack = tmpstack.rebinXToBinning({20,48,76,104,132,160});
                        else if (plotname == "lead jet pt 0,1 b-jets step 8") tmpstack = tmpstack.rebinXToBinning({30,50,100,200}); 
                        else if (plotname == "second jet pt 0,2 b-jets step 8") tmpstack = tmpstack.rebinXToBinning({30,50,120,200}); 
                        else if (plotname == "third jet pt 0,3 b-jets step 8") tmpstack = tmpstack.rebinXToBinning({30,60,200});
                        // else if (plotname == "third jet pt 1,3 b-jets step 8") tmpstack = tmpstack.rebinXToBinning({30,40,50,200});
                        // else if (plotname == "lead jet pt 2,1 b-jets step 8") tmpstack = tmpstack.rebinXToBinning({30,50,90,160}); 
                        // else if (plotname == "second jet pt 2,2 b-jets step 8") tmpstack = tmpstack.rebinXToBinning({30,45,100,300}); 
                        else if (plotname == "m_lb min step 8") tmpstack = tmpstack.rebinXToBinning({20,50,75,105,130,160});
                        else if (plotname == "m_lb min test step 8") tmpstack = tmpstack.rebinXToBinning({20,50,75,105,130,160});
                        else if (plotname == "last jet pt 2 b-jets step 8") tmpstack = tmpstack.rebinXToBinning({30,40,50,60,70,80,90,100,120,140,200});
                        else if (plotname == "last jet pt 2 b-jets mtt1 step 8") tmpstack = tmpstack.rebinXToBinning({30,40,60,200});
                        else if (plotname == "last jet pt 2 b-jets mtt2 step 8") tmpstack = tmpstack.rebinXToBinning({30,50,70,100,200});
                        else if (plotname == "last jet pt 2 b-jets mtt3 step 8") tmpstack = tmpstack.rebinXToBinning({30,50,70,120,200});

                        inputstacks_.at(bjetcount).push_back(tmpstack); //BEFORE ADDIND UNC!
                        
			addUncertainties(&tmpstack,bjetcount,removesyst,priorcorr);


			/*	if(newcsv) //not necessary if same csv where everything is ordered in the same manner
				for(size_t i=0;i<out.size();i++){
					tmpstack.equalizeSystematicsIdxs(out.at(i));
				}
			 */
			tmpstacks.push_back(tmpstack);
			//stack=stack.append(tmpstack);
			//	std::cout << "ttbarXsecFitter::readStacks: added "
			//			<< " stack(s) for nbjets: " << bjetcount << " njets: " << jetcount <<std::endl;
		}
		jetcount++;
	}
	if(out.size()>0)
		out.push_back(tmpstacks);//last push back
	if(out.size()<1)
		std::cout << "ttbarXsecFitter::readStacks: no plots for " << name_ << " TeV added." <<std::endl;
	else if(out.size() != 3)
		throw std::runtime_error("ttbarXsecFitter::readStacks: tried to add less or more than (allowed) 3 nbjets categories");

	for(size_t nbjet=0;nbjet<out.size();nbjet++)
		readStackVec(out.at(nbjet),nbjet);

	if(debug){
		std::cout << " ttbarXsecFitter::dataset::readStacks: done" <<std::endl;
	}

}


void ttbarXsecFitter::dataset::readStack(const histoStack& stack, size_t nbjet){
	std::vector<histoStack> vec;
	vec.push_back(stack);
	readStackVec(vec,nbjet);
}
void ttbarXsecFitter::dataset::readStackVec(std::vector<histoStack> & in,size_t nbjet){
	if(nbjet > 2){
		throw std::out_of_range("ttbarXsecFitter::dataset::readStackVec: nbjet out of range");
	}
	if(debug)
		std::cout << "ttbarXsecFitter::dataset::readStackVec: " << nbjet <<std::endl;
        
	size_t maxnbjetcat=dataset::nBjetCat();
        if (!parent_->mttfit_){
            signalconts_nbjets_.resize(maxnbjetcat);
            signalvisgenconts_nbjets_.resize(maxnbjetcat);
            signalpsmigconts_nbjets_.resize(maxnbjetcat);
        }
        else{
            signalconts_nbjets_v_.resize(n_signals); //hardcoded
            signalvisgenconts_nbjets_v_.resize(n_signals); //hardcoded
            signalpsmigconts_nbjets_v_.resize(n_signals); //hardcoded
            for (size_t s=0; s<signalconts_nbjets_v_.size(); ++s){
                signalconts_nbjets_v_.at(s).resize(maxnbjetcat);
                signalvisgenconts_nbjets_v_.at(s).resize(maxnbjetcat);
                signalpsmigconts_nbjets_v_.at(s).resize(maxnbjetcat);
            }
        }
	backgroundconts_nbjets_.resize(maxnbjetcat);
	dataconts_nbjets_.resize(maxnbjetcat);

        if (parent_->doToys_) 
            backgroundconts_split_nbjets_.resize(maxnbjetcat);
        
	histo1D sign,signvisps,signpsmig,backgr,data;
        std::vector<histo1D> bgs;
        std::vector<histo1D> sign_v,signvisps_v,signpsmig_v;

	for(size_t j=0;j<in.size();j++){
            std::vector<TString> signals;
            if (parent_->mttfit_){
                for (size_t s=1; s<=n_signals; ++s) signals.push_back((TString)"t#bar{t}mtt"+toString(s)); //hardcoded
                in.at(j).setsignals(signals);
            }
            // if (parent_->mttfit_) in.at(j).setsignal((TString)"t#bar{t}mtt"+toString(parent_->mttbin_));
            
		if(in.at(j).is1DUnfold()){
                    if (!parent_->mttfit_){
			sign          =sign.append(in.at(j).getSignalContainer1DUnfold().getRecoContainer());
			signvisps     =signvisps.append(in.at(j).getSignalContainer1DUnfold().getGenContainer());
			signpsmig=signpsmig.append(in.at(j).getSignalContainer1DUnfold().getBackground());
                    }

                    else{
                        size_t nsignals = signals.size();
                        sign_v.resize(nsignals);
                        signvisps_v.resize(nsignals);
                        signpsmig_v.resize(nsignals);

                        for (size_t s=0; s<nsignals; ++s){
                            sign_v.at(s)          = sign_v.at(s).append(in.at(j).getSignalContainer1DUnfold({(TString)"t#bar{t}mtt"+toString(s+1)}).getRecoContainer());
                            signvisps_v.at(s)     = signvisps_v.at(s).append(in.at(j).getSignalContainer1DUnfold({(TString)"t#bar{t}mtt"+toString(s+1)}).getGenContainer());
                            signpsmig_v.at(s)     = signpsmig_v.at(s).append(in.at(j).getSignalContainer1DUnfold({(TString)"t#bar{t}mtt"+toString(s+1)}).getBackground());
                        }
                    }
		
                    totalvisgencontsread_++;

		}
		else if(in.at(j).is1D()){
                    if (!parent_->mttfit_){
			sign          =sign.append(in.at(j).getSignalContainer());
			signvisps     =signvisps.append(in.at(j).getSignalContainer());
			signvisps.setAllZero();
			signpsmig     =sign;
                    }
                    else{
                        size_t nsignals = signals.size();
                        sign_v.resize(nsignals);
                        signvisps_v.resize(nsignals);
                        signpsmig_v.resize(nsignals);

                        for (size_t s=0; s<nsignals; ++s){
                            in.at(j).setsignal((TString)"t#bar{t}mtt"+toString(s+1));
                            sign_v.at(s)          = sign_v.at(s).append(in.at(j).getSignalContainer());
                            signvisps_v.at(s)     = signvisps_v.at(s).append(in.at(j).getSignalContainer());
                            signvisps_v.at(s).setAllZero();
                            signpsmig_v.at(s)     = signpsmig_v.at(s) = sign_v.at(s);
                        }
                        in.at(j).setsignals(signals);


                    }
                    
			if(debug)
				std::cout << "read 1D histo" <<std::endl;
		}
		backgr      =backgr.append(in.at(j).getBackgroundContainer());
		data          =data.append(in.at(j).getDataContainer());
                
                if (!parent_->doToys_) continue;
                
                if (j==0 && nbjet==1)
                    backgroundlegends_ = in.at(j).getBackgroundLegends();
                
                if (j==0) bgs = in.at(j).getBackgroundContainers();
                else {
                    std::vector<histo1D> tmp_bgs = in.at(j).getBackgroundContainers();  
                    for (size_t t=0; t<bgs.size(); ++t){
                        bgs.at(t) = bgs.at(t).append(tmp_bgs.at(t));
                    }
                }

	}
        if (!parent_->mttfit_){
            signalconts_nbjets_.at(nbjet) = sign;
            signalvisgenconts_nbjets_.at(nbjet)=signvisps;
            signalpsmigconts_nbjets_.at(nbjet)=signpsmig;
        }
        else{
            for (size_t s=0; s<sign_v.size(); ++s){
                signalconts_nbjets_v_.at(s).at(nbjet) = sign_v.at(s);
                signalvisgenconts_nbjets_v_.at(s).at(nbjet)=signvisps_v.at(s);
                signalpsmigconts_nbjets_v_.at(s).at(nbjet)=signpsmig_v.at(s);            
            }
        }
	backgroundconts_nbjets_.at(nbjet) = backgr;
	dataconts_nbjets_.at(nbjet)=data;

        if (parent_->doToys_)
            backgroundconts_split_nbjets_.at(nbjet) = bgs;

}

/*
 * Assumes consistent ordering within same dataset - is the case if input is read normally
 */
void ttbarXsecFitter::dataset::equalizeIndices(dataset & rhs){

	histo1D::equalizeSystematics(signalconts_nbjets_,rhs.signalconts_nbjets_);
	histo1D::equalizeSystematics(signalvisgenconts_nbjets_,rhs.signalvisgenconts_nbjets_);
	histo1D::equalizeSystematics(signalpsmigconts_nbjets_,rhs.signalpsmigconts_nbjets_);
	histo1D::equalizeSystematics(dataconts_nbjets_,rhs.dataconts_nbjets_);
	histo1D::equalizeSystematics(backgroundconts_nbjets_,rhs.backgroundconts_nbjets_);


}
void ttbarXsecFitter::dataset::adaptIndices(const dataset & rhs){
	for(size_t i=0;i<signalconts_nbjets_.size();i++){
            if (!parent_->mttfit_){
		signalconts_nbjets_.at(i).adaptSystematicsIdxs(rhs.signalconts_nbjets_.at(i));
		signalvisgenconts_nbjets_.at(i).adaptSystematicsIdxs(rhs.signalvisgenconts_nbjets_.at(i));
		signalpsmigconts_nbjets_.at(i).adaptSystematicsIdxs(rhs.signalpsmigconts_nbjets_.at(i));
            }
            else{
                for (size_t s=0; s<signalconts_nbjets_v_.size(); ++s){
                    signalconts_nbjets_v_.at(s).at(i).adaptSystematicsIdxs(rhs.signalconts_nbjets_v_.at(s).at(i));
                    signalvisgenconts_nbjets_v_.at(s).at(i).adaptSystematicsIdxs(rhs.signalvisgenconts_nbjets_v_.at(s).at(i));
                    signalpsmigconts_nbjets_v_.at(s).at(i).adaptSystematicsIdxs(rhs.signalpsmigconts_nbjets_v_.at(s).at(i));                   
                }
            }
            dataconts_nbjets_.at(i).adaptSystematicsIdxs(rhs.dataconts_nbjets_.at(i));
            backgroundconts_nbjets_.at(i).adaptSystematicsIdxs(rhs.backgroundconts_nbjets_.at(i));
	}
}
size_t ttbarXsecFitter::getDatasetIndex(const TString & name)const{
	for(size_t i=0;i<datasets_.size();i++)
		if(name==datasets_.at(i).getName())
			return i;
	throw std::runtime_error("ttbarXsecFitter::getDatasetIndex: dataset name not found");
}

void ttbarXsecFitter::dataset::addUncertainties(histoStack * stack,size_t nbjets,bool removesyst,
		const std::vector<std::pair<TString,double> >& priorcorrcoeff)const{ //can be more sophisticated

	if(debug)
		std::cout << "ttbarXsecFitter::dataset::addUncertainties: " <<name_ << ", "<< nbjets <<std::endl;

	// std::vector<TString> dymerges;
	// dymerges.push_back("DY#rightarrow#tau#tau");
	// dymerges.push_back("DY#rightarrowll");
	// //excludefromglobalvar.push_back("Z#rightarrowll");
	// stack->mergeLegends(dymerges,"DY",432,false);

	std::vector<TString> excludefromglobal;
	//excludefromglobal.push_back("t#bar{t}V");
        excludefromglobal.push_back("DY");

	if(parent_->topontop_){
		stack->setLegendOrder("t#bar{t}",90);
		stack->setLegendOrder("tW/#bar{t}W",89);
		stack->setLegendOrder("DY",88);
		stack->setLegendOrder("VV",87);
		stack->setLegendOrder("QCD/Wjets",86);
		//stack->setLegendOrder("t#bar{t}V",85);
		stack->setLegendOrder("t#bar{t}bg",84);
	}


	//debug
	//	stack->mergeLegends("t#bar{t}(#tau)","t#bar{t}","t#bar{t}",633,true);

	if(debug)
		std::cout << "ttbarXsecFitter::addUncertainties: merged DY" <<std::endl;

	try{
		stack->mergeVariationsFromFileInCMSSW(parent_->mergesystfile_);
	}
	catch(...){}
	if(debug)
		std::cout << "ttbarXsecFitter::addUncertainties: merged variations" <<std::endl;

        //more sophisticated approaches can be chosen here - like DY float etc
        stack->addRelErrorToBackgrounds(0.3,false,"BG",excludefromglobal);


	if(getName().Contains("7TeV"))
		stack->addRelErrorToContribution(0.6,"t#bar{t}V","BG_",true);
	else if(getName().Contains("8TeV"))
		stack->addRelErrorToContribution(0.3,"t#bar{t}V","BG_",true);

	if(debug)
		std::cout << "ttbarXsecFitter::addUncertainties: added t#bar{t}V var" <<std::endl;

        // stack->addRelErrorToContribution(0.3,"Wjets","BG_");

	float dy0bjetserr=0,dy1bjetserr=0,dy2bjetserr=0;
	if(nbjets==0)
		dy0bjetserr=0.3;
	else if(nbjets==1)
		dy1bjetserr=0.3;
	else if(nbjets==2)
		dy2bjetserr=0.3;

        int naddjets = -1;
        TString stackName = stack->getName();
        if (stackName.Contains(",0 b-jets")) naddjets=0;
        else if (stackName.Contains(",1 b-jets")) naddjets=1;
        else if (stackName.Contains(",2 b-jets")) naddjets=2;
        else if (stackName.Contains(",3 b-jets")) naddjets=3;
        else std::cerr<<"WARNING: undefined number of additional jets"<<std::endl;

	float dy0jetserr=0, dy1jetserr=0, dy2jetserr=0, dy3jetserr=0;

        if (nbjets==0){
            if      (naddjets==0) dy0jetserr=0.05;
            else if (naddjets==1) dy1jetserr=0.1;
            else if (naddjets==2) dy2jetserr=0.3;
            else if (naddjets==3) dy3jetserr=0.5;
        }
        else if (nbjets==1){
            if      (naddjets==0) dy0jetserr=0.1;
            else if (naddjets==1) dy1jetserr=0.3;
            else if (naddjets==2) dy2jetserr=0.5;
            else if (naddjets==3) dy3jetserr=0.5;
        }
        else if (nbjets==2){
            if      (naddjets==0) dy0jetserr=0.3;
            else if (naddjets==1) dy1jetserr=0.5;
            else if (naddjets==2) dy2jetserr=0.5;
            else if (naddjets==3) dy3jetserr=0.5;
        }

	//hardcoded scaling of QCD/Wjets....
	try{
		int wjetsqcdidx=stack->getContributionIdx("QCD/Wjets");
		stack->multiplyNorm(wjetsqcdidx,parent_->wjetsrescalefactor_);
	}
	catch(...){
		if(parent_->wjetsrescalefactor_!=(float)1.){
			std::cout << "Warning. wjetsrescalefactor!=1 but no QCD/Wjets contribution found" <<std::endl;
		}
	}

        stack->addRelErrorToContribution(dy0bjetserr,"DY","BG_0_bjets_");
        stack->addRelErrorToContribution(dy1bjetserr,"DY","BG_1_bjets_");
        stack->addRelErrorToContribution(dy2bjetserr,"DY","BG_2_bjets_");

        if (naddjets>=0 && !parent_->mttfit_){
            stack->addRelErrorToContribution(dy0jetserr,"DY","BG_0_addjets_");
            stack->addRelErrorToContribution(dy1jetserr,"DY","BG_1_addjets_");
            stack->addRelErrorToContribution(dy2jetserr,"DY","BG_2_addjets_");
            stack->addRelErrorToContribution(dy3jetserr,"DY","BG_3_addjets_");
        }

	if(debug)
		std::cout << "ttbarXsecFitter::addUncertainties: added DY var" <<std::endl;

	std::vector<TString> allsys=stack->getDataContainer().getSystNameList();

	float addlumiunc=0;
	addlumiunc=unclumi_/100;
	if(!getName().Contains("13TeV"))stack->addGlobalRelMCError("Lumi" ,addlumiunc);
	stack->addGlobalRelMCError("Lumi" ,addlumiunc);
	// stack->addGlobalRelBGError("Lumi_BG" ,addlumiunc);


	if(debug)
		std::cout << "ttbarXsecFitter::addUncertainties: added lumi var" <<std::endl;

	if(removesyst){
               if(std::find(allsys.begin(),allsys.end(),"TOPMASS")!=allsys.end())
			stack->removeAllSystematics("TOPMASS");

		else
			stack->removeAllSystematics();
		//}catch(...){}

		if(debug)
			std::cout << "ttbarXsecFitter::addUncertainties: removed syst" <<std::endl;
	}
	if(debug){
		std::vector<TString> names=stack->getContainer(0).getSystNameList();
		for(size_t i=0;i<names.size();i++)
			std::cout << names.at(i) <<std::endl;
	}

	//split uncertainties here
	if(!removesyst){
		for(size_t i=0;i<priorcorrcoeff.size();i++){
			double corrcoeff = priorcorrcoeff.at(i).second * priorcorrcoeff.at(i).second;
			if(debug)
				std::cout << priorcorrcoeff.at(i).second
				<< " " << priorcorrcoeff.at(i).first << " "
				<< priorcorrcoeff.at(i).first+"_"+getName() << std::endl;
			stack->splitSystematic(priorcorrcoeff.at(i).first,corrcoeff,priorcorrcoeff.at(i).first,priorcorrcoeff.at(i).first+"_"+getName());
		}
	}

	if(debug)
		std::cout << "ttbarXsecFitter::addUncertainties: split syst done" <<std::endl;

	//fake uncertainty
	if (!parent_->mttfit_) stack->addGlobalRelMCError("Xsec_"+name_,0);
        else {
            for (size_t s=1; s<=n_signals; ++s) //hardcoded
                stack->addGlobalRelMCError("Xsec_"+name_+toString(s),0);
        }
	if(debug)
		std::cout << "ttbarXsecFitter::addUncertainties: done" <<std::endl;



}

const std::vector<histoStack>& ttbarXsecFitter::dataset::getOriginalInputStacks(const size_t & nbjet)const{
	if(nbjet>=inputstacks_.size())
		throw std::out_of_range("ttbarXsecFitter::dataset::getOriginalInputStacks: nbjet too large or no stacks read");

	return inputstacks_.at(nbjet);
}


TString ttbarXsecFitter::translatePartName(const formatter& fmt,const TString& name)const{
	TString namecp=name;
	TString datasetname="";
	bool wasin=false;
	for(size_t i=0;i<datasets_.size();i++){
		if(namecp.EndsWith(datasets_.at(i).getName())){
			wasin=true;
			datasetname=datasets_.at(i).getName();
			namecp.ReplaceAll("_"+datasetname,"");
			break;
		}
	}
	if(wasin)
		return fmt.translateName(namecp) + " ("+datasetname+")";
	else
		return fmt.translateName(namecp);
}
bool ttbarXsecFitter::isXSecIdx(const size_t& idx)const{
	for(size_t i=0;i<datasets_.size();i++){
            if (!mttfit_){
		if(idx == datasets_.at(i).xsecIdx())
                    return true;
            }
            else{
                std::vector<size_t> idxs = datasets_.at(i).xsecIdxs();
                if (std::find(idxs.begin(),idxs.end(),idx) != idxs.end())
                    return true;
            }
	}
	return false;
}


}
