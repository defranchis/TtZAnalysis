#include "TtZAnalysis/Analysis/interface/AnalysisUtils.h"
#include "TtZAnalysis/Tools/interface/fileReader.h"
#include "../interface/discriminatorFactory.h"
#include <time.h>
#include <TtZAnalysis/Analysis/interface/top_analyzer_run1.h>
#include <TtZAnalysis/Analysis/interface/top_analyzer_run2.h>
#include <TtZAnalysis/Analysis/interface/topAnalyzer.h>

#include "TtZAnalysis/Tools/interface/applicationMainMacro.h"

#include "TopAnalysis/ZTopUtils/interface/version.h"

#include "../interface/sampleHelpers.h"
#include "../interface/KinematicReconstruction.h"
#include "../interface/KinematicReconstructionSolution.h"


//#include "Analyzer.cc"
//should be taken care of by the linker!



//void analyse(TString channel, TString Syst, TString energy, TString outfileadd,
//		double lumi, bool dobtag, bool status,bool testmode,bool tickonce,TString maninputfile,
//		TString mode,TString topmass,TString btagfile,bool createLH, std::string discrfile,float fakedatastartentries,bool interactive,float wpval){ //options like syst..

invokeApplication(){
	using namespace ztop;

	TString channel= parser->getOpt<TString>   ("c","emu","channel (ee, emu, mumu), default: emu");         //-c channel
	TString Syst   = parser->getOpt<TString>   ("s","nominal","systematic variation <var>_<up/down>, default: nominal");     //-s <syst>
	TString energy = parser->getOpt<TString>   ("e","8TeV","energy (8TeV, 7 TeV), default: 8TeV");        //-e default 8TeV
	double lumi    = parser->getOpt<float>     ("l",-1,"luminosity, default -1 (= read from config file)");            //-l default -1
	bool dobtag    = parser->getOpt<bool>      ("B",false,"produce b-tag efficiencies (switch)");         //-B switches on default false
	TString btagfile = parser->getOpt<TString> ("b","all_btags","use btagfile (default files in dir all_btags)");        //-b btagfile default all_btags.root
	//const float wpval = parser->getOpt<float>     ("bwp",-100,"btag discriminator cut value (default: not use)");            //-l default -1

	TString outfileadd= parser->getOpt<TString>("o","","additional output id");            //-o <outfile> should be something like channel_energy_syst.root // only for plots

	bool status    = parser->getOpt<bool>      ("S",false,"show regular status update (switch)");         //-S enables default false
	bool testmode  = parser->getOpt<bool>      ("T",false,"enable testmode: 8% of stat, more printout");         //-T enables default false

	bool tickonce  = parser->getOpt<bool>      ("TO",false,"enable tick once: breaks as soon as 1 event survived full selection (for software testing)");         //-T enables default false
	TString mode   = parser->getOpt<TString>   ("m","xsec","additional mode options");        //-m (xsec,....) default xsec changes legends? to some extend
	TString maninputfile= parser->getOpt<TString> ("i","","specify configuration file input manually (default is configs/analyse/<channel>_<energy>_config.txt");          //-i empty will use automatic
	TString topmass  = parser->getOpt<TString> ("mt","172.5","top mass value to be used, default: 172.5");          //-i empty will use automatic
	std::string discrfile=parser->getOpt<std::string>  ("lh","" , "specify discriminator input file. If not specified, likelihoods are created");          //-i empty will use automatic

	const bool interactive = parser->getOpt<bool>      ("I",false,"enable interactive mode: no fork limit");

	float fakedatastartentries = parser->getOpt<float>    ("-startdiv",0.9,"point to start fake data entries wrt to to evts");

	const bool doKinReco = parser->getOpt<bool>      ("-kinReco",false,"perform kinematic reconstruction");        

	bool createLH=false;
	if(discrfile.length()<1)
		createLH=true;

	parser->doneParsing();

	bool didnothing=false;
	//some env variables
	TString cmssw_base=getenv("CMSSW_BASE");
	TString scram_arch=getenv("SCRAM_ARCH");
	TString batchbase = getenv("ANALYSE_BATCH_BASE");




	TString database="data/analyse/";
	TString configbase="configs/analyse/";

	if(batchbase.Length()>0){
		database=batchbase+"/"+database;
		configbase=batchbase+"/"+configbase;
		std::cout << "Batch run from " << batchbase << std::endl;
	}
	else{
		database=cmssw_base+"/src/TtZAnalysis/Analysis/"+database;
		configbase=cmssw_base+"/src/TtZAnalysis/Analysis/"+configbase;
	}

	using namespace std;
	using namespace ztop;
	if(testmode){
		ztop::histo1D::debug =false;
		ztop::histoStack::debug=false;
		ztop::histoStackVector::debug=false;
	}

	TString inputfilewochannel="config.txt";
	TString inputfile=channel+"_"+energy+"_"+inputfilewochannel;
	// TString
	if(maninputfile!="")
		inputfile=maninputfile;
	//do not prepend absolute path (batch submission)
	inputfile=configbase+inputfile;

	fileReader fr;
	fr.setComment("$");
	fr.setDelimiter(",");
	fr.setStartMarker("[parameters-begin]");
	fr.setEndMarker("[parameters-end]");
	fr.readFile(inputfile.Data());


	TString treedir,jecfile,pufileGH,pufileBtoF,muonsffile,muonsffileGH,muonsffileBtoF,muonsfhisto,elecsffile,elecsfhisto,trigsffile,elecensffile,muonensffile,trackingsffile,trackingsfhisto,elecTrackingsffile,elecTrackingsfhisto; //...


	if(lumi<0)
		lumi=fr.getValue<double>("Lumi");
	treedir=              fr.getValue<TString>("inputFilesDir");
	jecfile=   cmssw_base+fr.getValue<TString>("JECFile");
        muonsffile=cmssw_base+fr.getValue<TString>("MuonSFFile");
        muonsffileBtoF=cmssw_base+fr.getValue<TString>("MuonSFFileBtoF");
	muonsffileGH=cmssw_base+fr.getValue<TString>("MuonSFFileGH");
	muonsfhisto=          fr.getValue<TString>("MuonSFHisto");
	trackingsffile=cmssw_base+fr.getValue<TString>("TrackingSFFile");
	trackingsfhisto=          fr.getValue<TString>("TrackingSFHisto");
        elecTrackingsffile=cmssw_base+fr.getValue<TString>("ElecTrackingSFFile");
        elecTrackingsfhisto= fr.getValue<TString>("ElecTrackingSFHisto");
	elecsffile=cmssw_base+fr.getValue<TString>("ElecSFFile");
	elecsfhisto=          fr.getValue<TString>("ElecSFHisto");
	trigsffile=cmssw_base+fr.getValue<TString>("TriggerSFFile");
        pufileBtoF=    cmssw_base+fr.getValue<TString>("PUFileBtoF");
	pufileGH=    cmssw_base+fr.getValue<TString>("PUFileGH");
	//elecensffile =cmssw_base+fr.getValue<TString>("ElecEnergySFFile");
	//muonensffile =cmssw_base+fr.getValue<TString>("MuonEnergySFFile");

	//btagWP=               fr.getValue<TString>("btagWP");
	const std::string btagSFFile       =  cmssw_base.Data()+fr.getValue<std::string>("btagSFFile");

	//mode options CAN be additionally parsed by file
	fr.setRequireValues(false);
	mode                 += fr.getValue<TString>("modeOptions",TString(mode));
	const TString elecEnsffile=cmssw_base+fr.getValue<TString>("ElecEnergySFFile","DEFFILEWILDCARDDONTREMOVE");
	const TString elecEnsfhisto=          fr.getValue<TString>("ElecEnergySFHisto","");




	//TString trigsfhisto="scalefactor_eta2d_with_syst";
        TString trigsfhisto="scalefactor_eta2d_with_syst";
        //TString trigsfhisto="lepton_eta2dfine_sf";

	std::cout << "inputfile read" << std::endl;

	//hard coded:

	system("mkdir -p output");
	TString outdir="output";
	TString sysc="mkdir -p "+outdir+"/btags";
	system(sysc.Data());
	if(dobtag){
		btagfile=outdir+"/btags/"+ channel+"_"+energy+"_"+topmass+"_"+Syst;
	}
	else if(Syst.BeginsWith("BTAG")){
		btagfile=database+btagfile+"/"+ channel+"_"+energy+"_"+topmass+"_"+"nominal";
	}
	else{
		btagfile=database+btagfile+"/"+ channel+"_"+energy+"_"+topmass+"_"+Syst;
	}


	///set input files list etc (common)


	topAnalyzer* ana;
	if(energy=="7TeV" || energy=="8TeV"){
		ana= new top_analyzer_run1();
	}
	else if(energy=="13TeV"){
		ana= new top_analyzer_run2();
		ana->addWeightBranch("NTWeight_nominal");
	}
	else
		throw std::runtime_error("Undefined Energy! Exit!");

	//only used in special cases!
	ana->setPathToConfigFile(inputfile);

	ana->setFakeDataStartNEntries(fakedatastartentries);
	///some checks


	ana->setMaxChilds(6);
	if(testmode)
		ana->setMaxChilds(1);
	if(interactive || tickonce)
		ana->setMaxChilds(20);
	ana->setMode(mode);
	ana->setShowStatus(status);
	ana->setTestMode(testmode);
	ana->setTickOnceMode(tickonce);
	ana->setLumi(lumi);
	ana->setDataSetDirectory(treedir);
	ana->setUseDiscriminators(!createLH);
	ana->setDiscriminatorInputFile(discrfile);
        ana->getPUReweighterBtoF()->setDataTruePUInput(pufileBtoF+".root");
	ana->getPUReweighterGH()->setDataTruePUInput(pufileGH+".root");
	ana->setChannel(channel);
	ana->setEnergy(energy);
	ana->setSyst(Syst);
	ana->setTopMass(topmass);
	if(energy == "7TeV"){
		ana->getPUReweighter()->setMCDistrSummer11Leg();
	}
	else if(energy == "8TeV"){
		ana->getPUReweighter()->setMCDistrSum12();
	}
	else if(energy == "13TeV"){
		ana->getPUReweighterBtoF()->setMCDistrMor17("25ns_poisson");
                ana->getPUReweighterGH()->setMCDistrMor17("25ns_poisson");
	}	   

	ana->getElecSF()->setInput(elecsffile,elecsfhisto);
        ana->getMuonSF()->setInput(muonsffile,muonsfhisto);
	ana->getMuonSFGH()->setInput(muonsffileGH,muonsfhisto);
        ana->getMuonSFBtoF()->setInput(muonsffileBtoF,muonsfhisto);
	ana->getTrackingSF()->setInput(trackingsffile,trackingsfhisto);
        ana->getElecTrackingSF()->setInput(elecTrackingsffile, elecTrackingsfhisto);

        ana->getElecBGSF()->setInput(elecsffile,elecsfhisto);
        ana->getMuonBGSF()->setInput(muonsffile,muonsfhisto);
        ana->getMuonBGSFBtoF()->setInput(muonsffileBtoF,muonsfhisto);
        ana->getMuonBGSFGH()->setInput(muonsffileGH,muonsfhisto);
        ana->getTrackingBGSF()->setInput(trackingsffile,trackingsfhisto);
        ana->getElecTrackingBGSF()->setInput(elecTrackingsffile, elecTrackingsfhisto);

	ana->getTriggerSF()->setInput(trigsffile,trigsfhisto);
        ana->getTriggerBGSF()->setInput(trigsffile,trigsfhisto);
        ana->getJERAdjuster()->setSystematics("def_2016");

        ana->setDoKinReco(doKinReco);
        if (doKinReco){
            std::vector<TString> channels {channel};
            KinematicReconstruction * kinReco = new KinematicReconstruction(Era::run2_13tev_2016_25ns, 0, true); // era, min b-tags, prefer b-tags
            KinematicReconstructionScaleFactors * kinRecoSF = new KinematicReconstructionScaleFactors(Era::run2_13tev_2016_25ns,Channel::convert(channels),Systematic::Systematic(Syst));
            kinRecoSF->prepareChannel(Channel::convert(channel));
            ana->setKinReco(kinReco, kinRecoSF);
        }

	if(elecEnsffile.EndsWith("DEFFILEWILDCARDDONTREMOVE")){
		if(energy == "7TeV" || energy == "8TeV")
			ana->getElecEnergySF()->setGlobal(1,0.15,0.15);
		// else if(energy == "13TeV")
		// 	ana->getElecEnergySF()->setGlobal(1,1.0,1.0);

	}
	else{
		ana->getElecEnergySF()->setInput(elecEnsffile,elecEnsfhisto);
	}

	if(energy == "7TeV" || energy == "8TeV")
		//https://twiki.cern.ch/twiki/bin/viewauth/CMS/MuonReferenceResolution
		//this is for muons without the corrections so it should be even better with
		ana->getMuonEnergySF()->setGlobal(1,0.3,0.3); //new from muon POG twiki
	// else if(energy == "13TeV")
	// 	ana->getMuonEnergySF()->setGlobal(1,0.5,0.5);

	ana->getTopPtReweighter()->setFunction(reweightfunctions::toppt_2016);
	ana->getTopPtReweighter()->setSystematics(reweightfunctions::nominal);

	ana->setOutFileAdd(outfileadd);
	ana->setOutDir(outdir);

        TString btagreffile = btagfile;
        btagreffile.ReplaceAll(Syst,"nominal");

	ana->getBTagSF()->setMakeEff(dobtag);
	ana->setBTagMCEffFile(btagfile);
	ana->setBTagMCRefEffFile(btagreffile);

	//change
	ana->getBTagSF()->setMode(NTBTagSF::simplereweighting_mode);
	BTagEntry::OperatingPoint btagop=BTagEntry::OP_TIGHT;
	if(energy == "13TeV"){
		//agrohsje
		//ana->getBTagSF()->loadSF  (btagSFFile, BTagEntry::OP_MEDIUM,"csvv2","mujets","up","down");
		ana->getBTagSF()->loadSF  (btagSFFile, btagop,"csvv2_2016","mujets","up","down");
	}else if (energy == "7TeV" || energy == "8TeV"){
		if(mode.Contains("Btagloosewp"))
			btagop=BTagEntry::OP_LOOSE;
		else if(mode.Contains("Btagmediumwp"))
			btagop=BTagEntry::OP_MEDIUM;
		ana->getBTagSF()->loadSF  (btagSFFile, btagop,"csv","mujets","up","down");
	}
	bool globalbsf=false;
	if(mode.Contains("Btagglobalsf"))
		globalbsf=true;

	//agrohsje 
	if(energy == "13TeV") {
                ana->getJECUncertainties()->setIs2012(false);
                ana->getJECUncertainties()->setIs2016(true);
        }
	ana->getJECUncertainties()->setFile((jecfile).Data());
	ana->getJECUncertainties()->setSystematics("no");

	//add indication for non-correlated syst by adding the energy to syst name!! then the getCrossSections stuff should recognise it

	//although it produces overhead, add background rescaling here?

	//using the sys nominal
	//btag efficiencies are NOT required here

	if(Syst.Contains("_sysnominal")){
		ana->getBTagSF()->setMakeEff(true);
		ana->setBTagMCEffFile(outdir+"/btag_dummy");
	}

        if(Syst=="TT_FRAG_up"){
                ana->addWeightBranch("NTWeight_FragUp");
        }
        else if(Syst=="TT_FRAG_down"){
                ana->addWeightBranch("NTWeight_FragDown");
        }
        else if(Syst=="TT_FRAG_PETERSON_up"){
                ana->addWeightBranch("NTWeight_FragPeterson");
        }
        else {
		ana->addWeightBranch("NTWeight_FragCentral");
        }


	if(Syst=="nominal"){
            // do nothing
	}
	///sys nominals...
	else if(Syst=="P11_sysnominal"){
		ana->setFilePostfixReplace("ttbar.root","ttbar_mgdecays_p11.root");
		ana->setFilePostfixReplace("ttbarviatau.root","ttbarviatau_mgdecays_p11.root");
	}
	else if(Syst=="P11_sysnominal_CR_up"){ //do nothing
		ana->setFilePostfixReplace("ttbar.root","ttbar_mgdecays_p11.root");
		ana->setFilePostfixReplace("ttbarviatau.root","ttbarviatau_mgdecays_p11.root");
	}
	else if(Syst=="P11_sysnominal_CR_down"){ //no CR
		ana->setFilePostfixReplace("ttbar.root","ttbar_mgdecays_p11nocr.root");
		ana->setFilePostfixReplace("ttbarviatau.root","ttbarviatau_mgdecays_p11nocr.root");
	}
	else if(Syst=="P11_sysnominal_UE_up"){
		ana->setFilePostfixReplace("ttbar.root","ttbar_mgdecays_p11mpihi.root");
		ana->setFilePostfixReplace("ttbarviatau.root","ttbarviatau_mgdecays_p11mpihi.root");
	}
	else if(Syst=="P11_sysnominal_UE_down"){
		ana->setFilePostfixReplace("ttbar.root","ttbar_mgdecays_p11tev.root");
		ana->setFilePostfixReplace("ttbarviatau.root","ttbarviatau_mgdecays_p11tev.root");
	}
	else if(Syst=="PDF_sysnominal"){
		if(energy=="7TeV" || energy=="8TeV"){
			ana->setFilePostfixReplace("ttbar.root","ttbar_pdf.root");
			ana->setFilePostfixReplace("ttbarviatau.root","ttbarviatau_pdf.root");
			ana->setFilePostfixReplace("ttbar_dil.root","ttbar_dil_pdf.root");
			ana->setFilePostfixReplace("ttbarviatau_dil.root","ttbarviatau_dil_pdf.root");
		}
		ana->getPdfReweighter()->setPdfIndex(0);
	}
	else if(Syst.Contains("PDF_sysnominal_")){
		if(energy=="7TeV" || energy=="8TeV"){
			ana->setFilePostfixReplace("ttbar.root","ttbar_pdf.root");
			ana->setFilePostfixReplace("ttbarviatau.root","ttbarviatau_pdf.root");
			ana->setFilePostfixReplace("ttbar_dil.root","ttbar_dil_pdf.root");
			ana->setFilePostfixReplace("ttbarviatau_dil.root","ttbarviatau_dil_pdf.root");
		}
		size_t pdfindex=0;
		for(size_t i=1;i<10000;i++){
			pdfindex++;
			if(Syst == "PDF_sysnominal_PDF" + toTString(i) + "_down"){
				ana->getPdfReweighter()->setPdfIndex(pdfindex);
				std::cout << "setting index to " << pdfindex << " for " << Syst << std::endl;
				break;
			}
			pdfindex++;
			if(Syst == "PDF_sysnominal_PDF" + toTString(i) + "_up"){
				ana->getPdfReweighter()->setPdfIndex(pdfindex);
				std::cout << "setting index to " << pdfindex << " for " << Syst << std::endl;
				break;
			}

		}
	}



	else if(Syst=="TRIGGER_down"){
		ana->getTriggerSF()->setSystematics("down");
		ana->getTriggerBGSF()->setSystematics("down");
	}
	else if(Syst=="TRIGGER_up"){
		ana->getTriggerSF()->setSystematics("up");
		ana->getTriggerBGSF()->setSystematics("up");
	}
	else if(Syst=="ELECSF_up"){
		ana->getElecSF()->setSystematics("up");
                ana->getElecBGSF()->setSystematics("up");
	}
	else if(Syst=="ELECSF_down"){
		ana->getElecSF()->setSystematics("down");
                ana->getElecBGSF()->setSystematics("down");
	}
        else if(Syst=="ELECTRACKSF_up"){
                ana->getElecTrackingSF()->setSystematics("up");
        }
        else if(Syst=="ELECTRACKSF_down"){
                ana->getElecTrackingSF()->setSystematics("down");
        }
	else if(Syst=="MUONSF_up"){
                ana->getMuonSFBtoF()->setSystematics("up");
		ana->getMuonSFGH()->setSystematics("up");
                ana->getMuonBGSFBtoF()->setSystematics("up");
                ana->getMuonBGSFGH()->setSystematics("up");
	}
	else if(Syst=="MUONSF_down"){
                ana->getMuonSFBtoF()->setSystematics("down");
		ana->getMuonSFGH()->setSystematics("down");
                ana->getMuonBGSFBtoF()->setSystematics("down");
                ana->getMuonBGSFGH()->setSystematics("down");
	}

        else if(Syst=="ELECBGSF_up"){
                ana->getElecBGSF()->setSystematics("up");
             //   ana->getElecTrackingBGSF()->setSystematics("up");
        }
        else if(Syst=="ELECBGSF_down"){
                ana->getElecBGSF()->setSystematics("down");
               // ana->getElecTrackingBGSF()->setSystematics("down");
        }
        else if(Syst=="MUONBGSF_up"){
                //ana->getMuonBGSF()->setSystematics("up");
                ana->getMuonBGSFBtoF()->setSystematics("up");
                ana->getMuonBGSFGH()->setSystematics("up");
        }
        else if(Syst=="MUONBGSF_down"){
                //ana->getMuonBGSF()->setSystematics("down");
                ana->getMuonBGSFBtoF()->setSystematics("down");
                ana->getMuonBGSFGH()->setSystematics("down");
        }
        else if(Syst=="TRIGGERBGSF_up"){
                ana->getTriggerBGSF()->setSystematics("up");
             //   ana->getElecTrackingBGSF()->setSystematics("up");
        }
        else if(Syst=="TRIGGERBGSF_down"){
                ana->getTriggerBGSF()->setSystematics("down");
               // ana->getElecTrackingBGSF()->setSystematics("down");
        }


	else if(Syst=="MUONES_up"){
		ana->getMuonEnergySF()->setSystematics("up");
	}
	else if(Syst=="MUONES_down"){
		ana->getMuonEnergySF()->setSystematics("down");
	}
	else if(Syst=="ELECES_up"){
		ana->getElecEnergySF()->setSystematics("up");
	}
	else if(Syst=="ELECES_down"){
		ana->getElecEnergySF()->setSystematics("down");
	}
	else if(Syst=="ELECER_PHI_up"){
		ana->getElecEnergyResolutionSF_phi()->setSystematics("up");
	}
	else if(Syst=="ELECER_PHI_down"){
            // one sided by construction
	}
	else if(Syst=="ELECER_RHO_up"){
		ana->getElecEnergyResolutionSF_rho()->setSystematics("up");
	}
	else if(Syst=="ELECER_RHO_down"){
		ana->getElecEnergyResolutionSF_rho()->setSystematics("down");
	}
	else if(Syst=="JES_up"){
		ana->getJECUncertainties()->setSystematics("up");
		//	ana->getBTagSF()->setSystematic(bTagSFBase::jesup);
	}
	else if(Syst=="JES_down"){
		ana->getJECUncertainties()->setSystematics("down");
		//	ana->getBTagSF()->setSystematic(bTagSFBase::jesdown);
	}


	/*
	 * Individual JES sources block!
	 *
	 */
	else if(Syst.BeginsWith("JES_")){
		std::vector<std::string> sources;

		sources = ana->getJECUncertainties()->getSourceNames();

		for(size_t i=0;i<=sources.size();i++){
			std::cout << sources.at(i) <<std::endl;
			if(i==sources.size())
				throw std::runtime_error("JES source not found! exit!");

			//jes systematics
			if("JES_"+(TString)sources.at(i)+"_down" == Syst || "JES_"+sources.at(i)+"_up" == Syst){
				ana->getJECUncertainties()->setSource(sources.at(i));
				//some specials

				if(sources.at(i) == "FlavorPureGluon"){
					ana->getJECUncertainties()->restrictToFlavour(23);
					ana->getJECUncertainties()->restrictToFlavour(0);
				}
				else if(sources.at(i) == "FlavorPureQuark"){
					ana->getJECUncertainties()->restrictToFlavour (1);
					ana->getJECUncertainties()->restrictToFlavour(-1);
					ana->getJECUncertainties()->restrictToFlavour (2);
					ana->getJECUncertainties()->restrictToFlavour(-2);
					ana->getJECUncertainties()->restrictToFlavour (3);
					ana->getJECUncertainties()->restrictToFlavour(-3);
				}
				else if(sources.at(i) == "FlavorPureCharm"){
					ana->getJECUncertainties()->restrictToFlavour(4);
					ana->getJECUncertainties()->restrictToFlavour(-4);
				}
				else if(sources.at(i) == "FlavorPureBottom"){
					ana->getJECUncertainties()->restrictToFlavour(5);
					ana->getJECUncertainties()->restrictToFlavour(-5);
				}

				if("JES_"+(TString)sources.at(i)+"_down" == Syst){
					ana->getJECUncertainties()->setSystematics("down");
					break;
				}
				else if("JES_"+sources.at(i)+"_up" == Syst){
					ana->getJECUncertainties()->setSystematics("up");
					break;
				}
			}
		}
	}



	/*
	 * END INDIVI JES SOURCES BLOCK
	 */


	else if(Syst=="JEC_residuals_up"){
		//use default
		//	ana->setFilePostfixReplace("_22Jan.root","_22Jan_ptres.root");
	}
	else if(Syst=="JEC_residuals_down"){
		ana->setFilePostfixReplace("_22Jan.root","_22Jan_noptres.root");
	}


	else if(Syst=="PU_up"){
                ana->getPUReweighterBtoF()->setDataTruePUInput(pufileBtoF+"_up.root");
		ana->getPUReweighterGH()->setDataTruePUInput(pufileGH+"_up.root");
		if(energy=="7TeV" || energy=="8TeV"){
			if(!dobtag&&!globalbsf){
				ana->getBTagSF()->loadBCSF(btagSFFile, btagop,"csv","mujets","up_PileUp","down_PileUp");
				ana->getBTagSF()->setSystematics(bTagSFBase::heavyup);}
		}
		else if(energy=="13TeV"){
			if(!dobtag&&!globalbsf){
				ana->getBTagSF()->loadBCSF(btagSFFile, btagop,"csvv2_2016","mujets","up_pileup","down_pileup");
				ana->getBTagSF()->setSystematics(bTagSFBase::heavyup);
                                ana->setBTagMCRefEffFile(btagfile);
                        }
		}
	}
	else if(Syst=="PU_down"){
                ana->getPUReweighterBtoF()->setDataTruePUInput(pufileBtoF+"_down.root");
		ana->getPUReweighterGH()->setDataTruePUInput(pufileGH+"_down.root");
		if(energy=="7TeV" || energy=="8TeV"){
			if(!dobtag&&!globalbsf){
				ana->getBTagSF()->loadBCSF(btagSFFile, btagop,"csv","mujets","up_PileUp","down_PileUp");
				ana->getBTagSF()->setSystematics(bTagSFBase::heavydown);}
		}
		else if(energy=="13TeV"){
			if(!dobtag&&!globalbsf){
				ana->getBTagSF()->loadBCSF(btagSFFile, btagop,"csvv2_2016","mujets","up_pileup","down_pileup");
				ana->getBTagSF()->setSystematics(bTagSFBase::heavydown);
                                ana->setBTagMCRefEffFile(btagfile);
                        }
		}
	}
	//btag uncertainties are correlated
	else if(Syst=="JER_up"){
		if(energy=="7TeV" || energy=="8TeV"){
                        ana->getJERAdjuster()->setSystematics("up");
			if(!dobtag&&!globalbsf){
                                
				ana->getBTagSF()->loadBCSF(btagSFFile, btagop,"csv","mujets","up_JER","down_JER");
				ana->getBTagSF()->setSystematics(bTagSFBase::heavyup);}
		}
                else if(energy=="13TeV")ana->getJERAdjuster()->setSystematics("up_2016");
	}
	else if(Syst=="JER_down"){
		if(energy=="7TeV" || energy=="8TeV"){
                        ana->getJERAdjuster()->setSystematics("down");
			if(!dobtag &&!globalbsf){
				ana->getBTagSF()->loadBCSF(btagSFFile, btagop,"csv","mujets","up_JER","down_JER");
				ana->getBTagSF()->setSystematics(bTagSFBase::heavydown);}
		}
                else if(energy=="13TeV")ana->getJERAdjuster()->setSystematics("down_2016");
	}
	/////////btag
	else if(Syst=="BTAGH_up"){
		ana->getBTagSF()->setSystematics(bTagSFBase::heavyup);
	}
	else if(Syst=="BTAGH_down"){
		ana->getBTagSF()->setSystematics(bTagSFBase::heavydown);
	}
	else if(Syst=="BTAGL_up"){
		ana->getBTagSF()->setSystematics(bTagSFBase::lightup);
	}
	else if(Syst=="BTAGL_down"){
		ana->getBTagSF()->setSystematics(bTagSFBase::lightdown);
	}
	else if(Syst.BeginsWith("BTAGH_")){ //this is not standard up/down but splitting! only for heavy SF
		TString btagsubstr=Syst;
		btagsubstr.ReplaceAll("BTAGH_","");
		btagsubstr.ReplaceAll("_up","");
		btagsubstr.ReplaceAll("_down","");
		TString btagsubstrup="up_"+btagsubstr;
		btagsubstr="down_"+btagsubstr;

		ana->getBTagSF()->loadBCSF(btagSFFile, btagop,"csvv2_2016","mujets",btagsubstrup.Data(),btagsubstr.Data());

		if(Syst.EndsWith("_up"))
			ana->getBTagSF()->setSystematics(bTagSFBase::heavyup);
		else if (Syst.EndsWith("_down"))
			ana->getBTagSF()->setSystematics(bTagSFBase::heavydown);
		else
			throw std::runtime_error("btag subsplitting has to end with up/down");
	}

	/////////top pt

	else if(Syst=="TOPPT_up"){
		ana->getTopPtReweighter()->setSystematics(reweightfunctions::up);
	}
	else if(Syst=="TOPPT_down"){
		ana->getTopPtReweighter()->setSystematics(reweightfunctions::down);
	}
	///////////////////file replacements/////////////
	else if(Syst=="TT_MATCH_up"){
                if (energy=="7TeV" || energy=="8TeV"){
		        ana->setFilePostfixReplace("ttbar.root","ttbar_ttmup.root");
		        ana->setFilePostfixReplace("ttbarviatau.root","ttbarviatau_ttmup.root");
		        ana->setFilePostfixReplace("ttbar_dil.root","ttbar_dil_ttmup.root");
		        ana->setFilePostfixReplace("ttbarviatau_dil.root","ttbarviatau_dil_ttmup.root");
                }
                else if(energy=="13TeV"){
                        ana->setFilePostfixReplace("ttbar.root","ttbar_hdampup.root");
                        ana->setFilePostfixReplace("ttbarbg.root","ttbarbg_hdampup.root");
                        ana->setIsSignalMerged(true);
                }
	}
	else if(Syst=="TT_MATCH_down"){
               if (energy=="7TeV" || energy=="8TeV"){
		        ana->setFilePostfixReplace("ttbar.root","ttbar_ttmdown.root");
		        ana->setFilePostfixReplace("ttbarviatau.root","ttbarviatau_ttmdown.root");
		        ana->setFilePostfixReplace("ttbar_dil.root","ttbar_dil_ttmdown.root");
		        ana->setFilePostfixReplace("ttbarviatau_dil.root","ttbarviatau_dil_ttmdown.root");
                }
                else if(energy=="13TeV"){
                        ana->setFilePostfixReplace("ttbar.root","ttbar_hdampdown.root");
                        ana->setFilePostfixReplace("ttbarbg.root","ttbarbg_hdampdown.root");
                        ana->setIsSignalMerged(true);
                }

	}
	else if(Syst=="TT_SCALE_up"){
		if (energy=="7TeV" || energy=="8TeV"){
			ana->setFilePostfixReplace("ttbar.root","ttbar_ttscaleup.root");
			ana->setFilePostfixReplace("ttbarviatau.root","ttbarviatau_ttscaleup.root");
			ana->setFilePostfixReplace("ttbar_dil.root","ttbar_dil_ttscaleup.root");
			ana->setFilePostfixReplace("ttbarviatau_dil.root","ttbarviatau_dil_ttscaleup.root");
		}
		else if(energy=="13TeV"){
			ana->addWeightBranch("NTWeight_scaleUp");
			//agrohsje pure ME variation
			ana->setFilePostfixReplace("ttbar.root","ttbar_ttscaleup.root");
			ana->setFilePostfixReplace("ttbarbg.root","ttbarbg_ttscaleup.root");
		}
	}
	else if(Syst=="TT_SCALE_down"){
		if (energy=="7TeV" || energy=="8TeV"){
			ana->setFilePostfixReplace("ttbar.root","ttbar_ttscaledown.root");
			ana->setFilePostfixReplace("ttbarviatau.root","ttbarviatau_ttscaledown.root");
			ana->setFilePostfixReplace("ttbar_dil.root","ttbar_dil_ttscaledown.root");
			ana->setFilePostfixReplace("ttbarviatau_dil.root","ttbarviatau_dil_ttscaledown.root");
		}
		else if(energy=="13TeV"){
			ana->addWeightBranch("NTWeight_scaleDown");
			//agrohsje pure ME variation
			ana->setFilePostfixReplace("ttbar.root","ttbar_ttscaledown.root");
			ana->setFilePostfixReplace("ttbarbg.root","ttbarbg_ttscaledown.root");

		}

	}

        ///////////////////13 TeV uncertainties
        else if(Syst=="TT_ISRSCALE_up"){
                ana->setFilePostfixReplace("ttbar.root","ttbar_ttisrup.root");
                ana->setFilePostfixReplace("ttbarbg.root","ttbarbg_ttisrup.root");
                // ana->setFilePostfixReplace("tW.root","tW_twisrup.root");
                // ana->setFilePostfixReplace("tbarW.root","tbarW_twisrup.root");
                // ana->setFileXsecReplace("tW_twisrup.root",19.3);
                // ana->setFileXsecReplace("tbarW_twisrup.root",19.3);
        }
        else if(Syst=="TT_ISRSCALE_down"){
                ana->setFilePostfixReplace("ttbar.root","ttbar_ttisrdown.root");
                ana->setFilePostfixReplace("ttbarbg.root","ttbarbg_ttisrdown.root");
                // ana->setFilePostfixReplace("tW.root","tW_twisrdown.root");
                // ana->setFilePostfixReplace("tbarW.root","tbarW_twisrdown.root");
                // ana->setFileXsecReplace("tW_twisrdown.root",19.3);
                // ana->setFileXsecReplace("tbarW_twisrdown.root",19.3);

        }
        else if(Syst=="TT_FSRSCALE_up"){
                ana->setFilePostfixReplace("ttbar.root","ttbar_ttfsrup.root");
                ana->setFilePostfixReplace("ttbarbg.root","ttbarbg_ttfsrup.root");
                ana->getAdditionalJEC()->setSystematics("up");
                ana->getAdditionalJEC()->setVariation(Syst.ReplaceAll("_up",""));
                // ana->setFilePostfixReplace("tW.root","tW_twfsrup.root");
                // ana->setFilePostfixReplace("tbarW.root","tbarW_twfsrup.root");
                // ana->setFileXsecReplace("tW_twfsrup.root",19.3);
                // ana->setFileXsecReplace("tbarW_twfsrup.root",19.3);

        }
        else if(Syst=="TT_FSRSCALE_down"){
                ana->setFilePostfixReplace("ttbar.root","ttbar_ttfsrdown.root");
                ana->setFilePostfixReplace("ttbarbg.root","ttbarbg_ttfsrdown.root");
                ana->getAdditionalJEC()->setSystematics("down");
                ana->getAdditionalJEC()->setVariation(Syst.ReplaceAll("_down",""));
                // ana->setFilePostfixReplace("tW.root","tW_twfsrdown.root");
                // ana->setFilePostfixReplace("tbarW.root","tbarW_twfsrdown.root");
                // ana->setFileXsecReplace("tW_twfsrdown.root",19.3);
                // ana->setFileXsecReplace("tbarW_twfsrdown.root",19.3);
        }
        else if(Syst=="ST_FSRSCALE_up"){
                ana->setFilePostfixReplace("tW_ext.root","tW_ext_fsrup.root" );
                ana->setFilePostfixReplace("tbarW_ext.root","tbarW_ext_fsrup.root" );
                // ana->getAdditionalJEC()->setSystematics("up");
                // ana->getAdditionalJEC()->setVariation(Syst.ReplaceAll("_up",""));

        }
        else if(Syst=="ST_FSRSCALE_down"){
                ana->setFilePostfixReplace("tW_ext.root","tW_ext_fsrdown.root" );
                ana->setFilePostfixReplace("tbarW_ext.root","tbarW_ext_fsrdown.root" );
                // ana->getAdditionalJEC()->setSystematics("down");
                // ana->getAdditionalJEC()->setVariation(Syst.ReplaceAll("_down",""));
        }
        else if(Syst=="ST_ISRSCALE_up"){
                ana->setFilePostfixReplace("tW_ext.root","tW_ext_isrup.root" );
                ana->setFilePostfixReplace("tbarW_ext.root","tbarW_ext_isrup.root" );

        }
        else if(Syst=="ST_ISRSCALE_down"){
                ana->setFilePostfixReplace("tW_ext.root","tW_ext_isrdown.root" );
                ana->setFilePostfixReplace("tbarW_ext.root","tbarW_ext_isrdown.root" );
        }
        else if(Syst=="TT_MESCALE_up"){
                ana->addWeightBranch("NTWeight_scaleUp");
        }
        else if(Syst=="TT_MESCALE_down"){
                ana->addWeightBranch("NTWeight_scaleDown");
        }
        else if(Syst=="TT_FRAG_up"){
                ana->getAdditionalJEC()->setSystematics("up");
                ana->getAdditionalJEC()->setVariation(Syst.ReplaceAll("_up",""));
        }
        else if(Syst=="TT_FRAG_down"){
                ana->getAdditionalJEC()->setSystematics("down");
                ana->getAdditionalJEC()->setVariation(Syst.ReplaceAll("_down",""));
        }
        else if(Syst=="TT_FRAG_PETERSON_up"){
            // already done
        }
        else if(Syst=="TT_FRAG_PETERSON_down"){
            // already done
        }
        else if(Syst=="TT_BRANCH_up"){
                ana->addWeightBranch("NTWeight_BranchUp");
        }
        else if(Syst=="TT_BRANCH_down"){
                ana->addWeightBranch("NTWeight_BranchDown");
        }
        else if(Syst=="DYSCALE_up"){
                ana->addWeightBranch("NTWeight_scaleUpDY");
        }
        else if(Syst=="DYSCALE_down"){
                ana->addWeightBranch("NTWeight_scaleDownDY");
        }
        else if(Syst=="TT_TTTUNE_up"){
                ana->setFilePostfixReplace("ttbar.root","ttbar_tttuneup.root");
                ana->setFilePostfixReplace("ttbarbg.root","ttbarbg_tttuneup.root");
                if(energy=="13TeV") ana->setIsSignalMerged(true);
        }
        else if(Syst=="TT_TTTUNE_down"){
                ana->setFilePostfixReplace("ttbar.root","ttbar_tttunedown.root");
                ana->setFilePostfixReplace("ttbarbg.root","ttbarbg_tttunedown.root");
                if(energy=="13TeV") ana->setIsSignalMerged(true);
        }
        else if(Syst=="TT_CRERD_up"){
                ana->setFilePostfixReplace("ttbar.root","ttbar_tterdon.root");
                ana->setFilePostfixReplace("ttbarbg.root","ttbarbg_tterdon.root");
        }
        else if(Syst=="TT_CRERD_down"){
               // Dummy uncertainty
        }     
        else if(Syst=="TT_CRQCD_up"){
                ana->setFilePostfixReplace("ttbar.root","ttbar_ttqcdcr.root");
                ana->setFilePostfixReplace("ttbarbg.root","ttbarbg_ttqcdcr.root");
                if(energy=="13TeV") ana->setIsSignalMerged(true);
        }
        else if(Syst=="TT_CRQCD_down"){
               // Dummy uncertainty
        } 
        else if(Syst=="TT_CRGLUON_up"){
                ana->setFilePostfixReplace("ttbar.root","ttbar_ttgluoncr.root");
                ana->setFilePostfixReplace("ttbarbg.root","ttbarbg_ttgluoncr.root");
                if(energy=="13TeV") ana->setIsSignalMerged(true);
        }
        else if(Syst=="TT_CRGLUON_down"){
               // Dummy uncertainty
        }         
        else if(Syst=="DY_GEN_up"){
                ana->setFilePostfixReplace("dy1050.root","dy1050_lo.root");
                ana->setFilePostfixReplace("dy50inf.root","dy50inf_lo.root");
        }
        else if(Syst=="DY_GEN_down"){
               // Dummy uncertainty
        }
        else if(Syst=="ST_MESCALE_up"){
            ana->addWeightBranch("NTWeight_scaleUpST");
        }
        else if(Syst=="ST_MESCALE_down"){
            ana->addWeightBranch("NTWeight_scaleDownST");
        }
 

	////////////
	else if(Syst=="Z_MATCH_up"){
		ana->setFilePostfixReplace("60120.root","60120_Zmup.root");
	}
	else if(Syst=="Z_MATCH_down"){
		ana->setFilePostfixReplace("60120.root","60120_Zmdown.root");
	}
	else if(Syst=="Z_SCALE_up"){
		ana->setFilePostfixReplace("60120.root","60120_Zscaleup.root");
	}
	else if(Syst=="Z_SCALE_down"){
		ana->setFilePostfixReplace("60120.root","60120_Zscaledown.root");
	}
	else if(Syst=="TT_GENPOWPY_down"){
		//nothing
	}
	else if(Syst=="TT_GENPOWPY_up"){
		ana->setFilePostfixReplace("ttbar.root","ttbar_pow2py.root");
		ana->setFilePostfixReplace("ttbarviatau.root","ttbarviatau_pow2py.root");
		ana->setFilePostfixReplace("ttbar_dil.root","ttbar_pow2py.root");
		ana->setFilePostfixReplace("ttbarviatau_dil.root","ttbarviatau_pow2py.root");
	}
	else if(Syst=="TT_GENPOWHERW_up"){
		//agrohsje fix after proper renaming
		ana->setFilePostfixReplace("ttbar.root","ttbar_powher.root");
		ana->setFilePostfixReplace("ttbarbg.root","ttbarbg_powher.root");
	}
	else if(Syst=="TT_GENPOWHERW_down"){
		//this is just default sample
	}
	else if(Syst=="TT_GENPOW_sysnominal"){
		ana->setFilePostfixReplace("ttbar.root","ttbar_pow2py.root");
		ana->setFilePostfixReplace("ttbarviatau.root","ttbarviatau_pow2py.root");
	}
	else if(Syst=="TT_GENPOW_sysnominal_HER_up"){
		ana->setFilePostfixReplace("ttbar.root","ttbar_powherw.root");
		ana->setFilePostfixReplace("ttbarviatau.root","ttbarviatau_powherw.root");
	}
	else if(Syst=="TT_GENPOW_sysnominal_HER_down"){
		ana->setFilePostfixReplace("ttbar.root","ttbar_pow2py.root");
		ana->setFilePostfixReplace("ttbarviatau.root","ttbarviatau_pow2py.root");
	}
	else if(Syst=="TT_GENMCATNLO_up"){
		if (energy=="7TeV" || energy=="8TeV"){
			ana->setFilePostfixReplace("ttbar.root","ttbar_mcatnlo.root");
			ana->setFilePostfixReplace("ttbarviatau.root","ttbarviatau_mcatnlo.root");
		}
		else if(energy=="13TeV"){
			ana->setFilePostfixReplace("ttbar.root","ttbar_ext_amc_mgbr.root");
			ana->setFilePostfixReplace("ttbarbg.root","ttbarbg_ext_amc_mgbr.root");
                        ana->setIsSignalMerged(true);
		}
	}
	else if(Syst=="TT_GENMCATNLO_down"){
		//this is just default sample
	}
	else if(Syst=="TT_BJESNUDEC_down"){
		ana->addWeightBranch("NTWeight_bJesweightNuDown");
	}
	else if(Syst=="TT_BJESNUDEC_up"){
		ana->addWeightBranch("NTWeight_bJesweightNuUp");
	}
	else if(Syst=="TT_BJESRETUNE_up"){
		ana->addWeightBranch("NTWeight_bJesweightRetune");
	}
	else if(Syst=="TT_BJESRETUNE_down"){
		//default
	}
	else if(Syst=="TOPMASS_up"){ //consider as systematic variation. for testing purp! leaves normalization fixed
		//default
		if (energy=="7TeV" || energy=="8TeV") topmass="178.5";
		else if (energy=="13TeV") {
                    topmass = "175.5";
                    ana->setIsSignalMerged(true);
                }
	}
	else if(Syst=="TOPMASS_down"){
		//default
		if (energy=="7TeV" || energy=="8TeV") topmass="166.5";
		else if (energy=="13TeV") {
                    topmass = "169.5";
                    ana->setIsSignalMerged(true);
                }
	}
	else{
		didnothing=true;
	}

	//top mass

	if(topmass != "172.5"){
		std::cout << "replacing top mass value of 172.5 with "<< topmass << std::endl;
		if (energy=="7TeV" || energy=="8TeV") {
			ana->setFilePostfixReplace("ttbar.root","ttbar_mt"+topmass+ ".root");
			ana->setFilePostfixReplace("ttbarviatau.root","ttbarviatau_mt"+topmass+ ".root");
			ana->setFilePostfixReplace("ttbar_dil.root","ttbar_dil_mt"+topmass+ ".root");
			ana->setFilePostfixReplace("ttbarviatau_dil.root","ttbarviatau_dil_mt"+topmass+ ".root");
		}
		else if (energy == "13TeV"){
			ana->setFilePostfixReplace("ttbar.root","ttbar_mt"+topmass+".root");
			ana->setFilePostfixReplace("ttbarbg.root","ttbarbg_mt"+topmass+".root");
                        ana->setFilePostfixReplace("tW_ext.root","tW_ext_mt"+topmass+".root" );
                        ana->setFilePostfixReplace("tbarW_ext.root","tbarW_ext_mt"+topmass+".root" );

                       // if(topmass == "175.5" || topmass == "169.5"){
                       //         ana->setFilePostfixReplace("tW.root","tW_mt"+topmass+".root" );
                       //         ana->setFilePostfixReplace("tbarW.root","tbarW_mt"+topmass+".root" );
                       //         ana->setFileXsecReplace("tW_mt"+topmass+".root",19.3);
                       //         ana->setFileXsecReplace("tbarW_mt"+topmass+".root",19.3);
    
                       // }
                       
		}
		if((energy=="7TeV" || energy=="8TeV")&&(topmass == "178.5" || topmass == "166.5")){
				ana->setFilePostfixReplace("_tWtoLL.root","_tWtoLL_mt"+topmass+ ".root");
				ana->setFilePostfixReplace("_tbarWtoLL.root","_tbarWtoLL_mt"+topmass+ ".root");
		}

	}

	ana->setFileList(inputfile);



	std::cout << "Calculating btag efficiencies: " << dobtag <<std::endl;



	if(didnothing){
		//create a file outputname_norun that gives a hint that nothing has been done
		//and check in the sleep loop whether outputname_norun exists
		//not needed because merging is done by hand after check, now
		std::cout << "analyse "<< channel <<"_" << energy <<"_" << Syst << " nothing done - maybe some input strings wrong" << std::endl;
	}
	else{
		//if PDF var not found make the same file as above
		//end of configuration

		// recreate file

		TString fulloutfilepath=ana->getOutPath()+".ztop";
		TString workdir=ana->getOutDir() +"../";
		TString batchdir=workdir+"batch/";

		system(("rm -rf "+fulloutfilepath).Data());

		time_t now = time(0);
		char* dt = ctime(&now);
		cout << "\nStarting analyse at: " << dt << endl;


		int fullsucc=ana->start();

		//clean up
		TString rmdummybtagfiles="rm -f "+outdir+"/btag_dummy*";
		system(rmdummybtagfiles.Data());

		ztop::histoStackVector csv;

		if(fullsucc>=0){

			csv.readFromFile(fulloutfilepath.Data()); //DEBUG


			if(testmode){
				std::cout << "drawing plots..." <<std::endl;
				csv.writeAllToTFile(fulloutfilepath+"_plots.root",true,!testmode);
			}

			system(("rm -f "+ana->getOutPath()+"_discr.root").Data());

			system(("touch "+batchdir+ana->getOutFileName()+"_fin").Data());

			sleep(1);
			now = time(0);
			cout << "Started  analyse at: " << dt;// << endl;
			dt = ctime(&now);
			cout << "Finished analyse at: " << dt << endl;

			std::cout <<"\nFINISHED with "<< channel <<"_" << energy <<"_" << Syst <<std::endl;


		}
		else{
			std::cout << "at least one job failed!\n"
					<< "error code meaning: \n"
					<< "-99    : std::exception thrown somewhere (see output)\n"
					<< "-88    : a suspiciously high weight was assigned to a MC event\n"
					<< "-3     : problems finding b-tagging efficiencies (file/file format) \n"
					<< "-2     : write to output file failed\n"
					<< "-1     : input file not found\n"
					<< "<-1000 : problems with output file\n"
					<< std::endl;
			//put more once you introduce one
			if(!testmode){
				system(("touch "+batchdir+ana->getOutFileName()+"_failed").Data());
			}
		}
	}

	delete ana;
	return 0;
}

