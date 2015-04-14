/*
 * analyzer_run2.cc based on former eventLoop.h
 *
 *  Created on: Jul 17, 2013
 *      Author: kiesej
 *
 *
 *      this file defines the analyze() function of MainAnalyzer and is put in a
 *      separate file only for structure reasons!
 *      do not include it in any other file than MainAnalyzer.cc
 */


#include "../interface/analyzer_run2.h"
#include "TtZAnalysis/Configuration/interface/version.h"

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
void  analyzer_run2::analyze(size_t anaid){
#ifndef CMSSW_LEQ_5
	using namespace std;
	using namespace ztop;



	TString inputfile=infiles_.at(anaid); //modified in some mode options
	const TString& legendname=legentries_.at(anaid);
	const int &    color=colz_.at(anaid);
	const size_t & legord=legord_.at(anaid);
	//const TString& extraopts=extraopts_.at(anaid); //not used right now




	bool issignal=issignal_.at(anaid);

	bool isMC=true;
	if(legendname==dataname_) isMC=false;

	if(!isMC || !issignal)
		getPdfReweighter()->switchOff(true);

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
		std::cout << "entering Nometcut mode" <<std::endl;
	}
	if(mode_.Contains("Nobcut")){
		nobcut=true;
		std::cout << "entering Nobcut mode" <<std::endl;
	}
	if(mode_.Contains("Topdiscr")){
		usetopdiscr=true;
		std::cout << "entering Topdiscr mode" <<std::endl;
	}
	//hmm this should actually go to readFiles function.. but then interferes with the mode idea.. leave it here for the moment
	if(mode_.Contains("Mgdecays")){
		std::cout << "entering Mgdecays mode: norm will be adapted" <<std::endl;
		if(inputfile.Contains("ttbar.root")){
			inputfile.ReplaceAll("ttbar.root", "ttbar_mgdecays.root");
			// infiles_.at(anaid).ReplaceAll("ttbar.root", "ttbar_mgdecays.root");
		}
		else if(inputfile.Contains("ttbarviatau.root")){
			inputfile.ReplaceAll("ttbarviatau.root", "ttbarviatau_mgdecays.root");
			// infiles_.at(anaid).ReplaceAll("ttbar.root", "ttbar_mgdecays.root");
		}
	}
	else{
		//adapt wrong MG BR for madspin and syst samples
		if(inputfile.Contains("ttbar.root") || inputfile.Contains("ttbarviatau.root")
				|| inputfile.Contains("ttbar_") ||
				inputfile.Contains("ttbarviatau_") ){
			if(  ! (syst_.BeginsWith("TT_GEN") && syst_.EndsWith("_up"))  ) //other generator
				normmultiplier=(0.1062/0.11104);//correct both W
			/*
			 * BR W-> lnu: 0.1086
			 * n_comb for leptonic: 1+1+1+2+2+2 (incl taus)
			 * total lept branching fraction for WW: 0.1086^2 * 9 = 0.1062
			 * In Madgraph: 0.11104
			 */
		}
	}
	if(inputfile.Contains("_mgdecays_") || inputfile.Contains("_tbarWtoLL")|| inputfile.Contains("_tWtoLL")){
		normmultiplier=0.1062; //fully leptonic branching fraction for both Ws
	}



	bool fakedata=false,isfakedatarun=false;
	if(mode_.Contains("Fakedata")){
		if(legendname ==  dataname_)
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
	else{
		jetptcut=30;
	}
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
		getElecSF()->setSystematics("nom");
		getMuonEnergySF()->setSystematics("nom");
		getMuonSF()->setSystematics("nom");

		getTrackingSF()->setSystematics("nom");

		// getPUReweighter()-> //setSystematics("nom");
		getBTagSF()->setSystematics(NTBTagSF::nominal);
		getJECUncertainties()->setSystematics("no");
		getJERAdjuster()->setSystematics("def");

		getTopPtReweighter()->setSystematics(reweightfunctions::nominal); //setSystematics("nom");

		getPdfReweighter()->switchOff(true);

		//for other parameters reread config
		fileReader fr;
		fr.setComment("$");
		fr.setDelimiter(",");
		fr.setStartMarker("[parameters-begin]");
		fr.setEndMarker("[parameters-end]");
		fr.readFile(pathtoconffile_.Data());

		getPUReweighter()->setDataTruePUInput(((std::string)getenv("CMSSW_BASE")+fr.getValue<string>("PUFile") +".root").data());


		// not needed anymore
		//re-adjust systematic filenames
		for(size_t i=0;i<ftorepl_.size();i++){
			if(inputfile.EndsWith(ftorepl_.at(i))){
				std::cout << "replacing fakedata syst names " << fwithfix_.at(i) << " with " << ftorepl_.at(i) << std::endl;
				//	inputfile.ReplaceAll(fwithfix_.at(i),ftorepl_.at(i));
			}
		}

		//this is bad style.. anyway
	}


	//per sample configuration


	//check if file exists
	if(testmode_)
		std::cout << "testmode("<< anaid << "): check input file "<<inputfile << " (isMC)"<< isMC << std::endl;

	TFile *f;
	if(!fileExists((datasetdirectory_+inputfile).Data())){
		std::cout << datasetdirectory_+inputfile << " not found!!" << std::endl;
		reportError(-1,anaid);
		return;
	}
	else{
		f=TFile::Open(datasetdirectory_+inputfile);
	}

	NTFullEvent evt;

	////////////////////  configure discriminator factories here  ////////////////////
	/*
	TString factname="topLH_"+channel_+"_"+energy_+"_"+topmass_;
	discriminatorFactory::c_makelist=false; //now will be ignored in output
	discriminatorFactory toplikelihood(factname.Data());

	discriminatorFactory::c_makelist=false;

	if(usediscr_){
		toplikelihood.readFromTFile(discrInput_,toplikelihood.getIdentifier());

	}
	else{
		toplikelihood.setNBins(40);
		toplikelihood.setStep(7); //includes met for ee/mumu
	}

	toplikelihood.setUseLikelihoods(usediscr_);

	toplikelihood.setSaveCorrelationPlots(false);

	toplikelihood.addVariable(&evt.lhi_dphillj,"#Delta#phi(ll,j)",0,M_PI);
	toplikelihood.addVariable(&evt.lhi_cosleplepangle,"#cos#theta_{ll}",-1.,1.);
	// if(!usediscr_)
	//      toplikelihood.addVariable(&evt.lhi_leadjetbtag,"D_{b}^{jet1}",-1,1);
	toplikelihood.addVariable(&evt.lhi_sumdphimetl,"#Delta#phi(met,l_{1})+#Delta#phi(met,l_{2})",0,2*M_PI);
	//  toplikelihood.addVariable(&evt.lhi_seljetmulti,"N_{jets}",-0.5,4.5);
	//  toplikelihood.addVariable(&evt.lhi_selbjetmulti,"N_{b-jets}",-0.5,4.5);
	toplikelihood.addVariable(&evt.lhi_leadleppt,"p_{T}^{1st lep}",0,150);
	toplikelihood.addVariable(&evt.lhi_secleadleppt,"p_{T}^{2nd lep}",0,150);
	toplikelihood.addVariable(&evt.lhi_leadlepeta,"#eta^{1st lep}",-2.4,2.4);
	toplikelihood.addVariable(&evt.lhi_secleadlepeta,"#eta^{2nd lep}",-2.4,2.4);
	toplikelihood.addVariable(&evt.lhi_leadlepphi,"#phi^{1st lep}",-M_PI,M_PI);
	toplikelihood.addVariable(&evt.lhi_secleadlepphi,"#phi^{2nd lep}",-M_PI,M_PI);
	//  toplikelihood.addVariable(&evt.lhi_leadjetpt,"p_{T}^{1st jet}",0,150);
	toplikelihood.addVariable(&evt.lhi_mll,"m_{ll}",0.,300);
	// toplikelihood.addVariable(&evt.lhi_ptllj,"p_{T}(llj) [GeV]",50,400);



	//   toplikelihood.addVariable(&evt.lhi_cosllvsetafirstlep,"cos#theta vs #eta_{l1}",-4,2);
	//   toplikelihood.addVariable(&evt.lhi_etafirstvsetaseclep,"#eta_{l1} vs #eta_{l2}",-3,3);
	//  toplikelihood.addVariable(&evt.lhi_deltaphileps,"#Delta#phi(l_{1},l_{2})",-2*M_PI,2*M_PI);
	//  toplikelihood.addVariable(&evt.lhi_coslepanglevsmll,"cos#theta vs m_{ll}",-1,1);
	//  toplikelihood.addVariable(&evt.lhi_mllvssumdphimetl,"m_{ll} vs #Delta#phi(met,l_{1})+#Delta#phi(met,l_{2})",-10,0.5);


	if(usediscr_){
		toplikelihood.setSystematics(getSyst());
	}
	else{
		toplikelihood.setSystematics("nominal");
	}

	 */

	/////////////////////////////////////////////////////////////////////////////////

	//just a test

	if(testmode_)
		std::cout << "testmode("<< anaid << "): preparing container1DUnfolds" << std::endl;

	//////////////analysis plots/////////////


	analysisPlotsMlbMt mlbmtplots_step8(8);
	analysisPlotsTtbarXsecFit xsecfitplots_step8(8);

	//xsecfitplots_step8.enable();
	mlbmtplots_step8.enable();
	mlbmtplots_step8.bookPlots();
	xsecfitplots_step8.enable();
	xsecfitplots_step8.bookPlots();

	//global settings for analysis plots
	histo1DUnfold::setAllListedMC(isMC && !fakedata);
	histo1DUnfold::setAllListedLumi((float)lumi_);
	if(testmode_)
		histo1DUnfold::setAllListedLumi((float)lumi_*0.08);

	//setup everything for control plots

	ttbarControlPlots plots;//for the Z part just make another class (or add a boolian)..
	////FIX!!
	//plots.limitToStep(8);
	ZControlPlots zplots;
	plots.linkEvent(evt);
	zplots.linkEvent(evt);
	ttXsecPlots xsecplots;
	xsecplots.enable(false);
	xsecplots.linkEvent(evt);
	xsecplots.limitToStep(8);
	xsecplots.initSteps(8);
	plots.initSteps(8);
	zplots.initSteps(8);
	mlbmtplots_step8.setEvent(evt);
	xsecfitplots_step8.setEvent(evt);


	//get normalization - switch on or off pdf weighter before!!!
	double norm=createNormalizationInfo(f,isMC,anaid);
	if(testmode_)
		std::cout << "testmode("<< anaid << "): multiplying norm with "<< normmultiplier <<" file: " << inputfile<< std::endl;
	norm*= normmultiplier;
	/////////////////////////// configure scalefactors ////


	//init b-tag scale factor utility
	if(testmode_)
		std::cout << "testmode("<< anaid << "): preparing btag SF" << std::endl;
/*
 * 	if(mode_.Contains("Btagcsvt") && ! mode_.Contains("Btagshape")){
 * 		getBTagSF()->setWorkingPoint("csvt");
 * 		std::cout << "entering btagcsvt mode" <<std::endl;
 * 	}
 * 	if(mode_.Contains("Btagcsvm")&& ! mode_.Contains("Btagshape")){
 * 		getBTagSF()->setWorkingPoint("csvm");
 * 		std::cout << "entering btagcsvm mode" <<std::endl;
 * 	}

	//make sure the nominal scale factors are used for varations of the SF
	TString btagSysAdd=topmass_+"_"+getSyst();
//	if(getBTagSF()->isRealSyst() || wasbtagsys)
//		btagSysAdd=topmass_+"_nominal";

	if(fakedata) //always use nominal
		btagSysAdd+="_fakedata";
	std::string btagsamplename=(channel_+"_"+btagSysAdd+"_"+toString(inputfile)).Data();
//	if(getBTagSF()->setSampleName(btagsamplename) < 0){
//		reportError(-3,anaid);
//		return;
//	}
*/
	TString btagfile=btagefffile_;
        btagfile+="_"+inputfile;
	getBTagSF()->setIsMC(isMC);
	if(!getBTagSF()->getMakeEff())
        getBTagSF()->readFromFile(btagfile.Data());
	//  if(testmode_)
	//    std::cout << "testmode(" <<anaid << ") setBtagSmaplename " <<channel_+"_"+btagSysAdd+"_"+toString(inputfile)).Data() <<std::endl;

	//range check switched off because of different ranges in bins compared to diff Xsec (leps)
	getTriggerSF()->setRangeCheck(false);
	getElecSF()->setRangeCheck(false);
	getMuonSF()->setRangeCheck(false);
	getTrackingSF()->setRangeCheck(false);

	getElecSF()->setIsMC(isMC);
	getMuonSF()->setIsMC(isMC);
	getTriggerSF()->setIsMC(isMC);
	getTrackingSF()->setIsMC(isMC);

	//some global checks
	getElecEnergySF()->setRangeCheck(false);
	getMuonEnergySF()->setRangeCheck(false);
	getElecEnergySF()->setIsMC(isMC);
	getMuonEnergySF()->setIsMC(isMC);





	/*
	 * Open Main tree,
	 * Set branches
	 * the handler is only a small wrapper
	 */
	TTree * t = (TTree*) f->Get("PFTree/PFTree");

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

	std::vector<ztop::MCReweighter> mcreweighters;

	for(size_t i=0;i<additionalweights_.size();i++){
		std::cout << "adding weight " << additionalweights_.at(i) << std::endl;
		tBranchHandler<NTWeight> * weight = new tBranchHandler<NTWeight>(t,additionalweights_.at(i));
		weightbranches.push_back(weight);
		//agrohsje 
		ztop::MCReweighter mcreweighter; 
		mcreweighters.push_back(mcreweighter);
	}

	//some helpers
	double sel_step[]={0,0,0,0,0,0,0,0,0};
	float count_samesign=0;

	if(!testmode_){
		// this enables some caching while reading the tree. Speeds up batch mode
		// significantly!
		setCacheProperties(t,datasetdirectory_+inputfile);
	}

	Long64_t nEntries=t->GetEntries();
	if(norm==0) nEntries=0; //skip for norm0
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
					<< " old norm: " << norm << " to " << norm*normmultif << " file: " << inputfile<< std::endl;
				norm*= normmultif;
			}
			else{// if(){
				skipregion=false;

				float normmultif=1/(1-splitfractionMC);
				if(testmode_)
					std::cout << "testmode("<< anaid << "):\t splitted MC fraction off for pseudo data "<< 1-splitfractionMC
					<< " old norm: " << norm << " to " << norm*normmultif << " file: " << inputfile<< std::endl;
				norm*=normmultif;

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
		std::cout << "testmode("<< anaid << "): starting mainloop with file "<< inputfile << " norm " << norm << " entries: "<<nEntries << " fakedata: " <<  fakedata<<std::endl;


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

		t->LoadTree(entry);

		////////////////////////////////////////////////////
		////////////////////  INIT EVENT ///////////////////
		/////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////


		size_t step=0;
		evt.reset();
		evt.event=b_Event.content();
		histo1DUnfold::flushAllListed(); //important to call each event

		//reports current status to parent
		reportStatus(entry,nEntries,anaid);

		b_Event.getEntry(entry);
		float puweight=1;
		if (isMC) puweight = getPUReweighter()->getPUweight(b_Event.content()->truePU());
		if(apllweightsone) puweight=1;
		if(testmode_ && entry==0)
			std::cout << "testmode("<< anaid << "): got first event entry" << std::endl;

		evt.puweight=&puweight;

		getPdfReweighter()->setNTEvent(b_Event.content());
		getPdfReweighter()->reWeight(puweight);
		if(apllweightsone) puweight=1;
		//agrohsje loop over additional weightbranches 
		for(size_t i=0;i<weightbranches.size();i++){
			weightbranches.at(i)->getEntry(entry);
			mcreweighters.at(i).setMCWeight(weightbranches.at(i)->content()->getWeight());
			mcreweighters.at(i).reWeight(puweight);
			if(apllweightsone) puweight=1;		
		}


		/////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////
		/////////////////// Generator Information////////////////////
		/////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////

		////define all collections
		// do not move somewhere else!

		vector<NTGenParticle*>  gentops,genbs,genbsrad;
		vector<NTGenParticle*>  genws;
		vector<NTGenParticle *> genleptons1,genleptons3;
		vector<NTGenJet *>      genjets;
		vector<NTGenParticle *> genbhadrons;

		evt.gentops=&gentops;
		evt.genbs=&genbs;
		evt.genbsrad=&genbsrad;
		evt.genbhadrons=&genbhadrons;
		evt.genleptons1=&genleptons1;
		evt.genleptons3=&genleptons3;
		evt.genjets=&genjets;


		//move back to geninfo part, only here for testing
		b_GenLeptons3.getEntry(entry);
		b_GenTops.getEntry(entry);
		b_GenWs.getEntry(entry);
		b_GenBs.getEntry(entry);
		b_GenBsRad.getEntry(entry);
		b_GenBHadrons.getEntry(entry);
		b_GenJets.getEntry(entry);
		b_GenLeptons1.getEntry(entry);


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
			genleptons1=produceCollection(b_GenLeptons1.content(),&genleptons3);

			//b-hadrons that stem from a b quark that itself originates in a top are
			//assoziated to that top by the bhadronmatcher (Nazar)
			//this logic is used and kept here
			genbhadrons=produceCollection(b_GenBHadrons.content(), &gentops);
			//try to associate the mothers. If successful, b-jet is matched to hadron -> to top
			genjets=produceCollection(b_GenJets.content());//,&genbhadrons);

			//}
			if(!fakedata){
				/*
				 * fill gen info here
				 */
				mlbmtplots_step8.fillPlotsGen();
				xsecfitplots_step8.fillPlotsGen();
			}
		} /// isMC ends

		/////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////
		///////////////////       Reco Part      ////////////////////
		/////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////



		/*
		 *  Trigger
		 */
		b_TriggerBools.getEntry(entry);
		if(testmode_ && entry==0)
			std::cout << "testmode("<< anaid << "): got trigger boolians" << std::endl;
		//agrohsje uncomment for time being if(!checkTrigger(b_TriggerBools.content(),b_Event.content(), isMC,anaid)) continue;


		/*
		 * Muons
		 */
		b_Muons.getEntry(entry);


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
			if(isMC)
			    muon->setP4(muon->p4() * getMuonEnergySF()->getScalefactor(muon->eta()));
			allleps << muon;
			if(muon->pt() < lepptthresh)       continue;
			if(fabs(muon->eta())>2.4) continue;
			kinmuons << &(b_Muons.content()->at(i));

			//tight muon selection: https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideMuonId#Tight_Muon
			//agrohsje isPF requirement already at ntuple level 
			
			if(muon->isGlobal()
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
			if(!mode_invertiso && fabs(muon->isoVal()) > 0.12) continue;
			if(mode_invertiso && muon->isoVal() < 0.12) continue;
			isomuons <<  muon;
			isoleptons << muon;
		}

		/*
		 * Electrons
		 */

		b_Electrons.getEntry(entry);


		vector<NTElectron *> kinelectrons,idelectrons,isoelectrons;
		evt.kinelectrons=&kinelectrons;
		evt.idelectrons=&idelectrons;
		evt.isoelectrons=&isoelectrons;

		for(size_t i=0;i<b_Electrons.content()->size();i++){
			NTElectron * elec=&(b_Electrons.content()->at(i));
			float ensf=1;
			if(isMC)
			    ensf=getElecEnergySF()->getScalefactor(elec->eta());
			elec->setECalP4(elec->ECalP4() * ensf);
			elec->setP4(elec->ECalP4() * ensf); //both the same now!!

			allleps << elec;
			if(elec->pt() < lepptthresh)  continue;
			float abseta=fabs(elec->eta());

			float suclueta = fabs(elec->ECalP4().eta());
			if(abseta > 2.5) continue;
			if(suclueta > 1.4442 && suclueta < 1.5660) continue; //transistion region
			kinelectrons  << elec;

			if(elec->storedId() > 0.9){  ////agrohsje 1 or 0 should work 
			    idelectrons <<  elec;
			    //if(fabs(elec->isoVal(/* agrohsje rhoIso()*/))<0.1){ // iso already done in id 
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

		sel_step[1]+=puweight;
		plots.makeControlPlots(step);
		zplots.makeControlPlots(step);



		//////// require two iso leptons  STEP 2  //////////////////////////
		step++;

		if(b_ee_ && isoelectrons.size() < 2) continue;
		if(b_mumu_ && isomuons.size() < 2 ) continue;
		if(b_emu_ && isomuons.size() + isoelectrons.size() < 2) continue;

		//make pair
		pair<vector<NTElectron*>, vector<NTMuon*> > oppopair,sspair;
		oppopair = ztop::getOppoQHighestPtPair(isoelectrons, isomuons);
		sspair = ztop::getOppoQHighestPtPair(isoelectrons, isomuons,-1);

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
			lepweight*=getElecSF()->getScalefactor((firstlep->eta()),firstlep->pt());
			lepweight*=getElecSF()->getScalefactor((seclep->eta()),seclep->pt());
			lepweight*=getTriggerSF()->getScalefactor(fabs(firstlep->eta()),fabs(seclep->eta()));
		}
		else if(b_mumu_){
			if(leppair->second.size() < 2) continue;
			dilp4=leppair->second[0]->p4() + leppair->second[1]->p4();
			mll=dilp4.M();
			firstlep=leppair->second[0];
			seclep=leppair->second[1];
			lepweight*=getMuonSF()->getScalefactor(firstlep->pt(),fabs(firstlep->eta()));
			lepweight*=getMuonSF()->getScalefactor(seclep->pt(),fabs(seclep->eta()));
			lepweight*=getTrackingSF()->getScalefactor((firstlep->eta()));
			lepweight*=getTrackingSF()->getScalefactor((seclep->eta()));
			lepweight*=getTriggerSF()->getScalefactor(fabs(firstlep->eta()),fabs(seclep->eta()));
		}
		else if(b_emu_){
		    
		    if(leppair->first.size() < 1 || leppair->second.size() < 1) continue;
			dilp4=leppair->first[0]->p4() + leppair->second[0]->p4();
			mll=dilp4.M();
			firstlep=leppair->first[0];
			seclep=leppair->second[0];
			lepweight*=getElecSF()->getScalefactor((firstlep->eta()),firstlep->pt());
			lepweight*=getMuonSF()->getScalefactor(seclep->pt(),fabs(seclep->eta()));
			lepweight*=getTrackingSF()->getScalefactor((seclep->eta()));
			lepweight*=getTriggerSF()->getScalefactor(fabs(firstlep->eta()),fabs(seclep->eta()));
		}
		//channel defined
		if(firstlep->pt() > seclep->pt()){
			leadingptlep=firstlep;
			secleadingptlep=seclep;
		}
		else{
			leadingptlep=seclep;
			secleadingptlep=firstlep;
		}
		//just to avoid warnings
		evt.leadinglep=leadingptlep;
		evt.secleadinglep=secleadingptlep;
		evt.mll=&mll;


		float leplepdr=dR_3d(leadingptlep->p4(),secleadingptlep->p4());
		evt.leplepdr=&leplepdr;
		float cosleplepangle=cosAngle_3d(leadingptlep->p4(),secleadingptlep->p4());
		evt.cosleplepangle=&cosleplepangle;

		puweight*=lepweight;
		if(apllweightsone) puweight=1;
		//just a quick faety net against very weird weights
		if(isMC && fabs(puweight) > 99999){
			reportError(-88,anaid);
			return;
		}

		sel_step[2]+=puweight;
		plots.makeControlPlots(step);
		zplots.makeControlPlots(step);

	//	if(leadingptlep->pt()<30)
	//		continue;

		///////// 20 GeV cut /// STEP 3 ///////////////////////////////////////
		step++;

		if(mll < 20)
			continue;

		// create jec jets for met and ID jets
		// create ID Jets and correct JER
		b_Jets.getEntry(entry);



		vector<NTJet *> treejets,idjets,medjets,hardjets;
		evt.idjets=&idjets;
		evt.medjets=&medjets;
		evt.hardjets=&hardjets;
		for(size_t i=0;i<b_Jets.content()->size();i++){
		        //agrohsje added fake scaling
		        b_Jets.content()->at(i).setP4(b_Jets.content()->at(i).p4()*1.0);
			treejets << &(b_Jets.content()->at(i));
		}

		double dpx=0;
		double dpy=0;

		for(size_t i=0;i<treejets.size();i++){ //ALSO THE RESOLUTION AFFECTS MET. HERE INTENDED!!! GOOD?
			NTLorentzVector<float>  oldp4=treejets.at(i)->p4();
			if(isMC){// && !is7TeV_){
				bool useJetForMet=false;
				if(treejets.at(i)->emEnergyFraction() < 0.9 && treejets.at(i)->pt() > 10)
					useJetForMet=true; //dont even do something

				getJECUncertainties()->applyToJet(treejets.at(i));
				//agrohsje uncomment 
				getJERAdjuster()->correctJet(treejets.at(i));
				//corrected
				if(useJetForMet){
					dpx += oldp4.Px() - treejets.at(i)->p4().Px();
					dpy += oldp4.Py() - treejets.at(i)->p4().Py();
				}
			}
			if(!(treejets.at(i)->id())) continue;
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



		b_Met.getEntry(entry);



		NTMet adjustedmet = *b_Met.content();
		evt.simplemet=b_Met.content();
		evt.adjustedmet=&adjustedmet;
		double nmpx=b_Met.content()->p4().Px() + dpx;
		double nmpy=b_Met.content()->p4().Py() + dpy;
		if(!usemvamet)
			adjustedmet.setP4(D_LorentzVector(nmpx,nmpy,0,sqrt(nmpx*nmpx+nmpy*nmpy))); //COMMENTED FOR MVA MET

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


	//	getBTagSF()->changeNTJetTags(selectedjets);
		for(size_t i=0;i<hardjets.size();i++){
		//	if(selectedjets->at(i)->btag() < getBTagSF()->getWPDiscrValue()){

		//		selectednonbjets.push_back(selectedjets->at(i));
		//		continue;
		//	}
			selectedbjets.push_back(selectedjets->at(i));
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


		///////////////////// at least one jet cut STEP 5 ////////////
		step++;

		if(!zerojet && selectedjets->size() < 1) continue;

		if(getBTagSF()->getMode() == NTBTagSF::shapereweighting_mode){
                        throw std::runtime_error("NTBTagSF::shapereweighting_mode: not impl");
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



		/////////////////////// at least two jets STEP 6 /////////////
		step++;

		//if(midphi && dphiplushardjets.size()<2) continue;
		if(!zerojet && !onejet && selectedjets->size() < 2) continue;
		//if(!midphi && medjets.size()<2) continue;
		//if(ptllj<140) continue;
		/*if(dphillj > 2.65 && dphillj < 3.55){
			if(ptllj < 190)
				continue;
		} */


		if(analysisMllRange){

			plots.makeControlPlots(step);
			sel_step[6]+=puweight;
		}
		if(isZrange){

			zplots.makeControlPlots(step);
		}


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
	//	for(size_t i=0;i<selectedbjets.size();i++){
	//		getBTagSF()->fillEff(selectedjets->at(i),puweight);
	//	}

		///////////////////// btag cut STEP 8 //////////////////////////
		step++;

		if(!usetopdiscr && !nobcut && selectedbjets.size() < 1) continue;
		//agrohsje uncomment if(usetopdiscr && lh_toplh<0.3) continue;
	
        	if(apllweightsone) puweight=1;

		float mlbmin=0;
		evt.mlbmin=&mlbmin;

		if(analysisMllRange){

			// std::cout << selectedjets->at(0)->pt() << std::endl;
			if(selectedbjets.size()>0){
				mlbmin=(leadingptlep->p4()+selectedbjets.at(0)->p4()).m();
				float tmp=(secleadingptlep->p4()+selectedbjets.at(0)->p4()).m();
				if(mlbmin>tmp)mlbmin=tmp;
			}

			xsecplots.makeControlPlots(step);

			plots.makeControlPlots(step);
			sel_step[8]+=puweight;

			mlbmtplots_step8.fillPlotsReco();
			xsecfitplots_step8.fillPlotsReco();

		}
		if(isZrange){
			zplots.makeControlPlots(step);
		}

		if(tickoncemode_) {
			std::cout << "tickOnce("<< anaid << "): finished main loop once. break"<<std::endl;
			break; //one event survived, sufficient
		}


	}

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
	norm *= renormfact;
	if(testmode_ )
		std::cout << "testmode("<< anaid << "): finished main loop, renorm factor top pt: " <<renormfact  << std::endl;

	//renorm after pdf reweighting (only acceptance effects!
	renormfact=getPdfReweighter()->getRenormalization();
	norm *= renormfact;
	if(testmode_ )
		std::cout << "testmode("<< anaid << "): finished main loop, renorm factor pdf weights: " <<renormfact  << std::endl;

	//renorm all mc weights 
	for(size_t i=0;i<weightbranches.size();i++){
		renormfact=mcreweighters.at(i).getRenormalization();
		norm *= renormfact;
		if(testmode_ )
			std::cout << "testmode("<< anaid << "): finished main loop, renorm factor for mc reweighting["<<i<<"]: " <<renormfact  << std::endl;
	}

	histo1DUnfold::flushAllListed(); // call once again after last event processed



	// Fill all containers in the stackVector


	// delete t;
	f->Close(); //deletes t
	delete f;
	/*s
	for(size_t i=0;i<ntestconts;i++)
			delete testconts.at(i);
	 */

	///////////////////////////////
	///////////////////////////////
	///////////////////////////////
	///////////////////////////////
	//     WRITE OUTPUT PART     //
	///////////////////////////////
	///////////////////////////////
	///////////////////////////////
	///////////////////////////////

	if(!singlefile_)
		p_askwrite.get(anaid)->pwrite(anaid);
	//std::cout << anaid << " (" << inputfile << ")" << " asking for write permissions to " <<getOutPath() << endl;
	int canwrite=0;
	if(!singlefile_)
		canwrite=p_allowwrite.get(anaid)->pread();
	if(canwrite>0 || singlefile_){ //wait for permission

		if(testmode_ )
			std::cout << "testmode("<< anaid << "): allowed to write to file " << getOutPath()+".root"<< std::endl;

		ztop::histoStackVector * csv=&allplotsstackvector_;

		if(fileExists((getOutPath()+".ztop").Data())) {
			csv->readFromFile((getOutPath()+".ztop").Data());
		}
		csv->addList1DUnfold(legendname,color,norm,legord);
		if(testmode_ )
			std::cout << "testmode("<< anaid << "): added 1DUnfold List"<< std::endl;
		csv->addList2D(legendname,color,norm,legord);
		if(testmode_ )
			std::cout << "testmode("<< anaid << "): added 2D List"<< std::endl;
		csv->addList(legendname,color,norm,legord);
		if(testmode_ )
			std::cout << "testmode("<< anaid << "): added 1D List"<< std::endl;

		if(issignal)
			csv->addSignal(legendname);

		csv->writeToFile((getOutPath()+".ztop").Data());
		
                if(testmode_ )
			std::cout << "testmode("<< anaid << "): written main output"<< std::endl;


		///btagsf
		if(btagsf_.makesEff()){
			if(testmode_ )
				std::cout << "testmode("<< anaid << "): writing btag file"<< std::endl;
			getBTagSF()->writeToFile((std::string)btagfile.Data()); //recreates the file
		}///makes eff

		std::cout << inputfile << ": " << std::endl;
		for(unsigned int i=0;i<9;i++){
			std::cout << "selection step "<< toTString(i)<< " "  << sel_step[i];
			if(i==3)
				cout << "  => sync step 1 \tmll>20";
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


		std::cout << "\nEvents total (normalized): "
				<< nEntries*norm << "\n"
				"nEvents_selected normd: "<< sel_step[8]*norm<< " " << inputfile<< std::endl;


		if(singlefile_) return;

		//all operations done
		//check if everything was written correctly
		bool outputok=true;

		if(outputok)
			p_finished.get(anaid)->pwrite(1); //turns of write blocking, too
	}
	else{
		std::cout << "testmode("<< anaid << "): failed to write to file " << getOutPath()+".root"<< std::endl;
		p_finished.get(anaid)->pwrite(-2); //write failed
	}



#endif
}



bool analyzer_run2::checkTrigger(std::vector<bool> * p_TriggerBools,ztop::NTEvent * pEvent, bool isMC,size_t anaid){


	//do trigger stuff - onlye 8TeV for now
	if(!is7TeV_){
		if(p_TriggerBools->size() < 3)
			return false;

		if(b_mumu_){
			if(!(p_TriggerBools->at(1) || p_TriggerBools->at(2)))
				return false;
			else
				return true;
		}
		else if(b_ee_){
			if(!p_TriggerBools->at(0))
				return false;
			else
				return true;
		}
		else if(b_emu_){
			if(p_TriggerBools->size()<10){
				//p_finished.get(anaid)->pwrite(-3);
				return false;
			}
			if(!(p_TriggerBools->at(10) || p_TriggerBools->at(11)))
				return false;
			else
				return true;
		}
	}
	else{ //is7TeV_
		if(p_TriggerBools->size() < 3)
			return false;

		if(b_mumu_){
			if(isMC && !p_TriggerBools->at(5))
				return false;
			if(!isMC && pEvent->runNo() < 163869 && !p_TriggerBools->at(5))
				return false;
			if(!isMC && pEvent->runNo() >= 163869 && !p_TriggerBools->at(6))
				return false;
		}
		else if(b_ee_){
			if(!(p_TriggerBools->at(3) || p_TriggerBools->at(4) || p_TriggerBools->at(1)))
				return false;
		}
		else if(b_emu_){

			if(p_TriggerBools->size()<26){ //emu are 20 - 25
				throw std::out_of_range("TriggerBools too small in size for 7 TeV trees");
			}
			if(!(p_TriggerBools->at(20)
					|| p_TriggerBools->at(21)
					|| p_TriggerBools->at(22)
					|| p_TriggerBools->at(23)
					|| p_TriggerBools->at(24)
					|| p_TriggerBools->at(25)
			))
				return false;
			else
				return true;
		}
	}
	return true;
}


