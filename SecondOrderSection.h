//---------------------------------------------------------------------------

#ifndef SecondOrderSectionH
#define SecondOrderSectionH
//---------------------------------------------------------------------------
#include <vector>


struct sos_coeffs
{
  float a0,a1,a2,b0,b1,b2;
};

class CSOS
{
  sos_coeffs cf ;
  float xnl[2],ynl[2],xnr[2],ynr[2];

public:

  CSOS() {xnr[0]=xnr[1]=ynr[0]=ynr[1]=xnl[0]=xnl[1]=ynl[0]=ynl[1]=
          cf.a0=cf.a1=cf.a2=cf.b0=cf.b1=cf.b2=0;};

  void SetCoefficients(sos_coeffs& coef)
  {
    cf = coef;

    //Normalize
    cf.b0 /= cf.a0;
    cf.b1 /= cf.a0;
    cf.b2 /= cf.a0;

    cf.a1 /= cf.a0;
    cf.a2 /= cf.a0;


  };

  //Keep integer data
  void Filter(char* in, char* out, int numsamples);

  //Keep float data
  void Filter(float* in, float* out, int numsamples);

  //Convert from float to integer
  void Filter(float* in, char* out, int numsamples);

  //Convert from integer to float
  void Filter(char* in, float* outdata, int numsamples);
};



  


#endif
