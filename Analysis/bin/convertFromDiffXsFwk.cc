#include <iostream>
#include <fstream>
#include <iomanip>

#include "TString.h"
#include "TFile.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TKey.h"
#include "TDirectoryFile.h"                        
#include "TList.h"
#include "TROOT.h"
#include "TError.h"
#include "TSystem.h"

#include "TtZAnalysis/Tools/interface/histo1D.h"
#include "TtZAnalysis/Tools/interface/applicationMainMacro.h"
#include "TtZAnalysis/Tools/interface/plotterMultiplePlots.h"

// #include "TtZAnalysis/Tools/interface/Priors.h"


invokeApplication(){

    using namespace ztop ;

    TString workDir = getenv("FIT_WORKDIR"); 

    std::ifstream in_syst (workDir+"/configs/diffXsFwk/listOfSystFromDiffXsFwk.txt" ,  std::ios::in) ;
    std::ifstream in_hist (workDir+"/configs/diffXsFwk/distribsFromDiffXsFwk.txt"   ,  std::ios::in) ;

    std::vector<TString> systNames ;
    std::vector<TString> histNames ;

    std::vector<TString> typeNames = {"data","ttsignal","ttother","zjets","wjets","singlet","diboson","ttV"};
    std::vector<TString> channels = {"ee","emu","mumu"};

    for ( TString read ; in_syst >> read ; )
        if ( !in_syst.eof() ) systNames.push_back(read);

    for ( TString read ; in_hist >> read ; )
        if ( !in_hist.eof() ) histNames.push_back(read);

    in_syst.close();
    in_hist.close();

    const TString selected_channel = parser->getOpt<TString>("c","","select a specific channel. possibilities: ee, emu, mumu");
    const TString selected_distribution = parser->getOpt<TString>("d","","select a specific distribution. possibilities: whatever is contained in configs/diffXsFwk/distribsFromDiffXsFwk.txt");
    const TString selected_process = parser->getOpt<TString>("p","","select a specific process. possibilities: data, ttsignal , ttother, zjets, wjets, singlet, diboson, ttV");

    parser->doneParsing();

    if (selected_channel!=""){
        if ( std::find(channels.begin(),channels.end(),selected_channel)==channels.end() ){
            std::cout << "Runtime error: channel " <<selected_channel<< " not allowed!" << std::endl;
            throw std::runtime_error("Error: invalid parameter");
        }
        channels.clear();
        channels.push_back(selected_channel);
    }
    
    if (selected_distribution!=""){
        if ( std::find(histNames.begin(),histNames.end(),selected_distribution)==histNames.end() ){
            std::cout << "Runtime error: distribution " <<selected_distribution<< " not allowed!" << std::endl;
            throw std::runtime_error("Error: invalid parameter");
        }
        histNames.clear();
        histNames.push_back(selected_distribution);
    }
    
    if (selected_process!=""){
        if ( std::find(typeNames.begin(),typeNames.end(),selected_process)==typeNames.end() ){
            std::cout << "Runtime error: process " <<selected_process<< " not allowed!" << std::endl;
            throw std::runtime_error("Error: invalid parameter");
        }
        typeNames.clear();
        typeNames.push_back(selected_process);
    }


    for ( TString & channel : channels ) {

        std::cout<<"processing channel "<<channel<<std::endl;

        for ( TString & histName : histNames ){

            std::vector<histo1D*> outHistos;  // vector of histos for different processes

            TFile * inFile_nominal = new TFile (workDir+"/diffXsPlots/Nominal/"+channel+"/"+histName+"_topmass.root");

            if (inFile_nominal->IsZombie()) {

                std::cout << "Runtime error: file " << workDir+"/diffXsPlots/Nominal/"+channel+"/"+histName+"_topmass.root"
                          << " not found.!" << std::endl;
                throw std::runtime_error("Error: file not found");

            }
                        
            for ( TString & typeName : typeNames ) {
                
                std::cout<<"channel: "<<channel<<"\t histogram: "<<histName<<"\t type: "<<typeName<<std::endl;

                TH1D * inHisto_nominal = (TH1D*) inFile_nominal->GetObjectChecked("mt_"+histName+"_"+typeName,"TH1D");
                histo1D * outHisto = new histo1D ();

                outHisto->import(inHisto_nominal, false);

                
                delete inHisto_nominal ;
                outHistos.push_back(outHisto);

            } //end first loop on type

            inFile_nominal->Close();
            delete inFile_nominal  ;


            /************************ now add systematics ************************/
            
            std::cout<<std::endl;

            for ( TString & systName : systNames ) {

                std::cout<<"channel: "<<channel<<"\t histogram: "<<histName<<"\t syst: "<<systName<<std::endl;

                TFile * inFile_systvar = new TFile (workDir+"/diffXsPlots/"+systName+"/"+channel+"/"+histName+"_topmass.root");

                if (inFile_systvar->IsZombie()) {

                    std::cout << "Runtime error: file " << workDir+"/diffXsPlots/"+systName+"/"+channel+"/"+histName+"_topmass.root"
                              << " not found.!" << std::endl;
                    throw std::runtime_error("Error: file not found");

                }



                for ( TString & typeName : typeNames ) {

                    unsigned int i_type = (unsigned int)(&typeName - &typeNames.at(0));

                    TH1D  * inHisto_systvar = (TH1D*) inFile_systvar->GetObjectChecked("mt_"+histName+"_"+typeName,"TH1D");
                    histo1D * temp_systvar = new histo1D ();

                    temp_systvar->import(inHisto_systvar, false);

                    TString errName = systName;
                    errName.ToLower();

                    if ( errName.Contains("_up") || errName.Contains("_down") )
                        outHistos.at(i_type)->addErrorContainer(errName, *temp_systvar);

                    else {

                        if (systName == "MCATNLO") errName = "had";
                        else if (systName == "POWHEG") errName = "mod";

                        errName += "_up";
                        outHistos.at(i_type)->addErrorContainer(errName, *temp_systvar);

                        errName.ReplaceAll("_up","_down");
                        outHistos.at(i_type)->addErrorContainer(errName, *(outHistos.at(i_type)));

                    }
                    
                    delete inHisto_systvar ;
                    delete temp_systvar;

                } //end second loop on type

                inFile_systvar->Close();
                delete inFile_systvar;
                
            }// end loop on systs

            std::cout<<std::endl<<"end loop on syst variations"<<std::endl<<std::endl;

            for ( TString & typeName : typeNames ) {

                unsigned int i_type = (unsigned int)(&typeName - &typeNames.at(0));
                std::string pathOutFile = (std::string) (workDir+"/ztopFiles/rawHistos/"+channel+"/"+typeName);  

                gSystem->Exec("mkdir -p " + (TString) pathOutFile );

                pathOutFile += (std::string) ("/"+channel+"_"+typeName+"_"+histName+".ztop");  

                outHistos.at(i_type)->writeToFile( pathOutFile );
                delete outHistos.at(i_type);

            }// end third loop on type

        } // end loop on histograms
    } // end loop on channels


    std::cout<<std::endl;

    return 0;

}



