#include "include/MMVII_all.h"


namespace MMVII
{
#define MSD_DEGUG()  StdOut() << "MSD_DEGUGMSD_DEGUG " << __LINE__ << " of " << __FILE__ << "\n";

/*  ============================================== */
/*                                                 */
/*                cColStrAObl                      */
/*                                                 */
/*  ============================================== */


const  cColStrAObl::tCont & cColStrAObl::V() const {return mV;}
cColStrAObl &  cColStrAObl::operator << (const std::string & aVal) {mV.push_back(aVal); return *this;}
void cColStrAObl::clear() {mV.clear();}
cColStrAObl::cColStrAObl() {}


/*  ============================================== */
/*                                                 */
/*                cColStrAOpt                      */
/*                                                 */
/*  ============================================== */


const  cColStrAOpt::tCont & cColStrAOpt::V() const {return mV;}
cColStrAOpt &  cColStrAOpt::operator << (const t2S & aVal) {mV.push_back(aVal); return *this;}
void cColStrAOpt::clear() {mV.clear();}
cColStrAOpt::cColStrAOpt() {}

const cColStrAOpt cColStrAOpt::Empty;

cColStrAOpt::cColStrAOpt(cExplicitCopy,const cColStrAOpt& aCSAO)  :
  mV (aCSAO.mV)
{
}

/*  ============================================== */
/*                                                 */
/*                cColStrAOpt                      */
/*                                                 */
/*  ============================================== */

cParamCallSys::cParamCallSys(const cSpecMMVII_Appli & aSpec,bool InArgSep) :
   mSpec   (&aSpec),
   mArgSep (InArgSep),
   mArgc   (0)
{
}

int cParamCallSys::Execute() const
{
   if (mArgSep)
   {
       static int aCpt=0; aCpt++;
/*
       std::vector<char *> aVArgv;
       for (auto & aStr : mArgv)
       {
          aVArgv.push_back(const_cast<char*>(aStr.c_str()));
       }

       char ** argv = aVArgv.data();
*/
       int aRes = mSpec->AllocExecuteDestruct(mArgv);
       return aRes;
   }
   else
   {
       // StdOut() << "EXTERN   cParamCallSys::  \n";
       int aRes = SysCall(mCom,false);
       // StdOut() << "   ##### EXTERN   cParamCallSys::  " << aRes << " \n";
       return aRes;
   }
}

void cParamCallSys::AddArgs(const std::string & aNewArg)
{
   if (mArgSep)
   {
       mArgv.push_back(aNewArg);
   }
   else
   { 
      if (mArgc)
         mCom += " ";
      mCom +=  aNewArg;
   }

   mArgc++;
}

const std::string &  cParamCallSys::Com() const 
{
   MMVII_INTERNAL_ASSERT_strong(! mArgSep,"Call com with arg non separated in cParamCallSys::Com");
   return mCom;
}


/*  ============================================== */
/*                                                 */
/*                cMMVII_Appli                     */
/*                                                 */
/*  ============================================== */

// May be used again for testing value inside initialization
/*
template <class Type> Type PrintArg(const Type & aVal,const std::string & aName)
{
    Std Out() << " For " << aName << " V=" << aVal << "\n";
    return aVal;
}
*/

// ========================= 3 Main function :
// 
//        cMMVII_Appli::~cMMVII_Appli()
//        cMMVII_Appli::cMMVII_Appli ( int argc, char ** argv, const cSpecMMVII_Appli & aSpec) 
//        void cMMVII_Appli::InitParam() => main initialisation must be done after Cstrctor as call virtual methods


cMMVII_Appli::~cMMVII_Appli()
{
   if (! mModeHelp)
   {
      RenameFiles(NameFileLog(false),NameFileLog(true));
      LogCommandOut(NameFileLog(true),false);
   }

   if (mGlobalMainAppli)
   {
      LogCommandOut(MVII_LogFile,true);
   }
  
   msInDstructor = (TheStackAppli.size()<=1);  // avoid problem with StdOut 
   if (msInDstructor) FreeRandom();   // Free memory only top called do it
   AssertInitParam();
   // ======= delete mSetInit;
   mArgObl.clear();
   mArgFac.clear();


   MMVII_INTERNAL_ASSERT_strong(ExistAppli(),"check in Appli Destructor");
   MMVII_INTERNAL_ASSERT_strong(this==TheStackAppli.back(),"check in Appli Destructor");
   TheStackAppli.pop_back();
   mStdCout.Clear();
   // Verifie que tout ce qui a ete alloue a ete desalloue 
   // cMemManager::CheckRestoration(mMemStateBegin);
   mMemStateBegin.SetCheckAtDestroy();
}

/*
static std::vector<std::string> InitFromArgcArgv(int argc, char ** argv)
{ 
   std::vector<std::string> aRes;
   for (int aK=0 ; aK<argc; aK++)
       aRes.push_back(argv[aK]);
   return aRes;
}
*/

template <class Type> const Type & MessageInCstr(const Type & aVal,const std::string & aMsg,int aLine)
{
    StdOut() << aMsg << " at line " << aLine << "\n";
    return aVal;
}

cMMVII_Appli::cMMVII_Appli
(
      const std::vector<std::string> & aVArgcv,
      const cSpecMMVII_Appli & aSpec,
      tVSPO                    aVSPO
)  :
   cMMVII_Ap_CPU(),
   mMemStateBegin (cMemManager::CurState()),
   mArgv          (aVArgcv),
   mArgc          (mArgv.size()),
   mSpecs         (aSpec),
   mDirBinMMVII   (DirBin2007),
   mTopDirMMVII   (UpDir(mDirBinMMVII,1)),
   mFullBin       (mDirBinMMVII + Bin2007),
   mBinMMVII      (Bin2007),
   mDirMicMacv1   (UpDir(mTopDirMMVII,1)),
   mDirMicMacv2   (mTopDirMMVII),
   mDirProject    (DirCur()),
   mDirTestMMVII  (mDirMicMacv2 + MMVIITestDir),
   mTmpDirTestMMVII   (mDirTestMMVII + "Tmp/"),
   mInputDirTestMMVII (mDirTestMMVII + "Input/"),
   mModeHelp      (false),
   mDoGlobHelp    (false),
   mDoInternalHelp(false),
   mShowAll       (false),
   mLevelCall     (0),
   mSetInit       (cExtSet<void *>(eTySC::US)),
   mInitParamDone (false),
   mVMainSets     (NbMaxMainSets,tNameSet(eTySC::NonInit)),
   mNumOutPut     (0),
   mOutPutV1      (false),
   mOutPutV2      (false),
   mHasInputV1    (false),
   mHasInputV2    (false),
   mStdCout       (std::cout),
   mSeedRand      (msDefSeedRand), // In constructor, don't use virtual, wait ...
   mVSPO          (aVSPO),
   mCarPPrefOut   (MMVII_StdDest),
   mCarPPrefIn    (MMVII_StdDest),
   mTiePPrefOut   (MMVII_StdDest),
   mTiePPrefIn    (MMVII_StdDest)
{
   mNumCallInsideP = TheNbCallInsideP;
   TheNbCallInsideP++;
   
   mMainAppliInsideP = (mNumCallInsideP==0);
   TheStackAppli.push_back(this);
   /// Minimal consistency test for installation, does the MicMac binary exist ?
   MMVII_INTERNAL_ASSERT_always(ExistFile(mFullBin),"Could not find MMVII binary (tried with " +  mFullBin + ")");
}

const std::vector<eSharedPO>    cMMVII_Appli::EmptyVSPO;  ///< Deafaut Vector  shared optional parameter


/// This one is always std:: cout, to be used by StdOut and cMMVII_Appli::StdOut ONLY

cMultipleOfs & StdStdOut()
{
   static cMultipleOfs aMOfs(std::cout);
   return aMOfs;
}

cMultipleOfs& StdOut()
{
   if (cMMVII_Appli::ExistAppli())
     return cMMVII_Appli::CurrentAppli().StdOut();
   return StdStdOut();
}
cMultipleOfs& HelpOut() {return StdOut();}
cMultipleOfs& ErrOut()  {return StdOut();}



cMultipleOfs &  cMMVII_Appli::StdOut()
{
   /// Maybe mStdCout not correctly initialized if we are in constructor or in destructor ?
   if ((!cMMVII_Appli::ExistAppli()) || msInDstructor)
      return StdStdOut();
   return mStdCout;
}
cMultipleOfs &  cMMVII_Appli::HelpOut() {return StdOut();}
cMultipleOfs &  cMMVII_Appli::ErrOut() {return StdOut();}


void TestMainSet(const cCollecSpecArg2007 & aVSpec,bool &aMain0,bool & aMain1)
{
    for (int aK=0 ; aK<int(aVSpec.size()) ; aK++)
    {
        std::string aNumPat;
        if (aVSpec[aK]->HasType(eTA2007::MPatIm,&aNumPat))
        {
             int aNum =   cStrIO<int>::FromStr(aNumPat);
             if (aNum==0)  aMain0 = true;
             if (aNum==1)  aMain1 = true;
        }
    }
}

bool   cMMVII_Appli::HasSharedSPO(eSharedPO aV) const
{
   return BoolFind(mVSPO,aV);
}




void cMMVII_Appli::InitParam()
{
  mSeedRand = DefSeedRand();
  cCollecSpecArg2007 & anArgObl = ArgObl(mArgObl); // Call virtual method
  cCollecSpecArg2007 & anArgFac = ArgOpt(mArgFac); // Call virtual method


  mInitParamDone = true;
  // MMVII_INTERNAL_ASSERT_always(msTheAppli==0,"cMMVII_Appli only one by process");
  // msTheAppli = this;

  // Check that  cCollecSpecArg2007 were used with the good values
  MMVII_INTERNAL_ASSERT_always((&anArgObl)==&mArgObl,"cMMVII_Appli dont respect cCollecSpecArg2007");
  MMVII_INTERNAL_ASSERT_always((&anArgFac)==&mArgFac,"cMMVII_Appli dont respect cCollecSpecArg2007");

  std::string aDP; // mDirProject is handled specially so dont put mDirProject in AOpt2007
                   // becauser  InitParam, it may change the correct value 

  // Add common optional parameters
  cSpecOneArg2007::tVSem aInternal{eTA2007::Internal,eTA2007::Global}; // just to make shorter lines
  cSpecOneArg2007::tVSem aGlob{eTA2007::Global}; // just to make shorter lines


  /*  Decoding AOpt2007(mIntervFilterMS[0],GOP_Int0,"File Filter Interval, Main Set"  ,{eTA2007::Common,{eTA2007::FFI,"0"}})
        mIntervFilterMS[0]  => string member, will store the value
        GOP_Int0 => const name, Global Optionnal Interval , num 0, declared in MMVII_DeclareCste.h
        {eTA2007::Common,{eTA2007::FFI,"0"}}  attibute, it's common, it's intervall with attribute "0"
  */

  if (HasSharedSPO(eSharedPO::eSPO_CarPO))
  {
     mArgFac << AOpt2007(mCarPPrefOut,"CarPOut","Name for Output caracteristic points",{eTA2007::HDV});
  }
  // To not put intervals in help/parameters when they are not usefull
  {
      bool HasMain0 = false;
      bool HasMain1 = false;
      TestMainSet(anArgObl,HasMain0,HasMain1);
      TestMainSet(anArgFac,HasMain0,HasMain1);
      if (HasMain0)
        mArgFac <<  AOpt2007(mIntervFilterMS[0],GOP_Int0,"File Filter Interval, Main Set"  ,{eTA2007::Shared,{eTA2007::FFI,"0"}});
      if (HasMain1)
        mArgFac <<  AOpt2007(mIntervFilterMS[1],GOP_Int1,"File Filter Interval, Second Set",{eTA2007::Shared,{eTA2007::FFI,"1"}});
  }
  mArgFac
      // <<  AOpt2007(mIntervFilterMS[0],GOP_Int0,"File Filter Interval, Main Set"  ,{eTA2007::Common,{eTA2007::FFI,"0"}})
      // <<  AOpt2007(mIntervFilterMS[1],GOP_Int1,"File Filter Interval, Second Set",{eTA2007::Common,{eTA2007::FFI,"1"}})
      <<  AOpt2007(mNumOutPut,GOP_NumVO,"Num version for output format (1 or 2)",aGlob)
      <<  AOpt2007(mSeedRand,GOP_SeedRand,"Seed for random,if <=0 init from time",aGlob)
      <<  AOpt2007(aDP ,GOP_DirProj,"Project Directory",{eTA2007::DirProject,eTA2007::Global})
      <<  AOpt2007(mParamStdOut,GOP_StdOut,"Redirection of Ouput (+File for add,"+ MMVII_NONE + "for no out)",aGlob)
      <<  AOpt2007(mLevelCall,GIP_LevCall,"Internal : Don't Use !! Level Of Call",aInternal)
      <<  AOpt2007(mShowAll,GIP_ShowAll,"Internal : Don't Use !!",aInternal)
      <<  AOpt2007(mPrefixGMA,GIP_PGMA,"Internal : Don't Use !! Prefix Global Main Appli",aInternal)
      <<  AOpt2007(mDirProjGMA,GIP_DirProjGMA,"Internal : Don't Use !! Folder Project Global Main Appli",aInternal)
  ;

  // Check that names of optionnal parameters begin with alphabetic caracters
  for (const auto & aSpec : mArgFac.Vec())
  {
      aSpec->ReInit();
      if (!std::isalpha(aSpec->Name()[0]))
      {
         MMVII_INTERNAL_ASSERT_always
         (
             false,
             "Name of optional param must begin with alphabetic => ["+aSpec->Name()+"]"
         );
      }
  }

  // Test if we are in help mode
  for (int aKArg=0 ; aKArg<mArgc ; aKArg++)
  {
      const char * aArgK = mArgv[aKArg].c_str();
      if (UCaseBegin("help",aArgK) || UCaseBegin("-help",aArgK)|| UCaseBegin("--help",aArgK))
      {
         mModeHelp = true;
         while (*aArgK=='-') aArgK++;
         mDoGlobHelp = (*aArgK=='H');
         mDoInternalHelp = CaseSBegin("HE",aArgK);

         std::string aName; 
         SplitStringArround(aName,mPatHelp,aArgK,'=',true,false);
      }
  }
  if (mModeHelp)
  {
      GenerateHelp();
      return;
  }


  int aNbObl = mArgObl.size(); //  Number of mandatory argument expected
  int aNbArgGot = 0; // Number of  Arg received untill now
  bool Optional=false; // Are we in the optional phase of argument parsing

  // To be abble to process in  the same loop mandatory and optional
  std::vector<std::string> aVValues;
  tVecArg2007              aVSpec;

  for (int aKArg=0 ; aKArg<mArgc ; aKArg++)
  {
      Optional = (aNbArgGot>=aNbObl);
      // If --Name replace by Name, maybe usefull for completion
      if (Optional && (mArgv[aKArg][0]=='-') && (mArgv[aKArg][1]=='-'))
         mArgv[aKArg] = mArgv[aKArg].substr(2);

      const char * aArgK = mArgv[aKArg].c_str();
      if (aKArg<2)
      {
          //  mArgv[0] => MMVII
          //  mArgv[1] => the name of commmand
      }
      else
      {
          if (Optional)
          {
             // while '
             std::string aName,aValue;
             SplitStringArround(aName,aValue,aArgK,'=',true,false);
             int aNbSpecGot=0;
             // Look for spec corresponding to name
             for (const auto  & aSpec : mArgFac.Vec())
             {
                 // if (aSpec->Name() == aName)
                 if (UCaseEqual(aSpec->Name(),aName))
                 {
                    aNbSpecGot++;
                    aVSpec.push_back(aSpec);
                    // Several space have the same name
                    if (aNbSpecGot==2)
                    {
                        MMVII_INTERNAL_ASSERT_always(false,"\""+ aName +"\" is multiple in specification");
                    }
                    // Same name was used several time
                    if (aSpec->NbMatch() !=0)
                    {
                        MMVII_INTERNAL_ASSERT_user(eTyUEr::eMulOptParam,"\""+aName +"\" was used multiple time");
                    }
                    aSpec->IncrNbMatch();
                 }
             }
             // Name does not correspond to spec
             if (aNbSpecGot==0)
             {
                MMVII_INTERNAL_ASSERT_user(eTyUEr::eBadOptParam,"\""+aName +"\" is not a valide optionnal value");
             }
             aVValues.push_back(aValue);
          }
          else
          {
             aVValues.push_back(aArgK);
             aVSpec.push_back(mArgObl[aNbArgGot]);
          }
          aNbArgGot ++;
      }
  }

  size_t aNbArgTot = aVValues.size();

  if (aNbArgGot < aNbObl)
  {
      // Tolerance, like in mmv1, no arg generate helps
      if (aNbArgGot==0)
      {
         mModeHelp = true;  // else Exe() will be executed !!
         GenerateHelp();
         return;
      }
      MMVII_INTERNAL_ASSERT_user
      (
          eTyUEr::eInsufNbParam,
          "Not enough Arg, expecting " + ToS(aNbObl)  + " , Got only " +  ToS(aNbArgGot)
      );
  }
  MMVII_INTERNAL_ASSERT_always(aNbArgTot==aVSpec.size(),"Interncl check size Value/Spec");


  // First compute the directory of project that may influence all other computation
     // Try with Optional value
  {
     bool HasDirProj=false;
     for (size_t aK=0 ; aK<aNbArgTot; aK++)
     {
        if (aVSpec[aK]->HasType(eTA2007::DirProject))
        {
           MMVII_INTERNAL_ASSERT_always(!HasDirProj,"Multiple dir project");
           HasDirProj = true;
           MakeNameDir(aVValues[aK]);
           mDirProject = aVValues[aK];
        }
     }

  
     for (size_t aK=0 ; aK<aNbArgTot; aK++)
     {
        if (aVSpec[aK]->HasType(eTA2007::FileDirProj))
        {
           if (!HasDirProj)
              mDirProject = DirOfPath(aVValues[aK],false);
           else
           {
              aVValues[aK] = mDirProject + aVValues[aK];
           }
        }
     }
  }

  // Add a / if necessary
  MakeNameDir(mDirProject);

  //  Initialize the paramaters
  for (size_t aK=0 ; aK<aNbArgTot; aK++)
  {
       aVSpec[aK]->InitParam(aVValues[aK]);
       mSetInit.Add(aVSpec[aK]->AdrParam()); ///< Memorize this value was initialized
  }
  mMainProcess   = (mLevelCall==0);
  mGlobalMainAppli = mMainProcess && mMainAppliInsideP;
  // Compute an Id , unique and (more or less ;-) understandable
  // tINT4 aIT0 = round_ni(mT0);
// std::cout << "Tttttt " << mT0 << " " << aIT0 << "\n";
  mPrefixNameAppli =   std::string("MMVII")
                     + std::string("_Tim")  + StrIdTime()
                     + std::string("_Num")  + ToStr(mNumCallInsideP) 
                     + std::string("_Pid")  + ToStr(mPid) 
                     + std::string("_") + mSpecs.Name()
                   ;
   if (mGlobalMainAppli)  // Pour communique aux sous process
   {
       mPrefixGMA  = mPrefixNameAppli;
       mDirProjGMA = mDirProject;
   }
   // StdOut() << "mPrefixNameAppliii " << mPrefixNameAppli << " " << mPrefixGMA << "\n";

  // Manange OutPut redirection
  if (IsInit(&mParamStdOut))
  {
     const char * aPSO = mParamStdOut.c_str();
     bool aModeFileInMore = false;
     bool aModeAppend = true;

     // Do it twice to accept 0+ and +0
     for (int aK=0 ; aK<2 ; aK++)
     {
         //  StdOut=0File.txt => Put in file, erase it before
         if (aPSO[0]=='0')
         {
            aModeAppend = false;
            aPSO++;  
         }
         //  StdOut=+File.txt => redirect output in file and in console
         //  StdOut=0+File.txt => work also
         if (aPSO[0]=='+')
         {
            aModeFileInMore = true;
            aPSO++;  
         }
     }
     // If not on console, supress std:: cout which was in mStdCout
     if (! aModeFileInMore)
     {
         mStdCout.Clear();
     }
     // Keyword NONE means no out at all
     if (MMVII_NONE != aPSO)
     {
         mFileStdOut.reset(new cMMVII_Ofs(aPSO,aModeAppend));
         // separator between each process , to refine ... (date ? Id ?)
         mFileStdOut->Ofs() << "=============================================" << ENDL;
         mStdCout.Add(mFileStdOut->Ofs());
     }
  }

  // If mNumOutPut was set, fix the output version
  if (IsInit(&mNumOutPut))
  {
     if (mNumOutPut==1)
        mOutPutV1 = true;
     else if (mNumOutPut==2)
        mOutPutV2 = true;
     else
     {
         MMVII_INTERNAL_ASSERT_always(false,"Output version must be in {1,2}, got: "+ToStr(mNumOutPut));
     }
  }


  // Test the size of vectors vs possible specifications in
  for (size_t aK=0 ; aK<aNbArgTot; aK++)
  {
      std::string aSpecSize;
      // If the arg contains the semantic
      if (aVSpec[aK]->HasType(eTA2007::ISizeV,&aSpecSize))
      {
         aVSpec[aK]->CheckSize(aSpecSize);  // Then test it
      }
  }

  // Analyse the possible main patterns
  for (size_t aK=0 ; aK<aNbArgTot; aK++)
  {
      std::string aNumPat;
      // Test the semantic
      if (aVSpec[aK]->HasType(eTA2007::MPatIm,&aNumPat))
      {
         int aNum =   cStrIO<int>::FromStr(aNumPat);
         // Check range
         CheckRangeMainSet(aNum);

         // don't accept multiple initialisation
         if (!mVMainSets.at(aNum).IsInit())
         {
            mVMainSets.at(aNum)= SetNameFromString(mDirProject+aVValues[aK],true);
            //  Filter with interval
            {
               std::string & aNameInterval = mIntervFilterMS[aNum];
               if (IsInit(&aNameInterval))
               {
                   mVMainSets.at(aNum).Filter(Str2Interv<std::string>(aNameInterval));
               }
            }
            // Test non empty
            if (! AcceptEmptySet(aNum) && (mVMainSets.at(aNum).size()==0))
            {
                MMVII_UsersErrror(eTyUEr::eEmptyPattern,"Specified set of files was empty");
            }
         }
         else
         {
            MMVII_INTERNAL_ASSERT_always(false,"Multiple main set im for num:"+ToStr(aNum));
         }
/*
         std::string & aNameInterval = mIntervFilterMS[aNum];
         if (IsInit(&aNameInterval))
         {
             mVMainSets.at(aNum).Filter(Str2Interv<std::string>(aNameInterval));
         }
*/
      }
  }
  // Check validity of main set initialization
  for (int aNum=0 ; aNum<NbMaxMainSets ; aNum++)
  {
      // Why should user init interval if there no set ?
      if (IsInit(&mIntervFilterMS[aNum]) && (!  mVMainSets.at(aNum).IsInit()))
      {
         MMVII_INTERNAL_ASSERT_user(eTyUEr::eIntervWithoutSet,"Interval without filter for num:"+ToStr(aNum));
      }
      if (aNum>0)
      {
         // would be strange to have Mainset2 without MainSet1; probably if this occurs
         // the fault would be from programer's side (not sure)
         if ((! mVMainSets.at(aNum-1).IsInit() ) && ( mVMainSets.at(aNum).IsInit()))
         {
            MMVII_INTERNAL_ASSERT_always(false,"Main set, init for :"+ToStr(aNum) + " and non init for " + ToStr(aNum-1));
         }
      }
  }


  // MakeNameDir(mDirProject);
  
  // Print the info, debugging
  if (mShowAll)
  {
     // Print the value of all parameter
     for (size_t aK=0 ; aK<aNbArgTot; aK++)
     {
         HelpOut() << aVSpec[aK]->Name()  << " => [" << aVValues[aK] << "]" << ENDL;
     }
     HelpOut() << "---------------------------------------" << ENDL;
     HelpOut() << "IS INIT  DP: " << IsInit(&aDP) << ENDL;
     HelpOut() << "DIRPROJ=[" << mDirProject << "]" << ENDL;
  }

  // By default, if calls is done at top level, assure that everything is init

  if (mGlobalMainAppli)
  {
     InitProject();
  }
  if (!mModeHelp)
     LogCommandIn(NameFileLog(false),false);

  if (mSeedRand<=0)
  {
      mSeedRand =  std::chrono::system_clock::to_time_t(mT0);
  }
}



std::string cMMVII_Appli::NameFileLog(bool Finished) const
{
   return   
              //mDirProject
               mDirProjGMA
             + TmpMMVIIDirGlob
             + TmpMMVIIProcSubDir 
             + mPrefixGMA + DirSeparator()
             + mPrefixNameAppli 
             + std::string(Finished ? "_Ok" : "_InProcess")
             + std::string(".txt")
          ;
}

std::string cMMVII_Appli::PrefixPCar(const std::string & aNameIm,const std::string & aPref) const
{
   return mDirProject +TmpMMVIIDirPCar + aNameIm + "/" + aPref;
}
std::string cMMVII_Appli::PrefixPCarOut(const std::string & aNameIm) const
{
   return PrefixPCar(aNameIm,mTiePPrefOut);
}
std::string cMMVII_Appli::PrefixPCarIn(const std::string & aNameIm) const
{
   return PrefixPCar(aNameIm,mTiePPrefIn);
}



// Is called only when global main applu
void cMMVII_Appli::InitProject()
{
   // Create Dir for tmp file, process etc ...
   std::string aDir = mDirProject+TmpMMVIIDirGlob;
   CreateDirectories(aDir,true);

   // Create Dir for Caracteristic point, if Appli output them
   if (HasSharedSPO(eSharedPO::eSPO_CarPO))
   {
      CreateDirectories(mDirProject +TmpMMVIIDirPCar,true);
   }

   aDir += TmpMMVIIProcSubDir;
   CreateDirectories(aDir,true);

   if (! mModeHelp)
   {
      aDir += mPrefixNameAppli;
      CreateDirectories(aDir,true);
      LogCommandIn(MVII_LogFile,true);
   }
}

void cMMVII_Appli::LogCommandIn(const std::string & aName,bool MainLogFile)
{
   cMMVII_Ofs  aOfs(aName,true);
   aOfs.Ofs() << "========================================================================\n";
   aOfs.Ofs() << "  Id : " <<  mPrefixNameAppli << "\n";
   aOfs.Ofs() << "  begining at : " <<  StrDateCur() << "\n\n";
   aOfs.Ofs() << "  " << Command() << "\n\n";
   aOfs.Ofs().close();
}

void cMMVII_Appli::LogCommandOut(const std::string & aName,bool MainLogFile)
{
   cMMVII_Ofs  aOfs(aName,true);
   aOfs.Ofs() << "  ending correctly at : " <<  StrDateCur() << "\n\n";
   aOfs.Ofs().close();
}


    // ========== Help ============

void cMMVII_Appli::GenerateHelp()
{
   HelpOut() << "\n";

   HelpOut() << "**********************************\n";
   HelpOut() << "*   Help project 2007/MMVII      *\n";
   HelpOut() << "**********************************\n";

   HelpOut() << "\n";
   HelpOut() << "  For command : " << mSpecs.Name() << " \n";
   HelpOut() << "   => " << mSpecs.Comment() << "\n";
   HelpOut() << "   => Srce code entry in :" << mSpecs.NameFile() << "\n";
   HelpOut() << "\n";

   HelpOut() << " == Mandatory unnamed args : ==\n";

   for (const auto & Arg : mArgObl.Vec())
   {
       HelpOut() << "  * " << Arg->NameType() << Arg->Name4Help() << " :: " << Arg->Com()  << "\n";
   }

   tNameSelector  aSelName =  BoostAllocRegex(mPatHelp);

   HelpOut() << "\n";
   HelpOut() << " == Optional named args : ==\n";
   bool InternalMet = false;
   bool GlobalMet   = false;
   for (const auto & Arg : mArgFac.Vec())
   {
       const std::string & aNameA = Arg->Name();
       if (aSelName.Match(aNameA))
       {
          bool IsIinternal = Arg->HasType(eTA2007::Internal);
          if ((! IsIinternal) || mDoInternalHelp)
          {
             bool isGlobHelp = Arg->HasType(eTA2007::Global);
             if ((!isGlobHelp) || mDoGlobHelp)
             {
                if (IsIinternal && (!InternalMet)) 
                {
                   HelpOut() << "       ####### INTERNAL #######\n" ; 
                   InternalMet = true;
                }
                else if (isGlobHelp && (!GlobalMet)) 
                {
                   HelpOut() << "       ####### GLOBAL   #######\n" ; 
                   GlobalMet = true;
                }

                HelpOut() << "  * [Name=" <<  Arg->Name()   << "] " << Arg->NameType() << Arg->Name4Help() << " :: " << Arg->Com() ;
                bool HasDefVal = Arg->HasType(eTA2007::HDV);
                if (HasDefVal)
                {
                   HelpOut() << " ,[Default="  << Arg->NameValue() << "]"; 
                }

                HelpOut()  << "\n";
             }
          }
       }
   }
   HelpOut() << "\n";
}

bool cMMVII_Appli::ModeHelp() const
{
   return mModeHelp;
}

    // ========== Handling of Mains Sets

const tNameSet &  cMMVII_Appli::MainSet0() const { return MainSet(0); }
const tNameSet &  cMMVII_Appli::MainSet1() const { return MainSet(1); }
const tNameSet &  cMMVII_Appli::MainSet(int aK) const 
{
   CheckRangeMainSet(aK);
   if (! mVMainSets.at(aK).IsInit())
   {
      MMVII_INTERNAL_ASSERT_always(false,"No mMainSet created for K="+ ToStr(aK));
   }
   return  mVMainSets.at(aK);
}
bool   cMMVII_Appli::AcceptEmptySet(int) const {return false;}

std::vector<std::string> cMMVII_Appli::VectMainSet(int aK) const
{
   return ToVect(MainSet(aK));
}

void cMMVII_Appli::CheckRangeMainSet(int aK) const
{
   if ((aK<0) || (aK>=NbMaxMainSets))
   {
      MMVII_INTERNAL_ASSERT_always(false,"CheckRangeMainSet, out for :" + ToStr(aK));
   }
}

    // ========== Handling of V1/V2 format for output =================

void cMMVII_Appli::SignalInputFormat(int aNumV)
{
   cMMVII_Appli & TheAp = CurrentAppli();
   if (aNumV==0)
   {
   }
   else if (aNumV==1)
   {
      TheAp.mHasInputV1 = true;
   }
   else if (aNumV==2)
   {
      TheAp.mHasInputV2 = true;
   }
   else 
   {
      MMVII_INTERNAL_ASSERT_always(false,"Input version must be in {0,1,2}, got: "+ToStr(aNumV));
   }
}

// Necessary for forward use of cMMVII_Appli::OutV2Forma
bool GlobOutV2Format() { return cMMVII_Appli::OutV2Format(); }
bool   cMMVII_Appli::OutV2Format() 
{
   const cMMVII_Appli & TheAp = CurrentAppli();
   // Priority to specified output if exist
   if (TheAp.mOutPutV2) return true;
   if (TheAp.mOutPutV1) return false;
   //  In input, set it, priority to V2
   if (TheAp.mHasInputV2) return true;
   if (TheAp.mHasInputV1) return false;
   // by default V2
   return true;
}

    // ========== Handling of global Appli =================

std::vector<cMMVII_Appli *>  cMMVII_Appli::TheStackAppli ;
int  cMMVII_Appli::TheNbCallInsideP=0;
bool  cMMVII_Appli::msInDstructor = false;
cMMVII_Appli & cMMVII_Appli::CurrentAppli()
{
  MMVII_INTERNAL_ASSERT_strong(ExistAppli(),"cMMVII_Appli not created");
  return *(TheStackAppli.back());
}
bool cMMVII_Appli::ExistAppli()
{
  return !TheStackAppli.empty();
}
 
    // ========== Random seed  =================

const int cMMVII_Appli::msDefSeedRand = 42;
int  cMMVII_Appli::SeedRandom()
{
    return ExistAppli() ?  CurrentAppli().mSeedRand  : msDefSeedRand;
}
int  cMMVII_Appli::DefSeedRand()
{
   return msDefSeedRand;
}

    // ========== Miscelaneous functions =================

void cMMVII_Appli::AssertInitParam() const
{
  MMVII_INTERNAL_ASSERT_always(mInitParamDone,"Init Param was forgotten");
}
bool  cMMVII_Appli::IsInit(void * aPtr)
{
    return  mSetInit.In(aPtr);
}

void cMMVII_Appli::MMVII_WARNING(const std::string & aMes)
{
   StdOut() << "===================================================================\n";
   StdOut() <<  aMes << "\n";
   StdOut() << "===================================================================\n";
}
              // Accessors
const std::string & cMMVII_Appli::TmpDirTestMMVII()   const {return mTmpDirTestMMVII;}
const std::string & cMMVII_Appli::InputDirTestMMVII() const {return mInputDirTestMMVII;}


void cMMVII_Appli::InitOutFromIn(std::string &aFileOut,const std::string& aFileIn)
{
   if (! IsInit(&aFileOut))
   {
      aFileOut = mDirProject + FileOfPath(aFileIn,false);
   }
   else
   {
      aFileOut = mDirProject + aFileOut;
   } 
}

    // ==========  MMVII  Call MMVII =================

cColStrAObl& cMMVII_Appli::StrObl() {return mColStrAObl;}
cColStrAOpt& cMMVII_Appli::StrOpt() {return mColStrAOpt;}


/// Quote when Not Separated, i.e. when call by process , else quote will not be removed
std::string QuoteWUS(bool Separate,const std::string & aStr)
{
   return Separate ? aStr : Quote(aStr);
}


cParamCallSys  cMMVII_Appli::StrCallMMVII
               (
                  const cSpecMMVII_Appli & aCom2007,
                  const cColStrAObl& anAObl,
                  const cColStrAOpt& anAOpt,
                  bool  Separate,
                  const cColStrAOpt&  aSubst
               )
{
  cParamCallSys aRes(aCom2007,Separate);
  MMVII_INTERNAL_ASSERT_always(&anAObl==&mColStrAObl,"StrCallMMVII use StrObl() !!");
  MMVII_INTERNAL_ASSERT_always(&anAOpt==&mColStrAOpt,"StrCallMMVII use StrOpt() !!");

   // std::string aComGlob = mFullBin + " ";
   aRes.AddArgs(mFullBin);
   int aNbSubst=0;
   std::vector<bool>  aVUsedSubst(aSubst.V().size(),false);
/*
   cSpecMMVII_Appli*  aSpec = cSpecMMVII_Appli::SpecOfName(aCom2007,false); // false => dont accept no match
   if (! aSpec)  // Will see if we can di better, however SpecOfNam has generated error
      return "";
*/

   // Theoretically we can create the command  (dealing with unik msTheAppli before !) and check
   // the parameters, but it will be done in the call so maybe it's not worth the price ?
  
   // aComGlob += aCom2007.Name() + " ";
   aRes.AddArgs(aCom2007.Name());

   
   // Add mandatory args
   int aK=0;
   for (const auto & aStr : anAObl.V())
   {
       std::string aStrK = ToStr(aK);
       std::string aVal = aStr;
       // See if there is a subst for arg K
       int aKSubst=0;
       for (const auto & aPSubst :  aSubst.V())
       {
           if (aPSubst.first==aStrK)
           {
              MMVII_INTERNAL_ASSERT_always(aVal==aStr,"Multiple KSubst in StrCallMMVII ");
              aVal = aPSubst.second;
              aNbSubst++; 
              aVUsedSubst[aKSubst] = true;
           }
           aKSubst++;
       }
       aRes.AddArgs(QuoteWUS(Separate,aVal));
       aK++;
   }

   // Add optionnal args
   for (const auto & aPOpt : anAOpt.V())
   {
       // Special case, it may have be add by the auto recal process , but it will be handled separately
       if ((aPOpt.first != GIP_LevCall) && (aPOpt.first !=GIP_PGMA) && (aPOpt.first !=GIP_DirProjGMA))
       {
          std::string aVal = aPOpt.second;
          int aKSubst=0;
          for (const auto & aPSubst :  aSubst.V())
          {
              if (aPSubst.first==aPOpt.first)
              {
                 MMVII_INTERNAL_ASSERT_always(aVal==aPOpt.second,"Multiple Opt-Subst in StrCallMMVII ");
                 aVal = aPSubst.second;
                 aNbSubst++; 
                 aVUsedSubst[aKSubst] = true;
              }
              aKSubst++;
          }
          aRes.AddArgs(QuoteWUS(Separate,aPOpt.first + "=" + aVal) );
       }
   }
   // MMVII_INTERNAL_ASSERT_always(aNbSubst==(int)aSubst.V().size(),"Impossible Subst in StrCallMMVII ");

   // Take into account the call level which must increase
   // aComGlob += GIP_LevCall + "=" + ToStr(mLevelCall+1);
   aRes.AddArgs(GIP_LevCall + "=" + ToStr(mLevelCall+1));
   aRes.AddArgs(GIP_PGMA + "=" + mPrefixGMA);
   aRes.AddArgs(GIP_DirProjGMA + "=" + mDirProjGMA);
   // aRes.AddArgs(GIP_PGMA + "=" + mPrefixGMA);

   // If no substitution, it means it was to be added simply
   int aKSubst=0;
   for (const auto & aPSubst :  aSubst.V())
   {
      if (!aVUsedSubst[aKSubst])
      {
         aRes.AddArgs(QuoteWUS(Separate,aPSubst.first + "=" + aPSubst.second));
      }
      aKSubst++;
   }

   mColStrAObl.clear();
   mColStrAOpt.clear();
   return aRes;
}

std::list<cParamCallSys>  cMMVII_Appli::ListStrCallMMVII
                        ( 
                              const cSpecMMVII_Appli & aCom2007,const cColStrAObl& anAObl,const cColStrAOpt& anAOpt,
                              const std::string & aNameOpt  , const std::vector<std::string> &  aLVals,
                              bool Separate
                        )
{
    std::list<cParamCallSys> aRes;
     
    for (const auto & aVal : aLVals)
    {
       cColStrAOpt  aNewSubst; 
       aNewSubst << t2S(aNameOpt,aVal);
       aRes.push_back(StrCallMMVII(aCom2007,anAObl,anAOpt,Separate,aNewSubst));
    }

    return aRes;
}



int  cMMVII_Appli::ExeCallMMVII
     (
         const  cSpecMMVII_Appli&  aCom2007,
         const cColStrAObl& anAObl,
         const cColStrAOpt& anAOpt,
         bool ByLineCom
      )
{
    cParamCallSys aComGlob = StrCallMMVII(aCom2007,anAObl,anAOpt,!ByLineCom);
    return  SysCall(aComGlob.Com(),false);
}

int cMMVII_Appli::ExeComSerial(const std::list<cParamCallSys> & aL)
{
    for (const auto & aPCS : aL)
    {
        int aRes = aPCS.Execute();
        if (aRes != EXIT_SUCCESS)
        {
            MMVII_INTERNAL_ASSERT_always(false,"Error in serial com");
            return aRes;
        }
    }
    return EXIT_SUCCESS;
}

int cMMVII_Appli::ExeComParal(const std::list<cParamCallSys> & aL)
{
    MMVII_WARNING("No Parallisation for now in MMVII, run serially");
    return ExeComSerial(aL);
}

void cMMVII_Appli::InitColFromVInit()
{
   mColStrAObl.clear();
   mColStrAOpt.clear();
   for (int aK=0; aK< (int)mArgObl.size() ; aK++)
   {
       mColStrAObl << mArgObl[aK]->Value();
   }

   for (int aK=0; aK< (int)mArgFac.size() ; aK++)
   {
      if ( mArgFac[aK]->NbMatch())
      {
          mColStrAOpt << t2S(mArgFac[aK]->Name(),mArgFac[aK]->Value());
      }
   }
}

std::list<cParamCallSys>  cMMVII_Appli::ListStrAutoRecallMMVII
                          ( 
                                const std::string & aNameOpt  , 
                                const std::vector<std::string> &  aLVals,
                                bool                 Separate,
                                const cColStrAOpt &  aLSubstInit
                          )
{
    std::list<cParamCallSys> aRes;

    for (const auto & aVal : aLVals) // For each value to substitute/add
    {
         InitColFromVInit(); // mColStrAObl and mColStrAOpt contains copy  command line

         cColStrAOpt  aNewSubst(cExplicitCopy(),aLSubstInit);  // make copy of aLSubstInit as it is const
         aNewSubst << t2S(aNameOpt,aVal); // subsitute/add  aVal with "named" arg aVal
         aRes.push_back(StrCallMMVII(mSpecs,mColStrAObl,mColStrAOpt,Separate,aNewSubst));
    }
    return aRes;
}

void   cMMVII_Appli::ExeMultiAutoRecallMMVII
       ( 
           const std::string & aNameOpt  , 
           const std::vector<std::string> &  aLVals,
           const cColStrAOpt &  aLSubstInit,
           eTyModeRecall  aMode
       )
{
    bool Separate = (aMode==eTyModeRecall::eTMR_Inside);
    std::list<cParamCallSys>  aLPCS = ListStrAutoRecallMMVII(aNameOpt,aLVals,Separate,aLSubstInit);
    if (aMode==eTyModeRecall::eTMR_Parall)
    {
       ExeComParal(aLPCS);
    }
    else
    {
           ExeComSerial(aLPCS);
    }
}

int   cMMVII_Appli::LevelCall() const { return mLevelCall; }

std::string  cMMVII_Appli::Command() const
{
    std::string  aRes;
    for (int aK=0 ; aK<(int) mArgv.size() ; aK++)
    {
        if (aK) aRes += " ";
        aRes += mArgv[aK];
    }
    return aRes;
}


};

