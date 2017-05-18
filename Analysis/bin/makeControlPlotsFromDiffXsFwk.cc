#include <iostream>
#include <fstream>
#include <iomanip>

#include "TString.h"
#include "TFile.h"
#include "TMath.h"
#include "TROOT.h"
#include "TError.h"
#include "TCanvas.h"
#include "TSystem.h"

#include "TtZAnalysis/Tools/interface/applicationMainMacro.h"
#include "TtZAnalysis/Tools/interface/histo1D.h"
#include "TtZAnalysis/Tools/interface/histoStack.h"
#include "TtZAnalysis/Tools/interface/plotterControlPlot.h"


// void doCosmetics();

invokeApplication(){

    using namespace ztop ;

    gErrorIgnoreLevel = kWarning;

    TString workDir = getenv("FIT_WORKDIR"); 

    std::vector<TString> channel_vec = {"ee","emu","mumu"};
    std::vector<TString> typeName_vec = {"data","ttsignal","ttother","zjets","wjets","singlet","diboson","ttV"};
    std::vector<TString> histName_vec ;

    std::vector<TString> histNameRange_vec ;
    std::vector<float> xMin_vec ;
    std::vector<float> xMax_vec ;

    std::ifstream in_hist (workDir+"/configs/diffXsFwk/distribsFromDiffXsFwk.txt", std::ios::in);
    std::ifstream in_range (workDir+"/configs/diffXsFwk/custom_ranges.txt", std::ios::in);

    for ( TString read ; in_hist >> read ; )
        if ( !in_hist.eof() ) histName_vec.push_back(read);

    while (!in_range.eof()){
        
        TString name;
        float min, max;

        in_range>>name>>min>>max;
        if (in_range.eof()) break;

        histNameRange_vec.push_back(name);
        xMin_vec.push_back(min);
        xMax_vec.push_back(max);

    }

    in_hist.close();
    in_range.close();

    const TString selected_channel = parser->getOpt<TString>("c","","select a specific channel. possibilities: ee, emu, mumu");
    const TString selected_distribution = parser->getOpt<TString>("d","","select a specific distribution. possibilities: whatever is contained in configs/diffXsFwk/distribsFromDiffXsFwk.txt");

    parser->doneParsing();

    if (selected_channel!=""){
        if ( std::find(channel_vec.begin(),channel_vec.end(),selected_channel)==channel_vec.end() ){
            std::cout << "Runtime error: channel " <<selected_channel<< " not allowed!" << std::endl;
            throw std::runtime_error("Error: invalid parameter");
        }
        channel_vec.clear();
        channel_vec.push_back(selected_channel);
    }
    
    if (selected_distribution!=""){
        if ( std::find(histName_vec.begin(),histName_vec.end(),selected_distribution)==histName_vec.end() ){
            std::cout << "Runtime error: distribution " <<selected_distribution<< " not allowed!" << std::endl;
            throw std::runtime_error("Error: invalid parameter");
        }
        histName_vec.clear();
        histName_vec.push_back(selected_distribution);
    }
    
    std::cout<<std::endl;

    for ( TString & channel : channel_vec ) {

        for ( TString & histName : histName_vec ){

            std::cout<<"channel: "<<channel<<"\t distribution: "<<histName<<std::endl;

            float xMin = -99;
            float xMax = -99;

            for (unsigned int iName = 0; iName < histNameRange_vec.size(); ++iName){
                if (histName == histNameRange_vec.at(iName)){
                 
                    xMin = xMin_vec.at(iName);
                    xMax = xMax_vec.at(iName);
                    break;
                }
            }

            histoStack * hStack = new histoStack();
            
            TString xTitle = "";

            for ( TString & typeName : typeName_vec ){

                TString pathToFile = workDir+"/ztopFiles/rawHistos/"+channel+"/"+typeName+"/"+channel+"_"+typeName+"_"+histName+".ztop";
                histo1D inHisto;

                inHisto.readFromFile( (std::string) pathToFile );

                if (typeName == "data") xTitle = inHisto.getXAxisName();

                inHisto.setXAxisName(xTitle);
                inHisto.setYAxisName("");
                    
                if ( xMin != xMax ){
                    if (xMin != 0) inHisto = inHisto.cutLeft(xMin);
                    inHisto = inHisto.cutRight(xMax);
                }

                TString legend = "";
                int color = 0;
                double norm = 1.;                
                int legord = 99999;

                if ( typeName.Contains("data")) {

                    legend = "data";
                    legord = 4;
                    color = 1;
                }

                else if ( typeName.Contains("signal")) {

                    legend = "t#bar{t} sig";
                    legord = 3;
                    color = 2;
                }

                else if ( typeName.Contains("other")) {

                    legend = "t#bar{t} bg";
                    legord = 2;
                    color = 3;
                }


                else {

                    legend = "other bg";
                    legord = 1;
                    color = 4;
                }

                hStack->push_back( inHisto, legend, color, norm, legord );

            } // end loop on types

            TCanvas * aCanvas = new TCanvas (histName) ;
            plotterControlPlot * pCP = new plotterControlPlot();

            pCP->readStyleFromFile( getenv("CMSSW_BASE")+(std::string)"/src/TtZAnalysis/Tools/styles/controlPlots_diffXsFwk.txt" );
            pCP->usePad( aCanvas );
            pCP->setStack( hStack );
            pCP->draw();

            std::string path_plots = (std::string) (workDir+"/ztopFiles/controlPlots/"+channel);

            gSystem->Exec("mkdir -p " + (TString) path_plots );
            path_plots += (std::string) ("/"+channel+"_"+histName);  

            aCanvas->cd();

            TLatex latexLabel1;
            latexLabel1.SetTextSize(0.06);
            latexLabel1.SetNDC();
            latexLabel1.DrawLatex(0.15, 0.95, "CMS");
            TLatex latexLabel3;
            latexLabel3.SetTextSize(0.04);
            latexLabel3.SetTextFont(52);
            latexLabel3.SetNDC();
            latexLabel3.DrawLatex(0.25, 0.95, "Work in progress");

            TLatex latexLabel2;
            latexLabel2.SetTextSize(0.04);
            latexLabel2.SetTextFont(42);
            latexLabel2.SetNDC();
            latexLabel2.DrawLatex(0.58, 0.95, "35.7 fb^{-1}, #sqrt{s} = 13 TeV, 2016");


            aCanvas->Print( (TString) path_plots+".pdf","pdf" );
            aCanvas->Print( (TString) path_plots+".eps","eps" );
            aCanvas->Print( (TString) path_plots+".png","png" );

            TFile * outFile = new TFile ( (TString) path_plots+".root", "RECREATE");
            aCanvas->Write();
            outFile->Close();
            
            delete outFile;
            delete pCP;
            delete aCanvas;
            delete hStack;


        } // end loop on distributions

        std::cout<<std::endl;

    } //end loop on channels


    return 0;

}
