//---------------------------------------------------------------------------

#ifndef TQMFBankH
#define TQMFBankH
//---------------------------------------------------------------------------

#include "SampleData.h"
#include <vector>

#define FILTER_LENGTH 32


namespace QMFBank{

class TQMFBank
{
  SampleData sd[FILTER_LENGTH-1];
  SampleDataFl sdsynlo[FILTER_LENGTH-1];
  SampleDataFl sdsynhi[FILTER_LENGTH-1];

public:
  TQMFBank();
  int Analyze(SampleData* input, unsigned int length, SampleData* out_lo,
         SampleData* out_hi);
  int Analyze(std::vector<SampleData>& inp, std::vector<SampleDataFl>& out_lo,
         std::vector<SampleDataFl>& out_hi);
  int Synthesize(SampleDataFl* in_lo, SampleDataFl* in_hi, unsigned int length,
         SampleData* out);
  int FirFilt(SampleData* inp, unsigned int length, SampleData* out);
};



} //namespace

#endif
