#ifndef  _MMVII_Matrix_H_
#define  _MMVII_Matrix_H_
namespace MMVII
{


/** \file MMVII_Matrix.h
    \brief Classes for matrix manipulation, 
*/

/**   \file MMVII_Matrix.h

    Also algorithm will mainly use eigen,  storage will be 
    done by MMVII class
*/



template <class Type> class cSparseVect ;
template <class Type> class cDenseVect ;
template <class Type> class cMatrix  ;
template <class Type> class cUnOptDenseMatrix ;
template <class Type> class cDenseMatrix ;

template <class Type> std::ostream & operator << (std::ostream & OS,const cDenseVect<Type> &aV);
template <class Type> std::ostream & operator << (std::ostream & OS,const cMatrix<Type> &aMat);

template <class Type> struct  cCplIV 
{
    public :
       cCplIV(const  int & aI,const Type & aV) : mInd(aI), mVal(aV) {}
       int   mInd;
       Type  mVal;
};

template <class Type> class  cSparseVect  : public cMemCheck
{
    public :
        typedef cCplIV<Type>            tCplIV;
        typedef std::vector<tCplIV>     tCont;
        typedef typename tCont::const_iterator   const_iterator;
        typedef typename tCont::iterator         iterator;

        const_iterator  end()   const {return  mIV.get()->end();}
        const_iterator  begin() const {return  mIV.get()->begin();}
        int  size() const {return mIV.get()->size();}
        const tCont & IV() const {return *(mIV.get());}
        tCont & IV() {return *(mIV.get());}

        void AddIV(const int & anInd,const Type & aV) {IV().push_back(tCplIV(anInd,aV));}

        /// SzInit fill with arbitray value, only to reserve space
        cSparseVect(int aSzReserve=-1,int aSzInit=-1) ;  
        bool IsInside(int aNb) const;
    private :
         std::shared_ptr<tCont>         mIV;
};


/** A dense vector is no more than a 1D Image, but with a different interface */

template <class Type> class  cDenseVect 
{
    public :
        typedef cIm1D<Type>  tIM;
        typedef cDataIm1D<Type>      tDIM;
        typedef cSparseVect<Type> tSpV;

        cDenseVect(int aSz, eModeInitImage=eModeInitImage::eMIA_NoInit);
        cDenseVect(tIM anIm);

        const Type & operator() (int aK) const {return DIm().GetV(aK);}
        Type & operator() (int aK) {return DIm().GetV(aK);}
        const int & Sz() const {return DIm().Sz();}

        double L1Dist(const cDenseVect<Type> & aV) const;
        double L2Dist(const cDenseVect<Type> & aV) const;

        Type * RawData();
        const Type * RawData() const;

        // No need to duplicate all services offered by Image Classe
        tDIM & DIm(){return mIm.DIm();}
        const tDIM & DIm() const {return mIm.DIm();}

        tIM & Im(){return mIm;}
        const tIM & Im() const {return mIm;}

        // operator -= 
        double DotProduct(const cDenseVect &) const;
        void TplCheck(const tSpV & aV)  const
        {
            MMVII_INTERNAL_ASSERT_medium(aV.IsInside(Sz()) ,"Sparse Vector out dense vect");
        }
        void  WeightedAddIn(Type aWeight,const tSpV & aColLine);
    private :

        tIM mIm;
};

/* To come, sparse vector, will be vect<int> + vect<double> */

/** a Interface class , derived class will be :
       - dense matrix
       - various sparse matrix
*/
template <class Type> class cMatrix  : public cRect2
{
     public :
         typedef cSparseVect<Type> tSpV;
         typedef Type              tVal;
         typedef cDenseVect<Type>  tDV;
         typedef cMatrix<Type>     tMat;

         virtual Type V_GetElem(int aX,int  aY) const = 0;
         virtual void  V_SetElem(int  aX,int  aY,const Type &) = 0;
         virtual eTyNums  TypeNum() const = 0;

         const cPt2di & Sz() const {return cRect2::Sz();}
         Type operator() (int aX,int  aY) const {return V_GetElem(aX,aY);} ///< Syntactic sugar


         //==== Put here to make bench easier, but else not very usefull
         virtual double TriangSupicity() const ; ///< How much is  it triangular sup
         virtual void SelfSymetrizeBottom() ;    ///< Symetrize by setting copying up in Bottom

                     //  ============= tREAL4 ===============
         virtual void  Add_tAB(const tDV & aCol,const tDV & aLine) ;
         virtual void  Add_tAA(const tDV & aColLine,bool OnlySup=true) ;
         virtual void  Sub_tAA(const tDV & aColLine,bool OnlySup=true) ;
         virtual void  Weighted_Add_tAA(Type aWeight,const tDV & aColLine,bool OnlySup=true) ;

         // Column operation
         virtual void  MulColInPlace(tDV &,const tDV &) const;
         virtual Type MulColElem(int  aY,const tDV &)const;
         tDV  MulCol(const tDV &) const; ///< Create a new vector
         virtual void ReadColInPlace(int aX,tDV &) const;
         virtual tDV  ReadCol(int aX) const;
         virtual void WriteCol(int aX,const tDV &) ;

         // Line operation
         virtual void  MulLineInPlace(tDV &,const tDV &) const;
         virtual Type MulLineElem(int  aX,const tDV &)const;
         tDV  MulLine(const tDV &) const;
         virtual void ReadLineInPlace(int aY,tDV &) const;
         virtual tDV ReadLine(int aY) const;
         virtual void WriteLine(int aY,const tDV &) ;



         //  Matrix multiplication
         void MatMulInPlace(const tMat & aM1,const tMat & aM2);


         // Multiplication of points, works for square matrix so that result is also a point,
         //  only for double now as the memory issue is not  crucial for points
/*
         virtual cPt2dr MulCol(const cPt2dr & aP) const;
         virtual cPt3dr MulCol(const cPt3dr & aP) const;
         virtual cPt2dr MulLine(const cPt2dr & aP) const;
         virtual cPt3dr MulLine(const cPt3dr & aP) const;
*/

              // Check size  inline so that they can be skipped in optimized mode
         /// Check that aVY * this  is valide
         void TplCheckSizeY(const tDV & aV) const
         {
            MMVII_INTERNAL_ASSERT_medium(Sz().y()== aV.Sz(),"Bad size for vect column multiplication")
         }
         /// Check that this * aVx  is valide
         void TplCheckSizeX(const tDV & aV) const
         {
            MMVII_INTERNAL_ASSERT_medium(Sz().x()== aV.Sz(),"Bad size for vect line multiplication")
         }
         /// Check that aVY * this * aVX  is valide, VY line vector of size SzY, VX Col vector
         void TplCheckSizeYandX(const tDV & aVY,const tDV & aVX) const
         {
              TplCheckSizeY(aVY);
              TplCheckSizeX(aVX);
         }

        // void TplCheckX(const cSparseVect & V) {}; template <class Type> class  cSparseVect 
         ///  Check that aM1 * aM2 is valide
      
         void TplCheckX(const cSparseVect<Type> & aV)  const
         {
            MMVII_INTERNAL_ASSERT_medium(aV.IsInside(Sz().x()) ,"Sparse Vector X-out matrix");
         }
         void TplCheckY(const cSparseVect<Type> & aV)  const
         {
            MMVII_INTERNAL_ASSERT_medium(aV.IsInside(Sz().y()) ,"Sparse Vector Y-out matrix");
         }

         static void CheckSizeMul(const tMat & aM1,const tMat & aM2)
         {
            MMVII_INTERNAL_ASSERT_medium(aM1.Sz().x()== aM2.Sz().y() ,"Bad size for mat multiplication")
         }
         ///  Check that this = aM1 * aM2 is valide
         void CheckSizeMulInPlace(const tMat & aM1,const tMat & aM2) const
         {
            CheckSizeMul(aM1,aM2);
            MMVII_INTERNAL_ASSERT_medium(aM1.Sz().y()== Sz().y() ,"Bad size for in place mat multiplication")
            MMVII_INTERNAL_ASSERT_medium(aM2.Sz().x()== Sz().x() ,"Bad size for in place mat multiplication")
         }
         static void CheckSquare(const tMat & aM1)
         {
            MMVII_INTERNAL_ASSERT_medium(aM1.Sz().x()== aM1.Sz().y() ,"Expected Square Matrix")
         }


      //  Sparse vector
        virtual void  Weighted_Add_tAA(Type aWeight,const tSpV & aColLine,bool OnlySup=true);

      //  Constructor && destr
         virtual ~cMatrix();  ///< Public because called by shared ptr 
     protected :
         cMatrix(int aX,int aY);

     private :
};

template <class Type> cDenseVect<Type> operator * (const cDenseVect<Type> &,const cMatrix<Type>&);
template <class Type> cDenseVect<Type> operator * (const cMatrix<Type>&,const cDenseVect<Type> &);

/** Unopt dense Matrix are not very Usefull in "final" version
of  MMVII, but it's the opportunity to test the default method
implemanted in cMatrix,   It contains all the data of cDenseMatrix
 */


template <class Type> class cUnOptDenseMatrix : public cMatrix<Type>
{
    public :
        typedef cIm2D<Type> tIm;
        typedef cDataIm2D<Type> tDIm;
        const cPt2di & Sz() const {return cRect2::Sz();}

        cUnOptDenseMatrix(int aX,int aY,eModeInitImage=eModeInitImage::eMIA_NoInit);
        cUnOptDenseMatrix(tIm);

        /// Non virtual for inline without specifying scope cUnOptDenseMatrix::
        const Type & GetElem(int aX,int  aY) const { return DIm().GetV(cPt2di(aX,aY));}
        const Type & GetElem(const cPt2di & aP) const { return DIm().GetV(aP);}
        // Type & GetElem(int aX,int  aY) { return DIm().GetV(cPt2di(aX,aY));}
        // Type & GetElem(const cPt2di & aP) { return DIm().GetV(aP);}  => non implementee car pas de check val


        /// Non virtual for inline without specifying scope cUnOptDenseMatrix::
        void  SetElem(int  aX,int  aY,const Type & aV) { DIm().SetV(cPt2di(aX,aY),aV);}

        Type V_GetElem(int aX,int  aY) const override ;
        void  V_SetElem(int  aX,int  aY,const Type &) override;
        virtual eTyNums  TypeNum() const override;

       tDIm & DIm() {return mIm.DIm();}
       const tDIm & DIm() const {return mIm.DIm();}
       tIm & Im() {return mIm;}
       const tIm & Im() const {return mIm;}
   protected :

       tIm  mIm;  ///< Image that contains the data
};

template <class Type> class cResulSymEigenValue;
template <class Type> class cConst_EigenMatWrap;
template <class Type> class cNC_EigenMatWrap;
template <class Type> class cResulQR_Decomp;

/**  Dense Matrix, probably one single class. 
     Targeted to be instantiated with 4-8-16 byte floating point
     It contains optimzed version of Mul Matrix*vect .  And all the
     algorithm specific to dense matrix decomposition
*/


template <class Type> class cDenseMatrix : public cUnOptDenseMatrix<Type>
{
   public :
        typedef Type            tVal;
        typedef cSparseVect<Type> tSpV;
        typedef cDenseVect<Type> tDV;
        typedef cDataIm2D<Type> tDIm;
        typedef cIm2D<Type> tIm;

        typedef  cMatrix<Type>           tMat;
        typedef  cUnOptDenseMatrix<Type> tUO_DM;
        typedef  cDenseMatrix<Type>      tDM;

        typedef cConst_EigenMatWrap<Type> tConst_EW;
        typedef cNC_EigenMatWrap<Type> tNC_EW;


        const cPt2di & Sz() const {return cRect2::Sz();}
        cDenseMatrix(int aX,int aY,eModeInitImage=eModeInitImage::eMIA_NoInit);
        cDenseMatrix(int aX,eModeInitImage=eModeInitImage::eMIA_NoInit);  ///< Square
        cDenseMatrix(tIm);
        cDenseMatrix Dup() const;
        static cDenseMatrix Diag(const tDV &);

        // To contourn borring new template scope ....
        const tDIm & DIm() const {return tUO_DM::DIm();}
        tDIm & DIm() {return tUO_DM::DIm();}
        tIm & Im() {return tUO_DM::Im();}
        const tIm & Im() const {return tUO_DM::Im();}
        const Type & GetElem(int aX,int  aY) const    { return tUO_DM::GetElem(cPt2di(aX,aY));}
        const Type & GetElem(const cPt2di & aP) const { return tUO_DM::GetElem(aP);}
        // Type & GetElem(int aX,int  aY)     { return tUO_DM::GetElem(cPt2di(aX,aY));}
        // Type & GetElem(const cPt2di & aP)  { return tUO_DM::GetElem(aP);}
        void  SetElem(int  aX,int  aY,const Type & aV) {  tUO_DM::SetElem(aX,aY,aV);}

        void Show() const;

        //  ====  Mul and inverse =========
              /** this does not override  cMatrix::MatMulInPlace as it is a specialization
                 to dense matrix, call eigen
              */
        void MatMulInPlace(const tDM & aM1,const tDM & aM2);

       
        tDM  Inverse() const;  ///< Basic inverse
        tDM  Inverse(double Eps,int aNbIter) const;  ///< N'amene rien, eigen fonctionne deja tres bien en general 

        tDM  Solve(const tDM &,eTyEigenDec aType=eTyEigenDec::eTED_PHQR) const;
        tDV  Solve(const tDV &,eTyEigenDec aType=eTyEigenDec::eTED_PHQR) const;


        //  ====  Orthognal matrix

        double Unitarity() const; ///< test the fact that M is unatiry, basic : distance of Id to tM M
        cResulSymEigenValue<Type> SymEigenValue() const;

        cResulQR_Decomp<Type>  QR_Decomposition() const;

        //  ====  Symetricity/Transpose/Triangularise manipulation

        double Symetricity() const; ///< how much close to a symetrix matrix, square only , 
        void SelfSymetrize() ; ///< replace by closest  symetrix matrix, square only
        tDM    Symetrize() const ; ///< return closest  symetrix matrix, square only

        double AntiSymetricity() const; ///< how much close to a symetrix matrix, square only
        void SelfAntiSymetrize() ; ///< closest  symetrix matrix, square only
        tDM    AntiSymetrize() const ; ///< closest  symetrix matrix, square only

        void   SelfTriangSup();         ///< Make the image triangular
        double TriangSupicity() const override;        ///< How much is  it triangular sup
        void SelfSymetrizeBottom() override ; ///< Symetrize by setting copying up in Bottom

        void TransposeIn(tDM & M2) const;  ///< Put transposate in M2
        void SelfTransposeIn() ;  ///< transposate in this, square only
        tDM  Transpose() const;  ///< Put transposate in M2
         
        double Diagonalicity() const; ///< how much close to a diagonal matrix, square only , 

        //  =====   Overridng of cMatrix classe  ==== 
        void  MulColInPlace(tDV &,const tDV &) const override;
        Type MulColElem(int  aY,const tDV &)const override;
        void  MulLineInPlace(tDV &,const tDV &) const override;
        Type MulLineElem(int  aX,const tDV &)const override;
        void  Add_tAB(const tDV & aCol,const tDV & aLine) override;
        void  Add_tAA(const tDV & aColLine,bool OnlySup=true) override;
        void  Sub_tAA(const tDV & aColLine,bool OnlySup=true) override;

        void  Weighted_Add_tAA(Type aWeight,const tDV & aColLine,bool OnlySup=true) override;

        // ====  Sparse vector 
        void  Weighted_Add_tAA(Type aWeight,const tSpV & aColLine,bool OnlySup=true) override;
};

template <class Type> class cResulSymEigenValue
{
    public :
        friend class cDenseMatrix<Type>;

        cResulSymEigenValue(int aNb);
        cDenseMatrix<Type>  OriMatr() const; ///< Check the avability to reconstruct original matrix

        const cDenseVect<Type>   &  EigenValues() const ;  ///< Eigen values
        const cDenseMatrix<Type> &  EigenVectors()const ; ///< Eigen vector

    private :
        cDenseVect<Type>    mEigenValues;  ///< Eigen values
        cDenseMatrix<Type>  mEigenVectors; ///< Eigen vector
};

template <class Type> class cResulQR_Decomp
{
    public :
        friend class cDenseMatrix<Type>;

        cResulQR_Decomp(int aSzX,int aSzY);
        cDenseMatrix<Type>  OriMatr() const;

        const cDenseMatrix<Type> &  Q_Matrix() const; ///< Unitary
        const cDenseMatrix<Type> &  R_Matrix() const; ///< Triang

    private :
        cDenseMatrix<Type>  mQ_Matrix; ///< Unitary Matrix
        cDenseMatrix<Type>  mR_Matrix; ///< Triangular superior

};




template <class Type> cDenseMatrix<Type> operator * (const cDenseMatrix<Type> &,const cDenseMatrix<Type>&);
template <class T1,class T2> cDenseVect<T1> operator * (const cDenseVect<T1> &,const cDenseMatrix<T2>&);
template <class T1,class T2> cDenseVect<T1> operator * (const cDenseMatrix<T2>&,const cDenseVect<T1> &);


// Not usefull  as cUnOptDenseMatrix is not usefull either, but required in bench
template <class Type> cUnOptDenseMatrix<Type> operator * (const cUnOptDenseMatrix<Type> &,const cUnOptDenseMatrix<Type>&);
template <class T1,class T2> cDenseVect<T1> operator * (const cDenseVect<T1> &,const cUnOptDenseMatrix<T2>&);
template <class T1,class T2> cDenseVect<T1> operator * (const cUnOptDenseMatrix<T2>&,const cDenseVect<T1> &);

#if(0)
#endif

/**  Class to calculate/manipulate 1-2 moment : aver/var/cov
*/
template <class Type> class cStrStat2
{
    public :
       cStrStat2(int aSz);
       /// Add a vectors to stats
       void Add(const cDenseVect<Type> & );
       /// Make average (instead of sums) and centered (for cov)
       void Normalise(bool CenteredAlso=true);
       ///  Compute eigen values
       const cResulSymEigenValue<Type> & DoEigen();
       /// Coordinate in the orthognal system were aver=0 and variables are uncorralted
       void ToNormalizedCoord(cDenseVect<Type>  & aV1,const cDenseVect<Type>  & aV2) const;
       /// Kth Coordinate of previous
       double KthNormalizedCoord(int,const cDenseVect<Type>  & aV2) const;
       // Accessors
       cDenseMatrix<Type>& Cov() ;
       const double              Pds() const;
       const cDenseVect<Type>  & Moy() const;
       const cDenseMatrix<Type>& Cov() const;
    private :
       int                       mSz;  ///<  Size
       double                    mPds; ///< Sum of weight (later we will have possibility to add weight)
       cDenseVect<Type>          mMoy; ///< Som/average
       cDenseVect<Type>          mMoyMulVE; ///< Moy * EigenVect
       mutable cDenseVect<Type>  mTmp; ///< Use as temporary for some computation
       cDenseMatrix<Type>        mCov;  ///< Cov Matrix
       cResulSymEigenValue<Type> mEigen;  ///< Eigen/Value vectors stored here after DoEigen
};


/** More a less special case to  cStrStat2 for case 2 variable, very current and
probably much more efficient */

template <class Type> class cMatIner2Var
{
    public :
       cMatIner2Var ();
       void Add(const double & aPds,const Type & aV1,const Type & aV2);
       const Type & S0()  const {return mS0;}
       const Type & S1()  const {return mS1;}
       const Type & S11() const {return mS11;}
       const Type & S2()  const {return mS2;}
       const Type & S12() const {return mS12;}
       const Type & S22() const {return mS22;}
       void Normalize();
    private :
        Type  mS0;   ///< Som of    W
        Type  mS1;   ///< Som of    W * V1
        Type  mS11;  ///< Som of    W * V1 * V1
        Type  mS2;   ///< Som of    W * V2
        Type  mS12;  ///< Som of    W * V1 * V2
        Type  mS22;  ///< Som of    W * V2 * V2
};

/// A function rather specific to bench, assimilate image to a distribution on var X,Y and compute it 0,1,2 moments
template <class Type> cMatIner2Var<double> StatFromImageDist(const cDataIm2D<Type> & aIm);

///  BUGED:  Class to compute non biased variance from a statisic 

/** Class to compute non biased variance from a statisic
    This generalise the standard formula
            EstimVar = N/(N-1) EmpirVar
    To the case where there is a weighting.

    It can be uses with N variable to factorize the computation on Weight

     The method id BUGED as the theoreticall formula is probably wrong, or maybe it is the bench itself ?
     However it's probably worth use it than nothing ...
 */

template <const int Dim> class cComputeStdDev
{
    public :
        typedef  double tTab[Dim];

        cComputeStdDev();

        void Add(const  double * aVal,const double & aPds);
        const double  *  ComputeUnBiasedVar() ;
        const double  *  ComputeBiasedVar() ;
        double  DeBiasFactor() const;

        bool OkForUnBiasedVar() const; ///< Test DeBiasFactor can be computed and is != 0

    private :
        double    mSomW; ///< Sum of Weight
        double    mSomWW; ///< Sum of Weight ^2
        tTab      mSomWV;  ///< Weighted som of vals
        tTab      mSomWVV; ///< Weighted som of vals ^2
        tTab      mVar;   ///< Buffer to compute the unbiased variance
        tTab      mBVar;   ///< Buffer to compute the empirical variance
};




};

#endif  //  _MMVII_Matrix_H_
