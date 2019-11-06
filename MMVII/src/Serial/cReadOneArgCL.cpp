#include "include/MMVII_all.h"

namespace MMVII
{

/* ========================== */
/*          cSemA2007         */
/* ========================== */

cSemA2007::cSemA2007(eTA2007 aType,const std::string & anAux) :
   mType      (aType),
   mAuxA2007  (anAux)
{
}

cSemA2007::cSemA2007(eTA2007 aType) :
   cSemA2007(aType,"")
{
}

eTA2007  cSemA2007::Type()            const {return mType;}
const std::string &  cSemA2007::AuxA2007() const {return mAuxA2007;}

std::string  cSemA2007::Name4Help() const
{
   if (int(mType) < int(eTA2007::Shared))
   {
      return E2Str(mType) + mAuxA2007;
   }

   return "";
}



/* ========================== */
/*          cSpecOneArg2007   */
/* ========================== */

const std::vector<cSemA2007>   cSpecOneArg2007::TheEmptySem;

cSpecOneArg2007::cSpecOneArg2007(const std::string & aName,const std::string & aCom,const tVSem & aVSem) :
   mName (aName),
   mCom  (aCom),
   mVSem (aVSem)
{
    ReInit();
}

cSpecOneArg2007::~cSpecOneArg2007()
{
}

void cSpecOneArg2007::ReInit()
{
   mNbMatch = 0;
}

std::string  cSpecOneArg2007::Name4Help() const
{
   std::string aRes;
   int aNb=0;
   for (const auto & aSem : mVSem)
   {
      std::string aStr = aSem.Name4Help();
      if (aStr!="")
      {
         if (aNb==0)
            aRes = " [";
         else
            aRes = aRes + ",";
         aRes = aRes + aStr;
         aNb++;
      }
   }
   if (aNb!=0)
      aRes += "]";
   return aRes;
}


void cSpecOneArg2007::IncrNbMatch()
{
   mNbMatch++;
}

int  cSpecOneArg2007::NbMatch () const
{
   return mNbMatch;
}

const cSpecOneArg2007::tVSem & cSpecOneArg2007::VSem() const
{
   return mVSem;
}

bool cSpecOneArg2007::HasType(const eTA2007 & aType,std::string * aValue) const
{
    for (const auto & aSem : mVSem)
    {
       if (aSem.Type() == aType)
       {
          if (aValue) 
             *aValue =  aSem.AuxA2007();
          return true;
       }
   }

    return false;
}

const std::string  & cSpecOneArg2007::Name() const
{
   return mName;
}

const std::string  & cSpecOneArg2007::Value() const
{
   return mValue;
}

const std::string  & cSpecOneArg2007::Com() const
{
   return mCom;
}

void  cSpecOneArg2007::InitParam(const std::string & aStr) 
{
   mValue = aStr;
   V_InitParam(aStr);
}


/* ============================ */
/*          cCollecSpecArg2007  */
/* ============================ */


cCollecSpecArg2007 & cCollecSpecArg2007::operator << (tPtrArg2007 aVal)
{
    mV.push_back(aVal);
    return *this; 
}

cCollecSpecArg2007::cCollecSpecArg2007()
{
}

size_t cCollecSpecArg2007::size() const
{
   return mV.size();
}

tPtrArg2007 cCollecSpecArg2007::operator [] (int aK) const
{
   return mV.at(aK);
}

void cCollecSpecArg2007::clear()
{
   mV.clear();
}

tVecArg2007 & cCollecSpecArg2007::Vec()
{
   return mV;
}




/* ============================================== */
/*                                                */
/*       cInstReadOneArgCL2007                    */
/*                                                */
/* ============================================== */

template <class Type> void  GlobCheckSize(const Type & ,const std::string & anArg) 
{
    MMVII_INTERNAL_ASSERT_always(false,"Check size vect for non vect arg");
}

template <class Type> void  GlobCheckSize(const std::vector<Type> & aVal,const std::string & anArg) 
{
    cPt2di aSz = cStrIO<cPt2di>::FromStr(anArg);
    if ((int(aVal.size()) < aSz.x()) || ((int(aVal.size()) > aSz.y()))) 
    {
       MMVII_UsersErrror(eTyUEr::eBadSize4Vect,"IntervalOk=" + anArg + " Got=" + ToStr(int(aVal.size())));
    }
}


template <class Type> class cInstReadOneArgCL2007 : public cSpecOneArg2007
{
    public :

       void  CheckSize(const std::string & anArg) const override 
       {
               GlobCheckSize(mVal,anArg);
       }


        void V_InitParam(const std::string & aStr) override
        {
            mVal = cStrIO<Type>::FromStr(aStr);
        }
        cInstReadOneArgCL2007 (Type & aVal,const std::string & aName,const std::string & aCom,const tVSem & aVSem) :
              cSpecOneArg2007(aName,aCom,aVSem),
              mVal         (aVal)
        {
        }
        const std::string & NameType() const override 
        {
            return  cStrIO<Type>::msNameType;
        }
        void * AdrParam() override {return &mVal;}
        std::string NameValue() const override {return ToStr(mVal);}

    private :
        Type &          mVal;
};


template <class Type> tPtrArg2007 Arg2007(Type & aVal, const std::string & aCom,const cSpecOneArg2007::tVSem & aVSem )
{
   return tPtrArg2007(new cInstReadOneArgCL2007<Type>(aVal,"",aCom,aVSem));
}




template <class Type> tPtrArg2007 AOpt2007(Type & aVal,const std::string & aName, const std::string &aCom,const cSpecOneArg2007::tVSem & aVSem)
{
   return  tPtrArg2007(new cInstReadOneArgCL2007<Type>(aVal,aName,aCom,aVSem));
}

#define MACRO_INSTANTIATE_ARG2007(Type)\
template tPtrArg2007 Arg2007<Type>(Type &, const std::string & aCom,const cSpecOneArg2007::tVSem & aVSem);\
template tPtrArg2007 AOpt2007<Type>(Type &,const std::string & aName, const std::string & aCom,const cSpecOneArg2007::tVSem & aVSem);

MACRO_INSTANTIATE_ARG2007(int)
MACRO_INSTANTIATE_ARG2007(double)
MACRO_INSTANTIATE_ARG2007(bool)
MACRO_INSTANTIATE_ARG2007(std::string)
MACRO_INSTANTIATE_ARG2007(std::vector<std::string>)
MACRO_INSTANTIATE_ARG2007(std::vector<int>)
MACRO_INSTANTIATE_ARG2007(std::vector<double>)
MACRO_INSTANTIATE_ARG2007(cPt2di)
MACRO_INSTANTIATE_ARG2007(cPt2dr)
MACRO_INSTANTIATE_ARG2007(cPt3di)
MACRO_INSTANTIATE_ARG2007(cPt3dr)

/*
template <> tPtrArg2007 AOpt2007<int>(int &,const std::string & aName, const std::string & aCom);

template <> tPtrArg2007 Arg2007<double>(double &, const std::string & aCom);
template <> tPtrArg2007 AOpt2007<double>(double &,const std::string & aName, const std::string & aCom);

template <> tPtrArg2007 Arg2007<std::string>(std::string &, const std::string & aCom);
template <> tPtrArg2007 AOpt2007<std::string>(std::string &,const std::string & aName, const std::string & aCom);

template <> tPtrArg2007 Arg2007<bool>(bool &, const std::string & aCom);
template <> tPtrArg2007 AOpt2007<bool>(bool &,const std::string & aName, const std::string & aCom);
*/




};
