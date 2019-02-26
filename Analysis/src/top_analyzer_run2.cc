/*
 * analyzer_run2.cc based on former eventLoop.h
 *
 *  Created on: Jul 17, 2013
 *      Author: kiesej
 *
 *
 *      this file defines the analyze() function of MainAnalyzer and is put in a
 *      separate file only for structure reasons!
 *      Do not include it in any other file than MainAnalyzer.cc
 */

#include <TtZAnalysis/Analysis/interface/top_analyzer_run2.h>
#include "../../DataFormats/interface/NTJES.h"
#include <TtZAnalysis/Analysis/interface/ttbarGenPlots.h>

/*
 * Running on the samples is parallelized.
 * This function is called for each sample individually.
 *
 * Do not just return from this function, you must give a report to
 * the parent process, why and that the function returned.
 *
 * This is automatically done after the event loop during the process of writing
 * output.
 * In case you want to indicate an abort due to an error, use the following syntax
 * if(error){
 *    //cout something in addition or so...
 *    reportError(<error code>, anaid);
 *    return;
 * }
 * This way the main program knows what happened and indicates that in the summary.
 * If you do not return this way, the main program will be stuck in an infinite loop.
 * (Sleeping 99.99% of the time)
 * Error codes will always be negative. Positive values are converted
 *
 * Please indicate the meaning of the error code in the cout at the end of ../app_src/analyse.cc
 */
void  top_analyzer_run2::analyze(size_t anaid){


	using namespace std;
	using namespace ztop;




	bool isMC=true;
	if(legendname_==datalegend_) isMC=false;

	if(!isMC || !signal_)
		getPdfReweighter()->switchOff(true);

        if (doGenPlotsOnly_ && doKinReco_ && !legendname_.Contains("mtt1")) return;

	//some mode options
	/* implement here not to overload MainAnalyzer class with private members
	 *  they are even allowed to overwrite existing configuration
	 *  with the main purpose of trying new things
	 *
	 *  each option has to start with a CAPITAL letter and must not contain other capital letters
	 *  This allows for parsing of more than one option at once
	 *
	 *  These options should get a cleanup once in a while
	 *  The standard configuration should be visible on top
	 */

	TString mettype="NTT0T1TxyMet";
	TString electrontype="NTPFElectrons";
	bool mode_samesign=false;
	bool mode_invertiso=false;
	bool usemvamet=false;
	bool onejet=false;
	bool zerojet=false;
	bool usetopdiscr=false;
	bool nometcut=false;
	bool nozcut=false;
	bool nobcut=false;

	float normmultiplier=1; //use in case modes need to change norm


	if(mode_.Contains("Samesign")){
		mode_samesign=true;
		std::cout << "entering same sign mode" <<std::endl;
	}

	if(mode_.Contains("Invertiso")){
		mode_invertiso=true;
		std::cout << "entering invert iso mode" <<std::endl;
	}
	if(mode_.Contains("Mvamet")){
		mettype="NTMvaMet";
		usemvamet=true;
		std::cout << "entering mvamet mode" <<std::endl;
	}
	if(mode_.Contains("Pfmet")){
		mettype="NTMet";
		std::cout << "entering pfmet mode" <<std::endl;
	}
	if(mode_.Contains("T0t1met")){
		mettype="NTT0T1Met";
		std::cout << "entering t0t1met mode" <<std::endl;
	}
	if(mode_.Contains("T1txymet")){
		mettype="NTT1TxyMet";
		std::cout << "entering t1txymet mode" <<std::endl;
	}
	if(mode_.Contains("Gsfelectrons")){
		electrontype="NTElectrons";
		std::cout << "entering gsfelectrons mode" <<std::endl;
	}
	if(mode_.Contains("Pfelectrons")){
		electrontype="NTPFElectrons";
		std::cout << "entering pfelectrons mode" <<std::endl;
	}

	if(mode_.Contains("Onejet")){
		onejet=true;
		std::cout << "entering Onejet mode" <<std::endl;
	}
	if(mode_.Contains("Zerojet")){
		zerojet=true;
		std::cout << "entering Zerojet mode" <<std::endl;
	}
	if(mode_.Contains("Nometcut")){
		nometcut=true;
		std::cout << "entering Nometcut mode" <<std::endl;
	}
	if(mode_.Contains("Nozcut")){
		nozcut=true;
		std::cout << "entering Nozcut mode" <<std::endl;
	}
	if(mode_.Contains("Nobcut")){
		nobcut=true;
		std::cout << "entering Nobcut mode" <<std::endl;
	}
	if(mode_.Contains("Topdiscr")){
		usetopdiscr=true;
		std::cout << "entering Topdiscr mode" <<std::endl;
	}
	//agrohsje: taken from Jan analyzer_run1 
	bool isdileptonexcl=false;
	if(inputfile_.Contains("ttbar_dil") || inputfile_.Contains("ttbarviatau_dil"))
		isdileptonexcl=true;

	//adapt wrong MG BR for madspin and syst samples
	if((inputfile_.Contains("ttbar.root")
			|| inputfile_.Contains("ttbarviatau.root")
			|| inputfile_.Contains("ttbar_")
			|| inputfile_.Contains("ttbarviatau_") )
			&&! isdileptonexcl
			//agrohsje
			&& inputfile_.Contains("_mgbr")){
		//agrohsje uncomment
		//if(  ! (syst_.BeginsWith("TT_GEN") && syst_.EndsWith("_up"))  ) //other generator
		normmultiplier=(0.104976/0.11104);//correct both W
		/*
		 * BR W-> lnu: 0.1086 take 0.108 as default is Powheg in run ii
		 * n_comb for leptonic: 1+1+1+2+2+2 (incl taus)
		 * total lept branching fraction for WW: 0.1086^2 * 9 = 0.1062 // 0.108^2 * 9 = 0.104976
		 * In Madgraph: 0.11104
		 */
	}
        //check which datastream the File Uses
        isSingleMu = false;
        isDilep = false;
        isSingleEle= false;
        if(inputfile_.Contains("SingleMuon")) isSingleMu=true;
        else if (inputfile_.Contains("SingleElectron")) isSingleEle=true;
        else if (inputfile_.Contains("MuonEG") || inputfile_.Contains("DoubleEG") || inputfile_.Contains("DoubleMuon") ) isDilep= true;
        
        // Check for which Runs is used
        isRunBtoD = false ;
        isRunEtoF = false;
        isRunFtoG = false;
        isRunH = false;
        bool isTtbarLike = false;
        bool isTtbar = false;
            
        if(!isMC){
                 if(inputfile_.Contains("RunB") || inputfile_.Contains("RunC") || inputfile_.Contains("RunD")) isRunBtoD=true;
                 else if(inputfile_.Contains("RunE") || inputfile_.Contains("RunF_v1")) isRunEtoF= true;
                 else if(inputfile_.Contains("RunF_v2") || inputfile_.Contains("RunG")) isRunFtoG=true;
                 else if(inputfile_.Contains("RunH") ) isRunH=true;
        }
        else {
                if(inputfile_.Contains("ttbar")||inputfile_.Contains("tbarW")||inputfile_.Contains("tW")) isTtbarLike=true;
                if(inputfile_.Contains("ttbar")) isTtbar=true;
        }
 
        //std::cout<<isSingleMu<<isDilep<<isSingleEle<<std::endl;
        
        
	// for pure dileptonic samples 
	if(isdileptonexcl || inputfile_.Contains("_mgdecays_") || inputfile_.Contains("_tbarWtoLL")|| inputfile_.Contains("_tWtoLL")){
		normmultiplier=0.1062; //fully leptonic branching fraction for both Ws
	}

	bool fakedata=false,isfakedatarun=false;
	if(mode_.Contains("Fakedata")){
		if(legendname_ ==  datalegend_)
			fakedata=true;
		isfakedatarun=true;
		isMC=true;
		std::cout << "THIS IS A PSEUDO-DATA RUN:" <<std::endl;// all samples MC samples will be used without weights!" << std::endl;

	}
	bool nosplitforfakedata=false;
	if(mode_.Contains("Nosplit")){
		nosplitforfakedata=true;
	}

	if(mode_.Contains("Notoppt")){
		/*	if(getTopPtReweighter()->getSystematic() != reweightfunctions::up
				|| fakedata)
			getTopPtReweighter()->setFunction(reweightfunctions::flat);
		else if(getTopPtReweighter()->getSystematic() == reweightfunctions::up)
			getTopPtReweighter()->setSystematics(reweightfunctions::nominal);
		 */
		if(getTopPtReweighter()->getSystematic() != reweightfunctions::up) //switch off for all BUT...
			getTopPtReweighter()->switchOff(true);
		else //...for "up" set "nominal" variation to data
			getTopPtReweighter()->setSystematics(reweightfunctions::nominal);

	}
	bool apllweightsone=false;
	if(mode_.Contains("Noweights")){
		apllweightsone=true;
	}

	bool leppt30=false;
	if(mode_.Contains("Leppt30")){
		leppt30=true;
	}
	//if(leppt30) leppt30=false;
	const float lepptthresh= leppt30 ? 30 : 20;

	float jetptcut;
	if(mode_.Contains("Jetpt40")){
		jetptcut=40;
	}
	else if(mode_.Contains("Jetpt50")){
		jetptcut=50;
	}
	else if(mode_.Contains("Jetpt60")){
		jetptcut=60;
	}
        else if(mode_.Contains("Jetpt20")){
                jetptcut=20;
        }
        else if(mode_.Contains("Jetpt25")){
                jetptcut=25;
        }
	else{
		jetptcut=30;
	}

	float mllcut = 20.;

	/*
	 * end of mode switches
	 */
	//bool wasbtagsys=false;
	//fake data configuration
	if(fakedata){
		//this whole section is bad style because the configuration should be done from the outside
		//e.g. in analyse.cc
		//but for pseudo data there needs to be an additional case distinction in addition to MC/data

		//turn off systematic variations here

		getTriggerSF()->setSystematics("nom");

		getElecEnergySF()->setSystematics("nom");
		getElecEnergyResolutionSF_rho()->setSystematics("nom");
		getElecEnergyResolutionSF_phi()->setSystematics("nom");
		getAdditionalJEC()->setSystematics("nom");
		getElecSF()->setSystematics("nom");
		getMuonEnergySF()->setSystematics("nom");
		getMuonSF()->setSystematics("nom");

		getTrackingSF()->setSystematics("nom");

		// getPUReweighter()-> //setSystematics("nom");
		getBTagSF()->setSystematics(NTBTagSF::nominal);
		getJECUncertainties()->setSystematics("no");
		getJERAdjuster()->setSystematics("def_2016");

		getTopPtReweighter()->setSystematics(reweightfunctions::nominal); //setSystematics("nom");

		getPdfReweighter()->switchOff(true);

		//for other parameters reread config
		fileReader fr;
		fr.setComment("$");
		fr.setDelimiter(",");
		fr.setStartMarker("[parameters-begin]");
		fr.setEndMarker("[parameters-end]");
		fr.readFile(pathtoconffile_.Data());

		getPUReweighterGH()->setDataTruePUInput(((std::string)getenv("CMSSW_BASE")+fr.getValue<string>("PUFile") +".root").data());


		// not needed anymore
		//re-adjust systematic filenames
		for(size_t i=0;i<ftorepl_.size();i++){
			if(inputfile_.EndsWith(ftorepl_.at(i))){
				std::cout << "replacing fakedata syst names " << fwithfix_.at(i) << " with " << ftorepl_.at(i) << std::endl;
				//	inputfile_.ReplaceAll(fwithfix_.at(i),ftorepl_.at(i));
			}
		}

		//this is bad style.. anyway
	}


	//per sample configuration


	//check if file exists
	if(testmode_)
		std::cout << "testmode("<< anaid << "): check input file "<<inputfile_ << " (isMC)"<< isMC << std::endl;



	NTFullEvent evt;

	//just a test

	if(testmode_)
		std::cout << "testmode("<< anaid << "): preparing container1DUnfolds" << std::endl;

	//////////////analysis plots/////////////


	analysisPlotsMlbMt mlbmtplots_step8(8);
	analysisPlotsTtbarXsecFit xsecfitplots_step8(8);
        analysisPlotsTtbarXsecFitSingleLep singlelepplots_step8(8);
	analysisPlotsKinReco kinrecoplots_step8(8);

	xsecfitplots_step8.enable();
	mlbmtplots_step8.enable();
	mlbmtplots_step8.bookPlots();
	xsecfitplots_step8.enable();
	xsecfitplots_step8.bookPlots();
        if (doKinReco_){
            kinrecoplots_step8.enable();
            kinrecoplots_step8.bookPlots();
            kinrecoplots_step8.setKinRecoPS(true);
            xsecfitplots_step8.setKinRecoPS(true);
            mlbmtplots_step8.setKinRecoPS(true);
        }
        singlelepplots_step8.enable();
        singlelepplots_step8.bookPlots();

	//global settings for analysis plots
	histo1DUnfold::setAllListedMC(isMC && !fakedata);
	histo1DUnfold::setAllListedLumi((float)lumi_);
	if(testmode_)
		histo1DUnfold::setAllListedLumi((float)lumi_*0.08);

	//setup everything for control plots

	ttbarControlPlots plots;//for the Z part just make another class (or add a boolian)..
        ttbarGenPlots genplots;
	////FIX!!
	//plots.limitToStep(8);
	ZControlPlots zplots;
	plots.linkEvent(evt);
	if (doGenPlotsOnly_) genplots.linkEvent(evt);
	zplots.linkEvent(evt);
	ttXsecPlots xsecplots;
	xsecplots.enable(true);
	xsecplots.linkEvent(evt);
	xsecplots.limitToStep(8);
	xsecplots.initSteps(8);
	plots.initSteps(8);
	if (doGenPlotsOnly_) genplots.initSteps(0);
	zplots.initSteps(8);
	mlbmtplots_step8.setEvent(evt);
	if (doKinReco_) kinrecoplots_step8.setEvent(evt);
	xsecfitplots_step8.setEvent(evt);
        singlelepplots_step8.setEvent(evt);

	if(!fileExists((datasetdirectory_+inputfile_).Data())){
		std::cout << datasetdirectory_+inputfile_ << " not found!!" << std::endl;
		reportError(-1,anaid);
		return;
	}
	TFile *intfile;
	intfile=TFile::Open(datasetdirectory_+inputfile_);
	//get normalization - switch on or off pdf weighter before!!!
        norm_=createNormalizationInfo(intfile,isMC,anaid,signal_);
	intfile->Close();
	delete intfile;

	if(testmode_)
		std::cout << "testmode("<< anaid << "): multiplying norm with "<< normmultiplier <<" file: " << inputfile_<< std::endl;
	norm_*= normmultiplier;

	//init b-tag scale factor utility
	if(testmode_)
		std::cout << "testmode("<< anaid << "): preparing btag SF" << std::endl;

	btagefffile_+="_"+inputfile_;
	btageffreffile_+="_"+originputfile_;
	getBTagSF()->setIsMC(isMC);
	if(!getBTagSF()->getMakeEff()){
		getBTagSF()->readFromFile(btagefffile_.Data());
                if (isTtbarLike) getBTagSF()->readReferenceFromFile(btageffreffile_.Data());
                else getBTagSF()->readReferenceFromFile(btagefffile_.Data());
        }
	//  if(testmode_)
	//    std::cout << "testmode(" <<anaid << ") setBtagSmaplename " <<channel_+"_"+btagSysAdd+"_"+toString(inputfile_)).Data() <<std::endl;

	//range check switched off because of different ranges in bins compared to diff Xsec (leps)
	getTriggerSF()->setRangeCheck(false);
        getTriggerBGSF()->setRangeCheck(false);
	getElecSF()->setRangeCheck(false);
        getMuonSFBtoF()->setRangeCheck(false);
	getMuonSFGH()->setRangeCheck(false);
	getTrackingSF()->setRangeCheck(false);
        getElecTrackingSF()->setRangeCheck(false);

        getElecBGSF()->setRangeCheck(false);
        getMuonBGSF()->setRangeCheck(false);
        getMuonBGSFBtoF()->setRangeCheck(false);
        getMuonBGSFGH()->setRangeCheck(false);
        getTrackingBGSF()->setRangeCheck(false);
        getElecTrackingBGSF()->setRangeCheck(false);

	getElecSF()->setIsMC(isMC);
        getMuonSFBtoF()->setIsMC(isMC);
	getMuonSFGH()->setIsMC(isMC);
	getTriggerSF()->setIsMC(isMC);
        getTriggerBGSF()->setIsMC(isMC);
	getTrackingSF()->setIsMC(isMC);
        getElecTrackingSF()->setIsMC(isMC);

        getElecBGSF()->setIsMC(isMC);
        getMuonBGSF()->setIsMC(isMC);
        getMuonBGSFBtoF()->setIsMC(isMC);
        getMuonBGSFGH()->setIsMC(isMC);
        getTrackingBGSF()->setIsMC(isMC);
        getElecTrackingBGSF()->setIsMC(isMC);


	//some global checks
	getElecEnergySF()->setRangeCheck(false);
	getElecEnergyResolutionSF_rho()->setRangeCheck(false);
	getElecEnergyResolutionSF_phi()->setRangeCheck(false);
	getAdditionalJEC()->setRangeCheck(false);
	getMuonEnergySF()->setRangeCheck(false);
	getElecEnergySF()->setIsMC(isMC);
	getElecEnergyResolutionSF_rho()->setIsMC(isMC);
	getElecEnergyResolutionSF_phi()->setIsMC(isMC);
	getAdditionalJEC()->setIsMC(isMC);
	getMuonEnergySF()->setIsMC(isMC);

	getAdditionalJEC()->setIsTtbar(isTtbar);
	getAdditionalJEC()->setIsTtbarLike(isTtbarLike);

	//REMOVE AGAIN OR DO PROPERLY !!! 
	//agrohsje/tarndt include jes at ana level for testing 
	NTJES jescorr = NTJES();
	//took files from https://twiki.cern.ch/twiki/bin/view/CMS/JECDataMC
	const TString* mcL2L3JECFile = new TString((std::string) getenv("CMSSW_BASE")+"/src/TtZAnalysis/Analysis/data/analyse/Summer16_23Sep2016V3_MC_L2L3Residual_AK4PFchs.txt");
	const TString* mcL1JECFile = new TString((std::string) getenv("CMSSW_BASE")+"/src/TtZAnalysis/Analysis/data/analyse/Summer16_23Sep2016V3_MC_L1FastJet_AK4PFchs.txt");
	const TString* mcL2JECFile = new TString((std::string) getenv("CMSSW_BASE")+"/src/TtZAnalysis/Analysis/data/analyse/Summer16_23Sep2016V3_MC_L2Relative_AK4PFchs.txt");
	const TString* mcL3JECFile = new TString((std::string) getenv("CMSSW_BASE")+"/src/TtZAnalysis/Analysis/data/analyse/Summer16_23Sep2016V3_MC_L3Absolute_AK4PFchs.txt");
      
        const TString* dataL2L3JECFile_RunsBtoD = new TString((std::string) getenv("CMSSW_BASE")+"/src/TtZAnalysis/Analysis/data/analyse/Summer16_23Sep2016BCDV3_DATA_L2L3Residual_AK4PFchs.txt");
        const TString* dataL1JECFile_RunsBtoD = new TString((std::string) getenv("CMSSW_BASE")+"/src/TtZAnalysis/Analysis/data/analyse/Summer16_23Sep2016BCDV3_DATA_L1FastJet_AK4PFchs.txt");
        const TString* dataL2JECFile_RunsBtoD = new TString((std::string) getenv("CMSSW_BASE")+"/src/TtZAnalysis/Analysis/data/analyse/Summer16_23Sep2016BCDV3_DATA_L2Relative_AK4PFchs.txt");
        const TString* dataL3JECFile_RunsBtoD = new TString((std::string) getenv("CMSSW_BASE")+"/src/TtZAnalysis/Analysis/data/analyse/Summer16_23Sep2016BCDV3_DATA_L3Absolute_AK4PFchs.txt");

        const TString* dataL2L3JECFile_RunsEtoF = new TString((std::string) getenv("CMSSW_BASE")+"/src/TtZAnalysis/Analysis/data/analyse/Summer16_23Sep2016EFV3_DATA_L2L3Residual_AK4PFchs.txt");
        const TString* dataL1JECFile_RunsEtoF = new TString((std::string) getenv("CMSSW_BASE")+"/src/TtZAnalysis/Analysis/data/analyse/Summer16_23Sep2016EFV3_DATA_L1FastJet_AK4PFchs.txt");
        const TString* dataL2JECFile_RunsEtoF = new TString((std::string) getenv("CMSSW_BASE")+"/src/TtZAnalysis/Analysis/data/analyse/Summer16_23Sep2016EFV3_DATA_L2Relative_AK4PFchs.txt");
        const TString* dataL3JECFile_RunsEtoF = new TString((std::string) getenv("CMSSW_BASE")+"/src/TtZAnalysis/Analysis/data/analyse/Summer16_23Sep2016EFV3_DATA_L3Absolute_AK4PFchs.txt");

        const TString* dataL2L3JECFile_RunsFtoG = new TString((std::string) getenv("CMSSW_BASE")+"/src/TtZAnalysis/Analysis/data/analyse/Summer16_23Sep2016GV3_DATA_L2L3Residual_AK4PFchs.txt");
        const TString* dataL1JECFile_RunsFtoG = new TString((std::string) getenv("CMSSW_BASE")+"/src/TtZAnalysis/Analysis/data/analyse/Summer16_23Sep2016GV3_DATA_L1FastJet_AK4PFchs.txt");
        const TString* dataL2JECFile_RunsFtoG = new TString((std::string) getenv("CMSSW_BASE")+"/src/TtZAnalysis/Analysis/data/analyse/Summer16_23Sep2016GV3_DATA_L2Relative_AK4PFchs.txt");
        const TString* dataL3JECFile_RunsFtoG = new TString((std::string) getenv("CMSSW_BASE")+"/src/TtZAnalysis/Analysis/data/analyse/Summer16_23Sep2016GV3_DATA_L3Absolute_AK4PFchs.txt");

        const TString* dataL2L3JECFile_RunsH = new TString((std::string) getenv("CMSSW_BASE")+"/src/TtZAnalysis/Analysis/data/analyse/Summer16_23Sep2016HV3_DATA_L2L3Residual_AK4PFchs.txt");
        const TString* dataL1JECFile_RunsH = new TString((std::string) getenv("CMSSW_BASE")+"/src/TtZAnalysis/Analysis/data/analyse/Summer16_23Sep2016HV3_DATA_L1FastJet_AK4PFchs.txt");
        const TString* dataL2JECFile_RunsH = new TString((std::string) getenv("CMSSW_BASE")+"/src/TtZAnalysis/Analysis/data/analyse/Summer16_23Sep2016HV3_DATA_L2Relative_AK4PFchs.txt");
        const TString* dataL3JECFile_RunsH = new TString((std::string) getenv("CMSSW_BASE")+"/src/TtZAnalysis/Analysis/data/analyse/Summer16_23Sep2016HV3_DATA_L3Absolute_AK4PFchs.txt");


        if(isMC) jescorr.setFilesCorrection(mcL1JECFile,mcL2JECFile, mcL3JECFile,mcL2L3JECFile,(const bool) isMC);
        else if(isRunBtoD) jescorr.setFilesCorrection(dataL1JECFile_RunsBtoD,dataL2JECFile_RunsBtoD, dataL3JECFile_RunsBtoD,dataL2L3JECFile_RunsBtoD,(const bool) isMC);
        else if(isRunEtoF) jescorr.setFilesCorrection(dataL1JECFile_RunsEtoF,dataL2JECFile_RunsEtoF, dataL3JECFile_RunsEtoF,dataL2L3JECFile_RunsEtoF,(const bool) isMC);
        else if(isRunFtoG) jescorr.setFilesCorrection(dataL1JECFile_RunsFtoG,dataL2JECFile_RunsFtoG, dataL3JECFile_RunsFtoG,dataL2L3JECFile_RunsFtoG,(const bool) isMC);
        else if(isRunH) jescorr.setFilesCorrection(dataL1JECFile_RunsH,dataL2JECFile_RunsH, dataL3JECFile_RunsH,dataL2L3JECFile_RunsH,(const bool) isMC);


        //std::cout<<"isRunH: "<<isRunH<<"   "<<"isSingleEle: "<<isSingleEle<<std::endl;


	/*
	 * Open Main tree,
	 * Set branches
	 * the handler is only a small wrapper
	 */
	tTreeHandler tree( datasetdirectory_+inputfile_ ,"PFTree/PFTree");
	tTreeHandler *t =&tree;

	tBranchHandler<std::vector<bool> >     b_TriggerBools(t,"TriggerBools");
	tBranchHandler<vector<NTElectron> >    b_Electrons(t,electrontype);
	tBranchHandler<vector<NTMuon> >        b_Muons(t,"NTMuons");
	tBranchHandler<vector<NTJet> >         b_Jets(t,"NTJets");
	tBranchHandler<NTMet>                  b_Met(t,mettype);
	tBranchHandler<NTEvent>                b_Event(t,eventbranch_);
	tBranchHandler<vector<NTGenParticle> > b_GenTops(t,"NTGenTops");
	tBranchHandler<vector<NTGenParticle> > b_GenWs(t,"NTGenWs");
	tBranchHandler<vector<NTGenParticle> > b_GenZs(t,"NTGenZs");
	tBranchHandler<vector<NTGenParticle> > b_GenBs(t,"NTGenBs");
	tBranchHandler<vector<NTGenParticle> > b_GenBHadrons(t,"NTGenBHadrons");
	tBranchHandler<vector<NTGenParticle> > b_GenLeptons3(t,"NTGenLeptons3");
	tBranchHandler<vector<NTGenParticle> > b_GenLeptons1(t,"NTGenLeptons1");
	tBranchHandler<vector<NTGenJet> >      b_GenJets(t,"NTGenJets");
	tBranchHandler<vector<NTGenParticle> > b_GenNeutrinos(t,"NTGenNeutrinos");

	//additional weights
	std::vector<tBranchHandler<NTWeight>*> weightbranches;
	tBranchHandler<NTWeight>::allow_missing =true;
	tBranchHandler<vector<NTGenParticle> >::allow_missing =true;
	tBranchHandler<vector<NTGenParticle> > b_GenBsRad(t,"NTGenBsRad");

	tBranchHandler<ULong64_t>::allow_missing =true;
	tBranchHandler<ULong64_t> b_EventNumber(t,"EventNumber");
	tBranchHandler<ULong64_t> b_RunNumber(t,"RunNumber");
	tBranchHandler<ULong64_t> b_LumiBlock(t,"LumiBlock");
	tBranchHandler<ULong64_t> b_AnalyseEvent(t,"Skim");
        tBranchHandler<ULong64_t> b_MetFilter(t,"Metfilter");
        

	std::vector<ztop::simpleReweighter> mcreweighters;

	for(size_t i=0;i<additionalweights_.size();i++){
		std::cout << "adding weight " << additionalweights_.at(i) << std::endl;
		tBranchHandler<NTWeight> * weight = new tBranchHandler<NTWeight>(t,additionalweights_.at(i));
		weightbranches.push_back(weight);
		//agrohsje 
		//assume: mcweights are filled if important otherwise ntweight is 1 
		ztop::simpleReweighter mcreweighter; 
		mcreweighters.push_back(mcreweighter);
	}

	//some helpers
	double sel_step[]={0,0,0,0,0,0,0,0,0};
	float count_samesign=0;

	if(!testmode_){
		// this enables some caching while reading the tree. Speeds up batch mode
		// significantly!
		t->setPreCache();
	}

	Long64_t nEntries=t->entries();
	if(norm_==0) nEntries=0; //skip for norm0
	if(testmode_ && ! tickoncemode_) nEntries*=0.08;

	Long64_t firstentry=0;
	//for fake data all signal samples are assumed to be used as fake data
	//split at 10%
	Long64_t regionlowerbound=0;
	Long64_t regionupperbound=nEntries+1;
	//skip if they are below OR above (if false events are skipped if they are below AND above)
	bool skipregion=false;
	if(isfakedatarun){
		float splitfractionMC=0.9;
		float subsetstarts=fakedata_startentries_;
		//if(!fakedata){//issignal || fakedata){
		skipregion=true;
		regionlowerbound=nEntries*subsetstarts;
		regionupperbound=nEntries*(subsetstarts+(1-splitfractionMC));
		if(nosplitforfakedata){
			regionlowerbound=-1;
			regionupperbound=-1;
			skipregion=true;
		}
		else{
			if(!fakedata){
				skipregion=true;
				float normmultif=1/splitfractionMC;

				if(testmode_)
					std::cout << "testmode("<< anaid << "):\t splitted MC fraction off for MC          "<< splitfractionMC
					<< " old norm: " << norm_ << " to " << norm_*normmultif << " file: " << inputfile_<< std::endl;
				norm_*= normmultif;
			}
			else{// if(){
				skipregion=false;

				float normmultif=1/(1-splitfractionMC);
				if(testmode_)
					std::cout << "testmode("<< anaid << "):\t splitted MC fraction off for pseudo data "<< 1-splitfractionMC
					<< " old norm: " << norm_ << " to " << norm_*normmultif << " file: " << inputfile_<< std::endl;
				norm_*=normmultif;

			}
		}

	}


	///////////////////////////////////////////////////////// /////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////// /////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////// /////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////// /////////////////////////////////////////////////////////
	///////////////////////////////////////// start main loop /////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////// /////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////// /////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////// /////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////// /////////////////////////////////////////////////////////

	if(testmode_)
		std::cout << "testmode("<< anaid << "): starting mainloop with file "<< inputfile_ << " norm " << norm_ << " entries: "<<nEntries << " fakedata: " <<  fakedata<<std::endl;
	//agrohsje 
	float pusum(0.); 
	float pusum_sel(0.);
	int nEntries_sel(0);
	float mcwsum(0.);
	for(Long64_t entry=firstentry;entry<nEntries;entry++){

		//for fakedata
		if(skipregion){
			if(entry >= regionlowerbound && entry <= regionupperbound)
				continue;
		}
		else{//one case includes the entries, this is always true for non fakedata
			// and lowerbound is 0, upper bound nEntries+1 -> no effect
			if(entry < regionlowerbound || entry > regionupperbound)
				continue;
		}

		t->setEntry(entry);


		////////////////////////////////////////////////////
		////////////////////  INIT EVENT ///////////////////
		/////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////


		size_t step=0;
		evt.reset();
		evt.event=b_Event.content();
		histo1DUnfold::flushAllListed(); //important to call each event

		//reports current status to parent
		reportStatus(entry,nEntries);
                double randomNum = random->Uniform();
		float puweight=1;
                if (isMC) { 
                    if(randomNum<0.548) puweight = getPUReweighterBtoF()->getPUweight(b_Event.content()->truePU());
                    else puweight = getPUReweighterGH()->getPUweight(b_Event.content()->truePU());
                }
		//agrohsje
		pusum+=puweight;
		if(apllweightsone) puweight=1;
		if(testmode_ && entry==0)
			std::cout << "testmode("<< anaid << "): got first event entry" << std::endl;

		evt.puweight=&puweight;

		getPdfReweighter()->setNTEvent(b_Event.content());
		getPdfReweighter()->reWeight(puweight);
		if(apllweightsone) puweight=1;
		//agrohsje loop over additional weightbranches 
		for(size_t i=0;i<weightbranches.size();i++){
			//puweight*=weightbranches.at(i)->content()->getWeight();
			//std::cout<<"check weight for " << additionalweights_.at(i)<<":"
			//<<weightbranches.at(i)->content()->getWeight() <<std::endl;
			mcreweighters.at(i).setNewWeight(weightbranches.at(i)->content()->getWeight());
			mcreweighters.at(i).reWeight(puweight);
			if(apllweightsone) puweight=1;
		}
		mcwsum += weightbranches.at(0)->content()->getWeight();
		/////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////
		/////////////////// Generator Information////////////////////
		/////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////

		////define all collections
		// do not move somewhere else!

		vector<NTGenParticle*>  gentops,genbs,genbsrad;
		vector<NTGenParticle*>  genws;
		vector<NTGenParticle *> genleptons1,genleptons3,tmp_genleptons1;
		vector<NTGenJet *>      genjets;
		vector<NTGenParticle *> genbhadrons;

		evt.gentops=&gentops;
		evt.genbs=&genbs;
		evt.genbsrad=&genbsrad;
		evt.genbhadrons=&genbhadrons;
		evt.genleptons1=&genleptons1;
		evt.genleptons3=&genleptons3;
		evt.genjets=&genjets;



		if(isMC){
			if(testmode_ && entry==0)
				std::cout << "testmode("<< anaid << "): got first MC gen entry" << std::endl;

			//if(b_GenLeptons3.content()->size()>1){ //gen info there

			///////////////TOPPT REWEIGHTING////////
			if(b_GenTops.content()->size()>1){ //ttbar sample
				getTopPtReweighter()->reWeight(b_GenTops.content()->at(0).pt(),b_GenTops.content()->at(1).pt() ,puweight);
				if(apllweightsone) puweight=1;
				gentops.push_back(&b_GenTops.content()->at(0));
				gentops.push_back(&b_GenTops.content()->at(1));
			}

			//recreate dependencies
			genbs=produceCollection(b_GenBs.content(), &gentops);
			genbsrad=produceCollection(b_GenBsRad.content(), &genbs);//,-1,-1,&genbs);
			genws=produceCollection(b_GenWs.content(), &gentops);

			genleptons3=produceCollection(b_GenLeptons3.content(),&genws);
			tmp_genleptons1=produceCollection(b_GenLeptons1.content(),&genleptons3);
                        
                        if (tmp_genleptons1.size() > 1){
                       if(b_ee_ && std::abs(tmp_genleptons1.at(0)->pdgId())==11 && std::abs(tmp_genleptons1.at(1)->pdgId())==11){
                             genleptons1 = tmp_genleptons1;}
                        if(b_mumu_ && std::abs(tmp_genleptons1.at(0)->pdgId())==13 && std::abs(tmp_genleptons1.at(1)->pdgId())==13 ){
                             genleptons1 = tmp_genleptons1;}
                        if(b_emu_ &&( (std::abs(tmp_genleptons1.at(0)->pdgId())==11 && std::abs(tmp_genleptons1.at(1)->pdgId())==13) || (std::abs(tmp_genleptons1.at(0)->pdgId())==13 && std::abs(tmp_genleptons1.at(1)->pdgId())==11))){
                             genleptons1 = tmp_genleptons1;}
                        }
                        if(b_smu_ && tmp_genleptons1.size() ==1 && std::abs(tmp_genleptons1.at(0)->pdgId())==13 ) genleptons1 = tmp_genleptons1;
                        //else{ genleptons1 = tmp_genleptons1;}
                        
                        //genleptons1=produceCollection(b_GenLeptons1.content(),&genleptons3);
			//b-hadrons that stem from a b quark that itself originates in a top are
			//assoziated to that top by the bhadronmatcher (Nazar)
			//this logic is used and kept here
			genbhadrons=produceCollection(b_GenBHadrons.content(), &gentops);
			//try to associate the mothers. If successful, b-jet is matched to hadron -> to top
			genjets=produceCollection(b_GenJets.content());//,&genbhadrons);

			//}
			if(!fakedata){
                               //split into gen mtt bins
                                if (signal_ && doKinReco_ && !doGenPlotsOnly_){
                                    if (gentops.size()<2) continue;
                                    float gen_mtt = (gentops.at(0)->p4() + gentops.at(1)->p4()).M();
                                    setMttCategories(gen_mtt,legendname_);
                                    if ((int)gen_mttcategory != (int)leg_mttcategory) continue;
                                }
				/*
				 * fill gen info here
				 */
                                if (b_smu_)singlelepplots_step8.fillPlotsGen();
				mlbmtplots_step8.fillPlotsGen();
				xsecfitplots_step8.fillPlotsGen();
                                if(doKinReco_) kinrecoplots_step8.fillPlotsGen();
                                if (doGenPlotsOnly_) {
                                    genplots.makeControlPlots(0);
                                    continue;
                                }
			}
		} /// isMC ends

		//agrohsje : check if event fails preselection and should be skipped
		//std::cout<<" agrohsje check flag " << *b_AnalyseEvent.content()<<std::endl;
		if (b_emu_ && *b_AnalyseEvent.content()!=1) continue;
                if ( isMC && *b_MetFilter.content() == 1) continue;



		/////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////
		///////////////////       Reco Part      ////////////////////
		/////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////

		/*
		 *  Trigger
		 */

		if(testmode_ && entry==0)
			std::cout << "testmode("<< anaid << "): got trigger boolians" << std::endl;

//if (true) std::cout<<b_TriggerBools.content()->at(32)<<"  "<<b_TriggerBools.content()->at(33)<<"  "<<b_TriggerBools.content()->at(43)<<"  "<<b_TriggerBools.content()->at(44)<<"  "<<b_TriggerBools.content()->at(36)<<std::endl;

		if(!checkTrigger(b_TriggerBools.content(),b_Event.content(), isMC,anaid)) continue;

                if(testmode_ && entry==0)
                        std::cout << "testmode("<< anaid << "): After Trigger Cut " << std::endl;

		/*
		 * Muons
		 */

		vector<NTLepton *> allleps;
		std::vector<NTLepton *> isoleptons;
		// TBI std::vector<NTLepton *> vetoleps;
		evt.allleptons=&allleps;
		evt.isoleptons=&isoleptons;

		vector<NTMuon*> kinmuons,idmuons,isomuons;
		evt.kinmuons=&kinmuons;
		evt.idmuons=&idmuons;
		evt.isomuons=&isomuons;
		for(size_t i=0;i<b_Muons.content()->size();i++){
			NTMuon* muon = & b_Muons.content()->at(i);
			if(isMC){
				// muon->setP4(muon->p4() * getMuonEnergySF()->getScalefactor(muon->eta()));
				muon->setP4(muon->p4() * getMuonEnergySF()->getMuonRochesterFactorFromEnvelope(muon) );
                                // std::cout<<"TEST_MD\t"<<getMuonEnergySF()->getMuonRochesterFactorFromEnvelope_up(muon)<<"\t"<<getMuonEnergySF()->getMuonRochesterFactorFromEnvelope_down(muon)<<"\t1\t1"<<std::endl;
                        }
                        // std::cout<<"muon Rochester SF from envelope = "<<getMuonEnergySF()->getMuonRochesterFactorFromEnvelope(muon)<<std::endl;
			allleps << muon;
			/*if (*b_EventNumber.content() ==19458568){
			  std::cout<<*b_EventNumber.content() 
			  <<" muon->pt() "<<muon->pt()
			  <<" muon->eta() "<<muon->eta()
                          <<" muon->phi() "<<muon->phi()
			  <<" muon->isGlobal() "<<muon->isGlobal()
                          <<" muon->isPf()     "<<muon->isPf()
			  <<" muon->normChi2() "<<muon->normChi2()
			  <<" muon->trkHits() "<<muon->trkHits()
			  <<" muon->matchedStations() "<<muon->matchedStations()
			  <<" fabs(muon->d0V()) "<<fabs(muon->d0V())
			  <<" fabs(muon->dzV()) "<<fabs(muon->dzV())
			  <<" muon->pixHits() "<<muon->pixHits()
			  <<" muon->muonHits() "<<muon->muonHits()
			  <<" fabs(muon->isoVal()) "<<fabs(muon->isoVal())
			  <<std::endl;
			 }*/
			if(muon->pt() < lepptthresh)       continue;
			if(fabs(muon->eta())>2.4) continue;
			//if(fabs(muon->eta())>2.1) continue;
			kinmuons << &(b_Muons.content()->at(i));

			//tight muon selection: https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideMuonId#Tight_Muon
			//agrohsje isPF requirement already at ntuple level 

			if(muon->isGlobal()
					&& muon->isPf()
					&& muon->matchedStations()>1
					&& muon->pixHits()>0
					&& muon->muonHits()>0
					&& muon->normChi2()<10.
					&& muon->trkHits()>5
					&& fabs(muon->d0V())<0.2
					&& fabs(muon->dzV())<0.5)
			{
				idmuons <<  &(b_Muons.content()->at(i));
			}

		}
		for(size_t i=0;i<idmuons.size();i++){
			NTMuon * muon =  idmuons.at(i);
			if(!mode_invertiso && fabs(muon->isoVal()) > 0.15) continue;
			if(mode_invertiso && muon->isoVal() < 0.15) continue;
			isomuons <<  muon;
			isoleptons << muon;
		}

		/*
		 * Electrons
		 */
		vector<NTElectron *> kinelectrons,idelectrons,isoelectrons;
		evt.kinelectrons=&kinelectrons;
		evt.idelectrons=&idelectrons;
		evt.isoelectrons=&isoelectrons;
		for(size_t i=0;i<b_Electrons.content()->size();i++){
			NTElectron * elec=&(b_Electrons.content()->at(i));
			float ensf=1;
			if(isMC){
                            // ensf=getElecEnergySF()->getScalefactor(elec->eta());
                            ensf=getElecEnergySF()->getElectronESFactor(elec);
                            ensf*=getElecEnergyResolutionSF_rho()->getElectronERFactor_rho(elec);
                            ensf*=getElecEnergyResolutionSF_phi()->getElectronERFactor_phi(elec);
                        }

                        // std::cout<<"TEST<_MD "<<getElecEnergySF()->getElectronESFactorUp(elec)<<" "<<getElecEnergySF()->getElectronESFactorDown(elec)<<" "
                        //          <<getElecEnergyResolutionSF()->getElectronERFactorUp(elec)<<" "<<getElecEnergyResolutionSF()->getElectronERFactorDown(elec)<<endl;
			//elec->setECalP4(elec->ECalP4() * ensf);
                        // std::cout<<"TESTMD: "<<ensf<<std::endl;
			elec->setP4(elec->p4() * ensf); //both the same now!!
		      	/*if (*b_EventNumber.content() ==19458568 ) {
			    std::cout<<" agrohsje check electrons "
				     <<*b_EventNumber.content()
				     <<" elec->pt() " <<elec->pt()
				     <<" fabs(elec->eta()) " <<fabs(elec->eta())  
				     <<" fabs(elec->suClu().eta()) " <<fabs(elec->suClu().eta()) 
				     <<" elec->storedId() " <<elec->storedId()
                                     <<" elec->q() "<<elec->q()
                                     <<" elec->d0V() "<<elec->d0V()
                                     <<" elec->dzV() "<<elec->dzV() 
                                   //  <<" mll: " << (b_Electrons.content()->at(0).p4() + b_Electrons.content()->at(1).p4()).m()
				     <<std::endl;
			}*/
			//selection fully following https://twiki.cern.ch/twiki/bin/viewauth/CMS/TopEGM l+jets except for pt cut
			allleps << elec;
			if(elec->pt() < lepptthresh)  continue;
			float abseta=fabs(elec->eta());

			float suclueta = fabs(elec->suClu().eta());//elec->ECalP4().eta());
			if(abseta > 2.4) continue;
			if(suclueta > 1.4442 && suclueta < 1.5660) continue; //transistion region
                        if(suclueta < 1.4442 && (fabs(elec->d0V()) > 0.05 ||fabs(elec->dzV())>0.10) ) continue;
                        if(suclueta > 1.5660 && (fabs(elec->d0V()) > 0.10 ||fabs(elec->dzV())>0.20) ) continue;                       
			kinelectrons  << elec;
                        idelectrons << elec;
			if(elec->storedId() > 0.9){  ////agrohsje 1 or 0 should work 
				//iso already included in ID
				isoelectrons <<  elec;
				isoleptons << elec;
			}
		}

		/*
		 * make all lepton collection (merged muons and electrons)
		 */
		std::sort(allleps.begin(),allleps.end(), comparePt<ztop::NTLepton*>);



		if(testmode_ && entry==0)
			std::cout << "testmode("<< anaid << "): first controlPlots" << std::endl;

		/*
		 * Step 0 after trigger and ntuple cuts on leptons
		 */
		sel_step[0]+=puweight;
		plots.makeControlPlots(step);
		zplots.makeControlPlots(step);

		//////////two ID leptons STEP 1///////////////////////////////
		step++;

		if(b_ee_ && idelectrons.size() < 2) continue;
		if(b_mumu_ && (idmuons.size() < 2 )) continue;
		if(b_emu_ && (idmuons.size() + idelectrons.size() < 2 )) continue;
                if(b_smu_ && (idmuons.size() != 1 || idelectrons.size() > 0))continue;

		sel_step[1]+=puweight;
		plots.makeControlPlots(step);
		zplots.makeControlPlots(step);

                if(testmode_ && entry==0)
                        std::cout << "testmode("<< anaid << "): STEP 1" << std::endl;


		//////// require two iso leptons  STEP 2  //////////////////////////
		step++;
		vector<NTElectron*> channelelectrons;
		vector<NTMuon*> channelmuons;
		if(b_ee_ ){
			if(isoelectrons.size() < 2) continue;
			else if (isomuons.size()>0 ){
				if (isomuons.at(0)->pt() > isoelectrons.at(1)->pt()) continue;
			}
			channelelectrons << isoelectrons.at(0) << isoelectrons.at(1);
		}
		if(b_mumu_ ){
			if(isomuons.size() < 2) continue;
			else if (isoelectrons.size()>0 ){
				if (isoelectrons.at(0)->pt() > isomuons.at(1)->pt()) continue;
			}
			channelmuons << isomuons.at(0) << isomuons.at(1);
		}
		if(b_emu_ ){
			if(isomuons.size() < 1 || isoelectrons.size() < 1) continue;
			else{
				if (isoelectrons.size()>1 ){
					if (isoelectrons.at(1)->pt() > isomuons.at(0)->pt()) continue;
				}
				if (isomuons.size()>1){
					if (isomuons.at(1)->pt() > isoelectrons.at(0)->pt()) continue;
				}
			}
			channelmuons << isomuons.at(0);
			channelelectrons << isoelectrons.at(0);
		}
                if(b_smu_){
                        if(isomuons.size() !=1) continue;
                        else channelmuons << isomuons.at(0);
                }

		//make pair
		pair<vector<NTElectron*>, vector<NTMuon*> > oppopair,sspair;
		//oppopair = ztop::getOppoQHighestPtPair(isoelectrons, isomuons);
		//sspair = ztop::getOppoQHighestPtPair(isoelectrons, isomuons,-1);
		//tarndt : Require the lepton pair to be the two highest pt leptons
		oppopair = ztop::getOppoQHighestPtPair(channelelectrons, channelmuons);
		sspair = ztop::getOppoQHighestPtPair(channelelectrons, channelmuons,-1);

                if(testmode_ && entry==0)
                        std::cout << "testmode("<< anaid << "): Made Dilepton pairs" << std::endl;
                
		pair<vector<NTElectron*>, vector<NTMuon*> > *leppair=&oppopair;
		if(mode_samesign)
			leppair=&sspair;

		//fast count_samesign counting
		if(b_ee_ && sspair.first.size() >1)
			count_samesign+=puweight;
		else if(b_mumu_&&sspair.second.size() > 1)
			count_samesign+=puweight;
		else if(b_emu_ && sspair.first.size() > 0 && sspair.second.size() > 0)
			count_samesign+=puweight;

		float mll=0;
		NTLorentzVector<float>  dilp4;

		/*
		 * prepare dilepton pairs
		 *
		 */

		NTLepton * firstlep=0,*seclep=0, *leadingptlep=0, *secleadingptlep=0;

		double lepweight=1;
		if(b_ee_){
			if(leppair->first.size() <2) continue;
			dilp4=leppair->first[0]->p4() + leppair->first[1]->p4();
			mll=dilp4.M();
			firstlep=leppair->first[0];
			seclep=leppair->first[1];
                        if(isTtbarLike){
			lepweight*=getElecSF()->getScalefactor(leppair->first[0]->suClu().eta(),firstlep->pt());
			lepweight*=getElecSF()->getScalefactor(leppair->first[1]->suClu().eta(),seclep->pt());
                        //lepweight*=getElecTrackingSF()->getScalefactor(leppair->first[0]->suClu().eta(),firstlep->pt());
                        //lepweight*=getElecTrackingSF()->getScalefactor(leppair->first[1]->suClu().eta(),seclep->pt());
                        lepweight*=getTriggerSF()->getScalefactor(fabs(firstlep->eta()),fabs(seclep->eta()));
                        }
                        else{
                        lepweight*=getElecBGSF()->getScalefactor(leppair->first[0]->suClu().eta(),firstlep->pt());
                        lepweight*=getElecBGSF()->getScalefactor(leppair->first[1]->suClu().eta(),seclep->pt());
                        //lepweight*=getElecTrackingBGSF()->getScalefactor(leppair->first[0]->suClu().eta(),firstlep->pt());
                        //lepweight*=getElecTrackingBGSF()->getScalefactor(leppair->first[1]->suClu().eta(),seclep->pt());
                         lepweight*=getTriggerBGSF()->getScalefactor(fabs(firstlep->eta()),fabs(seclep->eta()));
                        }
                        //tarndt 2016
		}
		else if(b_mumu_){
			if(leppair->second.size() < 2) continue;
			dilp4=leppair->second[0]->p4() + leppair->second[1]->p4();
			mll=dilp4.M();
			firstlep=leppair->second[0];
			seclep=leppair->second[1];
                        if(isTtbarLike && randomNum < 0.548){
			lepweight*=getMuonSFBtoF()->getScalefactor(fabs(firstlep->eta()),firstlep->pt());
			lepweight*=getMuonSFBtoF()->getScalefactor(fabs(seclep->eta()),seclep->pt());
                        lepweight*=getTriggerSF()->getScalefactor(fabs(firstlep->eta()),fabs(seclep->eta()));
                        }
                        else if (isTtbarLike && randomNum > 0.548){
                        lepweight*=getMuonSFGH()->getScalefactor(fabs(firstlep->eta()),firstlep->pt());
                        lepweight*=getMuonSFGH()->getScalefactor(fabs(seclep->eta()),seclep->pt());
                        lepweight*=getTriggerSF()->getScalefactor(fabs(firstlep->eta()),fabs(seclep->eta()));
                        }
                        else if(!isTtbarLike && randomNum < 0.548){
                        lepweight*=getMuonBGSFBtoF()->getScalefactor(fabs(firstlep->eta()),firstlep->pt());
                        lepweight*=getMuonBGSFBtoF()->getScalefactor(fabs(seclep->eta()),seclep->pt());
                        lepweight*=getTriggerBGSF()->getScalefactor(fabs(firstlep->eta()),fabs(seclep->eta()));
                        }
                        else if(!isTtbarLike && randomNum > 0.548){
                        lepweight*=getMuonBGSFGH()->getScalefactor(fabs(firstlep->eta()),firstlep->pt());
                        lepweight*=getMuonBGSFGH()->getScalefactor(fabs(seclep->eta()),seclep->pt());
                        lepweight*=getTriggerBGSF()->getScalefactor(fabs(firstlep->eta()),fabs(seclep->eta()));
                        }

			//agrohsje not used for time being
			//lepweight*=getTrackingSF()->getScalefactor(firstlep->eta());
			//lepweight*=getTrackingSF()->getScalefactor(seclep->eta());
			//tarndt 2016
		}
		else if(b_emu_){
			if(leppair->first.size() < 1 || leppair->second.size() < 1) continue;
			dilp4=leppair->first[0]->p4() + leppair->second[0]->p4();
			mll=dilp4.M();
			firstlep=leppair->first[0];
			seclep=leppair->second[0];
                        if(isTtbarLike){
			lepweight*=getElecSF()->getScalefactor(leppair->first[0]->suClu().eta(),firstlep->pt());
			if(randomNum < 0.548)lepweight*=getMuonSFBtoF()->getScalefactor(fabs(seclep->eta()),seclep->pt());
                        else lepweight*=getMuonSFGH()->getScalefactor(fabs(seclep->eta()),seclep->pt());
                        //lepweight*=getElecTrackingSF()->getScalefactor(leppair->first[0]->suClu().eta(),firstlep->pt());
                        lepweight*=getTriggerSF()->getScalefactor(fabs(firstlep->eta()),fabs(seclep->eta()));
                        }
                        else{
                        lepweight*=getElecBGSF()->getScalefactor(leppair->first[0]->suClu().eta(),firstlep->pt());
                        if (randomNum < 0.548)lepweight*=getMuonBGSFBtoF()->getScalefactor(fabs(seclep->eta()),seclep->pt());
                        else lepweight*=getMuonBGSFGH()->getScalefactor(fabs(seclep->eta()),seclep->pt());
                        //lepweight*=getElecTrackingBGSF()->getScalefactor(leppair->first[0]->suClu().eta(),firstlep->pt());
                        lepweight*=getTriggerBGSF()->getScalefactor(fabs(firstlep->eta()),fabs(seclep->eta()));
                        }
                  	//agrohsje not used for time being
			//lepweight*=getTrackingSF()->getScalefactor(seclep->eta());
			// tarndt 2016
		}
                else if (b_smu_){
                        mll=30;
                        firstlep=channelmuons.at(0);
                        seclep=channelmuons.at(0);
                        dilp4=leppair->first[0]->p4();
                        if(signal_ && randomNum < 0.548){
                        lepweight*=getMuonSFBtoF()->getScalefactor(fabs(firstlep->eta()),firstlep->pt());
                        lepweight*=getTriggerSF()->getScalefactor(fabs(firstlep->eta()),firstlep->pt());
                        }
                        else if (signal_ && randomNum > 0.548){
                        lepweight*=getMuonSFGH()->getScalefactor(fabs(firstlep->eta()),firstlep->pt());
                        lepweight*=getTriggerSF()->getScalefactor(fabs(firstlep->eta()),firstlep->pt());
                        }
                        else if(!signal_ && randomNum < 0.548){
                        lepweight*=getMuonBGSFBtoF()->getScalefactor(fabs(firstlep->eta()),firstlep->pt());
                        lepweight*=getTriggerBGSF()->getScalefactor(fabs(firstlep->eta()),firstlep->pt());
                        }
                        else if(!signal_ && randomNum > 0.548){
                        lepweight*=getMuonBGSFGH()->getScalefactor(fabs(firstlep->eta()),firstlep->pt());
                        lepweight*=getTriggerBGSF()->getScalefactor(fabs(firstlep->eta()),firstlep->pt());
                        }               
                }

		//channel defined
                bool isMuonLeadingLepton = false;
                if (b_mumu_ || b_smu_ ) isMuonLeadingLepton = true;

		if(firstlep->pt() > seclep->pt() || b_smu_){
			leadingptlep=firstlep;
			secleadingptlep=seclep;
		}
		else{
			leadingptlep=seclep;
			secleadingptlep=firstlep;
                        if (b_emu_) isMuonLeadingLepton = true;
		}
		//just to avoid warnings
		evt.leadinglep=leadingptlep;
		evt.secleadinglep=secleadingptlep;
		evt.mll=&mll;

                //2016 PT cut leading lep > 25 GEV
                if (leadingptlep->pt() < 25.) continue; //|| secleadingptlep->pt() < 25.) continue;

		float leplepdr=dR_3d(leadingptlep->p4(),secleadingptlep->p4());
		evt.leplepdr=&leplepdr;
		float cosleplepangle=cosAngle_3d(leadingptlep->p4(),secleadingptlep->p4());
		evt.cosleplepangle=&cosleplepangle;

		puweight*=lepweight;
		if(apllweightsone) puweight=1;

		//just a quick faety net against very weird weights
		/*if(isMC && fabs(puweight) > 99999){
			reportError(-88,anaid);
			return;
		}
		 */
		sel_step[2]+=puweight;
		plots.makeControlPlots(step);
		zplots.makeControlPlots(step);
                 
                if(testmode_ && entry==0)
                        std::cout << "testmode("<< anaid << "): STEP2 " << std::endl;


		///////// cut on invariant mll mass /// STEP 3 ///////////////////////////////////////
		step++;
		if(mll < mllcut)
			continue;

                if(testmode_ && entry==0)
                        std::cout << "testmode("<< anaid << "): STEP3 " << std::endl;

		//now agrohsje added for debugging
	        //std::cout<<*b_EventNumber.content()<</*" "<<leadingptlep->pt()<<" "<<leadingptlep->eta()<<" "<<secleadingptlep->pt()<<" "<<secleadingptlep->eta()<<*/std::endl;


		// create jec jets for met and ID jets
		// create ID Jets and correct JER

		vector<NTJet *> treejets,idjets,medjets,hardjets;
		evt.idjets=&idjets;
		evt.medjets=&medjets;
		evt.hardjets=&hardjets;
		for(size_t i=0;i<b_Jets.content()->size();i++){
			treejets << &(b_Jets.content()->at(i));
		}

		double dpx=0;
		double dpy=0;
                 
                //if (*b_EventNumber.content() != 19458568) continue;

		for(size_t i=0;i<treejets.size();i++){ //ALSO THE RESOLUTION AFFECTS MET. HERE INTENDED!!! GOOD?
			NTLorentzVector<float>  oldp4=treejets.at(i)->p4();
			bool useJetForMet=false;
			if(treejets.at(i)->emEnergyFraction() < 0.9 && treejets.at(i)->pt() > 10)
				useJetForMet=true; //dont even do something
			//agrohsje/tarndt just for testing REMOVE
			//std::cout<<"jet["<<i<<"] in event "<<*b_EventNumber.content()<<std::endl;
			//std::cout<<"agrohsje jet pt before: pt="<<treejets.at(i)->pt()<<" rho=" <<b_Event.content()->isoRho(0)<< " area="<< treejets.at(i)->getMember(0)<< "   uncorr pt: "<<treejets.at(i)->p4Uncorr().pt()<<"  gen pt: "<< treejets.at(i)->genP4().pt() <<std::endl;
			jescorr.correctJet(treejets.at(i), treejets.at(i)->getMember(0),b_Event.content()->isoRho(0));
			//std::cout<<"agrohsje jet pt after jes: pt="<<treejets.at(i)->pt()<<" eta="<<treejets.at(i)->eta()<<std::endl;
			if(isMC){
                            // if (treejets.at(i)->pt()>30){
                            //     std::cout<<"CHECK MD JEC: "<<treejets.at(i)->pt()<<" "<<getAdditionalJEC()->getAdditionalJECFactor(treejets.at(i))<<std::endl;
                            // }
                                treejets.at(i)->setP4( treejets.at(i)->p4() * getAdditionalJEC()->getAdditionalJECFactor(treejets.at(i)) );
				//agrohsje global 4% scaling for JESup/JESdown can be added to ZTopUtils/src/JECBase.cc
				//use NTJES w./ ZTopUtils/src/JESBase.cc for both correction/uncertainties
				getJECUncertainties()->applyToJet(treejets.at(i));
				//std::cout<<"agrohsje jet pt after sys var: pt="<<treejets.at(i)->pt()<<" eta="<<treejets.at(i)->eta()<<std::endl;
				getJERAdjuster()->correctJet(treejets.at(i));
				//std::cout<<"agrohsje jet pt after jer "<<treejets.at(i)->pt()<<" eta "<<treejets.at(i)->eta()<<std::endl;
				//corrected
			}
			if(useJetForMet){
				dpx += oldp4.Px() - treejets.at(i)->p4().Px();
				dpy += oldp4.Py() - treejets.at(i)->p4().Py();
			}
			if(!(treejets.at(i)->id())) continue;
			//agrohsje changed cut to 0.4 instead of 0.5 
			if(!noOverlap(treejets.at(i), isomuons,     0.4)) continue;
			if(!noOverlap(treejets.at(i), isoelectrons, 0.4)) continue;
			if(fabs(treejets.at(i)->eta())>2.4) continue;
			if(treejets.at(i)->pt() < 10) continue;
			idjets << (treejets.at(i));

			if(treejets.at(i)->pt() < 20) continue;
			medjets << treejets.at(i);
			if(treejets.at(i)->pt() <= jetptcut) continue;
			hardjets << treejets.at(i);
		}
		//jets
		//		std::sort
		std::sort(idjets.begin(),idjets.end(), comparePt<ztop::NTJet*>);
		std::sort(medjets.begin(),medjets.end(), comparePt<ztop::NTJet*>);
		std::sort(hardjets.begin(),hardjets.end(), comparePt<ztop::NTJet*>);







		NTMet adjustedmet = *b_Met.content();
		evt.simplemet=b_Met.content();
		evt.adjustedmet=&adjustedmet;
		double nmpx=b_Met.content()->p4().Px() + dpx;
		double nmpy=b_Met.content()->p4().Py() + dpy;
		if(!usemvamet)
			adjustedmet.setP4(D_LorentzVector(nmpx,nmpy,0,sqrt(nmpx*nmpx+nmpy*nmpy))); //COMMENTED FOR MVA MET

                if(testmode_ && entry==0)
                        std::cout << "testmode("<< anaid << "): Got jets+MET " << std::endl;


		///////////////combined variables////////////

		float dphillj=M_PI,dphilljj=M_PI,detallj=0,detalljj=0;
		float pi=M_PI;
		bool midphi=false;

		if(hardjets.size() >0){
			dphillj=fabs( (leadingptlep->p4()+secleadingptlep->p4()).Phi() - hardjets.at(0)->p4().Phi() );
			if(dphillj>pi)
				dphillj=2*pi-dphillj;
			//dphillj=pi-fabs(dphillj-pi);
			evt.dphillj=&dphillj;
			evt.midphi=&midphi;
			detallj=(leadingptlep->p4()+secleadingptlep->p4()).Eta() - hardjets.at(0)->p4().Eta();
			evt.detallj=&detallj;
		}



		if(hardjets.size() >1){
			dphilljj=fabs( (leadingptlep->p4()+secleadingptlep->p4()).Phi() - (hardjets.at(0)->p4()+hardjets.at(1)->p4()).Phi() );
			dphilljj=pi-fabs(dphilljj-pi);
			evt.dphilljj=&dphilljj;
			detalljj=(leadingptlep->p4()+secleadingptlep->p4()).Eta() - (hardjets.at(0)->p4()+hardjets.at(1)->p4()).Eta();
			evt.detalljj=&detalljj;
		}

		float ptllj=leadingptlep->pt()+secleadingptlep->pt();
		evt.ptllj=&ptllj;
		if(hardjets.size() >0)
			ptllj+=hardjets.at(0)->pt();





		vector<NTJet*> hardbjets;
		evt.hardbjets=&hardbjets;
		vector<NTJet*> medbjets;
		evt.medbjets=&medbjets;

		//PHI stuff
		if(dphillj > M_PI-1)
			midphi=true;
		//float jetpt_phi=dphillj*dphillj*40/(M_PI*M_PI);
		float jetptphifunc=(dphillj-M_PI+1)*20+1;
		float jetpt_phi=std::max(10.0f,jetptphifunc);
		vector<NTJet*> dphilljjets;
		for(size_t i=0;i<idjets.size();i++){
			if(idjets.at(i)->pt()<jetpt_phi) continue;
			dphilljjets << idjets.at(i);
		}

		vector<NTJet*> dphiplushardjets=mergeVectors(hardjets,dphilljjets);
		std::sort(dphiplushardjets.begin(),dphiplushardjets.end(), comparePt<ztop::NTJet*>);



		vector<NTJet*> * selectedjets=&hardjets;// &hardjets;

                vector<NTJet*> selectedFwdJets;

		for(size_t i=0;i<treejets.size();i++){

                    if(!noOverlap(treejets.at(i), isomuons,     0.4)) continue;
                    if(!noOverlap(treejets.at(i), isoelectrons, 0.4)) continue;
                    if(fabs(treejets.at(i)->eta())<2.5) continue;
                    if(fabs(treejets.at(i)->eta())>3.0) continue;
                    if(treejets.at(i)->pt() < 30) continue;
                    selectedFwdJets << (treejets.at(i));
                }

                std::sort(selectedFwdJets.begin(),selectedFwdJets.end(), comparePt<ztop::NTJet*>);
                evt.selectedFwdJets=&selectedFwdJets;


		evt.dphilljjets=&dphilljjets;
		evt.dphiplushardjets=&dphiplushardjets;
		evt.selectedjets=selectedjets;


		NTLorentzVector<float> H4;
		for(size_t i=0;i<idjets.size();i++)
			H4+=idjets.at(i)->p4();
		float ht=H4.Pt();
		evt.ht=&ht;

		NTLorentzVector<float> S4=leadingptlep->p4() + secleadingptlep->p4() -H4;
		evt.S4=&S4;

		NTLorentzVector<float> allobjects4=S4+ H4 + H4;
		evt.allobjects4=&allobjects4;


		vector<NTJet*>  selectedbjets,selectednonbjets;
		evt.selectedbjets=&selectedbjets;
		evt.selectednonbjets=&selectednonbjets;
                std::vector<int> bjet_indices;
                VLV jetVLV;

		if(getBTagSF()->getMode() == NTBTagSF::randomtagging_mode) 
                    getBTagSF()->changeNTJetTags(selectedjets);

		for(size_t i=0;i<hardjets.size();i++){

                        TLorentzVector jetTLV; 
                        if (doKinReco_){
                            jetTLV.SetPtEtaPhiE(selectedjets->at(i)->pt(),selectedjets->at(i)->eta(),selectedjets->at(i)->phi(),selectedjets->at(i)->e());
                            jetVLV.push_back(common::TLVtoLV(jetTLV));
                        }

			if(selectedjets->at(i)->btag() < getBTagSF()->getWPDiscrValue()){
				//std::cout<<"agrohsje btagging in event " << *b_EventNumber.content() <<" "<<selectedjets->at(i)->pt()<<"  "<<selectedjets->at(i)->btag() <<std::endl;
				selectednonbjets.push_back(selectedjets->at(i));
				continue;
			}
			selectedbjets.push_back(selectedjets->at(i));
                        if (doKinReco_) bjet_indices.push_back(i);
                        
		}

		float btagscontr=1;
		for(size_t i=0 ;i<selectedjets->size();i++){
			//if(i>0) break; //only use first jet for now
			if(selectedjets->at(i)->btag()>0 && selectedjets->at(i)->btag()<1){
				btagscontr*=(- selectedjets->at(i)->btag() +1);
				break;
			}
		}
		float topdiscr=1;
		evt.topdiscr=&topdiscr;
		topdiscr=1- ((cosleplepangle+1)/2 * dphillj/M_PI * btagscontr);

		float topdiscr2=1;
		evt.topdiscr2=&topdiscr2;
		topdiscr2=1- ((cosleplepangle+1)/2 * btagscontr);

		float topdiscr3=1;
		evt.topdiscr3=&topdiscr3;
		topdiscr3=1- (dphillj/M_PI * btagscontr);


		sel_step[3]+=puweight;
		plots.makeControlPlots(step);
		zplots.makeControlPlots(step);


		///////////////calculate likelihood inputs///////////




		float lhi_dphillj=dphillj;
		evt.lhi_dphillj=&lhi_dphillj;

		float lhi_cosleplepangle=cosleplepangle;
		evt.lhi_cosleplepangle=&lhi_cosleplepangle;

		float lhi_leadjetbtag=1;
		if(selectedjets->size()>0)
			lhi_leadjetbtag=(selectedjets->at(0)->btag());
		evt.lhi_leadjetbtag =&lhi_leadjetbtag;

		float lhi_sumdphimetl=0;
		lhi_sumdphimetl=absNormDPhi(leadingptlep->p4(),adjustedmet.p4()) + absNormDPhi(secleadingptlep->p4(),adjustedmet.p4());
		evt.lhi_sumdphimetl=&lhi_sumdphimetl;

		float lhi_seljetmulti=0;
		lhi_seljetmulti=selectedjets->size();
		evt.lhi_seljetmulti=&lhi_seljetmulti;

		float lhi_selbjetmulti=selectedbjets.size();
		evt.lhi_selbjetmulti=&lhi_selbjetmulti;

		float lhi_leadleppt=leadingptlep->pt();
		evt.lhi_leadleppt=&lhi_leadleppt;

		float lhi_secleadleppt=secleadingptlep->pt();
		evt.lhi_secleadleppt=&lhi_secleadleppt;

		float lhi_leadlepeta=leadingptlep->eta();
		evt.lhi_leadlepeta=&lhi_leadlepeta;

		float lhi_secleadlepeta=secleadingptlep->eta();
		evt.lhi_secleadlepeta=&lhi_secleadlepeta;

		float lhi_leadlepphi=leadingptlep->phi();
		evt.lhi_leadlepphi=&lhi_leadlepphi;

		float lhi_secleadlepphi=secleadingptlep->phi();
		evt.lhi_secleadlepphi=&lhi_secleadlepphi;

		float lhi_leadjetpt=0;
		if(selectedjets->size()>0)
			lhi_leadjetpt=selectedjets->at(0)->pt();
		evt.lhi_leadjetpt=&lhi_leadjetpt;

		evt.lhi_mll=&mll;

		float lhi_drlbl=0;
		if(selectedbjets.size()>0){
			NTLorentzVector<float> lbp4=selectedbjets.at(0)->p4()+leadingptlep->p4();
			NTLorentzVector<float> lbp42=selectedbjets.at(0)->p4()+secleadingptlep->p4();
			if(lbp4.m() > 165 && lbp42.m() < 165){
				lhi_drlbl=dR_3d(lbp42,leadingptlep->p4());
			}
			else{
				lhi_drlbl=dR_3d(lbp4,secleadingptlep->p4());
			}
			evt.lhi_drlbl=&lhi_drlbl;
		}

		evt.lhi_ptllj=&ptllj;


		//correlated variables


		float lhi_cosllvsetafirstlep =
				lhi_cosleplepangle - (0.2*(fabs(lhi_leadlepeta)-2.4)*(fabs(lhi_leadlepeta)-2.4) +1);
		if(lhi_cosleplepangle < -0.4)
			lhi_cosllvsetafirstlep =
					lhi_cosleplepangle - (0.2*(fabs(lhi_leadlepeta)+2.4)*(fabs(lhi_leadlepeta)+2.4) +1);

		evt.lhi_cosllvsetafirstlep=&lhi_cosllvsetafirstlep;
		float lhi_etafirstvsetaseclep =
				lhi_leadlepeta + lhi_secleadlepeta;
		evt.lhi_etafirstvsetaseclep=&lhi_etafirstvsetaseclep;
		float lhi_deltaphileps =
				lhi_leadlepphi - lhi_secleadlepphi;
		evt.lhi_deltaphileps=&lhi_deltaphileps;
		float  lhi_coslepanglevsmll =
				-lhi_cosleplepangle - 1/65 * (mll-90);
		evt.lhi_coslepanglevsmll=&lhi_coslepanglevsmll;
		float lhi_mllvssumdphimetl =
				1/14 * (mll-25) - lhi_sumdphimetl;
		evt.lhi_mllvssumdphimetl=&lhi_mllvssumdphimetl;



		////////////////////Z Selection//////////////////
		step++;
		bool isZrange=false;

		if(mll > 76 && mll < 106){
			isZrange=true;
		}

		bool analysisMllRange=!isZrange;
		if(b_emu_) analysisMllRange=true; //no z veto on emu
		if(nozcut) analysisMllRange=true;
		////////////////////Z Veto Cut STEP 4 (incl. hard jets)////////////////////////////////////



		if(analysisMllRange){

			plots.makeControlPlots(step);
			sel_step[4]+=puweight;

		}
		if(isZrange){
			zplots.makeControlPlots(step);

		}
                if(testmode_ && entry==0)
                        std::cout << "testmode("<< anaid << "): STEP4 " << std::endl;



		///////////////////// at least one jet cut STEP 5 ////////////
		step++;

		if(!zerojet && selectedjets->size() < 1) continue;

                // need 2 jets for kinematic reconstruction
                if(doKinReco_ && selectedjets->size() < 2) continue;

		if(getBTagSF()->getMode() == NTBTagSF::shapereweighting_mode){
			throw std::runtime_error("NTBTagSF::shapereweighting_mode: not impl");
		}
		else if(getBTagSF()->getMode() == NTBTagSF::simplereweighting_mode){
                    puweight *= getBTagSF()->getEventWeightSimple(selectedjets);
		}



		if(apllweightsone) puweight=1;
		//ht+=adjustedmet.met();
		//double mllj=0;
		//double phijl=0;


		float lh_toplh=0;
		evt.lh_toplh=&lh_toplh;

		if(analysisMllRange){


			sel_step[5]+=puweight;
			plots.makeControlPlots(step);


		}
		if(isZrange){
			zplots.makeControlPlots(step);
		}
                if(testmode_ && entry==0)
                        std::cout << "testmode("<< anaid << "): STEP5 " << std::endl;

		//agrohsje debug jet 
		
		/*if(*b_EventNumber.content()== 64270328){
			std::cout<< *b_EventNumber.content() <<" : "<<std::endl;  
			for(unsigned ijet=0; ijet<selectedjets->size();ijet++){
			std::cout<<"jet pt = "<<selectedjets->at(ijet)->pt() 
				 <<", jet eta = "<<selectedjets->at(ijet)->eta()
				 <<", jet id = "<<selectedjets->at(ijet)->id()
				 <<", jet btag() = "<<selectedjets->at(ijet)->btag()<<std::endl;
		    }
		    std::cout<<"######################################################################"<<std::endl;
		}*/
		 
		/////////////////////// at least two jets STEP 6 /////////////
		step++;

		//if(midphi && dphiplushardjets.size()<2) continue;
		if(!zerojet && !onejet && selectedjets->size() < 2) continue;


		if(analysisMllRange){

			plots.makeControlPlots(step);
			sel_step[6]+=puweight;
		}
		if(isZrange){

			zplots.makeControlPlots(step);
		}

                if(testmode_ && entry==0)
                        std::cout << "testmode("<< anaid << "): STEP6 " << std::endl;


		//////////////////// MET cut STEP 7//////////////////////////////////
		step++;
		if(!nometcut && !b_emu_ && adjustedmet.met() < 40) continue;



		if(analysisMllRange){
			/*
			lh_toplh=toplikelihood.getCombinedLikelihood();
			toplikelihood.fill(puweight);
			 */
			plots.makeControlPlots(step);
			sel_step[7]+=puweight;
		}
		if(isZrange){
			zplots.makeControlPlots(step);

		}

		//make the b-tag SF
		for(size_t i=0;i<selectedjets->size();i++){
			getBTagSF()->fillEff(selectedjets->at(i),puweight);
		}
                if(testmode_ && entry==0)
                        std::cout << "testmode("<< anaid << "): STEP7 " << std::endl;


		///////////////////// btag cut STEP 8 (and kinematic reconstruction) //////////////////////////
		step++;

                float mtt, m_mub, pt_top, eta_top, pt_antitop, eta_antitop, pt_ttbar, eta_ttbar;

                KinematicReconstructionSolutions kinRecoSols;
                if (doKinReco_){

                    if (leadingptlep->q()*secleadingptlep->q()>0) continue;
                    if (signal_) puweight *= kinRecoSF_->getSF();

                    bool isAntiMuon = false;
                    LV muon_lv;

                    VLV leptonsVLV;
                    TLorentzVector tlv_temp;

                    tlv_temp.SetPtEtaPhiE(leadingptlep->pt(),leadingptlep->eta(),leadingptlep->phi(),leadingptlep->e());
                    leptonsVLV.push_back(common::TLVtoLV(tlv_temp));

                    tlv_temp.SetPtEtaPhiE(secleadingptlep->pt(),secleadingptlep->eta(),secleadingptlep->phi(),secleadingptlep->e());
                    leptonsVLV.push_back(common::TLVtoLV(tlv_temp));

                    if (isMuonLeadingLepton) muon_lv = leptonsVLV.at(0);
                    else muon_lv = leptonsVLV.at(1);


                    int leptonIndex = 0;
                    int antileptonIndex = 0;
                    if (leadingptlep->q()<0) {
                        antileptonIndex = 1;
                        if (!isMuonLeadingLepton) isAntiMuon = true;
                    }
                    else{
                        leptonIndex = 1;
                        if (isMuonLeadingLepton) isAntiMuon = true;
                    }
                    TVector2 METv2; METv2.SetMagPhi(adjustedmet.met(),adjustedmet.phi());
                    LV METLV; METLV.SetXYZT(METv2.Px(),METv2.Py(),0,0);

                    std::vector<double> dummy;
                    std::vector<int> jet_indices;

                    for (unsigned int i=0; i<selectedjets->size(); ++i) jet_indices.push_back(i);

                    kinRecoSols = this->getKinRecoSolutions(leptonIndex, antileptonIndex, jet_indices, bjet_indices, leptonsVLV, jetVLV, dummy, METLV);

                    if (kinRecoSols.numberOfSolutions()<1) continue;

                    const KinematicReconstructionSolution solution = kinRecoSols.solution();

                    TLorentzVector top_sol = common::LVtoTLV(solution.top());
                    TLorentzVector antitop_sol = common::LVtoTLV(solution.antiTop());
                    TLorentzVector ttbar_sol = common::LVtoTLV(solution.ttbar());
                    // int n_bTags = solution.numberOfBtags();

                    mtt = ttbar_sol.M();
                    pt_top = top_sol.Pt();
                    eta_top = top_sol.Eta();
                    pt_antitop = antitop_sol.Pt();
                    eta_antitop = antitop_sol.Eta();
                    pt_ttbar = ttbar_sol.Pt();
                    eta_ttbar = ttbar_sol.Eta();

                    int bjet_index = solution.antiBjetIndex();
                    if (isAntiMuon) bjet_index = solution.bjetIndex();
                    m_mub = (muon_lv+jetVLV.at(bjet_index)).M();

                    evt.mtt = &mtt;
                    evt.m_mub = &m_mub;
                    evt.pt_top = &pt_top;
                    evt.eta_top = &eta_top;
                    evt.pt_antitop = &pt_antitop;
                    evt.eta_antitop = &eta_antitop;
                    evt.pt_ttbar = &pt_ttbar;
                    evt.eta_ttbar = &eta_ttbar;

                }


                if(apllweightsone) puweight=1;

		if(!usetopdiscr && !nobcut && selectedbjets.size() < 1) continue;
		// if(usetopdiscr && topdiscr3<0.9) continue;
		if(usetopdiscr && lh_toplh<0.3) continue;

                if(testmode_ && entry==0)
                        std::cout << "testmode("<< anaid << "): BTagCuts " << std::endl;

		//agrohsje print run and event numbers 
		//std::cout<< *b_EventNumber.content()<<std::endl;// "  "<< *b_RunNumber.content() <<std::endl;

		if(apllweightsone) puweight=1;

		float mlbmin=0;
		evt.mlbmin=&mlbmin;
                if(testmode_ && entry==0)
                        std::cout << "testmode("<< anaid << "): STEP8 " << std::endl;


		if(analysisMllRange){

			// std::cout << selectedjets->at(0)->pt() << std::endl;
			if(selectedbjets.size()>0){
				mlbmin=(leadingptlep->p4()+selectedbjets.at(0)->p4()).m();
				float tmp=(secleadingptlep->p4()+selectedbjets.at(0)->p4()).m();
				if(mlbmin>tmp)mlbmin=tmp;
			}
                if(testmode_ && entry==0)
                        std::cout << "testmode("<< anaid << "): before Control Plots " << std::endl;


			xsecplots.makeControlPlots(step);

			plots.makeControlPlots(step);
			sel_step[8]+=puweight;

                if(testmode_ && entry==0)
                        std::cout << "testmode("<< anaid << "): after Control Plots " << std::endl;


			mlbmtplots_step8.fillPlotsReco();
			xsecfitplots_step8.fillPlotsReco();
			if (doKinReco_) kinrecoplots_step8.fillPlotsReco();

		}
		if(isZrange){
			zplots.makeControlPlots(step);
		}

		if(tickoncemode_) {
			std::cout << "tickOnce("<< anaid << "): finished main loop once. break"<<std::endl;
			break; //one event survived, sufficient
		}
		//agrohsje debug pu 
		if (randomNum < 0.548)pusum_sel+=getPUReweighterBtoF()->getPUweight(b_Event.content()->truePU());
                else pusum_sel+=getPUReweighterGH()->getPUweight(b_Event.content()->truePU());
		nEntries_sel++;
	}
	//clear input tree and close
	tree.clear();

	///////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////
	///////////////////////    MAIN LOOP ENDED    /////////////////////////
	///////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////
	///////////////////////    POST PROCESSING    /////////////////////////
	///////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////


	//renorm for topptreweighting
	double renormfact=getTopPtReweighter()->getRenormalization();
	norm_ *= renormfact;
	if(testmode_ )
		std::cout << "testmode("<< anaid << "): finished main loop, renorm factor top pt: " <<renormfact  << std::endl;

	//renorm after pdf reweighting (only acceptance effects!
	renormfact=getPdfReweighter()->getRenormalization();
	norm_ *= renormfact;
	if(testmode_ )
		std::cout << "testmode("<< anaid << "): finished main loop, renorm factor pdf weights: " <<renormfact  << std::endl;

	//renorm all mc weights 
	for(size_t i=0;i<weightbranches.size();i++){
		renormfact=mcreweighters.at(i).getRenormalization();
		norm_ *= renormfact;
		if(testmode_ )
			std::cout << "testmode("<< anaid << "): finished main loop, renorm factor for mc reweighting["<<i<<"]: " <<renormfact  << std::endl;
	}

	histo1DUnfold::flushAllListed(); // call once again after last event processed


	///////////////////////////////
	///////////////////////////////
	///////////////////////////////
	///////////////////////////////
	//     WRITE OUTPUT PART     //
	///////////////////////////////
	///////////////////////////////
	///////////////////////////////
	///////////////////////////////


	std::cout << inputfile_ << ": " << std::endl;
	for(unsigned int i=0;i<9;i++){
		std::cout << "selection step "<< toTString(i)<< " "  << sel_step[i];
		if(i==3)
			cout << "  => sync step 1 \tmll>" << mllcut;
		if(i==4)
			cout << "  => sync step 2 \tZVeto";
		if(i==6)
			cout << "  => sync step 3 \t2 Jets";
		if(i==7)
			cout << "  => sync step 4 \tMET";
		if(i==8)
			cout << "  => sync step 5 \t1btag";
		std::cout  << std::endl;
	}


	std::cout << "\nEvents total (normalized): "<< nEntries*norm_
			<< "\n nEvents_selected normd: "<< sel_step[8]*norm_
			<< " with norm " << norm_ << " for " << inputfile_<< std::endl;
	//agrohsje check phase space bias in weights
	std::cout<<"selection bias for PU?: \n"
			<<"pusum=" << pusum << " for all nEntries=" << nEntries
			<< ", pusum_sel normd=" << pusum_sel*norm_ << " for selected events normd=" << nEntries_sel*norm_ <<"\n"
			<<"selection bias for mc weight? \n"
			<<"mcwsum="<< mcwsum << " for all nEntries=" << nEntries
			<< std::endl;
	//all operations done
	//check if everything was written correctly


	processEndFunction();

}


bool top_analyzer_run2::checkTrigger(std::vector<bool> * p_TriggerBools,ztop::NTEvent * pEvent, bool isMC,size_t anaid)
{
        //if (isMC) return true;
	if(b_emu_){
                //if (isDilep) return true;//std::cout<<p_TriggerBools->at(32)<<p_TriggerBools->at(33)<<std::endl;
                if (isMC &&(p_TriggerBools->at(32)||p_TriggerBools->at(33)|| p_TriggerBools->at(34)|| p_TriggerBools->at(35)|| p_TriggerBools->at(36) ) ) return true;
                if(!isRunH && isDilep && (p_TriggerBools->at(32)||p_TriggerBools->at(33)) ) return true;
                if(!isRunH && isSingleEle && !(p_TriggerBools->at(32)||p_TriggerBools->at(33)) && p_TriggerBools->at(36) ) return true;
                if(!isRunH && isSingleMu && !(p_TriggerBools->at(32)||p_TriggerBools->at(33)) && !(p_TriggerBools->at(36))  &&  (p_TriggerBools->at(34)|| p_TriggerBools->at(35)) ) return true;
                if(isRunH && isDilep && (p_TriggerBools->at(43)||p_TriggerBools->at(44))) return true ;
                if (isRunH && isSingleEle && !(p_TriggerBools->at(43)||p_TriggerBools->at(44))  && p_TriggerBools->at(36)) return true;
                if (isRunH && isSingleMu && !(p_TriggerBools->at(43)||p_TriggerBools->at(44)) && !(p_TriggerBools->at(36)) && (p_TriggerBools->at(34)|| p_TriggerBools->at(35))) return true;
                //else if (isRunH && isSingleEle &&!(p_TriggerBools->at(43)||p_TriggerBools->at(44))  && p_TriggerBools->at(36)) return true;
	}
	else if(b_mumu_){
                if (isMC && (p_TriggerBools->at(28)||p_TriggerBools->at(30) || p_TriggerBools->at(34)|| p_TriggerBools->at(35))) return true;
                if(!isRunH && isDilep && (p_TriggerBools->at(28)||p_TriggerBools->at(30)) ) return true;
                if(!isRunH && isSingleMu && ! (p_TriggerBools->at(28)||p_TriggerBools->at(30)) && (p_TriggerBools->at(34)|| p_TriggerBools->at(35)) ) return true;
                if(isRunH && isDilep && (p_TriggerBools->at(27)||p_TriggerBools->at(29))) return true;
                if(isRunH && isSingleMu && ! (p_TriggerBools->at(27)||p_TriggerBools->at(29)) && (p_TriggerBools->at(34)|| p_TriggerBools->at(35))) return true;

	}
	else if(b_ee_){
                if (isMC &&(p_TriggerBools->at(31) || p_TriggerBools->at(36))) return true;
		if(isDilep && p_TriggerBools->at(31) ) return true;
                if(isSingleEle && ! p_TriggerBools->at(31) && p_TriggerBools->at(36)) return true;
	}
        else if (b_smu_){
                if(p_TriggerBools->at(34)|| p_TriggerBools->at(35)) return true;
	}
        return false;
}



void top_analyzer_run2::setMttCategories(float gen_mtt, TString legendname){
    if ((int)cat_mttmax_leg != (int)cat_mttmax_gen)
        throw std::logic_error("top_analyzer_run2:::setMttCategories: gen and legened mtt bins must be the same");
    
    if (gen_mtt < bin_1_) gen_mttcategory = cat_mtt1_gen;
    else if (gen_mtt < bin_2_) gen_mttcategory = cat_mtt2_gen;
    else if (gen_mtt < bin_3_) gen_mttcategory = cat_mtt3_gen;
    else gen_mttcategory = cat_mttmax_gen;

    legendname.ReplaceAll(" ","");
    
    if (legendname.EndsWith("mtt1")) leg_mttcategory = cat_mtt1_leg;
    else if (legendname.EndsWith("mtt2")) leg_mttcategory = cat_mtt2_leg;
    else if (legendname.EndsWith("mtt3")) leg_mttcategory = cat_mtt3_leg;
    else if (legendname.EndsWith("mtt4")) leg_mttcategory = cat_mttmax_leg;
    else 
        throw std::runtime_error((std::string)"top_analyzer_run2:::setMttCategories: category for legend "+legendname+" not found");

}
