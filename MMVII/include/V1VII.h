#ifndef _V1V2_H_
#define _V1V2_H_

#include "StdAfx.h"
#include "include/MMVII_all.h"


namespace MMVII
{

template <class Type> Pt2d<Type>  ToMMV1(const cPtxd<Type,2> &  aP) {return  Pt2d<Type>(aP.x(),aP.y());}
template <class Type> cPtxd<Type,2> ToMMVII(const Pt2d<Type> &  aP) {return cPtxd<Type,2>(aP.x,aP.y);}

template <class Type> Box2d<Type>  ToMMV1(const cTplBox<Type,2> &  aBox) {return  Box2d<Type>(ToMMV1(aBox.P0()),ToMMV1(aBox.P1()));}


GenIm::type_el ToMMV1(eTyNums aV2);
eTyNums ToMMVII( GenIm::type_el );

void ExportHomMMV1(const std::string & aIm1,const std::string & aIm2,const std::string & SH,const std::vector<cPt2di> & aVP);
void ExportHomMMV1(const std::string & aIm1,const std::string & aIm2,const std::string & SH,const std::vector<cPt2dr> & aVP);

void MakeStdIm8BIts(cIm2D<tREAL4> aImIn,const std::string& aName);




template <class Type> class cMMV1_Conv
{
    public :
     typedef typename El_CTypeTraits<Type>::tBase   tBase;
     typedef  Im2D<Type,tBase>  tImMMV1;
     typedef  cDataIm2D<Type>       tImMMVII;

     static inline tImMMV1 ImToMMV1(const tImMMVII &  aImV2)  // To avoid conflict with global MMV1
     {
        Type * aDL = const_cast< tImMMVII &> (aImV2).RawDataLin();
        // return   tImMMV1(aImV2.RawDataLin(),nullptr,aImV2.Sz().x(),aImV2.Sz().y());
        return   tImMMV1(aDL,nullptr,aImV2.Sz().x(),aImV2.Sz().y());
     };


     static void ReadWrite(bool ReadMode,const tImMMVII &aImV2,const cDataFileIm2D & aDF,const cPt2di & aP0File,double aDyn,const cRect2& aR2Init);
};


};

#endif // _V1V2_H_
