
#ifndef LooseKinReco_h
#define LooseKinReco_h

#include <vector>
#include <map>
#include <iostream>
#include <cmath>
#include <utility>

#include <TLorentzVector.h>
#include <TString.h>
#include <TMath.h>
#include <TVector3.h>
#include <Math/VectorUtil.h>

#include "../../../TopAnalysis/Configuration/analysis/common/include/classes.h"
#include "sampleHelpers.h"
#include "analysisUtils.h"

class LooseKinRecoSolution;


class LooseKinReco{
    
public:

  LooseKinReco(const Era::Era era);
  ~LooseKinReco(){}

  LooseKinRecoSolution solution(const int leptonIndex, const int antiLeptonIndex,
				const std::vector<int>& jetIndices, const std::vector<int>& bjetIndices,
				const VLV& allLeptons, const VLV& jets, 
				const LV& met)const;

private:

  LooseKinRecoSolution KRLepMETConstrained(const std::pair<TLorentzVector, TLorentzVector> lvLLbar, const std::pair<TLorentzVector, const TLorentzVector> lvBBbar, const TLorentzVector v2MET, const bool& hasSolution)const;


  TLorentzVector ConstrainM(const TLorentzVector& lv, const double m)const;


  std::vector<std::vector<std::pair<TLorentzVector, TLorentzVector> > > GetCandidatesBBbar(const VLV& jets, const std::vector<int>& jetIndices, const std::vector<int>& bjetIndices, const TLorentzVector& l, const TLorentzVector& lbar, const float mlbMax)const;

  static bool compareJetPt(const LV& jet1, const LV& jet2);

  bool CheckMlb(const TLorentzVector& l, const TLorentzVector& lbar, const TLorentzVector& b, const TLorentzVector& bbar, const float& mlbMax)const;

  /// Analysis era
  const Era::Era era_;




};



class LooseKinRecoScaleFactors{
    
public:
    
    /// Constructor
    LooseKinRecoScaleFactors(const Era::Era era,
                                        const std::vector<Channel::Channel>& channels,
                                        const Systematic::Systematic& systematic);
    
    /// Destructor
    ~LooseKinRecoScaleFactors(){}
    
    
    
    /// Prepare scale factors per channel
    void prepareChannel(const Channel::Channel& channel);
    
    /// Get kinematic reconstruction per-event scale factor
    double getSF()const;
    
    
    
private:
    
    /// Enumeration for possible systematics
    enum SystematicInternal{nominal, vary_up, vary_down, undefined};
    
    /** Set up the scale factor for the Loose Kinematic Reconstruction
     *
     * Currently a flat per-channel SF is used. For the systematic KIN_UP and KIN_DOWN,
     * the SF is modified by its uncertainty.
     *
     * To calculate the SF, you need to set the SF to 1 and rerun. Then determine the
     * SF with kinRecoEfficienciesAndSF
     */
    void prepareSF(const SystematicInternal& systematic);
    
    
    /// Analysis era
    const Era::Era era_;
    
    /// Map containing the flat scale factors for all channels
    std::map<Channel::Channel, double> m_scaleFactor_;
    
    /// The per-channel scale factor
    double scaleFactor_;
};


#endif
