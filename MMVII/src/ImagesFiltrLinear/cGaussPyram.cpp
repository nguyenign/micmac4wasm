#include "include/MMVII_all.h"
#include "include/MMVII_Tpl_Images.h"
#include "include/V1VII.h"



namespace MMVII
{

/* ==================================================== */
/*                                                      */
/*              cGP_OneImage                            */
/*                                                      */
/* ==================================================== */

template <class Type> cGP_OneImage<Type>::cGP_OneImage(tOct * anOct,int aNumInOct,tGPIm * anImUp) :
    mOct       (anOct),
    mPyr       (mOct->Pyram()),
    mNumInOct  (aNumInOct),
    mUp        (anImUp),
    mDown      (nullptr),
    mOverlapUp (nullptr),
    mImG       (mOct->SzIm()),
    mIsTopPyr  ((anImUp==nullptr) && (mOct->Up()==nullptr))
{
    mNameSave =      mPyr->Params().mPrefixSave
                   + "-Ima-" 
                   + E2Str(mPyr->TypePyr()) 
                   + "-" + ShortId() 
                   // + "-" +  Prefix(mPyr->NameIm())
                   + "-" +  mPyr->Prefix()
                   + ".tif"
                ;
    if (mUp==nullptr)
    {
       mScaleAbs = anOct->Scale0Abs(); // initial value of octave
    }
    else
    {
       mUp->mDown = this; // create reciprocate link
       mScaleAbs =  mUp->mScaleAbs * mPyr->MulScale();  // geometric law
    }
    // Get possible image corresponding to same sigma in up octave
    if (mOct->Up() != nullptr)
    {
        mOverlapUp = mOct->Up()->ImageOfScaleAbs(mScaleAbs);
    }
    mScaleInO = mScaleAbs / anOct->Scale0Abs(); // This the relative sigma between different image

    // This is the "targeted" sigma we want to reach in the "absolute" (no decimation)
    //  scaled image it suppose that we know the  the sigma of first image, then simply multiply by it
    mTargetSigmAbs = mPyr->SigmIm0() * mScaleAbs;
   // For the value in octave, just divide by decimation factor
    mTargetSigmInO = mTargetSigmAbs / anOct->Scale0Abs();
}

     //  === Image processing methods

template <class Type> void cGP_OneImage<Type>::ComputGaussianFilter()
{
   if (mIsTopPyr)  // Case top image do not need filtering, except if specified in ConvolIm0
   {
       double aCI0 = mPyr->Params().mConvolIm0;
       if (aCI0!=0.0)
       {           
          ExpFilterOfStdDev(mImG.DIm(),mPyr->Params().mNbIter1,aCI0);
       }
   }
   else if (mOverlapUp != nullptr)  // Case were there exist an homologous image, use decimation
   {
       mImG.DecimateInThis(2,mOverlapUp->mImG);
   }
   else if (mUp != nullptr)  // Other case comput from Up image
   {
        // To maximize the gaussian aspect, compute from already filtered image
        double aSig = DifSigm(mTargetSigmInO,mUp->mTargetSigmInO);
        // First convulotion use more iteration as we dont benefit from accumulation
        int aNbIter = mPyr->Params().mNbIter1 - mUp->mNumInOct;
        // Be sure to have a minimum
        aNbIter = std::max(aNbIter,mPyr->Params().mNbIterMin);
        ExpFilterOfStdDev(mImG.DIm(),mUp->mImG.DIm(),aNbIter,aSig);
   }
   else  
   {
        // This should never happen by construction
        MMVII_INTERNAL_ASSERT_strong(mUp!=nullptr,"Up Image in ComputGaussianFilter");
   }
}

template <class Type> void cGP_OneImage<Type>::MakeCorner()
{
    // Compute Curvature tangent to level lines
    SelfCourbTgt(mImG);
    // Normalise of scale; theory && experiment show a dependance in pow 3
    SelfMulImageCsteInPlace(mImG.DIm(),pow(mScaleInO,3));
}



template <class Type> void cGP_OneImage<Type>::MakeDiff(const tGPIm &  aImDif)
{
   MMVII_INTERNAL_ASSERT_strong(aImDif.mDown!=nullptr,"Down Image in MakeDiff");

   DiffImageInPlace(mImG.DIm(),aImDif.mImG.DIm(),aImDif.mDown->mImG.DIm());
}

template <class Type> void cGP_OneImage<Type>::MakeOrigNorm(const tGPIm &  aOriGPI)
{
    // Create a duplicata of original image
    tIm aImBlur = aOriGPI.mImG.Dup();

    // Put in  aImBlur the convolution with  Gaussian filter
    ExpFilterOfStdDev(aImBlur.DIm(),3,mTargetSigmInO*mPyr->Params().mScaleDirOrig);
    // Put In Res, diff between gaussian filter an Ori
    DiffImageInPlace(mImG.DIm(),aOriGPI.mImG.DIm(),aImBlur.DIm());
    // Multiply by ScaleInO
    SelfMulImageCsteInPlace(mImG.DIm(),pow(mScaleInO,1));
}

     //  === Export

template <class Type> cPt2dr cGP_OneImage<Type>::Im2File(const cPt2dr & aP) const
{
    return mOct->Oct2File(aP);
}

template <class Type> void cGP_OneImage<Type>::SaveInFile() const
{
    mImG.DIm().ToFile(mNameSave);
    // MakeStdIm8BIts(mImG,mNameSave);
}

template <class Type> bool   cGP_OneImage<Type>::IsTopOct() const
{
    return  mUp == nullptr;
}

template <class Type> void cGP_OneImage<Type>::Show()  const
{
    StdOut() << "    "  << Id()  
             << " " << (mOverlapUp ? ("Ov:"+mOverlapUp->ShortId()) : "Ov:XXXXXX")  << "\n";
}

template <class Type> std::string cGP_OneImage<Type>::Id()  const
{
   return     
              " Oc:" + ToStr(mOct->NumInPyr()) 
          +   " Im:" + ToStr(mNumInOct) 
          +   " SOct:" + FixDigToStr(mScaleInO,4)   +  "/" + FixDigToStr(mTargetSigmInO,4)
          +   " SAbs:" + FixDigToStr(mScaleAbs,4)   +  "/" + FixDigToStr(mTargetSigmAbs,4)
          +   " T:"   + std::string(mIsTopPyr ? "1" : "0") 
   ;
}
template <class Type> std::string cGP_OneImage<Type>::ShortId()  const
{
   return     
              "o" + ToStr(mOct->NumInPyr()) +   "_i" + ToStr(mNumInOct)  ;
}

       // ==== Accessors ====
template <class Type> cIm2D<Type> cGP_OneImage<Type>::ImG() {return mImG;}
template <class Type> double  cGP_OneImage<Type>::ScaleAbs() const {return mScaleAbs;}
template <class Type> double  cGP_OneImage<Type>::ScaleInO() const {return mScaleInO;}

template <class Type> cGP_OneImage<Type> * cGP_OneImage<Type>::Up() const {return mUp;}
template <class Type> cGP_OneImage<Type> * cGP_OneImage<Type>::Down() const {return mDown;}
template <class Type> const std::string & cGP_OneImage<Type>::NameSave() const {return mNameSave;}
template <class Type> cGP_OneOctave<Type> * cGP_OneImage<Type>::Oct() const {return mOct;}

template <class Type> cGP_OneImage<Type> * cGP_OneImage<Type>::ImOriHom() 
{
   return mPyr->ImHomOri(this);
}

template <class Type> int  cGP_OneImage<Type>::NumInOct() const {return mNumInOct;}

/* ==================================================== */
/*                                                      */
/*              cGP_OneOctave                           */
/*                                                      */
/* ==================================================== */

template <class Type> cGP_OneOctave<Type>::cGP_OneOctave(tPyr * aPyr,int aNum,tOct * anOctUp) :
   mPyram (aPyr),
   mUp    (anOctUp),
   mDown  (nullptr),
   mNumInPyr  (aNum),
   mSzIm      (-1,-1)  // defined later
{
    if (mUp==nullptr)
    {
        mSzIm = aPyr->SzIm0();        // Sz Max
        mScale0Abs  = aPyr->Scale0();  // If Top Octave, set sigma top image to s0
    }
    else
    {
       mScale0Abs  = mUp->Scale0Abs() * 2;  // geometric law
       mSzIm = mUp->mSzIm / 2;            // octave law
       mUp->mDown = this; // create reciprocate link
    }
    // StdOut() << "=== Oct === " << mNumInPyr << " Sz=" << mSzIm << "\n";
    tGPIm * aPrec = nullptr;
    for (int aKIm=0 ; aKIm<mPyram->NbImByOct()  ; aKIm++)
    {
        mVIms.push_back(tSP_GPIm(new tGPIm(this,aKIm,aPrec)));
        aPrec = mVIms.back().get();
    }
}


template <class Type> cGP_OneImage<Type>* cGP_OneOctave<Type>::ImageOfScaleAbs(double aScale,double aTolRel)
{
   for (auto & aPtr : mVIms)
       if (RelativeDifference(aPtr->ScaleAbs(),aScale)<aTolRel)
          return  aPtr.get();
  return nullptr;
}

template <class Type> void cGP_OneOctave<Type>::MakeDiff(const tOct & anOct)
{
    for (int aKIm=0 ; aKIm<int(mVIms.size()) ; aKIm++)
    {
         mVIms.at(aKIm)->MakeDiff(*anOct.mVIms.at(aKIm));
    }
}

template <class Type> void cGP_OneOctave<Type>::MakeOrigNorm(const tOct & anOct)
{
    for (int aKIm=0 ; aKIm<int(mVIms.size()) ; aKIm++)
    {
         mVIms.at(aKIm)->MakeOrigNorm(*anOct.mVIms.at(aKIm));
    }
}


template <class Type> void cGP_OneOctave<Type>::ComputGaussianFilter()
{
   for (auto & aPtrIm : mVIms)
       aPtrIm->ComputGaussianFilter();
}

template <class Type> void cGP_OneOctave<Type>::Show() const
{
    StdOut() << "   --- Oct --- " << mNumInPyr << " Sz=" << mSzIm << "\n";
    for (const auto & aPtrIm : mVIms)
       aPtrIm->Show();
}

template <class Type> cPt2dr cGP_OneOctave<Type>::Oct2File(const cPt2dr & aP) const
{
    return mPyram->Pyr2File(aP*mScale0Abs);
}



template <class Type> cIm2D<Type> cGP_OneOctave<Type>::ImTop() const {return mVIms.at(0)->ImG();}
template <class Type> cGP_OneImage<Type>* cGP_OneOctave<Type>::GPImTop() const {return mVIms.at(0).get();}
    //  Accessors

template <class Type> cGaussianPyramid<Type>* cGP_OneOctave<Type>::Pyram() const {return mPyram;}
template <class Type> const cPt2di & cGP_OneOctave<Type>::SzIm() const {return mSzIm;}
template <class Type> const double & cGP_OneOctave<Type>::Scale0Abs() const {return mScale0Abs;}
template <class Type> cGP_OneOctave<Type>* cGP_OneOctave<Type>::Up() const {return mUp;}
template <class Type> const int & cGP_OneOctave<Type>::NumInPyr() const {return mNumInPyr;}
template <class Type> const  std::vector<std::shared_ptr<cGP_OneImage<Type>>> & cGP_OneOctave<Type>::VIms() const {return mVIms;}

/* ==================================================== */
/*                                                      */
/*              cFilterPCar                             */
/*                                                      */
/* ==================================================== */

cFilterPCar::cFilterPCar() :
   mAutoC  ({0.9}),
   mPSF    ({35,3.0,0.2}),
   mEQsf   ({2,1,1})
{
}

void cFilterPCar::FinishAC(double aVal)
{
    for (int aK=1 ; aK< 3 ; aK++)
    {
        if (int(mAutoC.size()) == aK)
           mAutoC.push_back(mAutoC.back()-0.05);
    }
}

const double & cFilterPCar::AC_Threshold() {return mAutoC.at(0);}
const double & cFilterPCar::AC_CutReal()   {return mAutoC.at(1);}
const double & cFilterPCar::AC_CutInt()    {return mAutoC.at(2);}

const double & cFilterPCar::DistSF()       {return mPSF.at(0);}
const double & cFilterPCar::MulDistSF()    {return mPSF.at(1);}
const double & cFilterPCar::PropNoSF()     {return mPSF.at(2);}

const double & cFilterPCar::PowAC()        {return mEQsf.at(0);}
const double & cFilterPCar::PowVar()       {return mEQsf.at(1);}
const double & cFilterPCar::PowScale()     {return mEQsf.at(2);}



/* ==================================================== */
/*                                                      */
/*              cGP_Params                              */
/*                                                      */
/* ==================================================== */

cGP_Params::cGP_Params(const cPt2di & aSzIm0,int aNbOct,int aNbLevByOct,int aOverlap) :
   mSzIm0        (aSzIm0),
   mNbOct        (aNbOct),
   mNbLevByOct   (aNbLevByOct),
   mNbOverlap    (aOverlap),
   mConvolIm0    (0.0),
   mNbIter1      (4),
   mNbIterMin    (2),
   mConvolC0     (1.0),
   mScaleDirOrig (4.0),
   mEstimSigmInitIm0  (DefStdDevImWellSample)
{
}


/* ==================================================== */
/*                                                      */
/*              cGaussianPyramid                        */
/*                                                      */
/* ==================================================== */


          //  - * - * - * - * - * - * - * - * - * - *
          //   Creators : Constructors + Allocators

template <class Type> cGaussianPyramid<Type>::cGaussianPyramid
                      (
                            const cGP_Params & aParams,
                            tPyr * aOrig,
                            eTyPyrTieP aType,
                            const std::string & aNameIm,
                            const std::string & aPrefix,
                            const cRect2 & aBIn,
                            const cRect2 & aBOut
                      ) :
   mPyrOrig           (aOrig ? aOrig : this),
   mTypePyr           (aType),
   mNameIm            (aNameIm),
   mPrefix            (aPrefix),
   mBoxIn             (aBIn),
   mBoxOut            (aBOut),
   mParams            (aParams),
   mMulScale          (pow(2.0,1/double(mParams.mNbLevByOct))),
   mScale0            (1.0),
   mEstimSigmInitIm0  (aParams.mEstimSigmInitIm0),
   mSigmIm0           (SomSigm(mEstimSigmInitIm0,mParams.mConvolIm0))
{
   tOct * aPrec = nullptr;
   for (int aKOct=0 ; aKOct<mParams.mNbOct ; aKOct++)
   {
       mVOcts.push_back(tSP_Oct(new tOct(this,aKOct,aPrec)));
       aPrec = mVOcts.back().get();
       std::copy(aPrec->VIms().begin(),aPrec->VIms().end(),std::back_inserter(mVAllIms));
   }

   // if (mPyrOrig==this)
   {
       StdOut() << "AAAAAPyyyyyyyrrrrr\n";
       for (int aK=0 ; aK<int(mVAllIms.size()) ; aK++)
       {
           tGPIm * aPIm = mVAllIms[aK].get();
           tOct  * aOct = aPIm->Oct();
           tOct  * aOctUp = aOct->Up();
           tGPIm * aPImUpEqui = (aOctUp ? aOctUp->ImageOfScaleAbs(aPIm->ScaleAbs()) : nullptr);

           StdOut() << "KKK " << aK 
                    <<  " O=" << aOct->NumInPyr() 
                    <<  " I=" << aPIm->NumInOct() 
                    <<  " S=" << ((aPImUpEqui==nullptr) ? "--" : "**")
                    << "\n";
       }
       getchar();
   }
}


template <class Type>  std::shared_ptr<cGaussianPyramid<Type>>  
      cGaussianPyramid<Type>::Alloc(const cGP_Params & aParams,const std::string& aNameIm,const std::string& aPref,const cRect2 & aBIn,const cRect2 & aBOut)
{
   return  tSP_Pyr(new tPyr(aParams,nullptr,eTyPyrTieP::eTPTP_Init,aNameIm,aPref,aBIn,aBOut));
}

template <class Type>  std::shared_ptr<cGaussianPyramid<Type>>  
      cGaussianPyramid<Type>::PyramDiff() 
{
    cGP_Params aParam = mParams;
    // Require one overlap less because  of diff
    aParam.mNbOverlap--;
    // Not Sure thi would create a problem, but it would not be very coherent ?
    MMVII_INTERNAL_ASSERT_strong(aParam.mNbOverlap>=0,"No overlap for PyramDiff");

    tSP_Pyr aRes(new tPyr(aParam,this,eTyPyrTieP::eTPTP_LaplG,mNameIm,mPrefix,mBoxIn,mBoxOut));

    for (int aKo=0 ; aKo<int(mVOcts.size()) ; aKo++)
    {
        aRes->mVOcts.at(aKo)->MakeDiff(*mVOcts.at(aKo));
    }

    return aRes;
}


template <class Type>  std::shared_ptr<cGaussianPyramid<Type>>
      cGaussianPyramid<Type>::PyramCorner() 
{
    cGP_Params aParam = mParams;
    aParam.mConvolIm0 = mParams.mConvolC0;
    aParam.mEstimSigmInitIm0 = mSigmIm0;

    //   mEstimSigmInitIm0  (aParam.mEstimSigmInitIm0),
    //   mSigmIm0           (SomSigm(mEstimSigmInitIm0,mParams.mConvolIm0))

    // Standard has created one overlap that we dont need
    aParam.mNbOverlap--;
    // Not Sure thi would create a problem, but it would not be very coherent ?
    MMVII_INTERNAL_ASSERT_strong(aParam.mNbOverlap>=0,"No overlap for PyramDiff");

    tSP_Pyr aRes(new tPyr(aParam,this,eTyPyrTieP::eTPTP_Corner,mNameIm,mPrefix,mBoxIn,mBoxOut));

    ImTop().DIm().DupIn(aRes->ImTop().DIm());
    aRes->ComputGaussianFilter();
    for (const auto & aSPIm : aRes->mVAllIms)
    {
        aSPIm->MakeCorner();
    }

    return aRes;
}

template <class Type>  std::shared_ptr<cGaussianPyramid<Type>>
      cGaussianPyramid<Type>::PyramOrigNormalize() 
{
    cGP_Params aParam = mParams;
    // Standard has created one overlap that we dont need
    aParam.mNbOverlap--;
    // Not Sure thi would create a problem, but it would not be very coherent ?
    MMVII_INTERNAL_ASSERT_strong(aParam.mNbOverlap>=0,"No overlap for PyramDiff");

    tSP_Pyr aRes(new tPyr(aParam,this,eTyPyrTieP::eTPTP_OriNorm,mNameIm,mPrefix,mBoxIn,mBoxOut));

    for (int aKo=0 ; aKo<int(mVOcts.size()) ; aKo++)
    {
        aRes->mVOcts.at(aKo)->MakeOrigNorm(*mVOcts.at(aKo));
    }

    return aRes;
}



          //  - * - * - * - * - * - * - * - * - * - *

template <class Type> void cGaussianPyramid<Type>::Show() const
{
   StdOut() << "============ Gaussian Pyramid ==============\n";
   StdOut() << "     type elem: " <<  E2Str(tElemNumTrait<Type>:: TyNum())  << "\n";
   for (const auto & aPtrOct : mVOcts)
       aPtrOct->Show();
}

template <class Type> void cGaussianPyramid<Type>::ComputGaussianFilter()
{
   for (const auto & aPtrOct : mVOcts)
       aPtrOct->ComputGaussianFilter();
}

template <class Type> cPt2dr cGaussianPyramid<Type>::Pyr2File(const cPt2dr & aP) const
{
    return ToR(mBoxIn.P0()) + aP;
}

template <class Type> void ScaleAndAdd
                           (
                                cInterf_ExportAimeTiep<Type> * aIExp,
                                const std::vector<cPt2di> & aLoc,
                                cGP_OneImage<Type> * aPtrI
                           )
{
    cAutoTimerSegm aATS("CreatePCar");
    for (const auto & aPtImInit : aLoc)
    {
       cProtoAimeTieP<Type>   aPATPCom(aPtrI,aPtImInit);
       aIExp->AddAimeTieP(aPATPCom);
    }
}

template <class Type> void cGaussianPyramid<Type>::SaveInFile (int aPowSPr,bool ForInspect) const
{
   bool DoPrint = (aPowSPr>=0) && ForInspect;

   std::unique_ptr<cInterf_ExportAimeTiep<Type>> aPtrExpMin(cInterf_ExportAimeTiep<Type>::Alloc(SzIm0(),true,int(mTypePyr),E2Str(mTypePyr),ForInspect,mParams));
   std::unique_ptr<cInterf_ExportAimeTiep<Type>> aPtrExpMax(cInterf_ExportAimeTiep<Type>::Alloc(SzIm0(),false,int(mTypePyr),E2Str(mTypePyr),ForInspect,mParams));

   if (DoPrint)
      StdOut() <<  "\n ######  STAT FOR " << E2Str(mTypePyr)  << " Pow " << aPowSPr << " ######\n";

   // std::vector<cPt2dr> aVGlobMin;
   // std::vector<cPt2dr> aVGlobMax;
   int aNbMinTot = 0;
   int aNbMaxTot = 0;

   for (auto & aPtrIm : mVAllIms)
   {
       if (ForInspect)
          aPtrIm->SaveInFile();
       {
           if (DoPrint && aPtrIm->IsTopOct())
              StdOut() <<  " ===================================================\n";
           cIm2D<Type> aI = aPtrIm->ImG();
           double aML =  MoyAbs(aI);
           double aS =  aPtrIm->ScaleInO();

           double aRadiusMM = 3.0;
           int aNbE = 0;
           if (DoPrint)
           {
              cResultExtremum aResE;
              {
                 cAutoTimerSegm aATS("1Extremum");
                 ExtractExtremum1(aI.DIm(),aResE,aRadiusMM);
              }
              aNbE = aResE.mPtsMin.size() + aResE.mPtsMax.size();

              StdOut()  <<  " Scale " <<  FixDigToStr(aS  ,2,2)
                       << " M- " << FixDigToStr(aML * pow(aS,aPowSPr-1),3,8 )
                       << " M= " << FixDigToStr(aML * pow(aS,aPowSPr  ),3,8 )
                       << " M+ " << FixDigToStr(aML * pow(aS,aPowSPr+1),3,8 )
                       << " E= " << FixDigToStr(aNbE * pow(aPtrIm->ScaleAbs(),1),6,2) ;
           }
           if (aPtrIm->Up() && aPtrIm->Down())
           {
               cResultExtremum aResE3;
               {
                  cAutoTimerSegm aATS("3Extremum");
                  ExtractExtremum3
                  (
                      aPtrIm->Up()->ImG().DIm(),
                      aPtrIm->ImG().DIm(),
                      aPtrIm->Down()->ImG().DIm(),
                      aResE3,
                      aRadiusMM
                  );
               }
               int aNbE3 = aResE3.mPtsMin.size() + aResE3.mPtsMax.size();
               if (DoPrint)
                  StdOut()  << " PROP=" << aNbE3 << " " << aNbE3 / double(aNbE) ;
               
               // tOct * aOctH = mPyrOrig->OctHom(aPtrIm->Oct());
               // std::string aNameMaster = aOctH->GPImTop()->NameSave();

               ScaleAndAdd(aPtrExpMin.get(),aResE3.mPtsMin,aPtrIm.get());
               ScaleAndAdd(aPtrExpMax.get(),aResE3.mPtsMax,aPtrIm.get());

               aNbMinTot += aResE3.mPtsMin.size();
               aNbMaxTot += aResE3.mPtsMax.size();
           }
        
           if (DoPrint)
              StdOut()  << "\n";
       }
   }
   StdOut() << " ======  NbTot , Min " << aNbMinTot << " Max " << aNbMaxTot << "\n";
   // std::string aPref =    "Aime-" + E2Str(TypePyr()) + "-" +  MMVII::Prefix(NameIm()) + "-" +mPrefix +".dmp";
   
   std::string aPref =     mParams.mPrefixSave + "-AimePCar-";
   std::string aPost =   E2Str(TypePyr()) + "-" +mPrefix +".dmp";

   aPtrExpMin->FiltrageSpatialPts();
   aPtrExpMax->FiltrageSpatialPts();

   aPtrExpMin->Export(aPref+"Min-"+ aPost);
   aPtrExpMax->Export(aPref+"Max-"+ aPost);
}

template <class Type>  cGP_OneOctave<Type> * cGaussianPyramid<Type>::OctHom(tOct *anOct)
{
   tOct * aRes = mVOcts.at(anOct->NumInPyr()).get();

   MMVII_INTERNAL_ASSERT_strong(RelativeDifference(aRes->Scale0Abs(),anOct->Scale0Abs())<1e-5,"OctHom");

   return aRes;
}

template <class Type>  cGP_OneImage<Type> * cGaussianPyramid<Type>::ImHom(tGPIm *anIm)
{
    tOct *  anOct = OctHom(anIm->Oct());
    tGPIm * aRes = anOct->ImageOfScaleAbs(anIm->ScaleAbs(),1e-5);
    MMVII_INTERNAL_ASSERT_strong(aRes!=nullptr,"ImHom");

    return aRes;
}

template <class Type>  cGP_OneImage<Type> * cGaussianPyramid<Type>::ImHomOri(tGPIm *anIm)
{
   return mPyrOrig->ImHom(anIm);
}

template <class Type> cIm2D<Type> cGaussianPyramid<Type>::ImTop() const {return mVOcts.at(0)->ImTop();}
template <class Type> cGP_OneImage<Type>* cGaussianPyramid<Type>::GPImTop() const {return mVOcts.at(0)->GPImTop();}

template <class Type> const cGP_Params & cGaussianPyramid<Type>::Params() const {return mParams;}
template <class Type> const double & cGaussianPyramid<Type>::MulScale() const {return mMulScale;}
template <class Type> int  cGaussianPyramid<Type>::NbImByOct() const {return  mParams.mNbLevByOct+mParams.mNbOverlap;}
template <class Type> const cPt2di & cGaussianPyramid<Type>::SzIm0() const {return  mParams.mSzIm0;}
template <class Type> const double & cGaussianPyramid<Type>::Scale0() const {return  mScale0;}
template <class Type> const double & cGaussianPyramid<Type>::SigmIm0() const {return mSigmIm0;}

template <class Type> const  std::vector<std::shared_ptr<cGP_OneOctave<Type>>> & cGaussianPyramid<Type>::VOcts() const {return mVOcts;}
template <class Type> const  std::vector<std::shared_ptr<cGP_OneImage<Type>>> & cGaussianPyramid<Type>::VAllIms() const {return mVAllIms;}

template <class Type> const std::string & cGaussianPyramid<Type>::NameIm() const {return mNameIm;}
template <class Type> const std::string & cGaussianPyramid<Type>::Prefix() const {return mPrefix;}
template <class Type> eTyPyrTieP cGaussianPyramid<Type>::TypePyr() const {return mTypePyr;}

/* ==================================================== */
/*              INSTANTIATION                           */
/* ==================================================== */

#define MACRO_INSTANTIATE_GaussPyram(Type) \
template class cGP_OneImage<Type>;\
template class cGP_OneOctave<Type>;\
template class cGaussianPyramid<Type>;\

MACRO_INSTANTIATE_GaussPyram(tREAL4)
MACRO_INSTANTIATE_GaussPyram(tINT2)



};
