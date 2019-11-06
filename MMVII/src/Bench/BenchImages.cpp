#include "include/MMVII_all.h"
#include "include/MMVII_Tpl_Images.h"

namespace MMVII
{



static int SomI(int i) {return (i * (i+1)) / 2;}
static int SomIntI(int i0,int i1) {return SomI(i1-1) - SomI(i0-1);}

static double FoncTestIm(const cPt2di & aP)
{
   return aP.x() - 1.234 * aP.y() + 1.0 * (1 +pow(std::abs(aP.x()),0.9) + std::pow(std::abs(aP.y()),1.4));
}

/*====================== IMAGE BENCH ===========================================*/
/*====================== IMAGE BENCH ===========================================*/
/*====================== IMAGE BENCH ===========================================*/

/**  Test that an image do what an image must do, store the value of a pixel
   and restor them when we ask for it (i.e GetV and SetV)
*/

template <class Type> void TestOneImage2D(const cPt2di & aP0,const cPt2di & aP1)
{
    // Test Id initialization + IndexeLinear + NormaliseCoord
    {
       cPt2di aSzR = aP1-aP0;
       cPt2di aSz(aSzR.x(),aSzR.x());

       cIm2D<Type> aPIm(aSz,nullptr,eModeInitImage::eMIA_MatrixId);
       cDataIm2D<Type>  & aIm = aPIm.DIm();
       int aCpt=0;
       for (const auto & aP : aIm)
       {
           Type aV1 = (aP.x()==aP.y());
           Type aV2 = aIm.GetV(aP);
           MMVII_INTERNAL_ASSERT_bench(aV1==aV2,"Bench image error");
           MMVII_INTERNAL_ASSERT_bench(aCpt==aIm.IndexeLinear(aP),"Bench image error");
           aCpt++;
           cPt2dr aPNorm = aIm.ToNormaliseCoord(aP);
           cPt2di aPAgain = aIm.FromNormaliseCoord(aPNorm);
           MMVII_INTERNAL_ASSERT_bench(aP==aPAgain,"Bench image error");
       }
    }
    {
       cPixBox<2> aRect(aP0,aP1);
       cIm2D<Type> aPIm(aP0,aP1);
       aPIm.DIm().Resize(aP0,aP1);
       aPIm.DIm().Resize(aP0-cPt2di(1,3),aP1+cPt2di(1,10));
       aPIm.DIm().Resize(aP0,aP1);

       cDataIm2D<Type>  & aIm = aPIm.DIm();
       aIm.InitRandom();
       cPt2di aPRand (aIm.GeneratePointInside());
       tINT8 aIndRand  = aIm.IndexeLinear(aPRand);
       MMVII_INTERNAL_ASSERT_bench(aIm.GetV(aPRand)==aIm.GetRDL(aIndRand),"Bench image error");

       aIm.InitNull();
       for (const auto & aP : aRect)
       {
           MMVII_INTERNAL_ASSERT_bench(aIm.GetV(aP)==0,"Bench image error");
       }
    }
    cPt2di aP0Pert = cPt2di(RandUnif_N(3), RandUnif_N(3));
    cPt2di aP1Pert = aP0Pert + cPt2di(1+RandUnif_N(3),1+RandUnif_N(3));
    cIm2D<Type> aPIm(aP0Pert,aP1Pert);
    aPIm.DIm().Resize(aP0,aP1);

    cDataIm2D<Type>  & aIm = aPIm.DIm();

    // Check the rectangle (number of point ....)
    int aNb = 0;
    int aNbX = 0;
    int aSomX = 0;
    int aSomY = 0;
    // for (aP.x()=aP0.x() ; aP.x()<aP1.x() ; aP.x()++)
    for (cPt2di aP =aP0 ; aP.x()<aP1.x() ; aP.x()++)
    {
        aNbX++;
        aSomX += aP.x();
        for (aP.y()=aP0.y() ; aP.y()<aP1.y() ; aP.y()++)
        {
            Type aV = tNumTrait<Type>::Trunc(FoncTestIm(aP));
            aIm.SetV(aP,aV);
            aNb++;
            aSomY += aP.y();
        }
    }
    cIm2D<Type> aIDup = aPIm.Dup();
    // Test sur le flux itere
    MMVII_INTERNAL_ASSERT_bench(aNbX==(aP1.x()-aP0.x()),"Bench image error");
    MMVII_INTERNAL_ASSERT_bench(aNb==(aP1.x()-aP0.x())*(aP1.y()-aP0.y()),"Bench image error");
    MMVII_INTERNAL_ASSERT_bench(aSomX==SomIntI(aP0.x(),aP1.x()),"Bench image error");
    MMVII_INTERNAL_ASSERT_bench(aSomY==(SomIntI(aP0.y(),aP1.y())*aNbX),"Bench image error");

    double aDif0 = 0.0; // Som of dif on raw function
    double aDifTr = 0.0; // Som on truncated function
    double aSomFonc = 0.0; // Will be used for iterator
    for (cPt2di aP=aP0 ; aP.y()<aP1.y() ; aP.y()++)
    {
        for (aP.x()=aP0.x() ; aP.x()<aP1.x() ; aP.x()++)
        {
            double aV0 = FoncTestIm(aP);
            aSomFonc += aV0;
            Type aVTrunc = tNumTrait<Type>::Trunc(aV0);
            Type aVIm = aIm.GetV(aP);
            Type aVDupIm = aIDup.DIm().GetV(aP);
            aDif0 += std::fabs(aV0-aVIm) + std::fabs(aVIm-aVDupIm);
            aDifTr += std::fabs(aVTrunc-aVIm);
        }
    }
    aDif0 /= aNb;
    aDifTr /= aNb;
    MMVII_INTERNAL_ASSERT_bench(aDifTr<1e-10,"Bench image error");
    if (!tNumTrait<Type>::IsInt())
    {
         MMVII_INTERNAL_ASSERT_bench(aDif0<1e-5,"Bench image error");
    }

    // Test on iterator, we check that we get the same sum on FoncTestIm than with
    // standard loop

    double aSomFonc2 = 0.0;
    double aSomFonc3 = 0.0;
    for (cPixBoxIterator<2> aP = aIm.begin() ; aP!=aIm.end() ; aP++)
    {
        aSomFonc2 += FoncTestIm(*aP);
        aSomFonc3 += FoncTestIm(cPt2di(aP->x(),aP->y()));
    }

    double aSomFonc4 = 0.0;
    for (const auto & aP : aIm)
    {
        aSomFonc4 += FoncTestIm(aP);
    }

    MMVII_INTERNAL_ASSERT_bench(std::abs(aSomFonc-aSomFonc2)<1e-10,"Bench image iterator");
    MMVII_INTERNAL_ASSERT_bench(std::abs(aSomFonc-aSomFonc3)<1e-10,"Bench image iterator");
    MMVII_INTERNAL_ASSERT_bench(std::abs(aSomFonc-aSomFonc4)<1e-10,"Bench image iterator");

}

    // Make test on operator, do not want to process overflow , just use  real types
template <class Type> void OperatorTestIm2D(const cPt2di & aP0,const cPt2di & aP1)
{
    {
       cIm2D<Type> aI1(aP0,aP1,nullptr,eModeInitImage::eMIA_Rand);
       cIm2D<Type> aI2 = aI1.Dup();
       cDataIm2D<Type> &aDI2 = aI2.DIm();
       
       aI2.DIm()  *= 2; // 2I1
       cIm2D<Type> aI3 = aI2*2 -aI1;  // 3 I1
       cDataIm2D<Type> &aDI3 = aI3.DIm();

       WeightedAddIn(aDI3,Type(0.5),aDI2);  // 4 I1
       for (const auto & aP : aI1.DIm())
       {
           double aDif = std::abs(aI3.DIm().GetV(aP)-4*aI1.DIm().GetV(aP));
           // StdOut() << "Dddd = " << aDif << "\n";
           MMVII_INTERNAL_ASSERT_bench(aDif<1e-5,"Bench image error");
       }
    }
}

template <class Type> void TestOneImage2D()
{
   TestOneImage2D<Type>(cPt2di(2,3),cPt2di(8,5));
   TestOneImage2D<Type>(cPt2di(5,3),cPt2di(8,5));

   TestOneImage2D<Type>(cPt2di(-2,-3),cPt2di(8,5));
   TestOneImage2D<Type>(cPt2di(-5,-3),cPt2di(8,5));

   TestOneImage2D<Type>(cPt2di(-35,-32),cPt2di(-20,-25));
   TestOneImage2D<Type>(cPt2di(-35,-32),cPt2di(-28,-25));
}


template <class Type> void TestInterBL(cPt2di aSz,Type aCste,Type aCoeffX,Type aCoeffY,Type aCXY)
{
    cIm2D<Type> aIm(aSz);
    cDataIm2D<Type>& aDIm(aIm.DIm());

    for (const auto & aP : aDIm)
    {
        Type aVal = aCste + aCoeffX * aP.x() + aCoeffY * aP.y() + aCXY * aP.x() * aP.y();
        aDIm.SetV(aP,aVal);
    }

    for (int aNb=0 ; aNb<10000 ; aNb++)
    {
        double UnMinEpsilon = 0.99999;
        cPt2dr aP(RandUnif_0_1() * (aSz.x()-1),RandUnif_0_1()*(aSz.y()-1));
        aP = UnMinEpsilon * aP;
        double aV1 = aCste + aCoeffX * aP.x() + aCoeffY * aP.y() + aCXY * aP.x() * aP.y();
        double aV2 = aDIm.GetVBL(aP);
        if(std::abs(aV1-aV2)>1e-5)
        {
             StdOut() << aP << "V1 " << aV1 << " dif " << aV1-aV2 << "\n";
             MMVII_INTERNAL_ASSERT_bench(false,"Bench image error");
        }
    }
}

void BenchGlobImage2d()
{
    cMemState  aState = cMemManager::CurState() ;
    {
         TestInterBL<tINT1>(cPt2di(10,10),-3,2,-5,0);
         TestInterBL<tU_INT1>(cPt2di(10,10), 3,5, 2,0);

         TestInterBL<tINT1>(cPt2di(8,8),-3,2,-5,-1);
         TestInterBL<tU_INT1>(cPt2di(8,8), 3,5, 2,1);

         TestInterBL<tREAL4>(cPt2di(10,10),-3.14,2.12,-5.988,-1.677);
         TestInterBL<tREAL8>(cPt2di(10,10), 3.89,5.73, 2.0001,1.007);
    }
    {
        TestOneImage2D<tREAL4>(cPt2di(2,2),cPt2di(4,5));

        TestOneImage2D<tREAL4>(cPt2di(0,0),cPt2di(10,10));
        TestOneImage2D<tREAL8>(cPt2di(0,0),cPt2di(10,10));
        TestOneImage2D<tREAL16>(cPt2di(0,0),cPt2di(10,10));


        OperatorTestIm2D<tREAL4>(cPt2di(-2,3),cPt2di(9,8));
        OperatorTestIm2D<tREAL8>(cPt2di(2,-3),cPt2di(5,7));

/*
*/
        TestOneImage2D<tINT1>();
        TestOneImage2D<tINT2>();
        TestOneImage2D<tINT4>();

        TestOneImage2D<tU_INT1>();
        TestOneImage2D<tU_INT2>();
        TestOneImage2D<tU_INT4>();
        TestOneImage2D<tREAL4>();
        TestOneImage2D<tREAL8>();
/*
*/
/*
        TestOneImage2D<tINT2>(cPt2di(2,3),cPt2di(8,5));
        TestOneImage2D<tINT2>(cPt2di(5,3),cPt2di(8,5));

        TestOneImage2D<tINT2>(cPt2di(-2,-3),cPt2di(8,5));
        TestOneImage2D<tINT2>(cPt2di(-5,-3),cPt2di(8,5));

        TestOneImage2D<tINT2>(cPt2di(-35,-32),cPt2di(-20,-25));
        TestOneImage2D<tINT2>(cPt2di(-35,-32),cPt2di(-28,-25));
*/
    }
    cMemManager::CheckRestoration(aState);
}


/*====================== FILE BENCH ===========================================*/
/*====================== FILE BENCH ===========================================*/
/*====================== FILE BENCH ===========================================*/

template <class TypeFile>  void TplBenchReadGlobIm(const std::string & aNameTiff,const cDataIm2D<TypeFile> & aDImDup)
{
   cIm2D<TypeFile> aICheck =  cIm2D<TypeFile>::FromFile(aNameTiff);
   MMVII_INTERNAL_ASSERT_bench(aICheck.DIm().Sz()==aDImDup.Sz(),"Bench image error");
   // for (const auto & aP : aFileIm)
   for (const auto & aP : aDImDup)
   {
       TypeFile aV1 = aDImDup.GetV(aP);
       TypeFile aV2 = aICheck.DIm().GetV(aP);
       MMVII_INTERNAL_ASSERT_bench(aV1==aV2,"Bench image error");
   }
}


template <class TypeImage,class tBase,class TypeFile>  void TplBenchFileImage(const cPt2di& aSz,double aDyn0)
{
    std::string aNameTiff = cMMVII_Appli::CurrentAppli().TmpDirTestMMVII() + "Test.tif";
    eTyNums aTypeF2 = tElemNumTrait<TypeFile>::TyNum();
    cDataFileIm2D  aFileIm = cDataFileIm2D::Create(aNameTiff,aTypeF2,aSz,1);
    // This image will be a copy of file
    cIm2D<TypeFile> aIDup(aSz);

    
    cDataIm2D<TypeFile> & aDImDup = aIDup.DIm();
    for (const auto & aP : aFileIm)
    {
        aDImDup.SetVTrunc(aP,FoncTestIm(aP));
    }
    aIDup.Write(aFileIm,cPt2di(0,0));  // Now File & Im contain the same data


    // Check write image
    TplBenchReadGlobIm(aNameTiff,aDImDup);
 
    // Ecriture avec des images origine en (0,0)
    for (int aK=0 ; aK<3 ; aK++)
    {
         double aDyn = aDyn0 * 2.0 * RandUnif_0_1();
         cRect2 aR (aFileIm.GenerateRectInside(3.0));
         cPt2di aP0 = aR.P0();
         cIm2D<TypeImage> aIW(aR.Sz());
         aIW.DIm().InitRandom();
         for (const auto & aP : aIW.DIm())
         {
            aDImDup.SetVTrunc(aP+aP0,aDyn*aIW.DIm().GetV(aP));
         }
         aIW.Write(aFileIm,aP0,aDyn);
        
    }
    TplBenchReadGlobIm(aNameTiff,aDImDup);

    // Ecriture avec des images origine variable
    for (int aK=0 ; aK<3 ; aK++)
    {
         double aDyn = aDyn0 * 2.0 * RandUnif_0_1();
         cRect2 aR (aFileIm.GenerateRectInside(3.0));
         cIm2D<TypeImage> aIW(aR.P0(),aR.P1());
         aIW.DIm().InitRandom();
         for (const auto & aP : aIW.DIm())
         {
            aDImDup.SetVTrunc(aP,aDyn*aIW.DIm().GetV(aP));
         }
         aIW.Write(aFileIm,cPt2di(0,0),aDyn);
        
    }
    TplBenchReadGlobIm(aNameTiff,aDImDup);

    // Lecture avec des image en (0,0)
    for (int aK=0 ; aK<20 ; aK++)
    {
         double aDyn = (1/aDyn0) * 2.0 * RandUnif_0_1();
         cRect2 aR (aFileIm.GenerateRectInside(3.0));
         cPt2di aP0 = aR.P0();
         cIm2D<TypeImage> aIW(aR.Sz());
         aIW.Read(aFileIm,aP0,aDyn);
         for (const auto & aP : aIW.DIm())
         {
             TypeImage aV1= aIW.DIm().GetV(aP) ;
             TypeImage aV2= tNumTrait<TypeImage>::Trunc( aDImDup.GetV(aP+aP0) *aDyn);
             MMVII_INTERNAL_ASSERT_bench(aV1==aV2,"Bench image error");
         }
        
    }
}

template <class TypeFile,class TypeImage>  void TplBenchFileImage()
{
    TplBenchFileImage<tU_INT1,tINT4,tINT4>(cPt2di(1000,500),100.0);
    TplBenchFileImage<tINT4,tINT4,tINT2>(cPt2di(1000,500),1e-2);
}

void BenchFileImage()
{
    TplBenchFileImage<tU_INT1,tINT4>();
}

/* ========================== */
/*          BenchIm1D         */
/* ========================== */

static double FoncTestIm(const double & aP)  {return aP + 1/(1+aP*aP);}
void TestInitIm1D(int aX0, int aX1)
{
  static int aCpt=0; aCpt++;
  cIm1D<double> aI(aX0+1,aX1-1);
  aI.DIm().Resize(cPt1di(aX0),cPt1di(aX1));
  if (aCpt%2)
  {
     aI.DIm().Resize(cPt1di(aX0-10),cPt1di(aX1+10));
     aI.DIm().Resize(cPt1di(aX0),cPt1di(aX1));
  }
  // cIm1D<double> aI(aX0,aX1);

  cDataIm1D<double> & aDI = aI.DIm();
  for (int aX=aX0 ; aX<aX1 ; aX++)
  {
     aDI.SetV(aX,FoncTestIm(aX));
  }
  if (0)
     aDI.SetV(aX1,0);  // detected in AssertInside
  if (0)
     aDI.SetV(aX0-1,0);  // detected in AssertInside
  if (0)
     aDI.RawDataLin()[-1] = 0; // detected in Free

  cIm1D<double> aDup = aI.Dup();
  for (int aX=aX0 ; aX<aX1 ; aX++)
  {
     double aV0 = aDI.GetV(aX);
     double aV1 = FoncTestIm(aX);
     double aV2 = aDup.DIm().GetV(aX);
     double aDif = std::abs(aV0-aV1);
     MMVII_INTERNAL_ASSERT_bench(aDif<1e-15,"Bench image-1D error");
     MMVII_INTERNAL_ASSERT_bench(std::abs(aV0-aV2)<1e-15,"Bench image-1D error");
  }
}

void BenchIm1D()
{
    if (0)  // Not freed
    {
       new cIm1D<double>(10);
    }
    if (1)
    {
        TestInitIm1D(0,10);
        TestInitIm1D(10,100);
        TestInitIm1D(-110,100);
    }
}

/* ========================== */
/*          BenchGlobImage    */
/* ========================== */


void BenchGlobImage()
{
    cIm2D<double> aI(cPt2di(1,1));
    if (1)
    {
        aI.DIm().SetV(cPt2di(0,0),1);
    }
 
    BenchIm1D();
    BenchFileImage();
    BenchRectObj();
    BenchBaseImage();
    BenchGlobImage2d();
}




};
