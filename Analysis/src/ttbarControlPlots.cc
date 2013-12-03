/*
 * ttbarControlPlots.cc
 *
 *  Created on: Nov 1, 2013
 *      Author: kiesej
 */

#include "../interface/ttbarControlPlots.h"
#include "../interface/NTFullEvent.h"
#include "TtZAnalysis/DataFormats/interface/mathdefs.h"

#include "TtZAnalysis/DataFormats/src/classes.h"

namespace ztop{
/**
 * setbins, addplot, FILL
 *
 * NEVER restrict the addPlot statement by an (if) statement
 * 	-> SegFaults if plots are filled/init before variable is available.
 * Otherwise safe
 *
 */
void ttbarControlPlots::makeControlPlots(const size_t & step){



	initStep(step);
	using namespace std;

	bool middphi=false;
	bool middphiInfo=false;
	if(event()->midphi){
		middphiInfo=true;
		middphi=*(event()->midphi);
	}

	///LEPTONS

	SETBINS << -2.5 << -2.1 << -1.47 << -0.8 << 0.8 << 1.47 << 2.1 << 2.5;
	addPlot("electron eta", "#eta_{l}","N_{e}");
	FILLFOREACH(isoelectrons,eta());


	SETBINSRANGE(50,0,1);
	addPlot("electron isolation", "Iso", "N_{e}");
	FILLFOREACH(idelectrons,rhoIso());

	SETBINS << 0 << 10 << 20 << 25 << 30 << 35 << 40 << 45 << 50 << 60 << 70 << 100 << 200;
	addPlot("electron pt", "p_{T} [GeV]", "N_{e}");
	FILLFOREACH(isoelectrons,pt());

	SETBINSRANGE(50,0,1);
	addPlot("electron mva id", "Id", "N_{e}");
	FILLFOREACH(kinelectrons,mvaId());


	SETBINSRANGE(10,0,10);
	addPlot("leadlep-secleadlep dR", "dR", "N_{e}*N_{#mu}/bw",true);
	FILLSINGLE(leplepdr);

	SETBINS << -2.4 << -2.1 << -1.47 << -0.8 << 0.8 << 1.47 << 2.1 << 2.4;
	addPlot("muon eta", "#eta_{l}","N_{#mu}");
	FILLFOREACH(isomuons,eta());

	SETBINS << 0 << 10 << 20 << 25 << 30 << 35 << 40 << 45 << 50 << 60 << 70 << 100 << 200;
	addPlot( "muon pt", "p_{T} [GeV]", "N_{#mu}");
	FILLFOREACH(isomuons,pt());

	SETBINSRANGE(50,0,1);
	addPlot("muon isolation", "Iso", "N_{#mu}");
	FILLFOREACH(idmuons,isoVal());

	SETBINS << 0 << 10 << 20 << 25 << 30 << 35 << 40 << 45 << 50 << 60 << 70 << 100 << 200;
	addPlot("leadOppoQLep pt", "p_{T} [GeV]", "N_{l}");
	FILL(leadinglep,pt());

	addPlot("secLeadOppoQLep pt", "p_{T} [GeV]", "N_{l}");
	FILL(secleadinglep,pt());

	addPlot("leadAllLep pt", "p_{T} [GeV]", "N_{l}");
	if(event()->allleptons && event()->allleptons->size()>0)
		FILL(allleptons->at(0),pt());
	addPlot("secLeadAllLep pt", "p_{T} [GeV]", "N_{l}");
	if(event()->allleptons && event()->allleptons->size()>1)
		FILL(allleptons->at(1),pt());

	SETBINSRANGE(30,0,0.01);
	addPlot("lept d0V", "d0V [cm]", "N_{l}");
	FILLFOREACH(allleptons,d0V());


	////JETS & MET

	SETBINS << 0 << 10 << 20 << 25 << 30 << 35 << 40 << 45 << 50 << 60 << 70 << 100 << 200;
	addPlot("leading id jet pt","p_{T} [GeV]", "N_{j}/bw");
	if(event()->idjets && event()->idjets->size()>0)
		FILL(idjets->at(0),pt());
	addPlot("secleading id jet pt","p_{T} [GeV]", "N_{j}/bw");
	if(event()->idjets && event()->idjets->size()>1)
		FILL(idjets->at(1),pt());

	SETBINSRANGE(10,-0.5,9.5);
	addPlot("hard jet multi", "N_{jet}","N_{evt}");
	FILL(hardjets,size());
	addPlot("med jet multi", "N_{jet}", "N_{evt}");
	FILL(medjets,size());
	addPlot("dphillj jets multi", "N_{jet}", "N_{evt}");
	FILL(dphilljjets,size());
	addPlot("dphiplushard jets multi", "N_{jet}", "N_{evt}");
	FILL(dphiplushardjets,size());


	SETBINSRANGE(5,-0.5,4.5);
	addPlot("hard b jet multi", "N_{bjet}","N_{evt}",true);
	FILL(hardbjets,size());
	addPlot("med b jet multi","N{bjet}","N_{evt}",true);
	FILL(medbjets,size());

	SETBINSRANGE(20,0,1);
	addPlot("leading jet btag","D_{1^{st}jet}","evt/bw");
	if(event()->hardjets && event()->hardjets->size()>0)
		FILL(hardjets->at(0),btag()) ;

	SETBINSRANGE(20,0,1);
	addPlot("secleading jet btag","D_{1^{st}jet}","evt/bw");
	if(event()->hardjets && event()->hardjets->size()>1)
		FILL(hardjets->at(1),btag()) ;


	SETBINSRANGE(40,0,200);
	addPlot("met adjusted","E_{T}^{miss} [GeV]","N_{evt}/bw");
	FILL(adjustedmet,met());

	SETBINSRANGE(80,-0.5,0.5);
	addPlot("jet phi resolution", "#Delta_{}^{#phi}(gen,reco)","/bw");
	if(event()->idjets){
		for(size_t i=0;i<event()->idjets->size();i++){
			float genpt=event()->idjets->at(i)->genP4().Pt();
			if(genpt >0){
				last()->fill((event()->idjets->at(i)->phi() - event()->idjets->at(i)->genP4().Phi()),*(event()->puweight));
			}
		}
	}

	SETBINSRANGE(80,0,300);
	addPlot("S4 pt","p_{T}^{S4} [GeV]","N_{evt}/bw");
	FILL(S4,pt());



	//Combined vars

	SETBINSRANGE(120,0,240);
	addPlot("mll Range","m_{ll}[GeV]","N_{evt}/bw",true);
	FILLSINGLE(mll);

	SETBINSRANGE(40,0,300);
	addPlot("HT","H_{T}","N_{evt}/bw");
	FILLSINGLE(ht);

	SETBINSRANGE(20,60,300);
	addPlot("pt llj","p_{T}(llj)","N_{evt}/bw");
	FILLSINGLE(ptllj);

	SETBINSRANGE(30,0,M_PI);
	addPlot("dphi (ll,j)","#Delta#phi(ll,j)","N_{evt}/bw");
	FILLSINGLE(dphillj);

	SETBINSRANGE(30,0,M_PI);
	addPlot("dphi (ll,jj)","#Delta#phi(ll,jj)","N_{evt}/bw");
	FILLSINGLE(dphilljj);

	SETBINSRANGE(30,-8,8);
	addPlot("deta (ll,j)","#Delta#eta(ll,j)","N_{evt}/bw");
	FILLSINGLE(detallj);

	SETBINSRANGE(30,-8,8);
	addPlot("deta (ll,jj)","#Delta#eta(ll,jj)","N_{evt}/bw");
	FILLSINGLE(detalljj);

	///middphi range


	SETBINSRANGE(30,0,M_PI);
	addPlot("dphi ((ll),j) midphi","#Delta#phi(ll,j)","N_{evt}/bw");
	if(middphi && middphiInfo) FILLSINGLE(dphillj);

	SETBINSRANGE(20,60,300);
	addPlot("pt llj midphi","p_{T}(llj)","N_{evt}/bw");
	if(middphi && middphiInfo) FILLSINGLE(ptllj);

	SETBINSRANGE(10,-0.5,9.5);
	addPlot("med jet multi midphi", "N_{jet}","N_{evt}");
	if(middphi && middphiInfo) FILL(medjets,size());
	addPlot("hard jet multi midphi", "N_{jet}","N_{evt}");
	if(middphi && middphiInfo) FILL(hardjets,size());


	SETBINS << 0 << 10 << 20 << 25 << 30 << 35 << 40 << 45 << 50 << 60 << 70 << 100 << 200;
	addPlot("leading id jet pt midphi","p_{T} [GeV]", "N_{j}/bw");
	if(event()->idjets && event()->idjets->size()>0)
		if(middphi && middphiInfo) FILL(idjets->at(0),pt());
	addPlot("secleading id jet pt midphi","p_{T} [GeV]", "N_{j}/bw");
	if(event()->idjets && event()->idjets->size()>1)
		if(middphi && middphiInfo) FILL(idjets->at(1),pt());

	SETBINSRANGE(30,-8,8);
	addPlot("deta (ll,j) midphi","#Delta#eta(ll,j)","N_{evt}/bw");
	if(middphi && middphiInfo)FILLSINGLE(detallj);

	SETBINSRANGE(30,-8,8);
	addPlot("deta (ll,jj) midphi","#Delta#eta(ll,jj)","N_{evt}/bw");
	if(middphi && middphiInfo)FILLSINGLE(detalljj);

	///NOT MIDPHI RANGE


	SETBINSRANGE(30,0,M_PI);
	addPlot("dphi ((ll),j) NOTmidphi","#Delta#phi(ll,j)","N_{evt}/bw");
	if(!middphi && middphiInfo) FILLSINGLE(dphillj);

	SETBINSRANGE(20,60,300);
	addPlot("pt llj NOTmidphi","p_{T}(llj)","N_{evt}/bw");
	if(!middphi && middphiInfo) FILLSINGLE(ptllj);

	SETBINSRANGE(10,-0.5,9.5);
	addPlot("med jet multi NOTmidphi", "N_{jet}","N_{evt}");
	if(!middphi && middphiInfo) FILL(medjets,size());
	addPlot("hard jet multi NOTmidphi", "N_{jet}","N_{evt}");
	if(!middphi && middphiInfo) FILL(hardjets,size());

	SETBINS << 0 << 10 << 20 << 25 << 30 << 35 << 40 << 45 << 50 << 60 << 70 << 100 << 200;
	addPlot("leading id jet pt NOTmidphi","p_{T} [GeV]", "N_{j}/bw");
	if(event()->idjets && event()->idjets->size()>0)
		if(!middphi && middphiInfo) FILL(idjets->at(0),pt());
	addPlot("secleading id jet pt NOTmidphi","p_{T} [GeV]", "N_{j}/bw");
	if(event()->idjets && event()->idjets->size()>1)
		if(!middphi && middphiInfo) FILL(idjets->at(1),pt());


	SETBINSRANGE(30,-8,8);
	addPlot("deta (ll,j) NOTmidphi","#Delta#eta(ll,j)","N_{evt}/bw");
	if(!middphi && middphiInfo) FILLSINGLE(detallj);

	SETBINSRANGE(30,-8,8);
	addPlot("deta (ll,jj) NOTmidphi","#Delta#eta(ll,jj)","N_{evt}/bw");
	if(!middphi && middphiInfo) FILLSINGLE(detalljj);


	///fancy vars

	SETBINSRANGE(70,0,1200);
	addPlot("top discr","D_{top}","N_{evt}/bw",true);
	FILLSINGLE(topdiscr);

	///EVT VARS

	SETBINSRANGE(50,0,50);
	addPlot("vertex multiplicity", "n_{vtx}", "N_{evt}");
	FILL(event,vertexMulti());

}

}//namespace

