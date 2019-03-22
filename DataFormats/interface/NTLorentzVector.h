/*
 * NTLorentzVector.h
 *
 *  Created on: Jan 28, 2014
 *      Author: kiesej
 */

#ifndef NTLORENTZVECTOR_H_
#define NTLORENTZVECTOR_H_
//#include "math.h"
#include <cmath>
#include <limits>
#include "NTVector.h"

// in principle implementation of simplified polar lorentz vector
// as found in root::math
// because all hassle with root::math whatever is annoying
//
/**
 * templated, no implicit type conversions!
 */
namespace ztop{
template <class T>
class NTLorentzVector{
public:
    NTLorentzVector():pt_(0.00000001),eta_(0),phi_(0),m_(0) {}
    NTLorentzVector(T pt_in, T eta_in, T phi_in, T m_in){
        pt_=pt_in; eta_=eta_in; phi_=phi_in; m_=m_in;
        if(pt_==0) pt_=0.00000001;
    }
    ~NTLorentzVector(){}
    NTLorentzVector& operator =(const NTLorentzVector& rhs){
        pt_=rhs.pt_; eta_=rhs.eta_; phi_=rhs.phi_; m_=rhs.m_;
        return *this;
    }
    NTLorentzVector(const NTLorentzVector& rhs){
        pt_=rhs.pt_; eta_=rhs.eta_; phi_=rhs.phi_; m_=rhs.m_;
    }

    void setPt(T PT){if(PT!=0)pt_=PT;else pt_=std::numeric_limits<T>::min();}
    void setEta(T ETA){eta_=ETA;}
    void setPhi(T PHI){phi_=PHI;}
    void setM(T Mass){m_=Mass;}

    const T & pt()const{return pt_;}
    const T & eta()const{return eta_;}
    const T & phi()const{return phi_;}
    const T & m()const{return m_;}
    const T rapidity()const{
        T y = 0.5*log( (E()+Pz()) / (E()-Pz()) );
        return y;
    }

    //some root-like style
    const T & Pt()const{return pt_;}
    const T & Eta()const{return eta_;}
    const T & Phi()const{return phi_;}
    const T & M()const{return m_;}

    //advanced functionality
    //getters

    //some root-like style getters
    T P()const {return pt_*std::cosh(eta_);}
    T M2() const   {
        return ( m_  >= 0 ) ?  m_*m_ :  -m_*m_;
    }
    T Px() const { return (T) pt_*cos(phi_);}
    T Py() const { return (T) pt_*sin(phi_);}
    T Pz() const {return pt_*std::sinh(eta_);}
    T E2() const {
        T e2out =  P()*P() + M2();
        return e2out > 0 ? e2out : 0;
    }
    T E() const {return std::sqrt(E2() );}


    T p()  const {return P();}
    T m2() const {return M2();}
    T px() const {return Px();}
    T py() const {return Py();}
    T pz() const {return Pz();}
    T e2() const {return E2();}
    T e() const  {return E();}

    //setters

    void setPxPyPzE(const T & px_in, const T &  py_in, const T &  pz_in, const T &  e_in){
        pt_=std::sqrt(px_in*px_in+py_in*py_in);
        if(pt_==0) pt_=0.00000001;
        phi_ = (px_in == 0.0 && py_in == 0.0) ? 0 : std::atan2(py_in,px_in);
        static const T big_z_scaled =
                std::pow(std::numeric_limits<T>::epsilon(),static_cast<T>(-.25));

        T z_scaled = pz_in/pt_;
        if (std::fabs(z_scaled) < big_z_scaled) {
            eta_= std::log(z_scaled+std::sqrt(z_scaled*z_scaled+1.0));
        } else {
            // apply correction using first order Taylor expansion of sqrt
            eta_=  pz_in>0 ? std::log(2.0*z_scaled + 0.5/z_scaled) : -std::log(-2.0*z_scaled);
        }
        T m2tmp=e_in*e_in - px_in*px_in - py_in*py_in - pz_in*pz_in;
        if(m2tmp<0){
            if(bepicky)
                throw std::out_of_range("NTLorentzVector:setPxPyPzE: M2 < 0, tachyonic.");
            m_=-std::sqrt(-m2tmp);

        }
        else{
            m_=std::sqrt(m2tmp);}
    }


    //operations

    void scale(float val){
        if (val < 0) {
            phi_ = ( (phi_ > 0) ? phi_ - 3.14159265358979323846f : phi_ + 3.14159265358979323846f  );
            eta_ = - eta_;
            val = -val;
        }
        pt_*=val;m_*=val;
    }
    NTLorentzVector & operator *= (float val){
        scale(val);
        return *this;}

    NTLorentzVector operator * (float val)const{
        NTLorentzVector out=*this;
        return out*=val;}

    NTLorentzVector & operator +=(const NTLorentzVector& rhs){
        T spx=px()+rhs.px();
        T spy=py()+rhs.py();
        T spz=pz()+rhs.pz();
        T se=e()+rhs.e();
        setPxPyPzE(spx,spy,spz,se);
        return *this;
    }
    NTLorentzVector operator + (const NTLorentzVector& rhs)const{
        NTLorentzVector out=*this;
        return out+=rhs;
    }
    NTLorentzVector & operator -=(const NTLorentzVector& rhs){
        scale(-1.);
        *this+=rhs;
        return *this;
    }
    NTLorentzVector operator - (const NTLorentzVector& rhs)const{
        NTLorentzVector out=*this;
        return out-=rhs;
    }

    T operator * (const NTLorentzVector& rhs)const{
        return rhs.e()*e()-rhs.px()*px()-rhs.py()*py()-rhs.pz()*pz();
    }


    NTVector getNTVector()const{
        return NTVector ((float)px(),(float)py(),(float)pz());
    }


    /**
     * if switched on exceptions are thrown in some cases (e.g. tachyonic lorentz vectors)
     */
    static bool bepicky;

private:
	////////////////////////
	/*
	 * Data members. changes here produce incompatibilities between ntuple versions!
	 */
	////////////////////////


    T pt_,eta_,phi_,m_;


};
template<class T>
bool NTLorentzVector<T>::bepicky=false;

template<class T>
float dR_3d(const NTLorentzVector<T>& a,const NTLorentzVector<T>& b){
    T deta2=(a.eta() - b.eta())*(a.eta() - b.eta());
    T dphi2=absNormDPhi(a,b);
    return sqrt(deta2+dphi2);
}

template<class T>
float cosAngle_3d(const NTLorentzVector<T>& a,const NTLorentzVector<T>& b){
    float mag_l1, mag_l2, px_l1, py_l1, pz_l1, px_l2, py_l2, pz_l2, scal_pr;
    px_l1 = a.Px();
    py_l1 = a.Py();
    pz_l1 = a.Pz();
    px_l2 = b.Px();
    py_l2 = b.Py();
    pz_l2 = b.Pz();

    mag_l1 = (px_l1*px_l1 + py_l1*py_l1 + pz_l1*pz_l1);
    mag_l2 = (px_l2*px_l2 + py_l2*py_l2 + pz_l2*pz_l2);
    scal_pr = (px_l1*px_l2 + py_l1*py_l2 + pz_l1*pz_l2);
    return scal_pr/sqrt(mag_l1*mag_l2);
}
template<class T>
float absNormDPhi(const NTLorentzVector<T>& a,const NTLorentzVector<T>& b){
    float dphi=fabs(a.phi()-b.phi());
    if(dphi > M_PI) dphi=2*M_PI-dphi;
    return dphi;
}
template<class T>
float absNormDPhi(const NTLorentzVector<T>& a,const float& b){
    float dphi=fabs(a.phi()-b);
    if(dphi > M_PI) dphi=2*M_PI-dphi;
    return dphi;
}

template<class T>
float absNormDPhi(const float& a,const NTLorentzVector<T>& b){
    float dphi=fabs(a-b.phi());
    if(dphi > M_PI) dphi=2*M_PI-dphi;
    return dphi;
}




}


#endif /* NTLORENTZVECTOR_H_ */
