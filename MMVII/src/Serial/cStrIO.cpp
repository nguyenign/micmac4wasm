#include "include/MMVII_all.h"

namespace MMVII
{

/**
    This file contains the implemenation of conversion between strings and 
   atomic object
*/

static char BufStrIO[1000];

//  vector<int>  => [1,2,3]

template <class Type>  std::string Vect2Str(const std::vector<Type>  & aV)
{
   std::string aRes ="[";
   for (int aK=0 ; aK<(int)aV.size() ; aK++)
   {
      if (aK>0)
         aRes += ",";
      aRes += ToStr(aV[aK]);
   }
   aRes += "]";
   return aRes;
}

template <class Type>  std::vector<Type> Str2Vec(const std::string & aStrGlob)
{
   std::vector<Type> aRes;
   const char * aC=aStrGlob.c_str();
   if (*aC!='[')
       MMVII_UsersErrror(eTyUEr::eParseError,"expected [ at beging of vect");
   aC++;
   while((*aC) && *aC!=']')
   {
       std::string aStrV;
       while ((*aC) && (*aC!=',') && (*aC!=']'))
          aStrV += *(aC++);
       if (!(*aC))
          MMVII_UsersErrror(eTyUEr::eParseError,"unexpected end of string while expecting \",\"");
       aRes.push_back(cStrIO<Type>::FromStr(aStrV)); 
       if (*aC==',')
          aC++;
   }
   if (*aC!=']')
      MMVII_UsersErrror(eTyUEr::eParseError,"unexpected end of string while expecting \"]\"");
   aC++;

   return  aRes;
}

                          //   - - std::vector<Type>  - -

#define MACRO_INSTANTITATE_STRIO_VECT_TYPE(TYPE)\
template <>  std::string cStrIO<std::vector<TYPE>>::ToStr(const std::vector<TYPE>  & aV)\
{\
   return  Vect2Str(aV);\
}\
template <>  std::vector<TYPE> cStrIO<std::vector<TYPE> >::FromStr(const std::string & aStr)\
{\
    return Str2Vec<TYPE>(aStr);\
}\
template <>  const std::string cStrIO<std::vector<TYPE>>::msNameType = "std::vector<"  #TYPE  ">";\

MACRO_INSTANTITATE_STRIO_VECT_TYPE(std::string)
MACRO_INSTANTITATE_STRIO_VECT_TYPE(int)
MACRO_INSTANTITATE_STRIO_VECT_TYPE(double)

                          //   - - cPtxd  - -


#define MACRO_INSTANTITATE_STRIO_CPTXD(TYPE,DIM)\
template <>  std::string cStrIO<cPtxd<TYPE,DIM> >::ToStr(const cPtxd<TYPE,DIM>  & aV)\
{\
  return Vect2Str(std::vector<TYPE>(aV.PtRawData(),aV.PtRawData()+cPtxd<TYPE,DIM>::TheDim));\
}\
template <>  cPtxd<TYPE,DIM> cStrIO<cPtxd<TYPE,DIM> >::FromStr(const std::string & aStr)\
{\
    std::vector<TYPE> aV = cStrIO<std::vector<TYPE>>::FromStr(aStr);\
    if (aV.size()!=DIM)\
       MMVII_UsersErrror(eTyUEr::eBadDimForPt,"Expect="+ MMVII::ToStr(DIM) + " Got=" + MMVII::ToStr(int(aV.size())) );\
    cPtxd<TYPE,DIM> aRes;\
    for (int aK=0 ; aK<DIM ; aK++)\
        aRes[aK] = aV[aK];\
    return aRes;\
}\
template <>  const std::string cStrIO<cPtxd<TYPE,DIM> >::msNameType = "cPtxd<" #TYPE ","  #DIM ">";\

MACRO_INSTANTITATE_STRIO_CPTXD(int,2)
MACRO_INSTANTITATE_STRIO_CPTXD(double,2)
MACRO_INSTANTITATE_STRIO_CPTXD(int,3)
MACRO_INSTANTITATE_STRIO_CPTXD(double,3)



/*
template <>  std::string cStrIO<cPtxd<double,2> >::ToStr(const cPtxd<double,2>  & aV)
{
  return Vect2Str(std::vector<double>(aV.PtRawData(),aV.PtRawData()+cPtxd<double,2>::TheDim));
}
template <>  cPtxd<double,2> cStrIO<cPtxd<double,2> >::FromStr(const std::string & aStr)
{
    std::vector<double> aV = cStrIO<std::vector<double>>::FromStr(aStr);
    cPtxd<double,2> aRes;
    for (int aK=0 ; aK<2 ; aK++)
        aRes[aK] = aV[aK];
    return aRes;
}
template <>  const std::string cStrIO<cPtxd<double,2> >::msNameType = "cPt2dr";
*/



void OneBenchStrIO(std::string aStr,const  std::vector<std::string> & aV)
{
   // std::string aStr =
   std::vector<std::string> aV2 =  cStrIO<std::vector<std::string> >::FromStr(aStr);
   if(aV!=aV2)
   {
      StdOut() << "STR=" << aStr << "\n";
      StdOut() << "VEC=[";
      for (int aK=0 ; aK<int(aV2.size()) ; aK++)
      {
         if (aK!=0) StdOut() << ",";
         StdOut() << "{" << aV2[aK] << "}";
      }
      StdOut() << "]\n";
      MMVII_INTERNAL_ASSERT_bench((aV==aV2),"OneBenchStrIO");
   }
}

void BenchStrIO()
{
   OneBenchStrIO("[1,2,3]",{"1","2","3"});
   OneBenchStrIO("[1]",{"1"});
   OneBenchStrIO("[]",{});
   OneBenchStrIO("[,1]",{"","1"});
   OneBenchStrIO("[,,,1]",{"","","","1"});
   OneBenchStrIO("[,]",{""});
   OneBenchStrIO("[1,2,]",{"1","2"});
   OneBenchStrIO("[1,2,,]",{"1","2",""});
   OneBenchStrIO("[1,2,,,,]",{"1","2","","",""});
   // OneBenchStrIO("[,,1,,3,]",{"","","1","","3",""});
   // Check that we get an error catched
   //    OneBenchStrIO("[",{});
   //    OneBenchStrIO("[1,2",{});
   // getchar();
}

   // ================  bool ==============================================

template <>  std::string cStrIO<bool>::ToStr(const bool & anI)
{
   return  anI ? "true" : "false";
}
template <>  bool cStrIO<bool>::FromStr(const std::string & aStr)
{
    if ((aStr=="1") || UCaseEqual(aStr,"true")) return true;
    if ((aStr=="0") || UCaseEqual(aStr,"false")) return false;

    MMVII_INTERNAL_ASSERT_user(eTyUEr::eBadBool,"Bad value for boolean :["+aStr+"]");

    return false;
}

template <>  const std::string cStrIO<bool>::msNameType = "bool";

   // ================  int ==============================================

template <>  std::string cStrIO<int>::ToStr(const int & anI)
{
   sprintf(BufStrIO,"%d",anI);
   return BufStrIO;
}
template <>  int cStrIO<int>::FromStr(const std::string & aStr)
{
    // can be convenient that empty string correspond to zero
    if (aStr.empty())
       return 0;
    int anI;
    sscanf(aStr.c_str(),"%d",&anI);
    return anI;
}
template <>  const std::string cStrIO<int>::msNameType = "int";

std::string ToStr(int aVal,int aSzMin)
{
   std::string aRes = ToStr(std::abs(aVal));
   while (int(aRes.size())<aSzMin)
       aRes = "0" + aRes;
   if (aVal<0)
       aRes = "-" + aRes;
   return aRes;
}
/*
std::string  ToS_NbDigit(int aNb,int aNbDig,bool AcceptOverFlow)
{
   std::string aRes = ToS(aNb);
   int aSz = (int)aRes.size();
   if ((!AcceptOverFlow) && (aSz>aNbDig))
   {
       MMVII_INTERNAL_ASSERT_user(eTyUEr::eTooBig4NbDigit,"Pas assez de digit dans ToStringNBD")
   }
   for (;aSz<aNbDig ; aSz++)
   {
       aRes = "0" + aRes;
   }
   return aRes;
}
*/


   // ================  double ==============================================

template <>  std::string cStrIO<double>::ToStr(const double & anI)
{
   sprintf(BufStrIO,"%lg",anI);
   return BufStrIO;
}
template <>  double cStrIO<double>::FromStr(const std::string & aStr)
{
    double anI;
    sscanf(aStr.c_str(),"%lf",&anI);
    return anI;
}
template <>  const std::string cStrIO<double>::msNameType = "double";

std::string FixDigToStr(double aSignedVal,int aNbDig)
{
   std::string aFormat = "%."+ToS(aNbDig) + "f";
   char aBuf[100];
   sprintf(aBuf,aFormat.c_str(),aSignedVal);
   return aBuf;
}

std::string FixDigToStr(double aSignedVal,int aNbBef,int aNbAfter)
{
   std::string aFormat = "%0" + ToS(aNbBef+aNbAfter+1) + "."+ToS(aNbAfter) + "f";
   char aBuf[100];
   sprintf(aBuf,aFormat.c_str(),aSignedVal);
   return aBuf;
}



   // ================  std::string ==============================================

template <>  std::string cStrIO<std::string>::ToStr(const std::string & aStr)
{
   return aStr;
}
template <>  std::string cStrIO<std::string>::FromStr(const std::string & aStr)
{
    return aStr;
}

template <>  const std::string cStrIO<std::string>::msNameType = "string";

};
