/*
 * analysisPlotsJan.h
 *
 *  Created on: Feb 12, 2014
 *      Author: kiesej
 */

#ifndef ANALYSISPLOTSKINRECO_H_
#define ANALYSISPLOTSKINRECO_H_

#include "analysisPlots.h"

namespace ztop{

    class analysisPlotsKinReco : public analysisPlots{
    public:
    analysisPlotsKinReco(size_t step):analysisPlots(step),
            mtt(0),
            mtt_coarse(0),
            pt_top(0),
            eta_top(0),
            pt_antitop(0),
            eta_antitop(0),
            pt_ttbar(0),
            eta_ttbar(0),
            bin_1_(420),
            bin_2_(550),
            bin_3_(810),
            mlbcombthresh_(165)
            {
                m_mub_3_.resize(cat_bjetmax,0);
                m_mub_5_.resize(cat_bjetmax,0);
                m_mub_7_.resize(cat_bjetmax,0);
                m_mub_10_.resize(cat_bjetmax,0);
                last_jet_pt_.resize(cat_bjetmax,0);

                resizeVec(total_mtt_bjet_,cat_mttmax_reco,cat_bjetmax);
                resizeVec(m_lb_3_mtt_bjet_,cat_mttmax_reco,cat_bjetmax);
                resizeVec(m_mub_3_mtt_bjet_,cat_mttmax_reco,cat_bjetmax);
                resizeVec(m_lb_5_mtt_bjet_,cat_mttmax_reco,cat_bjetmax);
                resizeVec(m_mub_5_mtt_bjet_,cat_mttmax_reco,cat_bjetmax);
                resizeVec(last_jet_pt_mtt_bjet_,cat_mttmax_reco,cat_bjetmax);

            }
  	~analysisPlotsKinReco(){ /* destruction is handled in base class! */
	}

	void bookPlots();
	void fillPlotsReco();
	void fillPlotsGen();

        void resizeVec( std::vector<std::vector<histo1DUnfold*>> &vec , const size_t mtt_size , const size_t btag_size){
            vec.resize(mtt_size);
            for( auto &it : vec ) it.resize(btag_size,0);
        }

    private:

	enum bjetnjetscategories{
            cat_0bjet0jet,cat_0bjet1jet,cat_0bjet2jet,cat_0bjet3jet,
            cat_1bjet0jet,cat_1bjet1jet,cat_1bjet2jet,cat_1bjet3jet,
            cat_2bjet0jet,cat_2bjet1jet,cat_2bjet2jet,cat_2bjet3jet,cat_bjetjetmax}
	jetcategory;

	enum bjetcategories{
            cat_0bjet, cat_1bjet, cat_2bjet, cat_bjetmax}
	bjetcategory;

	enum reco_mttcategories{
            cat_mtt1_reco, cat_mtt2_reco, cat_mtt3_reco, cat_mtt4_reco, cat_mttmax_reco}
	reco_mttcategory;
	enum gen_mttcategories{
            cat_mtt1_gen, cat_mtt2_gen, cat_mtt3_gen, cat_mttmax_gen}
	gen_mttcategory;

	void setJetCategory(size_t nbjets,size_t njets);
	void setBJetCategory(size_t nbjets);

	void setGenMttCategory(float gen_mtt);
	void setRecoMttCategory(float reco_mtt);

        std::vector<histo1DUnfold*> m_mub_3_, m_mub_5_, m_mub_7_, m_mub_10_;
        std::vector<histo1DUnfold*> last_jet_pt_;

        std::vector<std::vector<histo1DUnfold*>> total_mtt_bjet_;
        std::vector<std::vector<histo1DUnfold*>> m_lb_3_mtt_bjet_;
        std::vector<std::vector<histo1DUnfold*>> m_mub_3_mtt_bjet_;
        std::vector<std::vector<histo1DUnfold*>> m_lb_5_mtt_bjet_;
        std::vector<std::vector<histo1DUnfold*>> m_mub_5_mtt_bjet_;
        std::vector<std::vector<histo1DUnfold*>> last_jet_pt_mtt_bjet_;

        histo1DUnfold  
            * mtt,
            * mtt_coarse,
            * pt_top,
            * eta_top,
            * pt_antitop,
            * eta_antitop,
            * pt_ttbar,
            * eta_ttbar;

        float bin_1_; // 420 (mtt> 345 by construction)
        float bin_2_; // 550
        float bin_3_; // 810
	float mlbcombthresh_;

    };

}
#endif /* ANALYSISPLOTSMLBMT_H_ */
