//---------------------------------------------------------------------------

#ifndef IIRButterH
#define IIRButterH
//---------------------------------------------------------------------------
#include "SecondOrderSection.h"

class CIIRButter
{
protected:

  CSOS sos[10]; //maximally 20th order
  double cutoff; //-60 dB frequency [Hz]

  double fs; //sample frequency  [Hz]
  double fc; //corner freq analog [Hz]
  int order;

  

  //retrieves the analog coefficients for a Butterworth section
  void GetAnalogCoeffs(double& A, double& b, double& c, int pole_number);
  virtual void CreateFilter()=0;


public:
  CIIRButter() : cutoff(0), fs(0), fc(0), order(2) {};
  CIIRButter(double fsample, int aorder) : cutoff(0), fs(fsample), fc(0), order(aorder)
    {};

  //When these change, the filter must be recalculated;
  void SetSampleFreq(double fsamp);
  void SetCutOff(double c_off);
  void SetOrder(int new_order);

  void Filter(char* in, char* out, int nsamples);

};
//---------------------------------------------------------------------------

class CIIRButterLow : public CIIRButter
{
  virtual void CreateFilter();
public:
  CIIRButterLow(){};
  CIIRButterLow(double fsample, int aorder) : CIIRButter(fsample,aorder){};

} ;
//---------------------------------------------------------------------------

class CIIRButterHi : public CIIRButter
{
  virtual void CreateFilter();
public:
  CIIRButterHi(){};
  CIIRButterHi(double fsample, int aorder) : CIIRButter(fsample,aorder){};

} ;





#endif
