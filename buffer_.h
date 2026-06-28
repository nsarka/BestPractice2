//---------------------------------------------------------------------------

#ifndef bufferH
#define bufferH

#include <vector>


struct Buffer {
  std::vector<char> data;
  std::vector<char>::iterator pos;

  int data_left() {return data.end() - pos;}
  int buffer_pos() {return pos - data.begin();}
  Buffer() {data.resize(4,0);
            pos = data.end();}; 
};
//---------------------------------------------------------------------------
#endif
