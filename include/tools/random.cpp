#include "random.hpp"

#include <cstdlib>

int RandomRange(int param_1,int param_2)
{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar2 = param_2 - param_1;
  if (iVar2 != 0) {
    iVar3 = rand();
    iVar1 = 0;
    if (iVar2 != 0) {
      iVar1 = iVar3 / iVar2;
    }
    param_2 = (iVar3 - iVar1 * iVar2) + param_1;
  }
  return param_2;
}

float RandomRangeFloat(float param_1,float param_2)
{
  int iVar1;
  
  iVar1 = rand();
  return (param_2 - param_1) * (float)iVar1 * 4.656613e-10 + param_1;
}
