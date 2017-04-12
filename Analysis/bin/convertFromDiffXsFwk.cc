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

    using namespace std  ;
    using namespace ztop ;

    // Prior aprior = Prior::Gauss;
    
    // if (aprior == Prior::Gauss)
    //     cout<<"yes"<<endl;

    // else cout<<"no"<<endl;

    // return 0;

    // gErrorIgnoreLevel = kWarning;
    // gErrorIgnoreLevel = kError;
    // gErrorIgnoreLevel = kBreak;

    // unsigned int rebin_mlb = 5;

    TString workDir = getenv("FIT_WORKDIR"); 

    // ifstream in_path (workDir+"/myUtils/diLept_dir.txt"    ,  ios::in) ;
    ifstream in_syst (workDir+"/configs/diffXsFwk/listOfSystFromDiffXsFwk.txt" ,  ios::in) ;
    ifstream in_hist (workDir+"/configs/diffXsFwk/distribsFromDiffXsFwk.txt"   ,  ios::in) ;

    // ifstream in_syst (workDir+"/myUtils/reducedSyst.txt"   ,  ios::in) ;

    // TString path_to_diLept ; in_path >> path_to_diLept;

    vector<TString> systNames ;
    vector<TString> histNames ;

    vector<TString> typeNames = {"data","ttsignal","ttother","zjets","wjets","singlet","diboson"};
    vector<TString> channels = {"ee","emu","mumu"};

    // vector<TString> shapeSysts {"MASS_", "SCALE_", "MATCH_", "HAD_", "MOD_", "JES_", "JER_", "PU_", "LEPT_", "TRIG_",
    //         "BTAG_", "BTAG_PT_", "BTAG_ETA_","BTAG_LJET_", "BTAG_LJET_PT_", "BTAG_LJET_ETA_","POWHEG","MCATNLO"};


/*************************************************************************************/


    for ( TString read ; in_syst >> read ; )
        if ( !in_syst.eof() ) systNames.push_back(read);


    for ( TString read ; in_hist >> read ; )
        if ( !in_hist.eof() ) histNames.push_back(read);


    in_syst.close();
    in_hist.close();


/*************************************************************************************/


    // TFile * aFile = new TFile ( path_to_diLept + "/selectionRoot/Nominal/emu/emu_run2015D.root" );
    // ofstream out_hist (workDir+"/myUtils/histoList.txt", ios::out);

    // for ( auto const && aKey : *aFile->GetListOfKeys()){
    //     auto key = (TKey*) aKey;
                        
    //     if ( ((TString)(key->GetClassName())).Contains("TH1") )
    //         if ( (((TString)key->GetName()).Contains("Hyp")) && (!(((TString)key->GetName()).Contains("bcp_"))) ) {

    //             histNames.push_back( key->GetName() );
    //             out_hist<<key->GetName()<<std::endl;
    //         }

    // } //end loop on keys
    
    // out_hist.close();

    // aFile->Close();
    // delete aFile;

    // aFile = new TFile ( path_to_diLept + "/Plots/Nominal/emu/"+histNames.at(0)+"_source.root" );
    // ofstream out_type (workDir+"/myUtils/typeList.txt", ios::out);

    // for ( auto const && aKey : *aFile->GetListOfKeys()){
    //     auto key = (TKey*) aKey;
                        
    //     if ( ((TString)(key->GetClassName())).Contains("TH1D") ) {

    //        TString keyName = (TString) key->GetName();

    //         if ( keyName.BeginsWith("m_") ) {

    //             keyName.ReplaceAll("m_","");
    //             keyName.ReplaceAll(histNames.at(0),"");
    //             keyName.ReplaceAll("_","");

    //             typeNames.push_back( keyName );
    //             out_type<<keyName<<std::endl;

    //         }
    //     }
    // } //end loop on keys

    // out_type.close();

    // std::cout<<std::endl;

    // ofstream outFile (workDir+"/myUtils/found_distrib.txt", ios::out);

    for ( TString & channel : channels ) {
        // if ( !(channel=="emu") ) continue; // comment this line later!

        std::cout<<"processing channel "<<channel<<std::endl;

        for ( TString & histName : histNames ){
            if (!histName.Contains("LeptonBjetMass")) continue; // comment this line later!
            // if (!histName.Contains("LeptonBjetMass") && !histName.Contains("HypTopMass")) continue; // comment this line later!

            vector<histo1D*> outHistos;  // vector of histos for different processes

            TFile * inFile_nominal = new TFile (workDir+"/diffXsPlots/Nominal/"+channel+"/"+histName+"_source.root");

            if (inFile_nominal->IsZombie()) {

                std::cout << "Runtime error: file " << workDir+"/diffXsPlots/Nominal/"+channel+"/"+histName+"_source.root"
                          << " not found.!" << std::endl;
                throw std::runtime_error("Error: file not found");

            }
                        

            // if (channel=="emu") outFile<<histName<<std::endl;

            for ( TString & typeName : typeNames ) {
                // if ( !(typeName.Contains("data")) ) continue; // comment this line later!
                
                std::cout<<"channel: "<<channel<<"\t histogram: "<<histName<<"\t type: "<<typeName<<std::endl;

                TH1D * inHisto_nominal = (TH1D*) inFile_nominal->GetObjectChecked("m_"+histName+"_"+typeName,"TH1D");
                histo1D * outHisto = new histo1D ();

                // if ( histName == "HypLeptonBjetMass" ) {
                //     inHisto_nominal->Rebin(rebin_mlb);
                //     // if (channel!="emu") inHisto_nominal->Rebin(2);
                // }

                outHisto->import(inHisto_nominal, false);

                // if ( ! typeName.Contains("data") ){

                //     double lumi_unc = 0.027;

                //     TH1D * inHisto_lumi_up = (TH1D*) inHisto_nominal->Clone();
                //     TH1D * inHisto_lumi_down = (TH1D*) inHisto_nominal->Clone();

                //     inHisto_lumi_up->Scale( 1 + lumi_unc );
                //     inHisto_lumi_down->Scale( 1 - lumi_unc );

                //     histo1D * lumi_up = new histo1D ();
                //     histo1D * lumi_down = new histo1D ();

                //     lumi_up->import(inHisto_lumi_up, true);
                //     lumi_down->import(inHisto_lumi_down, true);
                    
                //     outHisto->addErrorContainer("lumi_up", *lumi_up);
                //     outHisto->addErrorContainer("lumi_down", *lumi_down);
                    
                //     delete inHisto_lumi_up;
                //     delete inHisto_lumi_down;

                //     delete lumi_up;
                //     delete lumi_down;

                // }

                // histo1D * temp_had = new histo1D();
                // histo1D * temp_mod = new histo1D();

                // temp_had->import(inHisto_nominal, true);
                // temp_mod->import(inHisto_nominal, true);

                // outHisto->addErrorContainer("had_down", *temp_had);
                // outHisto->addErrorContainer("mod_down", *temp_mod);

                // delete temp_had;
                // delete temp_mod;
                
                delete inHisto_nominal ;
                outHistos.push_back(outHisto);

            } //end first loop on type

            inFile_nominal->Close();
            delete inFile_nominal  ;


            /************************ now add systematics ************************/
            
            std::cout<<std::endl;

            for ( TString & systName : systNames ) {
                // if ( ! (systName.Contains("_UP") || systName.Contains("_DOWN")) ) continue;
                
                // bool isShapeVariation = false;
                // for ( TString & shapeSyst: shapeSysts )
                //     if (systName.Contains(shapeSyst)){

                //         isShapeVariation = true;
                //         break;
                //     }
                std::cout<<"channel: "<<channel<<"\t histogram: "<<histName<<"\t syst: "<<systName<<std::endl;


                // if (!isShapeVariation) continue;
                
                TFile * inFile_systvar = new TFile (workDir+"/diffXsPlots/"+systName+"/"+channel+"/"+histName+"_source.root");

                if (inFile_systvar->IsZombie()) {

                    std::cout << "Runtime error: file " << workDir+"/diffXsPlots/"+systName+"/"+channel+"/"+histName+"_source.root"
                              << " not found.!" << std::endl;
                    throw std::runtime_error("Error: file not found");

                }



                for ( TString & typeName : typeNames ) {
                    // if ( !(typeName.Contains("data")) ) continue; // comment this line later!

                    // bool isTTbar = (typeName.Contains("ttsignal")) || (typeName.Contains("ttother"));
                    // if (!isTTbar) continue; // comment this line later!
                    
                    unsigned int i_type = (unsigned int)(&typeName - &typeNames.at(0));
                    // std::cout<<i_type<<"\t";

                    TH1D  * inHisto_systvar = (TH1D*) inFile_systvar->GetObjectChecked("m_"+histName+"_"+typeName,"TH1D");
                    histo1D * temp_systvar = new histo1D ();

                    // if ( histName == "HypLeptonBjetMass" ) {
                    //     inHisto_systvar->Rebin(rebin_mlb);
                    //     // if (channel!="emu") inHisto_systvar->Rebin(2);
                    // }

                    temp_systvar->import(inHisto_systvar, false);

                    TString errName = systName;
                    errName.ToLower();

                    // if (errName.Contains("lumi_")) errName.ReplaceAll("lumi_","lumi_mod_");
                    // if (errName.Contains("mass_")) errName.ReplaceAll("mass_","TOPMASS_");


                    // if (histName.Contains("_diLep")) std::cout<<histName<<"\t"<<typeName<<"\t"<<errName<<std::endl;
                    // CHECK!

                    if ( errName.Contains("_up") || errName.Contains("_down") )
                        outHistos.at(i_type)->addErrorContainer(errName, *temp_systvar);

                    else {

                        if (systName == "MCATNLO") errName = "had";
                        else if (systName == "POWHEG") errName = "mod";

                        errName += "_up";
                        outHistos.at(i_type)->addErrorContainer(errName, *temp_systvar);

                        errName.ReplaceAll("_up","_down");
                        // outHistos.at(i_type)->addErrorContainer(errName, *(outHistos.at(i_type)) + *(outHistos.at(i_type)) - *temp_systvar); // symmetrized error
                        outHistos.at(i_type)->addErrorContainer(errName, *(outHistos.at(i_type))); // not symmetrized

                    }
                    
                    // std::cout<<channel<<": added "<<systName<<" to type "<<typeName<<std::endl;

                    delete inHisto_systvar ;
                    delete temp_systvar;


                } //end second loop on type

                inFile_systvar->Close();
                delete inFile_systvar;
                
            }// end loop on systs

            std::cout<<std::endl<<"end loop on syst variations"<<std::endl<<std::endl;

            for ( TString & typeName : typeNames ) {
                // if ( !(typeName.Contains("data")) ) continue; // comment this line later!

                unsigned int i_type = (unsigned int)(&typeName - &typeNames.at(0));
                string pathOutFile = (string) (workDir+"/ztopFiles/rawHistos/"+channel+"/"+typeName);  

                gSystem->Exec("mkdir -p " + (TString) pathOutFile );

                pathOutFile += (string) ("/"+channel+"_"+typeName+"_"+histName+".ztop");  

                outHistos.at(i_type)->writeToFile( pathOutFile );
                delete outHistos.at(i_type);

            }// end third loop on type

        } // end loop on histograms
    } // end loop on channels


    std::cout<<std::endl;
    // outFile.close();

    return 0;

}



