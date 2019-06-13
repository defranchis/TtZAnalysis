/*
 * analysisPlotsJan.cc
 *
 *  Created on: Feb 12, 2014
 *      Author: kiesej
 */


#include "../interface/analysisPlotsKinReco.h"
#include "../interface/AnalysisUtils.h"
#include "TopAnalysis/ZTopUtils/interface/miscUtils.h"
#include "TtZAnalysis/DataFormats/src/classes.h"
#include "TtZAnalysis/DataFormats/interface/helpers.h"

namespace ztop{

    void analysisPlotsKinReco::setJetCategory(size_t nbjets,size_t njets){
	if(nbjets < 1 || nbjets>2){
            bjetcategory=cat_0bjet;
            if(njets<1)
                jetcategory=cat_0bjet0jet;
            else if(njets<2)
                jetcategory=cat_0bjet1jet;
            else if(njets<3)
                jetcategory=cat_0bjet2jet;
            else if(njets>2)
                jetcategory=cat_0bjet3jet;
	}
	else if(nbjets < 2){
            bjetcategory=cat_1bjet;
            if(njets<1)
                jetcategory=cat_1bjet0jet;
            else if(njets<2)
                jetcategory=cat_1bjet1jet;
            else if(njets<3)
                jetcategory=cat_1bjet2jet;
            else if(njets>2)
                jetcategory=cat_1bjet3jet;
	}
	else if(nbjets < 3){
            bjetcategory=cat_2bjet;
            if(njets<1)
                jetcategory=cat_2bjet0jet;
            else if(njets<2)
                jetcategory=cat_2bjet1jet;
            else if(njets<3)
                jetcategory=cat_2bjet2jet;
            else if(njets>2)
                jetcategory=cat_2bjet3jet;
	}
    }

    void analysisPlotsKinReco::setBJetCategory(size_t nbjets){
	if(nbjets < 1 || nbjets>2) bjetcategory=cat_0bjet;
	else if(nbjets < 2)        bjetcategory=cat_1bjet;
	else if(nbjets < 3)        bjetcategory=cat_2bjet;
    }

    void analysisPlotsKinReco::setGenMttCategory(float gen_mtt){
        if ((int)cat_mttmax_reco != (int)cat_mttmax_gen)
            throw std::logic_error("analysisPlotsKinReco::setMttCategory: gen and reco mtt bins must be the same");

        if (gen_mtt < bin_1_) gen_mttcategory = cat_mtt1_gen;
        else if (gen_mtt < bin_2_) gen_mttcategory = cat_mtt2_gen;
        else gen_mttcategory = cat_mtt3_gen;
    }

    void analysisPlotsKinReco::setRecoMttCategory(float reco_mtt){
        if (event()->selectedjets->size()<2) reco_mttcategory = cat_mttmax_reco;
        else if (reco_mtt < bin_1_) reco_mttcategory = cat_mtt1_reco;
        else if (reco_mtt < bin_2_) reco_mttcategory = cat_mtt2_reco;
        else if (reco_mtt < bin_3_) reco_mttcategory = cat_mtt3_reco;
        else reco_mttcategory = cat_mtt4_reco;
    }

    void analysisPlotsKinReco::bookPlots(){
	if(!use()) return;
	using namespace std;
	using namespace ztop;

	vector<float> veryfinebins=histo1D::createBinning(10,20,160);
	vector<float> finebins=histo1D::createBinning(7,20,160);
	vector<float> coarsebins=histo1D::createBinning(5,20,160);
	vector<float> verycoarsebins=histo1D::createBinning(3,20,160);

        vector<float> eta_bins=histo1D::createBinning(60,-3.,3.);        
        vector<float> mtt_bins, pt_bins;
        vector<float> mtt_bins_fine;

	std::vector<float> nobins=histo1D::createBinning(1,0,1);
	std::vector<float> bins=histo1D::createBinning(1,0,1); bins.clear();
        bins << 30 << 35 << 40<< 45 << 50 << 55 << 60 << 70 << 80 << 90 << 100 << 120 << 140 << 160 << 200;
	std::vector<float> ptbins=histo1D::createBinning(100,0,1000);

	for(float i=0;i<=700;i+=20){
            pt_bins<<i;	}	
	for(float i=300;i<=2000;i+=50){
            mtt_bins<<i; }
        pt_bins<<2000;
        mtt_bins<<5000;

        mtt_bins_fine << 300 << 420 << 550 << 810 << 2000 ;

        mtt=addPlot(mtt_bins,mtt_bins,"m_tt kin reco","m_{t#bar{t}} [GeV]", "Events/GeV");
        mtt_coarse=addPlot(mtt_bins_fine,mtt_bins_fine,"m_tt kin reco coarse","m_{t#bar{t}} [GeV]", "Events/GeV");
        pt_top=addPlot(pt_bins,pt_bins,"top pt kin reco","top p_{T} [GeV]", "Events/GeV");
        eta_top=addPlot(eta_bins,eta_bins,"top eta kin reco","top #eta", "Events");
        pt_antitop=addPlot(pt_bins,pt_bins,"antitop pt kin reco","antitop p_{T} [GeV]", "Events/GeV");
        eta_antitop=addPlot(eta_bins,eta_bins,"antitop eta kin reco","antitop #eta", "Events");
        pt_ttbar=addPlot(pt_bins,pt_bins,"ttbar pt kin reco","t#bar{t} p_{T} [GeV]", "Events/GeV");
        eta_ttbar=addPlot(eta_bins,eta_bins,"ttbar eta kin reco","t#bar{t} #eta", "Events");

	for(size_t nbjet=0;nbjet<cat_bjetmax;nbjet++){
            m_mub_3_.at(nbjet)=addPlot(verycoarsebins,verycoarsebins,"m_mub very coarse "+toTString(nbjet)+" b-jets","m_{#mub} [GeV]", "Events/GeV");
            m_mub_5_.at(nbjet)=addPlot(coarsebins,coarsebins,"m_mub coarse "+toTString(nbjet)+" b-jets","m_{#mub} [GeV]", "Events/GeV");
            m_mub_7_.at(nbjet)=addPlot(finebins,finebins,"m_mub fine "+toTString(nbjet)+" b-jets","m_{#mub} [GeV]", "Events/GeV");
            m_mub_10_.at(nbjet)=addPlot(veryfinebins,veryfinebins,"m_mub very fine "+toTString(nbjet)+" b-jets","m_{#mub} [GeV]", "Events/GeV");
            last_jet_pt_.at(nbjet)=addPlot(bins,bins,"last jet pt "+toTString(nbjet)+" b-jets","p_{T} [GeV]","Events/GeV");

            mtt_bjet_.at(nbjet)=addPlot(mtt_bins,mtt_bins,"m_tt kin reco "+toString(nbjet)+" b-jets","m_{t#bar{t}} [GeV]", "Events/GeV");
            pt_top_bjet_.at(nbjet)=addPlot(pt_bins,pt_bins,"top pt kin reco "+toString(nbjet)+" b-jets","top p_{T} [GeV]", "Events/GeV");
            eta_top_bjet_.at(nbjet)=addPlot(eta_bins,eta_bins,"top eta kin reco "+toString(nbjet)+" b-jets","top #eta", "Events");
            pt_antitop_bjet_.at(nbjet)=addPlot(pt_bins,pt_bins,"antitop pt kin reco "+toString(nbjet)+" b-jets","antitop p_{T} [GeV]", "Events/GeV");
            eta_antitop_bjet_.at(nbjet)=addPlot(eta_bins,eta_bins,"antitop eta kin reco "+toString(nbjet)+" b-jets","antitop #eta", "Events");
            pt_ttbar_bjet_.at(nbjet)=addPlot(pt_bins,pt_bins,"ttbar pt kin reco "+toString(nbjet)+" b-jets","t#bar{t} p_{T} [GeV]", "Events/GeV");
            eta_ttbar_bjet_.at(nbjet)=addPlot(eta_bins,eta_bins,"ttbar eta kin reco "+toString(nbjet)+" b-jets","t#bar{t} #eta", "Events");

            for(size_t mtt_cat=0; mtt_cat<cat_mttmax_reco; mtt_cat++){
                total_mtt_bjet_.at(mtt_cat).at(nbjet)=addPlot(nobins,nobins,"total "+toTString(nbjet)+" b-jets mtt"+toTString((int)mtt_cat+1),"", "Event yield");
                last_jet_pt_mtt_bjet_.at(mtt_cat).at(nbjet)=addPlot(bins,bins,"last jet pt "+toTString(nbjet)+" b-jets mtt"+toTString((int)mtt_cat+1),"p_{T} [GeV]", "Events/GeV");
                m_mub_3_mtt_bjet_.at(mtt_cat).at(nbjet)=addPlot(verycoarsebins,verycoarsebins,"m_mub very coarse "+toTString(nbjet)+" b-jets mtt"+toTString((int)mtt_cat+1),"m_{#mub} [GeV]","Events/GeV");
                m_mub_5_mtt_bjet_.at(mtt_cat).at(nbjet)=addPlot(coarsebins,coarsebins,"m_mub coarse "+toTString(nbjet)+" b-jets mtt"+toTString((int)mtt_cat+1),"m_{#mub} [GeV]", "Events/GeV");
                m_lb_5_mtt_bjet_.at(mtt_cat).at(nbjet)=addPlot(coarsebins,coarsebins,"m_lb min coarse "+toTString(nbjet)+" b-jets mtt"+toTString((int)mtt_cat+1),"m_{lb}^{min} [GeV]", "Events/GeV");
                m_lb_3_mtt_bjet_.at(mtt_cat).at(nbjet)=addPlot(verycoarsebins,verycoarsebins,"m_lb min very coarse "+toTString(nbjet)+" b-jets mtt"+toTString((int)mtt_cat+1),"m_{lb}^{min} [GeV]", "Events/GeV");            
                top_pt_mtt_bjet_.at(mtt_cat).at(nbjet)=addPlot(ptbins,ptbins,"top pt "+toTString(nbjet)+" b-jets mtt"+toTString((int)mtt_cat+1),"p_{T} [GeV]", "Events/GeV");
                antitop_pt_mtt_bjet_.at(mtt_cat).at(nbjet)=addPlot(ptbins,ptbins,"antitop pt "+toTString(nbjet)+" b-jets mtt"+toTString((int)mtt_cat+1),"p_{T} [GeV]", "Events/GeV");
                ttbar_pt_mtt_bjet_.at(mtt_cat).at(nbjet)=addPlot(ptbins,ptbins,"ttbar pt "+toTString(nbjet)+" b-jets mtt"+toTString((int)mtt_cat+1),"p_{T} [GeV]", "Events/GeV");
            }
            if (nbjet < 2) total_mtt_bjet_.at(cat_mttmax_reco).at(nbjet)=addPlot(nobins,nobins,"total "+toTString(nbjet)+" b-jets mtt0","", "Event yield");
        }

    }


    void analysisPlotsKinReco::fillPlotsGen(){
	if(!use()) return;
	if(!event()) return;
	using namespace std;
	using namespace ztop;
        
	std::vector<NTGenParticle*> genvisleptons1=produceCollection(event()->genleptons1,20,2.4);
	std::vector<NTGenJet*> genvisjets=produceCollection(event()->genjets,30,2.4);

	if(!requireNumber(2,genvisleptons1)) return;
        if(useKinRecoPS() && !requireNumber(2,genvisjets)) return;
        if(genvisleptons1.at(0)->pt()<25 && genvisleptons1.at(1)->pt()<25) return;

        for(size_t i=0;i<cat_bjetmax;i++){
            m_mub_3_.at(i)->fillGen(20.5,puweight());
            m_mub_5_.at(i)->fillGen(20.5,puweight());
            m_mub_7_.at(i)->fillGen(20.5,puweight());
            m_mub_10_.at(i)->fillGen(20.5,puweight());
            last_jet_pt_.at(i)->fillGen(30.5,puweight());

            mtt_bjet_.at(i)->fillGen(400.,puweight());
            pt_top_bjet_.at(i)->fillGen(100.,puweight());
            eta_top_bjet_.at(i)->fillGen(0.,puweight());
            pt_antitop_bjet_.at(i)->fillGen(100.,puweight());
            eta_antitop_bjet_.at(i)->fillGen(0.,puweight());
            pt_ttbar_bjet_.at(i)->fillGen(100.,puweight());
            eta_ttbar_bjet_.at(i)->fillGen(0.,puweight());

            for(size_t j=0;j<cat_mttmax_reco;j++){
                total_mtt_bjet_.at(j).at(i)->fillGen(.5,puweight());
                last_jet_pt_mtt_bjet_.at(j).at(i)->fillGen(30.5,puweight());
                m_lb_3_mtt_bjet_.at(j).at(i)->fillGen(20.5,puweight());
                m_lb_5_mtt_bjet_.at(j).at(i)->fillGen(20.5,puweight());
                m_mub_3_mtt_bjet_.at(j).at(i)->fillGen(20.5,puweight());
                m_mub_5_mtt_bjet_.at(j).at(i)->fillGen(20.5,puweight());
                top_pt_mtt_bjet_.at(j).at(i)->fillGen(0.5,puweight());
                antitop_pt_mtt_bjet_.at(j).at(i)->fillGen(0.5,puweight());
                ttbar_pt_mtt_bjet_.at(j).at(i)->fillGen(0.5,puweight());
            }
            if (i<2) total_mtt_bjet_.at(cat_mttmax_reco).at(i)->fillGen(0.5,puweight());

        }

        if (!event()->gentops) return;
        if (event()->gentops->size()<2) return;

        mtt->fillGen((event()->gentops->at(0)->p4()+event()->gentops->at(1)->p4()).m(),puweight());
        mtt_coarse->fillGen((event()->gentops->at(0)->p4()+event()->gentops->at(1)->p4()).m(),puweight());
        pt_ttbar->fillGen((event()->gentops->at(0)->p4()+event()->gentops->at(1)->p4()).pt(),puweight());
        eta_ttbar->fillGen((event()->gentops->at(0)->p4()+event()->gentops->at(1)->p4()).eta(),puweight());

        int itop = 0; int iantitop = 1;
        if(event()->gentops->at(0)->pdgId()<0){
            itop=1; iantitop=1; }

        pt_top->fillGen(event()->gentops->at(itop)->pt(),puweight());
        eta_top->fillGen(event()->gentops->at(itop)->eta(),puweight());
        pt_antitop->fillGen(event()->gentops->at(iantitop)->pt(),puweight());
        eta_antitop->fillGen(event()->gentops->at(iantitop)->eta(),puweight());
 
    }

    void analysisPlotsKinReco::fillPlotsReco(){
	if(!use()) return;
	if(!event()) return;
	using namespace std;
	using namespace ztop;

	size_t nbjets=0;
	if(event()->selectedbjets){
		nbjets=event()->selectedbjets->size();
		setBJetCategory(nbjets);
	}
        if(event()->selectedjets->size()<2){
            total_mtt_bjet_.at(cat_mttmax_reco).at(bjetcategory)->fillReco(.5,puweight());
            return;
        } 

        setRecoMttCategory(*event()->mtt);

        mtt->fillReco(*event()->mtt,puweight());
        mtt_coarse->fillReco(*event()->mtt,puweight());
        pt_ttbar->fillReco(*event()->pt_ttbar,puweight());
        eta_ttbar->fillReco(*event()->eta_ttbar,puweight());
        pt_top->fillReco(*event()->pt_top,puweight());
        eta_top->fillReco(*event()->eta_top,puweight());
        pt_antitop->fillReco(*event()->pt_antitop,puweight());
        eta_antitop->fillReco(*event()->eta_antitop,puweight());

        m_mub_3_.at(bjetcategory)->fillReco(*event()->m_mub,puweight());
        m_mub_5_.at(bjetcategory)->fillReco(*event()->m_mub,puweight());
        m_mub_7_.at(bjetcategory)->fillReco(*event()->m_mub,puweight());
        m_mub_10_.at(bjetcategory)->fillReco(*event()->m_mub,puweight());

        mtt_bjet_.at(bjetcategory)->fillReco(*event()->mtt,puweight());
        pt_top_bjet_.at(bjetcategory)->fillReco(*event()->pt_top,puweight());
        eta_top_bjet_.at(bjetcategory)->fillReco(*event()->eta_top,puweight());
        pt_antitop_bjet_.at(bjetcategory)->fillReco(*event()->pt_antitop,puweight());
        eta_antitop_bjet_.at(bjetcategory)->fillReco(*event()->eta_antitop,puweight());
        pt_ttbar_bjet_.at(bjetcategory)->fillReco(*event()->pt_ttbar,puweight());
        eta_ttbar_bjet_.at(bjetcategory)->fillReco(*event()->eta_ttbar,puweight());


        if (event()->selectedjets->size()>0){
            float last_jet_pt = event()->selectedjets->at(event()->selectedjets->size()-1)->pt();
            last_jet_pt_.at(bjetcategory)->fillReco( last_jet_pt , puweight());
            last_jet_pt_mtt_bjet_.at(reco_mttcategory).at(bjetcategory)->fillReco( last_jet_pt , puweight());
        }


        top_pt_mtt_bjet_.at(reco_mttcategory).at(bjetcategory)->fillReco( *event()->pt_top , puweight());
        antitop_pt_mtt_bjet_.at(reco_mttcategory).at(bjetcategory)->fillReco( *event()->pt_antitop , puweight());
        ttbar_pt_mtt_bjet_.at(reco_mttcategory).at(bjetcategory)->fillReco( *event()->pt_ttbar , puweight());

        total_mtt_bjet_.at(reco_mttcategory).at(bjetcategory)->fillReco(.5,puweight());
        m_mub_3_mtt_bjet_.at(reco_mttcategory).at(bjetcategory)->fillReco( *event()->m_mub , puweight());                
        m_mub_5_mtt_bjet_.at(reco_mttcategory).at(bjetcategory)->fillReco( *event()->m_mub , puweight());                

        if(event()->leadinglep && event()->secleadinglep && event()->selectedbjets ){
            if(event()->selectedbjets->size()>0 ){
                NTLorentzVector<float> bjetp4=event()->selectedbjets->at(0)->p4();
                NTLorentzVector<float> llepp4=event()->leadinglep->p4();
                NTLorentzVector<float> slepp4=event()->secleadinglep->p4();

                float fMlb=(bjetp4+llepp4).m();
                float fm2lb=(bjetp4+slepp4).m();
                float fmlbmin=fMlb;
                if(fMlb>fm2lb) fmlbmin=fm2lb;

                m_lb_3_mtt_bjet_.at(reco_mttcategory).at(bjetcategory)->fillReco( fmlbmin , puweight());                
                m_lb_5_mtt_bjet_.at(reco_mttcategory).at(bjetcategory)->fillReco( fmlbmin , puweight());                

            }
        }

    }

}//ns
