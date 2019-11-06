#include "include/MMVII_all.h"
#include "include/MMVII_Tpl_Images.h"

namespace MMVII
{


/*
   General note on this file :

      use of static fonction in first version to allow use of function
      on various vector (REAL4,8,16) as  the method could not be template & virtural
   
*/

/* ========================== */
/*          cSParseVect        */
/* ========================== */

template <class Type> cSparseVect<Type>::cSparseVect(int aSzReserve,int aSzInit) :
   mIV (new tCont)
{
  if (aSzReserve>0) 
     IV().reserve(aSzReserve);
  if (aSzInit>0)
  {
      for (int aK=0 ; aK<aSzInit ; aK++)
          AddIV(aK,1.0);      // arbitrary values
  }
}

template <class Type>  bool cSparseVect<Type>::IsInside(int aNb) const
{
    for (const auto & aP : *(mIV.get()))
    {
        if ((aP.mInd<0) || (aP.mInd>= aNb))
           return false;
    }
    return true;
}

/* ========================== */
/*          cDenseVect        */
/* ========================== */

template <class Type> cDenseVect<Type>::cDenseVect(tIM anIm) :
   mIm  (anIm) 
{
}

template <class Type> cDenseVect<Type>::cDenseVect(int aSz,eModeInitImage aModeInit) :
   cDenseVect<Type> (tIM  (aSz,nullptr,aModeInit))
{
}

/*
template <class Type> cDenseVect<Type>::cDenseVect(int aSz,eModeInitImage aModeInit) :
   mIm  (aSz,nullptr,aModeInit)
{
}
*/

template <class Type> double cDenseVect<Type>::L1Dist(const cDenseVect<Type> & aV) const
{
   return mIm.DIm().L1Dist(aV.mIm.DIm());
}
template <class Type> double cDenseVect<Type>::L2Dist(const cDenseVect<Type> & aV) const
{
   return mIm.DIm().L2Dist(aV.mIm.DIm());
}
template <class Type> double cDenseVect<Type>::DotProduct(const cDenseVect<Type> & aV) const
{
   return MMVII::DotProduct(DIm(),aV.DIm());
}
// double L2Dist(const cDenseVect<Type> & aV) const;

template <class Type> Type*       cDenseVect<Type>::RawData()       {return DIm().RawDataLin();}
template <class Type> const Type* cDenseVect<Type>::RawData() const {return DIm().RawDataLin();}

// const Type * RawData() const;

template <class Type> void  cDenseVect<Type>::WeightedAddIn(Type aW,const tSpV & aVect)
{
    Type * aD =  RawData();

    for (const auto & aP : aVect)
       aD[aP.mInd] += aW * aP.mVal;
}


template <class Type> std::ostream & operator << (std::ostream & OS,const cDenseVect<Type> &aV)
{
   OS << "[";
   for (int aK=0 ; aK<aV.DIm().Sz() ; aK++)
   {
         if (aK!=0) OS << " ";
         OS << aV(aK);
   }
   OS << "]";
   return OS;
}


/* ========================== */
/*          cMatrix       */
/* ========================== */
template <class Type> cMatrix<Type>::cMatrix(int aX,int aY) :
   cRect2(cPt2di(0,0),cPt2di(aX,aY))
{
}

template <class Type> cMatrix<Type>::~cMatrix() 
{
}


template <class Type> double cMatrix<Type>::TriangSupicity() const   ///< How close to triangular sup
{
     double aNb=0;
     double aSom =0.0;
     for (const auto & aP : *this)
     {
         if (aP.x() < aP.y())
         {
            aNb++;
            aSom += Square(V_GetElem(aP.x(),aP.y()));
         }
     }
     aSom /= std::max(1.0,aNb);
     return std::sqrt(aSom);
}
template <class Type> void cMatrix<Type>::SelfSymetrizeBottom()
{
   cMatrix<Type>::CheckSquare(*this);
   int aNb = Sz().x();
   for (int aX=0 ; aX<aNb ; aX++)
   {
       for (int aY=aX+1 ; aY<aNb ; aY++)
       {
            V_SetElem(aX,aY,V_GetElem(aY,aX));
       }
   }
}


         // ============  Mul Col ========================

template <class Type> static Type TplMulColElem(int aY,const cMatrix<Type> & aMat,const cDenseVect<Type> & aVIn)
{
    aMat.TplCheckSizeX(aVIn);

    Type aRes = 0.0;
    for (int aX=0 ; aX<aMat.Sz().x() ; aX++)
        aRes += aVIn(aX) * aMat.V_GetElem(aX,aY);

    return aRes;
}


template <class Type> static void TplMulCol(cDenseVect<Type> & aVOut,const cMatrix<Type> & aMat,const cDenseVect<Type> & aVIn) 
{
   aMat.TplCheckSizeYandX(aVOut,aVIn);

   if (&aVOut==&aVIn) // Will see later if we handle this case
   {
       MMVII_INTERNAL_ASSERT_strong(false,"Aliasing in TplMulCol")
   }

   for (int aY=0 ; aY<aMat.Sz().y() ; aY++)
   {
       aVOut(aY) = TplMulColElem(aY,aMat,aVIn);
   }
}

template <class Type> static cDenseVect<Type> TplMulCol(const cMatrix<Type> & aMat,const cDenseVect<Type> & aVIn) 
{
    cDenseVect<Type> aRes(aMat.Sz().y());
    TplMulCol(aRes,aMat,aVIn);

    return aRes;
}

template <class Type> static void TplReadColInPlace(const cMatrix<Type> & aMat,int aX,cDenseVect<Type>& aV)
{
    aMat.TplCheckSizeY(aV);
    for (int aY=0 ; aY<aMat.Sz().y() ; aY++)
        aV(aY) = aMat.V_GetElem(aX,aY);
}

template <class Type> static void TplWriteCol(cMatrix<Type> & aMat,int aX,const cDenseVect<Type>& aV)
{
    aMat.TplCheckSizeY(aV);
    for (int aY=0 ; aY<aMat.Sz().y() ; aY++)
        aMat.V_SetElem(aX,aY,aV(aY)) ;
}


template <class Type> static void TplAdd_tAB(cMatrix<Type> & aMat,const cDenseVect<Type> & aCol,const cDenseVect<Type> & aLine)
{
    aMat.TplCheckSizeY(aCol);
    aMat.TplCheckSizeX(aLine);
    for (int aY=0 ; aY<aMat.Sz().y() ; aY++)
    {
        for (int aX=0 ; aX<aMat.Sz().x() ; aX++)
        {
           aMat.V_SetElem(aX,aY,aMat.V_GetElem(aX,aY) + aCol(aY) * aLine(aX));
        }
    }
}

template <class Type> static void Weighted_TplAdd_tAA(Type aW,cMatrix<Type> & aMat,const cDenseVect<Type> & aV,bool OnlySup)
{
    aMat.TplCheckSizeY(aV);
    aMat.TplCheckSizeX(aV);
    for (int aY=0 ; aY<aMat.Sz().y() ; aY++)
    {
        Type aVy = aV(aY) * aW;
        for (int aX= OnlySup ? aY : 0 ; aX<aMat.Sz().x() ; aX++)
        {
           aMat.V_SetElem(aX,aY,aMat.V_GetElem(aX,aY) + aVy * aV(aX));
        }
    }
}

template <class Type> static void TplAdd_tAA(cMatrix<Type> & aMat,const cDenseVect<Type> & aV,bool OnlySup)
{
     Weighted_TplAdd_tAA(Type(1.0),aMat,aV,OnlySup);
/*
    aMat.TplCheckSizeY(aV);
    aMat.TplCheckSizeX(aV);
    for (int aY=0 ; aY<aMat.Sz().y() ; aY++)
    {
        for (int aX= OnlySup ? aY : 0 ; aX<aMat.Sz().x() ; aX++)
        {
           aMat.V_SetElem(aX,aY,aMat.V_GetElem(aX,aY) + aV(aY) * aV(aX));
        }
    }
*/
}
template <class Type> static void TplSub_tAA(cMatrix<Type> & aMat,const cDenseVect<Type> & aV,bool OnlySup)
{
    Weighted_TplAdd_tAA(Type(-1.0),aMat,aV,OnlySup);
/*
    aMat.TplCheckSizeY(aV);
    aMat.TplCheckSizeX(aV);
    for (int aY=0 ; aY<aMat.Sz().y() ; aY++)
    {
        for (int aX= OnlySup ? aY : 0 ; aX<aMat.Sz().x() ; aX++)
        {
           aMat.V_SetElem(aX,aY,aMat.V_GetElem(aX,aY) - aV(aY) * aV(aX));
        }
    }
*/
}


         // ============  Mul Line ========================

template <class Type> static Type TplMulLineElem(int aX,const cMatrix<Type> & aMat,const cDenseVect<Type> & aVIn)
{
    aMat.TplCheckSizeY(aVIn);
    Type aRes = 0.0;
    for (int aY=0 ; aY<aMat.Sz().y() ; aY++)
        aRes += aVIn(aY) * aMat.V_GetElem(aX,aY);

    return aRes;
}


template <class Type> static void TplMulLine(cDenseVect<Type> & aVOut,const cMatrix<Type> & aMat,const cDenseVect<Type> & aVIn)  
{
   aMat.TplCheckSizeYandX(aVIn,aVOut);

   if (&aVOut==&aVIn) // Will see later if we handle this case
   {
       MMVII_INTERNAL_ASSERT_strong(false,"Aliasing in TplMulCol")
   }

   for (int aX=0 ; aX<aMat.Sz().x() ; aX++)
   {
       aVOut(aX) = TplMulLineElem(aX,aMat,aVIn);
   }
}

template <class Type> static cDenseVect<Type> TplMulLine(const cMatrix<Type> & aMat,const cDenseVect<Type> & aVIn) 
{
    cDenseVect<Type> aRes(aMat.Sz().x());
    TplMulLine(aRes,aMat,aVIn);

    return aRes;
}

template <class Type> static void TplReadLineInPlace(const cMatrix<Type> & aMat,int aY,cDenseVect<Type>& aV)
{
    aMat.TplCheckSizeX(aV);
    for (int aX=0 ; aX<aMat.Sz().x() ; aX++)
        aV(aX) = aMat.V_GetElem(aX,aY);
}

template <class Type> static void TplWriteLine(cMatrix<Type> & aMat,int aY,const cDenseVect<Type>& aV)
{
    aMat.TplCheckSizeX(aV);
    for (int aX=0 ; aX<aMat.Sz().x() ; aX++)
        aMat.V_SetElem(aX,aY,aV(aX)) ;
}



     // Virtuals tREAL4

template <class Type> void  cMatrix<Type>::Add_tAB(const tDV & aCol,const tDV & aLine) { TplAdd_tAB(*this,aCol,aLine); }
template <class Type> void  cMatrix<Type>::Add_tAA(const tDV & aV,bool OnlySup) {TplAdd_tAA(*this,aV,OnlySup);}
template <class Type> void  cMatrix<Type>::Sub_tAA(const tDV & aV,bool OnlySup) {TplSub_tAA(*this,aV,OnlySup);}
template <class Type> void  cMatrix<Type>::Weighted_Add_tAA(Type aW,const tDV & aV,bool OnlySup) 
{
   Weighted_TplAdd_tAA(aW,*this,aV,OnlySup);
}



template <class Type> void cMatrix<Type>::MulColInPlace(tDV & aOut,const tDV & aIn) const
{
    TplMulCol(aOut,*this,aIn);
}
template <class Type> cDenseVect<Type> cMatrix<Type>::MulCol(const tDV & aIn) const
{
    return TplMulCol(*this,aIn);
}

template <class Type> Type cMatrix<Type>::MulColElem(int aY,const tDV & aIn) const
{
    return TplMulColElem(aY,*this,aIn);
}
template <class Type> void cMatrix<Type>::MulLineInPlace(tDV & aOut,const tDV & aIn) const
{
    TplMulLine(aOut,*this,aIn);
}
template <class Type> cDenseVect<Type> cMatrix<Type>::MulLine(const tDV & aIn) const
{
    return TplMulLine(*this,aIn);
}
template <class Type> Type cMatrix<Type>::MulLineElem(int aX,const tDV & aIn) const
{
    return TplMulLineElem(aX,*this,aIn);
}

template <class Type> void cMatrix<Type>::ReadColInPlace(int aX,tDV & aV)  const {TplReadColInPlace(*this,aX,aV);}
template <class Type> void cMatrix<Type>::WriteCol(int aX,const tDV  &aV)        {TplWriteCol(*this,aX,aV);}
template <class Type> void cMatrix<Type>::ReadLineInPlace(int aY,tDV & aV) const {TplReadLineInPlace(*this,aY,aV);}
template <class Type> void cMatrix<Type>::WriteLine(int aY,const tDV  &aV)       {TplWriteLine(*this,aY,aV);}

template <class Type> cDenseVect<Type>  cMatrix<Type>::ReadCol(int aX) const
{
     cDenseVect<Type> aRes(Sz().y());
     ReadColInPlace(aX,aRes);

     return aRes;
}

template <class Type> cDenseVect<Type>  cMatrix<Type>::ReadLine(int aX) const
{
     cDenseVect<Type> aRes(Sz().x());
     ReadLineInPlace(aX,aRes);

     return aRes;
}

     //    ===   MulMat ====


template <class Type> void cMatrix<Type>::MatMulInPlace(const tMat & aM1,const tMat & aM2)
{
   CheckSizeMulInPlace(aM1,aM2);
   cDenseVect<tREAL16> aLine(this->Sz().x());

   for (int aY= 0 ; aY< this->Sz().y() ; aY++)
   {
       for (int aX= 0 ; aX< this->Sz().x() ; aX++)
       {
           tREAL16 aVal = 0.0;
           for (int aK=0 ; aK<aM1.Sz().x() ; aK++)
              aVal += aM1.V_GetElem(aK,aY) * aM2.V_GetElem(aX,aK);
           V_SetElem(aX,aY,aVal);
       }
   }
}
#if (0)
#endif

     /* ========== Methods with sparse vectors ============ */


template <class Type>  void  cMatrix<Type>::Weighted_Add_tAA(Type aWeight,const tSpV & aSparseV,bool OnlySup)
{
   CheckSquare(*this);
   TplCheckX(aSparseV);
   const typename cSparseVect<Type>::tCont & aIV =  aSparseV.IV();
   int aNb  = aIV.size();

   if (OnlySup)
   {
       for (int aKY=0 ; aKY<aNb ; aKY++)
       {
          Type aVy = aIV[aKY].mVal * aWeight;
          int  aY = aIV[aKY].mInd;
          for (int aKX=  0 ; aKX<aNb ; aKX++)
          {
              int aX = aIV[aKX].mInd;
              if (aX>=aY)
                 V_SetElem(aX,aY,V_GetElem(aX,aY) + aVy * aIV[aKX].mVal);
          }
       }
   }
   else
   {
       for (int aKY=0 ; aKY<aNb ; aKY++)
       {
          Type aVy = aIV[aKY].mVal * aWeight;
          int  aY = aIV[aKY].mInd;
          for (int aKX= 0 ; aKX<aNb ; aKX++)
          {
              int aX = aIV[aKX].mInd;
              V_SetElem(aX,aY,V_GetElem(aX,aY) + aVy * aIV[aKX].mVal);
          }
       }
   }
}



     /* ========== operator  ============ */

template <class Type> cDenseVect<Type> operator * (const cDenseVect<Type> & aLine,const cMatrix<Type>& aMat)
{
   return aMat.MulLine(aLine);
}

template <class Type> cDenseVect<Type> operator * (const cMatrix<Type> & aMat,const cDenseVect<Type> & aCol)
{
   return aMat.MulCol(aCol);
}

template <class Type> std::ostream & operator << (std::ostream & OS,const cMatrix<Type> &aMat)
{
   OS << "[\n";
   for (int aY=0 ; aY<aMat.Sz().y() ; aY++)
   {
      cDenseVect<Type> aV(aMat.Sz().x());
      aMat.ReadLineInPlace(aY,aV);
      OS << " " << aV << "\n";
         // if (aK!=0) OS << " ";
         // OS << aV(aK);
   }
   OS << "]\n";
   return OS;
}


/* ===================================================== */
/* ===================================================== */
/* ===================================================== */

#define INSTANTIATE_BASE_MATRICES(Type)\
template  class  cSparseVect<Type>;\
template  class  cDenseVect<Type>;\
template  class  cMatrix<Type>;\
template  cDenseVect<Type> operator * (const cDenseVect<Type> & aLine,const cMatrix<Type> & aMat);\
template  cDenseVect<Type> operator * (const cMatrix<Type> & aMat ,const cDenseVect<Type> & aCol);\
template  std::ostream & operator << (std::ostream & OS,const cMatrix<Type> &aMat);\
template  std::ostream & operator << (std::ostream & OS,const cDenseVect<Type> &aV);\




INSTANTIATE_BASE_MATRICES(tREAL4)
INSTANTIATE_BASE_MATRICES(tREAL8)
INSTANTIATE_BASE_MATRICES(tREAL16)

};


/* ========================== */
/*          cMatrix           */
/* ========================== */

