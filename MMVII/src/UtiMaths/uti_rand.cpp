#include "include/MMVII_all.h"
#include <random>

/** \file uti_rand.cpp
    \brief Implementation of random generator

    Use C++11, implement a very basic interface,
    will evolve probably when object manipulation
    will be needed for more sophisticated services.

*/


namespace MMVII
{

void AssertIsSetKN(int aK,int aN,const std::vector<int> &aSet)
{
  MMVII_INTERNAL_ASSERT_bench(int(aSet.size())==aK,"Random Set");
  for (const auto & aE : aSet)
  {
       MMVII_INTERNAL_ASSERT_bench((aE>=0) && (aE<aN),"Random Set");
       int aNbEq=0;
       for (const auto & aE2 : aSet)
       {
           if (aE==aE2)
              aNbEq++;
       }
       MMVII_INTERNAL_ASSERT_bench(aNbEq==1,"Random Set");
  }
}

void Bench_Random()
{
   for (int aTime=0 ; aTime< 100 ; aTime++)
   {
       int aNb = 10 + aTime/10;
       int aK = 3+aTime/10;
       std::vector<int> aSet =  RandSet(aK,aNb);
       AssertIsSetKN(aK,aNb,aSet);
       for (int aD=1 ; aD<=3 ; aD++)
       {
          aSet = RandNeighSet(aD,aNb,aSet);
          AssertIsSetKN(aK,aNb,aSet);
       }
   }
   StdOut() << "Begin Bench_Random\n";
   {
      int aNb = 1e6;
      std::vector<double> aVR;
      for (int aK=0 ; aK< aNb ; aK++)
          aVR.push_back(RandUnif_0_1());

      std::sort(aVR.begin(),aVR.end());

      double aDistMoy=0;
      double aDistMax=0;
      double aCorrel = 0;
      double aCorrel10 = 0;
      for (int aK=0 ; aK< aNb ; aK++)
      {
          double aD = std::abs(aVR[aK] - aK/double(aNb));
          aDistMoy += aD;
          aDistMax = std::max(aD,aDistMax);
          if (aK!=0)
             aCorrel += (aVR[aK]-0.5) * (aVR[aK-1]-0.5);
          if (aK>=10)
             aCorrel10 += (aVR[aK]-0.5) * (aVR[aK-10]-0.5);
      }
      aDistMoy /= aNb;
      aCorrel /= aNb-1;
      aCorrel10 /= aNb-10;
      // Purely heuristique bound, on very unlikely day may fail
      MMVII_INTERNAL_ASSERT_bench(aDistMoy<1.0/sqrt(aNb),"Random Moy Test");
      MMVII_INTERNAL_ASSERT_bench(aDistMax<4.0/sqrt(aNb),"Random Moy Test");

      // => Apparently correlation is very high : 0.08 !! maybe change the generator ?
      
   }
}



/// class cRandGenerator maybe exported later if  more sophisticated services are required

class cRandGenerator : public cMemCheck
{
   public :
       virtual double Unif_0_1() = 0;
       virtual int    Unif_N(int aN) = 0;
       static cRandGenerator * TheOne();
       virtual ~cRandGenerator(){};
    private :
       static cRandGenerator * msTheOne;
};

void FreeRandom() 
{
   delete cRandGenerator::TheOne();
}
double RandUnif_0_1()
{
   return cRandGenerator::TheOne()->Unif_0_1();
}

std::vector<double> VRandUnif_0_1(int aNb)
{
    std::vector<double> aRes;
    for (int aK=0 ; aK<aNb ; aK++)
        aRes.push_back(RandUnif_0_1());
    return aRes;
}


double RandUnif_C()
{
   return (RandUnif_0_1()-0.5) * 2.0;
}

double RandUnif_N(int aN)
{
   return cRandGenerator::TheOne()->Unif_N(aN);
}

bool HeadOrTail()
{
     return  RandUnif_0_1() > 0.5;
}


/*
class cFctrRR
{
   public :
      virtual  double F (double) const;
      static cFctrRR  TheOne;
};
*/

cFctrRR cFctrRR::TheOne;
double cFctrRR::F(double) const {return 1.0;}

std::vector<int> RandSet(int aK,int aN,cFctrRR & aBias )
{
    MMVII_INTERNAL_ASSERT_strong(aK<=aN,"RandSet");
    std::vector<cPt2dr> aVP;
    for (int aK=0; aK<aN ; aK++)
       aVP.push_back(cPt2dr(aK,aBias.F(aK)*RandUnif_0_1()));
    // Sort on y()
    std::sort(aVP.begin(),aVP.end(),CmpCoord<double,2,1>);
  
    std::vector<int> aRes;
    for (int aJ=0 ; aJ<aK ; aJ++)
       aRes.push_back(round_ni(aVP.at(aJ).x()));
    return aRes;
}

std::vector<int> RandPerm(int aN,cFctrRR & aBias)
{
    return RandSet(aN,aN,aBias);
}

int MaxElem(const std::vector<int> & aSet)
{
   int aRes=-1;
   for (const auto & anElem : aSet)
   {
      aRes = std::max(aRes,anElem);
   }
   return aRes;
}

std::vector<int> ComplemSet(int aN,const std::vector<int> & aSet)
{
   std::vector<bool> aVBelong(std::max(aN,MaxElem(aSet)+1),false);
   for (const auto & anElem : aSet)
   {
      aVBelong.at(anElem) = true;
   }

   std::vector<int>  aRes;
   for (int aK=0 ; aK<int(aVBelong.size()) ; aK++)
      if (!aVBelong.at(aK))
         aRes.push_back(aK);

   return aRes;
}

std::vector<int> RandNeighSet(int aK,int aN,const std::vector<int> & aSet)
{
    std::vector<int> aComp = ComplemSet(aN,aSet);
    std::vector<int> aIndToAdd = RandSet(aK,aComp.size());
    std::vector<int> aIndToSupr = RandSet(aK,aSet.size());

    std::vector<int> aRes = aSet;
    for (int aJ=0 ; aJ<aK ; aJ++)
        aRes.at(aIndToSupr.at(aJ)) = aComp.at(aIndToAdd.at(aJ));

   return aRes;
}





/// class cRand19937 concrete implemenation

class cRand19937 : public cRandGenerator
{
     public :
         cRand19937(int aSeed);
         double Unif_0_1() override ;
         int    Unif_N(int aN) override;
         ~cRand19937() {}
     private :
          // std::random_device mRD;                    //Will be used to obtain a seed for the random number engine
          std::mt19937       mGen;                   //Standard mersenne_twister_engine seeded with rd()
          std::uniform_real_distribution<> mDis01;
          std::unique_ptr<std::uniform_int_distribution<> > mDisInt;
          int                                               mLastN;
          //uniform_01<mt19937> mU01;
};


cRand19937::cRand19937(int aSeed) :
    // mRD      (),
    mGen     (aSeed),
    mDis01   (0.0,1.0),
    mDisInt  (nullptr)
{
}

double cRand19937::Unif_0_1()
{
    return mDis01(mGen);
}

int    cRand19937::Unif_N(int aN) 
{
   if ((mDisInt==nullptr) || (mLastN!=aN))
   {
       mLastN = aN;
       mDisInt.reset(new  std::uniform_int_distribution<>(0,aN-1));
   }

   return (*mDisInt)(mGen);
}



cRandGenerator * cRandGenerator::msTheOne = nullptr;

cRandGenerator * cRandGenerator::TheOne()
{
   if (msTheOne==0)
      msTheOne = new cRand19937(cMMVII_Appli::SeedRandom());
   return msTheOne;
}


};

