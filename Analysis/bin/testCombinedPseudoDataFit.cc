
#include "TtZAnalysis/Tools/interface/histo1D.h"
#include "TtZAnalysis/Tools/interface/simpleFitter.h"
#include "TtZAnalysis/Tools/interface/applicationMainMacro.h"
#include "TtZAnalysis/Tools/interface/variateHisto1D.h"
#include "TtZAnalysis/Tools/interface/plotterMultiplePlots.h"
#include "TtZAnalysis/Tools/interface/plotterCompare.h"
#include "TtZAnalysis/Tools/interface/texTabler.h"
#include "TMath.h"
#include "TRandom.h"
#include "TSystem.h"
#include <iostream>
#include <iomanip>
#include <cmath>

TString findPath ( const TString typeName, const TString histName, const TString channel );
// ztop::histo1D createPseudoData ( const ztop::histo1D& mc );

invokeApplication(){


    using namespace ztop;
    using namespace std;

    vector<TString> channel_vec = {"emu","ee","mumu"};

    // parser->bepicky=true;
    const bool useData=parser->getOpt<bool>("-data",false,"fit to data. default is pseudo-data");
    const bool useMC=parser->getOpt<bool>("-mc",false,"fit to MC. default is pseudo-data");
    const TString selected_channel = parser->getOpt<TString>("c","","select a specific channel. possibilities: ee, emu, mumu");

    parser->doneParsing();

    if (useData && useMC){
        std::cout<<"ERROR: options --data and --mc are not compatible. Please choose either of the two, or nothing to use pseudo-data"<<std::endl;
        throw std::runtime_error("Error: invalid parameters");
    }

    if (selected_channel!=""){
        if ( std::find(channel_vec.begin(),channel_vec.end(),selected_channel)==channel_vec.end() ){
            std::cout << "Runtime error: channel " <<selected_channel<< " not allowed!" << std::endl;
            throw std::runtime_error("Error: invalid parameter");
        }
        channel_vec.clear();
        channel_vec.push_back(selected_channel);
    }


    // const bool usePseudoData = true;

    double nominal_xsec = 831.76;
    double nominal_mass = 172.50;
    double lumi_unc = 0.025;

    TString workDir = getenv("FIT_WORKDIR"); 

    TString histName = "HypLeptonBjetMass";
    // TString histName = "HypTopMass";


    // vector<TString> channel_vec = {"emu"};

    histo1D data ;
    histo1D ttsignal ;
    histo1D ttother ;
    histo1D singlet ;
    histo1D diboson ;
    histo1D wjets ;
    histo1D zjets ;
    histo1D ttV ;

    for ( TString & channel : channel_vec ) {

        // if (channel == "ee") histName = "HypjetMulti";
        // if (channel == "mumu") histName = "HypBjetMulti_noBTag";

        // if (channel == "ee") histName = "HypLeptonEtaLead";
        // if (channel == "mumu") histName = "HypLeptonEtaLead";

        if (channel=="emu"){

            data.readFromFile( (string) findPath("data",histName, channel) );
            ttsignal.readFromFile( (string) findPath("ttsignal",histName, channel) );
            ttother.readFromFile( (string) findPath("ttother",histName, channel) );
            singlet.readFromFile( (string) findPath("singlet",histName, channel) );
            diboson.readFromFile( (string) findPath("diboson",histName, channel) );
            wjets.readFromFile( (string) findPath("wjets",histName, channel) );
            zjets.readFromFile( (string) findPath("zjets",histName, channel) );
            ttV.readFromFile( (string) findPath("ttV",histName, channel) );

            // data=data.cutLeft(160);
            // ttsignal=ttsignal.cutLeft(160);
            // ttother=ttother.cutLeft(160);
            // singlet=singlet.cutLeft(160);
            // diboson=diboson.cutLeft(160);
            // wjets=wjets.cutLeft(160);
            // zjets=zjets.cutLeft(160);
            // ttV=ttV.cutLeft(160);

            if (histName == "HypLeptonBjetMass"){
                data=data.cutRight(190);
                ttsignal=ttsignal.cutRight(190);
                ttother=ttother.cutRight(190);
                singlet=singlet.cutRight(190);
                diboson=diboson.cutRight(190);
                wjets=wjets.cutRight(190);
                zjets=zjets.cutRight(190);
                ttV=ttV.cutRight(190);

                data=data.rebin(5);
                ttsignal=ttsignal.rebin(5);
                ttother=ttother.rebin(5);
                singlet=singlet.rebin(5);
                diboson=diboson.rebin(5);
                wjets=wjets.rebin(5);
                zjets=zjets.rebin(5);
                ttV=ttV.rebin(5);
            }
        }

        else{

            histo1D data_temp ;
            histo1D ttsignal_temp ;
            histo1D ttother_temp ;
            histo1D singlet_temp ;
            histo1D diboson_temp ;
            histo1D wjets_temp ;
            histo1D zjets_temp ;
            histo1D ttV_temp ;

            data_temp.readFromFile( (string) findPath("data",histName, channel) );
            ttsignal_temp.readFromFile( (string) findPath("ttsignal",histName, channel) );
            ttother_temp.readFromFile( (string) findPath("ttother",histName, channel) );
            singlet_temp.readFromFile( (string) findPath("singlet",histName, channel) );
            diboson_temp.readFromFile( (string) findPath("diboson",histName, channel) );
            wjets_temp.readFromFile( (string) findPath("wjets",histName, channel) );
            zjets_temp.readFromFile( (string) findPath("zjets",histName, channel) );
            ttV_temp.readFromFile( (string) findPath("ttV",histName, channel) );

            // data_temp=data_temp.cutLeft(160);
            // ttsignal_temp=ttsignal_temp.cutLeft(160);
            // ttother_temp=ttother_temp.cutLeft(160);
            // singlet_temp=singlet_temp.cutLeft(160);
            // diboson_temp=diboson_temp.cutLeft(160);
            // wjets_temp=wjets_temp.cutLeft(160);
            // zjets_temp=zjets_temp.cutLeft(160);
            // ttV_temp=ttV_temp.cutLeft(160);

            if (histName == "HypLeptonBjetMass"){
                data_temp=data_temp.cutRight(190);
                ttsignal_temp=ttsignal_temp.cutRight(190);
                ttother_temp=ttother_temp.cutRight(190);
                singlet_temp=singlet_temp.cutRight(190);
                diboson_temp=diboson_temp.cutRight(190);
                wjets_temp=wjets_temp.cutRight(190);
                zjets_temp=zjets_temp.cutRight(190);
                ttV_temp=ttV_temp.cutRight(190);

                data_temp=data_temp.rebin(10);
                ttsignal_temp=ttsignal_temp.rebin(10);
                ttother_temp=ttother_temp.rebin(10);
                singlet_temp=singlet_temp.rebin(10);
                diboson_temp=diboson_temp.rebin(10);
                wjets_temp=wjets_temp.rebin(10);
                zjets_temp=zjets_temp.rebin(10);
                ttV_temp=ttV_temp.rebin(10);
            }

            else if (histName == "HypLeptonEtaLead"){

                data_temp=data_temp.cutLeft(-2.6);
                ttsignal_temp=ttsignal_temp.cutLeft(-2.6);
                ttother_temp=ttother_temp.cutLeft(-2.6);
                singlet_temp=singlet_temp.cutLeft(-2.6);
                diboson_temp=diboson_temp.cutLeft(-2.6);
                wjets_temp=wjets_temp.cutLeft(-2.6);
                zjets_temp=zjets_temp.cutLeft(-2.6);
                ttV_temp=ttV_temp.cutLeft(-2.6);

                data_temp=data_temp.cutRight(2.6);
                ttsignal_temp=ttsignal_temp.cutRight(2.6);
                ttother_temp=ttother_temp.cutRight(2.6);
                singlet_temp=singlet_temp.cutRight(2.6);
                diboson_temp=diboson_temp.cutRight(2.6);
                wjets_temp=wjets_temp.cutRight(2.6);
                zjets_temp=zjets_temp.cutRight(2.6);
                ttV_temp=ttV_temp.cutRight(2.6);

                if (channel=="ee"){
                    data_temp=data_temp.rebin(2);
                    ttsignal_temp=ttsignal_temp.rebin(2);
                    ttother_temp=ttother_temp.rebin(2);
                    singlet_temp=singlet_temp.rebin(2);
                    diboson_temp=diboson_temp.rebin(2);
                    wjets_temp=wjets_temp.rebin(2);
                    zjets_temp=zjets_temp.rebin(2);
                    ttV_temp=ttV_temp.rebin(2);
                }
            }


            data=data.append(data_temp);
            ttsignal=ttsignal.append(ttsignal_temp);
            ttother=ttother.append(ttother_temp);
            singlet=singlet.append(singlet_temp);
            diboson=diboson.append(diboson_temp);
            wjets=wjets.append(wjets_temp);
            zjets=zjets.append(zjets_temp);
            ttV=ttV.append(ttV_temp);
        }

    }


    histo1D signal = ttsignal + ttother ;
    histo1D bgsum = singlet + diboson + zjets + wjets + ttV ;

    signal.addErrorContainer("signalxsec_up",signal*1.5);
    signal.addErrorContainer("signalxsec_down",signal*0.5);

    bgsum.addErrorContainer("bgnorm_up",bgsum*1.3);
    bgsum.addErrorContainer("bgnorm_down",bgsum*0.7);


    histo1D mcsum = signal + bgsum;

    mcsum.addErrorContainer("lumi_up",mcsum*(1.+lumi_unc));
    mcsum.addErrorContainer("lumi_down",mcsum*(1.-lumi_unc));

    // mcsum=mcsum.rebin(5);
    // data=data.rebin(5);

    variateHisto1D hv;
    hv.import(mcsum);

    // hv.setPrior("mass",  Prior::Gauss, 3.);
    // hv.setPrior("mass",  Prior::Float);
    // hv.setPrior("had",   Prior::BoxAuto  );
    // hv.setPrior("mod",   Prior::BoxAuto  );
    // hv.setPrior("scale", Prior::Box  );
    // hv.setPrior("pdf_*", Prior::Gauss, TMath::ErfInverse(.9), true );

    //these are OK
    hv.setPrior("*scale", Prior::Box, 1. , true  );
    hv.setPrior("pdf_*", Prior::Gauss, TMath::ErfInverse(.9), true );
    hv.setPrior("psfsrscale", Prior::Box, sqrt(2.));
    hv.setPrior("pdf_alphas", Prior::Fixed);
    hv.setPrior("top_pt", Prior::Fixed);
    hv.setPrior("mass",  Prior::Float);


    // hv.setPrior("*scale", Prior::Box, 1. , true  );
    // hv.setPrior("pdf_*", Prior::Fixed,1., true );
    // // hv.setPrior("btag*", Prior::Fixed ,1., true);
    // // hv.setPrior("jes*", Prior::Fixed ,1., true);
    // hv.setPrior("psfsrscale", Prior::Box, sqrt(2.));
    // hv.setPrior("pdf_alphas", Prior::Fixed);
    // hv.setPrior("top_pt", Prior::Fixed);
    // hv.setPrior("mass",  Prior::Float);




    // hv.setPrior("mass",  Prior::Gauss, 3);

    hv.setPrior("signalxsec", Prior::Float );





    hv.printPriors();


    vector<TString> list = mcsum.getSystNameList();
    cout<<endl;

    // here create pseudodata
    // histo1D pseudodata = createPseudoData(mcsum);

    TRandom3 tr;
    histo1D histotofit;
    if (useData) histotofit = data;
    else if (useMC) histotofit = mcsum ;
    else histotofit = mcsum.createPseudoExperiment(&tr);

    // histo1D pseudodata = mcsum.createPseudoExperiment(&tr);
    

    // simpleFitter result = hv.fitToConstHisto(data,true);
    // simpleFitter result = hv.fitToConstHisto(mcsum,true, true);

    // simpleFitter result = hv.fitToConstHisto(pseudodata,true,true);

    simpleFitter result;
    if (useData) result = hv.fitToConstHisto(histotofit,true);
    else result = hv.fitToConstHisto(histotofit,true,true);

    std::cout<<std::endl;
    if (result.wasSuccess()) std::cout<<"Fit succesful!"<<std::endl;
    // if (result.minosWasSuccess()) std::cout<<"Fit succesful!"<<std::endl;
    else std::cout<<"WARNING: fit failed"<<std::endl;
    std::cout<<std::endl;


    double pull_xsec = 0;
    double err_xsec = 0;

    double pull_mass = 0;
    double err_mass = 0;

    bool read_xsec = false;
    bool read_mass = false;

    ofstream outFit ("outdir/fitResults_combined.tex");

    for(size_t i=0;i<result.getParameters()->size();i++){
        std::cout<< i<< " " << list.at(i) << " " << result.getParameter(i) << "  " << result.getParameterErr(i) << std::endl;
        outFit<<list.at(i)<< " & " << result.getParameter(i) << " & " << result.getParameterErr(i)<< "\\\\" << endl;

        if (list.at(i) == "signalxsec"){
            pull_xsec = result.getParameter(i);
            err_xsec = result.getParameterErr(i);
            read_xsec = true;
        }

        if (list.at(i) == "mass"){
            pull_mass = result.getParameter(i);
            err_mass = result.getParameterErr(i);
            read_mass = true;
        }
        

    }

    outFit.close();

    if (!read_xsec || !read_mass){
        cout<<"ERROR"<<endl;
        return 0;
    }



    cout<<fixed<<setprecision(1);



    cout<<endl;
    cout<<"******************************************"<<endl;
    cout<<endl;
    cout<<"fit: tot xsec = "<<nominal_xsec*(1+pull_xsec/2.)<<" +/- "<<err_xsec*nominal_xsec/2.<<" pb  ("<<(err_xsec*nominal_xsec/2.)/(nominal_xsec*(1+pull_xsec/2.))*100<<" %)"<<endl;
    cout<<"fit:  MC mass = "<<nominal_mass + pull_mass*3.<<" +/- "<<err_mass*3.<<" GeV  ("<<(err_mass*3.)/(nominal_mass + pull_mass*3.)*100<<" %)"<<endl;
    cout<<endl;
    cout<<"******************************************"<<endl;
    cout<<endl;

    // cout<<endl;
    // cout<<"**********************************"<<endl;
    // cout<<endl;
    // cout<<"fit: tot xsec = "<<nominal_xsec*(1+pull_xsec/2.)<<" +/- "<<err_xsec*nominal_xsec/2.<<" pb"<<endl;
    // cout<<"fit:  MC mass = "<<nominal_mass + pull_mass*3.<<" +/- "<<err_mass*3.<<" GeV"<<endl;
    // cout<<endl;
    // cout<<"**********************************"<<endl;
    // cout<<endl;


    ofstream out_xSec ("outdir/xSecMassResults_combined.tex");
    out_xSec<<fixed<<setprecision(1);
    out_xSec<<"$\\sigma_{tot} = "<<nominal_xsec*(1+pull_xsec/2.)<<" \\pm "<<err_xsec*nominal_xsec/2.<<" ~\\mathrm{pb} $"<<endl;
    out_xSec<<"$m_{t}^{\\mathrm{MC}} = "<<nominal_mass + pull_mass*3.<<" \\pm "<<err_mass*3.<<" ~\\mathrm{GeV}$"<<endl;



    histo1D postfit = hv.exportContainer( *result.getParameters() );

    // postfit = postfit.cutRight(180.);
    // data = data.cutRight(180.);
    // pseudodata = pseudodata.cutRight(180.);

    // postfit = postfit.cutLeft(10.);
    // data = data.cutLeft(10.);

    // data_mlb.setName("data");
    // postfit_mlb.setName("MC");
    
    // plotterCompare pl_mlb;
    // pl_mlb.setComparePlot(& postfit_mlb, 0);
    // pl_mlb.setNominalPlot(& data_mlb);

    // gSystem->Exec("mkdir -p " + workDir + "/output");
    // pl_mlb.printToPdf("output/test_combined_mlb");


    data.setName("data");
    // pseudodata.setName("pseudo-experiment");

    if (useData) histotofit.setName("data");
    else if (useMC) histotofit.setName("simulation");
    else histotofit.setName("pseudo-experiment");

    postfit.setName("MC postfit");
    
    plotterCompare pl;
    pl.setComparePlot(& postfit, 0);
    pl.setNominalPlot(& histotofit);
    // pl.setNominalPlot(& pseudodata);
    // pl.setNominalPlot(& data);

    std::cout<<std::endl;
    if (result.wasSuccess()) std::cout<<"Fit succesful!"<<std::endl;
    // if (result.minosWasSuccess()) std::cout<<"Fit succesful!"<<std::endl;
    else std::cout<<"WARNING: fit failed"<<std::endl;
    std::cout<<std::endl;


    gSystem->Exec("mkdir -p " + workDir + "/output");
    pl.printToPdf("output/test_combined");

    texTabler corr = result.makeCorrTable();
    corr.writeToFile("output/tabCorr_combined.tex");
    corr.writeToPdfFile("output/tabCorr_combined.pdf");


    return 0;

}





TString findPath ( const TString typeName, const TString histName, const TString channel ){


    // if (channel == "ee") histName = "HypBjetMulti_noBTag";
    // else if (channel == "mumu") histName = "HypTTBarMass";

    TString workDir = getenv("FIT_WORKDIR"); 
    TString path = workDir+"/ztopFiles/rawHistos/"+channel+"/"+typeName+"/"+channel+"_"+typeName+"_"+histName+".ztop";

    // std::cout<<path<<std::endl;

    return path;
}

// ztop::histo1D createPseudoData ( const ztop::histo1D& mc ){

//     // std::vector<float> bins = mc.getBins();
//     // ztop::histo1D h_pd; h_pd.setBins(bins);

//     // TRandom3 tr;
    
//     // for (size_t ibin=1; ibin<=mc.getNBins(); ++ibin){
//     //     h_pd.setBinContent(ibin, tr.Poisson(mc.getBinContent(ibin)));
//     //     h_pd.setBinStat(ibin,sqrt(h_pd.getBinContent(ibin)));
//     // }

//     // ztop::histo1D h_pd = mc ;
//     // h_pd.removeAllSystematics();

//     // TRandom3 tr;
    
//     // for (size_t ibin=1; ibin<=mc.getNBins(); ++ibin){
//     //     h_pd.setBinContent(ibin, tr.Poisson(mc.getBinContent(ibin)));
//     //     h_pd.setBinStat(ibin,sqrt(h_pd.getBinContent(ibin)));
//     // }



//     return h_pd;

// }

