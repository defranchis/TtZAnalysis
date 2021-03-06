/*
 * fitTtBarXsec.cc
 *
 *  Created on: Oct 14, 2014
 *      Author: kiesej
 */


#include "TtZAnalysis/Tools/interface/applicationMainMacro.h"
#include "../interface/ttbarXsecFitter.h"
#include <iostream>
#include <string>
#include <algorithm>
#include "TtZAnalysis/Tools/interface/histoStack.h"
#include "TtZAnalysis/Tools/interface/plotterControlPlot.h"
#include "TtZAnalysis/Tools/interface/plotterMultiplePlots.h"
#include "TtZAnalysis/Tools/interface/plotterCompare.h"
#include "TtZAnalysis/Tools/interface/plotter2D.h"
#include "TFile.h"
#include <TError.h>
#include <TtZAnalysis/Tools/interface/plotterMultiColumn.h>
#include "TtZAnalysis/Tools/interface/fileReader.h"
#include "TtZAnalysis/Tools/interface/histoStackVector.h"

invokeApplication(){
	using namespace ztop;

	parser->bepicky=true;
	const TString lhmode = parser->getOpt<TString>("m","poissondata",
			"modes for the likelihood assumption in the fit.  possible: chi2data, chi2datamc, chi2datafullmc, poissondata default: poissondata");
	const TString inputconfig = parser->getOpt<TString>("i","ttbarXsecFitter_mll.txt","input config (located in TtZAnalysis/Analysis/configs");
	const bool onlyMC=parser->getOpt<bool>("-onlyMC",false,"use plain MC, no data. Compare sum(MC) to MC. useful to check if a fit could work theoretically");
	const TString fitToVariation=parser->getOpt<TString>("-fitToVariation","","fit to MC variation");
	if(lhmode!="chi2datamc" && lhmode!="chi2data" && lhmode!="poissondata" && lhmode!="chi2datafullmc"){
		parser->coutHelp();
		exit(-1);
	}
	const bool exclude0bjetbin  = parser->getOpt<bool>("-exclude0bjet",false,"Excludes 0,3+ bjet bin from the fit");
	const int npseudoexp = parser->getOpt<int>("p",0,"number of pseudo experiments");
	const unsigned int nToys = parser->getOpt<int>("-nToys",0,"number of toys");
	const unsigned int seed = parser->getOpt<int>("-seed",0,"RNG seed for toys");
	const bool debug = parser->getOpt<bool>("d",false,"switches on debug output");
	const TString pseudoOpts = parser->getOpt<TString>("-pdopts","",
			"additional options for pseudodata: Gaus: use Gaussian random distribution ");
	const bool fitsystematics =! parser->getOpt<bool>("-nosyst",false,"removes systematics");
	const bool onlyemu = parser->getOpt<bool>("-emu",false,"fit only emu channel");
	const bool onlytotalerror = parser->getOpt<bool>("-onlytotal",false,"no syst breakdown");
	const bool docontrolplots = parser->getOpt<bool>("-controlplots",false,"only control plots");
	const bool nominos = parser->getOpt<bool>("-nominos",false,"switches off systematics breakdown");
	const float topmass = parser->getOpt<float>("-topmass",0,"Set top mass");
	const bool tmpcheck = parser->getOpt<bool>("M",false,"top mass fit");
	const bool candc = parser->getOpt<bool>("-cutandcount",false,"also produce fast cut and count (2 jets 1 b-tag) result");

	const bool printplots = ! parser->getOpt<bool>("-noplots",false,"switches off all plotting");
	const bool dummyrun =  parser->getOpt<bool>("-dummyrun",false,"fit will not be performed, only dummy values will be returned (for testing of plots etc)");

	const bool variationplots = parser->getOpt<bool>("-varplots",false,"switches on variation plots");

	const bool topontop =  parser->getOpt<bool>("-topontop",false,"plots ttbar signal on top");
	const bool likelihoodscan =  parser->getOpt<bool>("-scan",false,"Maps the likelihood around the minimum as a function of the cross section(s). Needs a lot of CPU time!");

	const TString scanParameter =  parser->getOpt<TString>("-scanParameter","","scans specified parameter");

	const bool mlbCrossCheck = parser->getOpt<bool>("-mlbCrossCheck",false,"cross check with only one mlb distribution");
	const bool mttfit = parser->getOpt<bool>("-mttfit",false,"differential mtt analysis");
	const unsigned int n_mttbins = parser->getOpt<int>("-mttbins",4,"number of mtt bins");

	TString outfile;

	outfile = parser->getOpt<TString>("o","xsecFit","output file name");


	parser->doneParsing();
	std::string cmsswbase=getenv("CMSSW_BASE");


	const std::string fullcfgpath=
			inputconfig.BeginsWith("/") || inputconfig.BeginsWith("./") ? "" :
					(cmsswbase+"/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/");
	if(!fileExists((fullcfgpath+inputconfig).Data())){
		std::cout << "fitTtBarXsec: input file not found. \nAvailable files in " <<cmsswbase+"/src/TtZAnalysis/Analysis/configs/fitTtBarXsec:"<<std::endl;
		system(("ls "+fullcfgpath).data());
		return -1;
	}

        if (npseudoexp && nToys){
            throw std::logic_error("fitTtBarXsec: cannot perform pseudo-experiments and systematic toys at the same time");
        }

	ttbarXsecFitter mainfitter;
	mainfitter.setTopOnTop(topontop);
	mainfitter.setDummyFit(dummyrun);
        mainfitter.setMassFit(tmpcheck);
        mainfitter.setDoMttFit(mttfit, n_mttbins); 

        if (mttfit) std::cout<<"number of mtt bins (signals) = "<<n_mttbins<<std::endl;

	if(lhmode=="chi2datamc")
		mainfitter.setLikelihoodMode(ttbarXsecFitter::lhm_chi2datamcstat);
	if(lhmode=="chi2data")
		mainfitter.setLikelihoodMode(ttbarXsecFitter::lhm_chi2datastat);
	if(lhmode=="poissondata")
		mainfitter.setLikelihoodMode(ttbarXsecFitter::lhm_poissondatastat);
	if(lhmode=="chi2datafullmc")
		mainfitter.setLikelihoodMode(ttbarXsecFitter::lhm_chi2datafullmcstat);

	if(topmass)
		mainfitter.setReplaceTopMass(topmass);

	mainfitter.setExcludeZeroBjetBin(exclude0bjetbin);
	mainfitter.setUseMCOnly(onlyMC);
	mainfitter.setEmuOnly(onlyemu);
        mainfitter.setFitToVariation(fitToVariation);
        mainfitter.setNoMinos(nominos);
	mainfitter.setNoSystBreakdown((onlytotalerror));
	mainfitter.setIgnorePriors(!fitsystematics);
	mainfitter.setRemoveSyst(!fitsystematics);
	mainfitter.setMlbCrossCheck(mlbCrossCheck);

	//extendedVariable::debug=true;
	ttbarXsecFitter::debug=debug;



	if(npseudoexp){
		mainfitter.setRemoveSyst(true);
		if (npseudoexp>1) mainfitter.setSilent(true);
		mainfitter.setIgnorePriors(true);
	}
        if (nToys){
            mainfitter.setSeed(seed);
            mainfitter.setSilent(true);
            mainfitter.setDoToys(true);
        }

        if (nToys) std::cout<<nToys<<" toys with seed "<<seed<<std::endl;
	//simpleFitter::printlevel=1;

	mainfitter.readInput((fullcfgpath+inputconfig).Data());
	std::cout << "Input file sucessfully read. Free for changes." << std::endl;

	if(docontrolplots){

		//get list of input files, fast brute force -> can be changed since input is read by
		// fitter before, now

		fileReader fr;
		fr.setComment("#");
		fr.setDelimiter(",");
		fr.setStartMarker("");
		fr.setEndMarker("");
		fr.readFile((fullcfgpath+inputconfig).Data());
		std::vector<std::string>  filestoprocess;
		std::vector<std::string>  datasets;
		for(size_t i=0;i<fr.nLines();i++){
			if(fr.nEntries(i)){
				for(size_t j=0;j<fr.nEntries(i);j++){
					std::string tmp=fr.getData<std::string>(i,j);
					if(tmp.find(".ztop") != std::string::npos){
						textFormatter ftm;
						ftm.setDelimiter("|");
						std::vector<std::string> formline=ftm.getFormatted(tmp);
						for(size_t lentr=0;lentr<formline.size();lentr++){
							if(fileExists(formline.at(lentr).data())){
								if(std::find(filestoprocess.begin(),filestoprocess.end(),formline.at(lentr)) == filestoprocess.end()){

									filestoprocess.push_back(formline.at(lentr));
									//get dataset name
									ftm.setDelimiter("_");
									std::vector<std::string> nameparts=ftm.getFormatted(formline.at(lentr));
									for(size_t part=0;part<nameparts.size();part++){
										if(nameparts.at(part).find("TeV")!=std::string::npos){

											datasets.push_back(nameparts.at(part));
										}
									}
								}
							}
						}
					}
				}
			}
		}

		std::string specialplotsfile=cmsswbase+"/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/selected_controlplots.txt";
                if (mttfit) specialplotsfile=cmsswbase+"/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/selected_controlplots_mtt.txt";
                
		//get files

		for(size_t file=0;file<filestoprocess.size();file++){
                        TString channel_cp = "emu";
                        if ( ((TString)filestoprocess.at(file)).BeginsWith("ee_") ){
                            specialplotsfile=cmsswbase+"/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/selected_controlplots_ee.txt";
                            channel_cp = "ee";
                        }
                        else if ( ((TString)filestoprocess.at(file)).BeginsWith("mumu_") ){
                            specialplotsfile=cmsswbase+"/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/selected_controlplots_mumu.txt";
                            channel_cp = "mumu";
                        }

			TString ctrplotsoutir=outfile+"_controlPlots_"+channel_cp;
			//ctrplotsoutir+="_"+datasets.at(file);
			system(((TString)"mkdir -p " +ctrplotsoutir).Data());
			histoStackVector vec;
			vec.readFromFile(filestoprocess.at(file));
			fileReader specialcplots;
			specialcplots.setComment("$");
			std::cout << datasets.at(file) << std::endl;
			//fileReader::debug=true;
			specialcplots.setStartMarker("[additional plots "+ datasets.at(file)+ "]");
			specialcplots.setEndMarker("[end - additional plots]");
			specialcplots.setDelimiter(",");
			specialcplots.readFile(specialplotsfile);
			//fileReader::debug=false;
			//get plot names
			std::vector<std::string> plotnames= specialcplots.getMarkerValues("plot");
                        std::vector<std::string> formatedplotnames= specialcplots.getMarkerValues("plot");
                        for(size_t pln=0;pln<formatedplotnames.size();pln++){
                                std::replace(formatedplotnames.at(pln).begin(),formatedplotnames.at(pln).end(),';',',');
                        }

			std::string fracfile=specialcplots.dumpFormattedToTmp();

			std::vector<size_t> namemulti(plotnames.size(),0);
			//check for doubles
			for(size_t pln=0;pln<plotnames.size();pln++){
				if(std::find(plotnames.begin()+pln+1,plotnames.end(),plotnames.at(pln))!=plotnames.end()){
					namemulti.at(pln)++;
					std::cout << "found plot "<< plotnames.at(pln) << "twice. will add to outname "<<std::endl;
				}
			}

			for(size_t stackit=0;stackit<plotnames.size();stackit++){
				histoStack stack=vec.getStack(formatedplotnames.at(stackit).data());
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

                                else if (stack.getName() == "lead lepton pt step 8") stack = stack.cutLeft(20-0.1);
                                else if (stack.getName() == "seclead lepton pt step 8") stack = stack.cutLeft(20-0.1);


				plotterControlPlot pl;
				//plotterControlPlot::debug=true;
				//histoStack::debug=true;
				pl.readStyleFromFileInCMSSW(specialcplots.getValue<std::string>("defaultStyle"));
				std::string mergelgdefs=(std::string)getenv("CMSSW_BASE")+(std::string)"/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/controlPlots_mergeleg.txt";
				pl.addStyleFromFile( mergelgdefs,"[merge for control plots]","[end - merge for control plots]");
				//	textBoxes::debug=true;
				if(stackit+1<plotnames.size())
					pl.addStyleFromFile(fracfile,  "[plot - " + plotnames.at(stackit)+"]", "[plot - "+ plotnames.at(stackit+1) +"]" );
				else
					pl.addStyleFromFile(fracfile,  "[plot - " + plotnames.at(stackit) +"]","[end - additional plots]");
				//pl.readTextBoxesInCMSSW("/src/TtZAnalysis/Analysis/configs/general/CMS_boxes.txt","CMSPaperSplit03Left");
				size_t datasetidx=mainfitter.getDatasetIndex(datasets.at(file).data() );
				mainfitter.addUncertainties(&stack, datasetidx);
                                for (TString m_syst : stack.getSystNameList()){
                                    if (m_syst.BeginsWith("BG_") || m_syst=="TOPMASS" || (m_syst=="TT_GENMCATNLO"&&(!tmpcheck))){
                                        stack.removeError(m_syst+"_up");
                                        stack.removeError(m_syst+"_down");
                                        if (debug) std::cout<<"removed systematics "<<m_syst<<" from control plots"<<std::endl;
                                    }
                                }

				pl.setStack(&stack);
				std::string outname=(ctrplotsoutir+"/"+stack.getFormattedName()+"_"+datasets.at(file)).Data();
				if(namemulti.at(stackit)>0)
					outname+=toString(namemulti.at(stackit));
				pl.printToPdf(outname);
				//pl.saveAsCanvasC((ctrplotsoutir+"/"+stack.getFormattedName()+"_"+datasets.at(file)).Data());
				pl.printToPng(outname);
			}
			system(("rm -f "+fracfile).data());
		}

                if(npseudoexp>1 || nToys > 1) return 0;
	} //docontrolplots


	//simpleFitter::printlevel=-1; //for now



	if(candc){
		std::cout << "C&C cross check:" <<std::endl;
		for(size_t i=0;i<mainfitter.nDatasets();i++){
			mainfitter.cutAndCountSelfCheck(i);
			std::cout << std::endl;
		}
	}
	bool fitsucc=true;
	//ttbarXsecFitter::debug=true;

	if(npseudoexp || nToys){
		size_t failcount=0;
		gErrorIgnoreLevel = 3000;
		size_t ndatasets=mainfitter.nDatasets();

		std::vector<histo1D> pulls;
		pulls.resize(ndatasets,histo1D(histo1D::createBinning(40,-8,8)));

		//plotterControlPlot pl;
		TCanvas cvv;

		//	pl.usePad(&cvv);

		//configure mode
		histo1D::pseudodatamodes pdmode=histo1D::pseudodata_poisson;
		if(pseudoOpts.Contains("Gaus"))
			pdmode=histo1D::pseudodata_gaus;
		std::cout << std::endl;
                int n_iterations = npseudoexp;
                if (nToys) n_iterations = nToys;
		for(int i=0;i<n_iterations;i++){
			if(i>0)
				displayStatusBar(i,n_iterations);
			std::vector<float> xsecs,errup,errdown;
                        if (npseudoexp)	mainfitter.createPseudoDataFromMC(pdmode);
                        else mainfitter.createToysFromSyst(pdmode);
			mainfitter.createContinuousDependencies();
			if(!i){
                            if (npseudoexp) std::cout << "creating pseudo experiments...\n" <<std::endl;
                            else std::cout << "creating toy experiments...\n" <<std::endl;
                        }
			bool succ=true;
			try{
                               mainfitter.fit(xsecs,errup,errdown);
			}catch(...){
				failcount++;
				succ=false;
			}
                        if(nToys>0 && succ){
                            if (tmpcheck){
                                mainfitter.createSystematicsBreakdown(0,"TOPMASS");
                                texTabler tab=mainfitter.makeSystBreakDownTable(0,true,"TOPMASS");
                                tab.writeToFile(outfile+"_tab_TOPMASS_"+std::to_string(seed)+"_"+std::to_string(i+1)+".tex");
                            }
                            else{
                                mainfitter.createSystematicsBreakdown(0);
                                texTabler tab=mainfitter.makeSystBreakDownTable(0,true);
                                tab.writeToFile(outfile+"_tab13TeV_"+std::to_string(seed)+"_"+std::to_string(i+1)+".tex");
                            }
                        }
			if(succ && ! tmpcheck){
				for(size_t ndts=0;ndts<ndatasets;ndts++){
					double pull=xsecs.at(ndts)-mainfitter.getXsecOffset(ndts);
					if(pull>0)
						pull/=fabs(errup.at(ndts));
					else
						pull/=fabs(errdown.at(ndts));
					pulls.at(ndts).fill ( pull);
				}
			}

			if(succ && i<3 && 0){ //off for testing
				for(size_t nbjet=0;nbjet<3;nbjet++){
					for(size_t ndts=0;ndts<ndatasets;ndts++){
						mainfitter.printControlStack(false,nbjet,ndts,"pd_"+toString(i));
					}
				}
			}
		}
		std::cout << std::endl;
		histo1D abspull=(histo1D::createBinning(40,-1,1));
		if(tmpcheck){

			const std::vector<float> & par=mainfitter.tempdata;
			for(size_t i=0;i<par.size();i++){
				abspull.fill(par.at(i)*3); //HARDCODED
				double stat=(std::max(fabs(par.at(i+1)),fabs(par.at(i+2))));
                                // double stat = fabs(par.at(i+3));
				double pull = par.at(i) / stat;
				if(!i){
					std::cout << "parameter stat error: " << stat << std::endl;
				}

				++i;++i;// ++i;
				for(size_t ndts=0;ndts<ndatasets;ndts++){
					pulls.at(ndts).fill ( pull);
				}
			}
		}

		//fit
		for(size_t i=0;i<pulls.size()+1;i++){ //both pulls
                    if (nToys > 0) break;
			histo1D * c=0;
                        TH1D * h=0;
			if(i<pulls.size())
				c=&pulls.at(i);
			else
				c=&abspull;

                        h = c->getTH1D();
                        h->SetName("pull");
			graph tofit; tofit.import(c,true);
			graphFitter fitter;
			fitter.readGraph(&tofit);
			fitter.setFitMode(graphFitter::fm_gaus);
			fitter.setParameter(0,0.5*c->integral(false)); //norm
			fitter.setParameter(1,0); //xshift
			fitter.setParameter(2,1); //width
			fitter.setStrategy(1);
			if(i==pulls.size())
				fitter.setParameter(2,0.05); //width

			//fitter.printlevel =2;
			//graphFitter::debug=true;

			fitter.fit();
			//	bool succ=	fitter.wasSuccess();
			graph fitted=fitter.exportFittedCurve(500);
			formatter fmt;
			float roundpeak=fmt.round(fitter.getParameter(1),0.01);
			float roundwidth=fmt.round(fitter.getParameter(2),0.01);

			TCanvas * cv=new TCanvas();
			plotterMultiplePlots plm;
                        if (tmpcheck) plm.readStyleFromFileInCMSSW("/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/multiplePlots_pulls_mass.txt");
                        else plm.readStyleFromFileInCMSSW("/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/multiplePlots_pulls.txt");
			plm.usePad(cv);
			plm.addPlot(c);
			plm.setLastNoLegend();
			fitted.setName("#splitline{peak: "+fmt.toTString(roundpeak) + "}{width: " +toTString(roundwidth)+"}");
			plm.addPlot(&fitted);
			plm.draw();
			std::string outfilefull="";
			if(i<pulls.size()){
				TFile f(outfile+"_pull_" + mainfitter.datasetName(i)+ ".root","RECREATE");
				cv->Write();
                                h->Write();
				f.Close();
				cv->Print(outfile+"_pull_" + mainfitter.datasetName(i)+ ".pdf");
				outfilefull=toString(outfile+"_pull_" + mainfitter.datasetName(i)+".ztop");
			}
			else{
				TFile f(outfile+"_pull_abs.root","RECREATE");
				cv->Write();
                                h->Write();
				f.Close();
				cv->Print(outfile+"_pull_abs.pdf");
				outfilefull=toString(outfile+"_pull_abs.ztop");
			}
			delete cv;
			c->writeToFile(outfilefull); //for post-fits

		}
		if (npseudoexp) std::cout << "Pseudodata run done. " << failcount << " failed out of " << npseudoexp << std::endl;
                else std::cout << "Syst toys run done. " << failcount << " failed out of " << nToys << std::endl;

                if(n_iterations>1) return 0;
	}
	else{
		//ttbarXsecFitter::debug=false;
		//simpleFitter::printlevel=1; //for now
		try{
			mainfitter.fit();
		}
		catch(std::exception &e){
			std::cout << "Fit failed, will do plots only" <<std::endl;
			fitsucc=false;
			e.what();
		}
	}


	///////plotting here

	TFile * f = new TFile(outfile+".root","RECREATE");

	//compare input stacks before and after

	if(printplots){




		histoStack stack;
		//plotterControlPlot pl;
		//pl.readStyleFromFileInCMSSW("/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/controlPlots_combined.txt");
		TCanvas c;
		c.Print(outfile+".pdf(");
		//pl.usePad(&c);
		if(debug)
			std::cout << "printing pre/postfit fit-distributions" << std::endl;
		for(size_t ndts=0;ndts<mainfitter.nDatasets();ndts++){
                    if(likelihoodscan){
                        if (!mttfit) mainfitter.printXsecScan(ndts,outfile.Data());
                        else {
                            for (size_t mttbin=0; mttbin<n_mttbins; ++mttbin) //hardcoded
                                mainfitter.printXsecScan(ndts,outfile.Data(),mttbin);
                        }
                    }
                    if (scanParameter!=""){
                        mainfitter.printScan(ndts,outfile.Data(),scanParameter);
                    }
			TString dir=outfile+"_vars/";
			system( ("mkdir -p "+dir).Data());
			for(size_t nbjet=0;nbjet<3;nbjet++){
                                // if (!onlyemu && nbjet==0) continue;

				mainfitter.printControlStack(false,nbjet,ndts,outfile.Data());
				mainfitter.printControlStack(true,nbjet,ndts,outfile.Data());

				if(variationplots){
					mainfitter.printVariations(nbjet,ndts,dir.Data());
					if(!dummyrun)
						mainfitter.printVariations(nbjet,ndts,dir.Data(),true);
				}
			}
		}


		if(debug)
			std::cout << "printing correlations" << std::endl;
		c.SetName("correlations");
		plotter2D pl2d;
		pl2d.readStyleFromFileInCMSSW("src/TtZAnalysis/Analysis/configs/fitTtBarXsec/plot2D_-1to1.txt");
		pl2d.usePad(&c);
		histo2D corr2d=mainfitter.getCorrelations();
		pl2d.setPlot(&corr2d);
		if(fitsucc){
			pl2d.draw();
			c.Print(outfile+".pdf");
			c.Write();
		}

		c.Print(outfile+".pdf)");
	}
	if(debug)
		std::cout << "producing tables" << std::endl;
	for(size_t ndts=0;ndts<mainfitter.nDatasets();ndts++){
		TString dtsname=mainfitter.datasetName(ndts);
		texTabler tab;

                if (!mttfit){
                    tab=mainfitter.makeSystBreakDownTable(ndts);
                    tab.writeToFile(outfile+"_tab" +dtsname + ".tex");
                    tab.writeToPdfFile(outfile+"_tab" +dtsname + ".pdf");
                    std::cout << tab.getTable() <<std::endl;
                    tab=mainfitter.makeSystBreakDownTable(ndts,false); //vis PS
                    tab.writeToFile(outfile+"_tab_simple" +dtsname + ".tex");
                    tab.writeToPdfFile(outfile+"_tab_simple" +dtsname + ".pdf");
                    std::cout << tab.getTable() <<std::endl;
                }
                else{
                    for (size_t s=0; s<n_mttbins; ++s){ //hardcoded
                        tab=mainfitter.makeSystBreakDownTable(ndts,true,"",s);
                        tab.writeToFile(outfile+"_tab" +dtsname +"_mtt"+toString(s+1)+ ".tex");
                        tab.writeToPdfFile(outfile+"_tab" +dtsname+"_mtt"+toString(s+1) + ".pdf");
                        std::cout << tab.getTable() <<std::endl;
                        tab=mainfitter.makeSystBreakDownTable(ndts,false,"",s); //vis PS
                        tab.writeToFile(outfile+"_tab_simple" +dtsname+"_mtt"+toString(s+1) + ".tex");
                        tab.writeToPdfFile(outfile+"_tab_simple" +dtsname+"_mtt"+toString(s+1) + ".pdf");
                        std::cout << tab.getTable() <<std::endl;
                    }
                }
                
		//graph out
                if (!mttfit){
                    graph resultsgraph=mainfitter.getResultsGraph(ndts,topmass);
                    if(fitsucc)
			resultsgraph.writeToFile((outfile+"_graph" +dtsname + ".ztop").Data());
                }

	}
	if(mainfitter.nDatasets()>1 && fitsucc){
		std::cout << "producing ratio..." <<std::endl;
		for(size_t ndts=0;ndts<mainfitter.nDatasets();ndts++){
			for(size_t ndts2=ndts;ndts2<mainfitter.nDatasets();ndts2++){
				if(ndts2==ndts) continue;
				double errup,errdown;
				formatter fmt;
				double ratio=mainfitter.getXsecRatio(ndts,ndts2,errup,errdown);//full ps
				texTabler ratiotab(" c | l l l");
				ratiotab << "Ratio: " + mainfitter.datasetName(ndts) + "/" + mainfitter.datasetName(ndts2) ;
				ratiotab << fmt.round(ratio,0.01) << "$\\pm$" << "$^{" + fmt.toTString(fmt.round(errup,0.01)) +"}_{" +
						fmt.toTString(fmt.round(errdown,0.01)) +"}$";
				ratiotab.writeToFile(outfile+"_ratio_" +mainfitter.datasetName(ndts) +"_"+mainfitter.datasetName(ndts2)+".tex");
				ratiotab.writeToPdfFile(outfile+"_ratio_" +mainfitter.datasetName(ndts) +"_"+mainfitter.datasetName(ndts2)+".pdf");

			}
		}
	}
	std::cout << "writing correlation table..." <<std::endl;
	texTabler corr=mainfitter.makeCorrTable();
	corr.writeToFile(outfile+"_tabCorr.tex");
	corr.writeToPdfFile(outfile+"_tabCorr.pdf");

	mainfitter.writeCorrelationMatrix((outfile+"_corrRaw.txt").Data(),true);

	//std::cout << corr.getTable() <<std::endl;

	if(tmpcheck){
		mainfitter.createSystematicsBreakdown(0,"TOPMASS");
		texTabler tab=mainfitter.makeSystBreakDownTable(0,true,"TOPMASS"); //vis PS
		tab.writeToFile(outfile+"_tab_TOPMASS.tex");
		tab.writeToPdfFile(outfile+"_tab_TOPMASS.pdf");
		tab=mainfitter.makeSystBreakDownTable(0,false,"TOPMASS"); //vis PS
		tab.writeToFile(outfile+"_tab_simple_TOPMASS.tex");
		tab.writeToPdfFile(outfile+"_tab_simple_TOPMASS.pdf");
	}

	f->Close();
	delete f;
	if(printplots){
		std::cout << "making additional plots (can be canceled)..." <<std::endl;
		//make some plots with and without constraints
		//test area
		for(size_t ndts=0;ndts<mainfitter.nDatasets();ndts++){
			std::string infile="emu_";
			infile+=mainfitter.datasetName(ndts).Data();
			infile+="_172.5_nominal_syst.ztop";
			std::string dir=outfile.Data();
			dir+="_emu_";
			dir+=mainfitter.datasetName(ndts).Data();
			system(("mkdir -p "+dir).data());
			dir+="/";
                        if (mttfit) mainfitter.printAdditionalControlplots(infile,cmsswbase+"/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/prefit_postfit_plots_mtt.txt",dir);
                        else if (!mlbCrossCheck) mainfitter.printAdditionalControlplots(infile,cmsswbase+"/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/prefit_postfit_plots.txt",dir);
                        else mainfitter.printAdditionalControlplots(infile,cmsswbase+"/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/prefit_postfit_plots_mlb.txt",dir);
		}
                if (!onlyemu){
                    for(size_t ndts=0;ndts<mainfitter.nDatasets();ndts++){
                        std::string infile="mumu_";
                        infile+=mainfitter.datasetName(ndts).Data();
                        infile+="_172.5_nominal_syst.ztop";
                        std::string dir=outfile.Data();
                        dir+="_mumu_";
                        dir+=mainfitter.datasetName(ndts).Data();
                        system(("mkdir -p "+dir).data());
                        dir+="/";
                        mainfitter.printAdditionalControlplots(infile,cmsswbase+"/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/prefit_postfit_plots_mumu.txt",dir);
                    }
                    for(size_t ndts=0;ndts<mainfitter.nDatasets();ndts++){
                        std::string infile="ee_";
                        infile+=mainfitter.datasetName(ndts).Data();
                        infile+="_172.5_nominal_syst.ztop";
                        std::string dir=outfile.Data();
                        dir+="_ee_";
                        dir+=mainfitter.datasetName(ndts).Data();
                        system(("mkdir -p "+dir).data());
                        dir+="/";
                        mainfitter.printAdditionalControlplots(infile,cmsswbase+"/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/prefit_postfit_plots_ee.txt",dir);
                    }
                }
	}
	std::cout << "fitTtBarXsec done" << std::endl;
	return 0;
}
