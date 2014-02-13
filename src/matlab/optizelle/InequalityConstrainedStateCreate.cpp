#include "optizelle.h"

void mexFunction(
    int nOutput,mxArray* pOutput[],
    int nInput,mxArray const * pInput[]
) {
    Optizelle::Matlab::InequalityConstrained::State::create(
        nOutput,pOutput,nInput,const_cast <mxArray **> (pInput));
}
