#include <vector>
#include <map>
#include <iostream>

#include <TLorentzVector.h>
#include <TH1F.h>
#include <TRandom3.h>
#include <TFile.h>
#include <TString.h>
#include <TMath.h>
#include <TVector3.h>
#include <Math/VectorUtil.h>
#include <algorithm>
#include <utility>

#include "../interface/LooseKinReco.h"
#include "../interface/LooseKinRecoSolution.h"
#include "../interface/utils.h"
#include "../../../TopAnalysis/Configuration/analysis/common/include/classes.h"
#include "../interface/analysisUtils.h"


LooseKinRecoSolution LooseKinReco::KRLepMETConstrained(const std::pair<TLorentzVector, TLorentzVector> lvLLbar, const std::pair<TLorentzVector, const TLorentzVector> lvBBbar, const TLorentzVector v2MET, const bool& hasSolution)const
{

  // corrected by taking z,E(nunubar) = z,E(llbar), x,y(nunubar) = MET
  // requering M(nunubar) > 0, M(llbarnunubar) > 2M_W
  const TLorentzVector llbar = lvLLbar.first + lvLLbar.second;

  TLorentzVector nunubar(v2MET.Px(), v2MET.Py(), llbar.Pz(), llbar.E());
  if(nunubar.M() < 0.0)
    nunubar = ConstrainM(nunubar, 0.0);
  assert(nunubar.M() > -0.1);

  const double mw = 80.4;
  TLorentzVector llnn = llbar + nunubar;
  if(llnn.M() < (2.0 * mw))
    llnn = ConstrainM(llnn, 2.0 * mw);
  assert(llnn.M() > (2.0 * mw - 0.1));

  TLorentzVector ttbarLLMETTotal = llnn + lvBBbar.first + lvBBbar.second;
  std::vector<float> mlbKR(4,0.);
  mlbKR[0] = (lvLLbar.first + lvBBbar.first).M();
  mlbKR[1] = (lvLLbar.first + lvBBbar.second).M();
  mlbKR[2] = (lvLLbar.second + lvBBbar.first).M();
  mlbKR[3] = (lvLLbar.second + lvBBbar.second).M();
  const float mwwKR = llnn.M();
  float mlbmax1 = std::max(mlbKR[0], mlbKR[3]);
  float mlbmax2 = std::max(mlbKR[1], mlbKR[2]);
  const float minmlbkr = std::min(mlbmax1, mlbmax2);

  const LV ttbarLLMETTotalLV = common::TLVtoLV(ttbarLLMETTotal);
  const LooseKinRecoSolution solution(minmlbkr, mwwKR, ttbarLLMETTotalLV, hasSolution,lvBBbar,lvLLbar);
  return solution;
}

TLorentzVector LooseKinReco::ConstrainM(const TLorentzVector& lv, const double m)const
{
  if(lv.M() > m)
    throw(std::logic_error("Error in LooseKinReco::ConstrainM(): lv.M() = %f > %f = m"), lv.M(), m);
  const double eNew = TMath::Sqrt(m * m + lv.Perp2()) * TMath::CosH(lv.Rapidity());
  const double zNew =eNew * TMath::TanH(lv.Rapidity());
  TLorentzVector lvNew(lv.X(), lv.Y(), zNew, eNew);
  assert(TMath::Abs(lvNew.M() - m) < 0.1);
  return lvNew;
}


std::vector<std::vector<std::pair<TLorentzVector, TLorentzVector> > > LooseKinReco::GetCandidatesBBbar(const VLV& jets, const std::vector<int>& jetIndices, const std::vector<int>& bjetIndices, const TLorentzVector& l, const TLorentzVector& lbar, const float mlbMax)const
{
  std::vector<std::vector<std::pair<TLorentzVector, TLorentzVector> > > res;
  std::vector<LV> vJetsBTagged;
  std::vector<LV> vJetsNonBTagged;

  // Indices of non b-tagged jets
  for(const int index : jetIndices)
    {
      if(std::find(bjetIndices.begin(), bjetIndices.end(), index) == bjetIndices.end())
	{
	  LV jetNonBTagged = jets.at(index);
	  vJetsNonBTagged.push_back(jetNonBTagged);
	}
    }
  for(const int index : bjetIndices)
    {
      LV jetBTagged = jets.at(index);
      vJetsBTagged.push_back(jetBTagged);
    }

  // assert(vJetsBTagged.size() >= 1);
  assert((vJetsBTagged.size() + vJetsNonBTagged.size()) >= 2);

  // sort jets in pT
  std::sort(vJetsBTagged.begin(), vJetsBTagged.end(), compareJetPt);
  std::sort(vJetsNonBTagged.begin(), vJetsNonBTagged.end(), compareJetPt);

  // make sure jets are sorted in pT
  for(size_t j = 0; j < vJetsBTagged.size(); j++)
  {
    assert(j == 0 || vJetsBTagged[j].Pt() <= vJetsBTagged[j - 1].Pt());
  }
  for(size_t j = 0; j < vJetsNonBTagged.size(); j++)
  {
    assert(j == 0 || vJetsNonBTagged[j].Pt() <= vJetsNonBTagged[j - 1].Pt());
  }

  // make all possible pairs [b-tagging][pT]
  //const double mlbMax = 100000.0;
  // 2 b tags
  if(vJetsBTagged.size() >= 2)
  {
    res.resize(res.size() + 1);
    for(size_t i = 0; i < vJetsBTagged.size(); i++)
      for(size_t ii = i + 1; ii < vJetsBTagged.size(); ii++)
      {
        const TLorentzVector j1 = common::LVtoTLV(vJetsBTagged[i]);
        const TLorentzVector j2 = common::LVtoTLV(vJetsBTagged[ii]);
        if(CheckMlb(l, lbar, j1, j2, mlbMax))
          res.back().push_back(std::pair<TLorentzVector, TLorentzVector>(j1, j2));
      }
    // remove last vector if it is empty
    if(res.back().size() == 0)
      res.pop_back();
  }
  // 1 b tag
  if(vJetsBTagged.size() >= 1)
  {
    res.resize(res.size() + 1);
    for(size_t i = 0; i < vJetsBTagged.size(); i++)
      for(size_t ii = 0; ii < vJetsNonBTagged.size(); ii++)
      {
        const TLorentzVector j1 = common::LVtoTLV(vJetsBTagged[i]);
        const TLorentzVector j2 = common::LVtoTLV(vJetsNonBTagged[ii]);
        if(CheckMlb(l, lbar, j1, j2, mlbMax))
          res.back().push_back(std::pair<TLorentzVector, TLorentzVector>(j1, j2));
      }
    // remove last vector if it is empty
    if(res.back().size() == 0)
      res.pop_back();
  }
  else{
    res.resize(res.size() + 1);
    for(size_t i = 0; i < vJetsNonBTagged.size(); i++)
      for(size_t ii = i + 1; ii < vJetsNonBTagged.size(); ii++)
      {
        const TLorentzVector j1 = common::LVtoTLV(vJetsNonBTagged[i]);
        const TLorentzVector j2 = common::LVtoTLV(vJetsNonBTagged[ii]);
        if(CheckMlb(l, lbar, j1, j2, mlbMax))
          res.back().push_back(std::pair<TLorentzVector, TLorentzVector>(j1, j2));
      }
    // remove last vector if it is empty
    if(res.back().size() == 0)
      res.pop_back();
      
  }
    // throw std::logic_error("LooseKinReco::GetCandidatesBBbar() should not be here");

  return res;
}

bool LooseKinReco::compareJetPt(const LV& jet1, const LV& jet2)
{

  bool result = (jet1.Pt() > jet2.Pt());
  return result;

}

bool LooseKinReco::CheckMlb(const TLorentzVector& l, const TLorentzVector& lbar, const TLorentzVector& b, const TLorentzVector& bbar, const float& mlbMax)const
{
  if(mlbMax < 0.0)
    return true;
  bool flagPassComb1 = ((b + l).M() < mlbMax) && ((bbar + lbar).M() < mlbMax);
  bool flagPassComb2 = ((bbar + l).M() < mlbMax) && ((b + lbar).M() < mlbMax);
  return (flagPassComb1 || flagPassComb2);
}

LooseKinRecoSolution LooseKinReco::solution(const int leptonIndex, const int antiLeptonIndex,
					    const std::vector<int>& jetIndices, const std::vector<int>& bjetIndices,
					    const VLV& allLeptons, const VLV& jets,
					    const LV& met)const
{
  const TLorentzVector lvLep = common::LVtoTLV(allLeptons.at(leptonIndex));
  const TLorentzVector lvLepBar = common::LVtoTLV(allLeptons.at(antiLeptonIndex));
  // bbar combinations with m(lb) cut
  const std::vector<std::vector<std::pair<TLorentzVector, TLorentzVector> > > vvBBbarMlbCut = GetCandidatesBBbar(jets, jetIndices, bjetIndices, lvLep, lvLepBar, 180.);

  if(vvBBbarMlbCut.size())
    {
      const auto lvBBbar = vvBBbarMlbCut[0][0];
      const auto lvLLbar = std::pair<TLorentzVector, TLorentzVector>(lvLep, lvLepBar);
      const TLorentzVector v2MET = TLorentzVector(met.Px(),met.Py(),0,0);

      const LooseKinRecoSolution solution(KRLepMETConstrained(lvLLbar, lvBBbar, v2MET, true));
      return solution;
    }
  else
    {
      const LooseKinRecoSolution solution;
      return solution;
    }
}

LooseKinReco::LooseKinReco(const Era::Era era):
era_(era)
{

    std::cout<<"--- Beginning preparation of loose kinematic reconstruction\n";

    std::cout<<"=== Finishing preparation of loose kinematic reconstruction\n\n";

}




// -------------------------------------- Methods for LooseKinRecoScaleFactors --------------------------------------

LooseKinRecoScaleFactors::LooseKinRecoScaleFactors(const Era::Era era,
                                                   const std::vector<Channel::Channel>& channels,
                                                   const Systematic::Systematic& systematic):
era_(era),
scaleFactor_(-999.)
{
    std::cout<<"--- Beginning preparation of loose kinematic reconstruction scale factors\n";

    // Set up proper internal systematic
    SystematicInternal systematicInternal(nominal);
    if(systematic.type() == Systematic::kin){
        if(systematic.variation() == Systematic::up) systematicInternal = vary_up;
        else if(systematic.variation() == Systematic::down) systematicInternal = vary_down;
    }

    // Set the scale factors according to specific systematic, and check whether all requested channels are defined
    this->prepareSF(systematicInternal);
    for(const auto& channel : channels){
        if(m_scaleFactor_.find(channel) == m_scaleFactor_.end()){
            std::cerr<<"ERROR in constructor of LooseKinRecoScaleFactors! No scale factors defined for given channel: "
                     <<Channel::convert(channel)<<"\n...break\n"<<std::endl;
            exit(857);
        }
    }
    std::cout<<"Found scale factors for all requested channels\n";

    std::cout<<"=== Finishing preparation of loose kinematic reconstruction scale factors\n\n";
}



void LooseKinRecoScaleFactors::prepareSF(const SystematicInternal& systematic)
{
    double m_sfNomEE, m_sfNomEMu, m_sfNomMuMu = 1.0;
    double m_sfUncEE, m_sfUncEMu, m_sfUncMuMu = 0.0;

    if(era_ == Era::run1_8tev) {
        // SF for newKinReco flat N013
        m_sfNomEE = 0.9864; m_sfNomEMu = 0.9910; m_sfNomMuMu = 0.9945;
        m_sfUncEE = 0.0042; m_sfUncEMu = 0.0018; m_sfUncMuMu = 0.0035;
    }
    else if(era_ == Era::run2_13tev_50ns) {
        // SF for 50ns EA (noHFMET)
        m_sfNomEE = 0.9426; m_sfNomEMu = 1.0337; m_sfNomMuMu = 1.0200;
        m_sfUncEE = 0.0819; m_sfUncEMu = 0.0273; m_sfUncMuMu = 0.0561;
    }
    else if(era_ == Era::run2_13tev_25ns || era_ == Era::run2_13tev_2015_25ns) {
        // SF for 76X (2225pb)
        m_sfNomEE = 0.9830; m_sfNomEMu = 0.9881; m_sfNomMuMu = 0.9918;
        m_sfUncEE = 0.0091; m_sfUncEMu = 0.0039; m_sfUncMuMu = 0.0071;
    }
    else if(era_ == Era::run2_13tev_2016_25ns) {
        // SF for 80X (35922pb)
        m_sfNomEE = 1.0070; m_sfNomEMu = 1.0010; m_sfNomMuMu = 0.9993;
        m_sfUncEE = 0.0027; m_sfUncEMu = 0.0011; m_sfUncMuMu = 0.0019;
    }
    else if(era_ == Era::run2_13tev_2016) {
        // Hack SF for 94X (??? ) update soon?
        m_sfNomEE = 1.0070; m_sfNomEMu = 1.0010; m_sfNomMuMu = 0.9993;
        m_sfUncEE = 0.0027; m_sfUncEMu = 0.0011; m_sfUncMuMu = 0.0019;
    }
    else if(era_ == Era::run2_13tev_2017) {
        //Hack to be updated soon for SF of 94X (41298pb)
        m_sfNomEE = 1.0070; m_sfNomEMu = 1.0010; m_sfNomMuMu = 0.9993;
	m_sfUncEE = 0.0027; m_sfUncEMu = 0.0011; m_sfUncMuMu = 0.0019;
    }
    else if(era_ == Era::run2_13tev_2018) {
        //Hack to be updated soon for SF of 102X (???)
        m_sfNomEE = 1.0070; m_sfNomEMu = 1.0010; m_sfNomMuMu = 0.9993;
	m_sfUncEE = 0.0027; m_sfUncEMu = 0.0011; m_sfUncMuMu = 0.0019;
    }

    else{
        std::cerr<<"ERROR in LooseKinRecoScaleFactors::prepareSF() ! Era is not supported: "
                     <<Era::convert(era_)<<"\n...break\n"<<std::endl;
        exit(358);
    }

    const std::map<Channel::Channel, double> m_sfNominal { {Channel::ee, m_sfNomEE},{Channel::emu, m_sfNomEMu},{Channel::mumu, m_sfNomMuMu}};
    const std::map<Channel::Channel, double> m_sfUnc { {Channel::ee, m_sfUncEE},{Channel::emu, m_sfUncEMu},{Channel::mumu, m_sfUncMuMu}};

    for(const auto& sfNominal : m_sfNominal){
        const Channel::Channel& channel = sfNominal.first;
        const double& centralValue = sfNominal.second;
        const double& uncertainty = m_sfUnc.at(channel);
        if(systematic == nominal) m_scaleFactor_[channel] = centralValue;
        else if(systematic == vary_up) m_scaleFactor_[channel] = centralValue + uncertainty;
        else if(systematic == vary_down) m_scaleFactor_[channel] = centralValue - uncertainty;
        else{
           std::cerr<<"ERROR in LooseKinRecoScaleFactors::prepareSF()! "
                    <<"No treatment defined for specified systematic\n...break\n"<<std::endl;
           exit(857);
        }
    }
}



void LooseKinRecoScaleFactors::prepareChannel(const Channel::Channel& channel)
{
    scaleFactor_ = m_scaleFactor_.at(channel);
}



double LooseKinRecoScaleFactors::getSF()const
{
    return scaleFactor_;
}
