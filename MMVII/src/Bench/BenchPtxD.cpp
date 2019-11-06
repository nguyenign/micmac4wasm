#include "include/MMVII_all.h"

namespace MMVII
{

void  Bench_cPt2dr()
{
   // Bench Polar function is correct on some test values
   MMVII_INTERNAL_ASSERT_bench(Norm2(cPt2dr(1,1)-FromPolar(sqrt(2.0),M_PI/4.0))<1e-5,"cPt2r Bench");
   MMVII_INTERNAL_ASSERT_bench(Norm2(cPt2dr(-2,0)-FromPolar(2,-M_PI))<1e-5,"cPt2r Bench");
   MMVII_INTERNAL_ASSERT_bench(Norm2(cPt2dr(0,2)-FromPolar(2,M_PI/2.0))<1e-5,"cPt2r Bench");
   MMVII_INTERNAL_ASSERT_bench(Norm2(cPt2dr(0,-2)-FromPolar(2,3*M_PI/2.0))<1e-5,"cPt2r Bench");
   MMVII_INTERNAL_ASSERT_bench(Norm2(cPt2dr(0,-2)-FromPolar(2,7*M_PI/2.0))<1e-5,"cPt2r Bench");
   for (int aK=0 ; aK< 100; aK++)
   {

        double aRho1 = 1e-5 + RandUnif_0_1();
        double aTeta1 =  100.0 * RandUnif_C();
        cPt2dr aP1 = FromPolar(aRho1,aTeta1);

        // Bench Rho
        MMVII_INTERNAL_ASSERT_bench(std::abs(aRho1 - Norm2(aP1))<1e-5,"cPt2r Bench");

        cPt2dr aP1Bis = FromPolar(ToPolar(aP1));
        // Bench FromPolar and ToPolar are invert
        MMVII_INTERNAL_ASSERT_bench(Norm2(aP1-aP1Bis)<1e-5,"cPt2r Bench");

        double aRho2 = 1e-5 + RandUnif_0_1();
        double aTeta2 =  100.0 * RandUnif_C();
        cPt2dr aP2 = FromPolar(aRho2,aTeta2);
        cPt2dr aP1m2 = aP1 * aP2;
        cPt2dr aQ1m2 = FromPolar(aRho1*aRho2,aTeta1+aTeta2);
        // Bench mul complex vis rho-teta
        MMVII_INTERNAL_ASSERT_bench(Norm2(aP1m2 - aQ1m2)<1e-5,"cPt2r Bench");

        cPt2dr aP1d2 = aP1 / aP2;
        cPt2dr aQ1d2 = FromPolar(aRho1/aRho2,aTeta1-aTeta2);
        // Bench div complex vis rho-teta
        MMVII_INTERNAL_ASSERT_bench(Norm2(aP1d2 - aQ1d2)<1e-5,"cPt2r Bench");

        //  StdOut() << "CcccMul " << aP1d2 - aQ1d2 << "\n";
   }
}

void  Bench_0000_Ptxd()
{
    
    Bench_cPt2dr();
    cPt1dr aA1(1);
    cPt2dr aA2(1,2);
    cPt3dr aA3(1,2,3);
    // Test x,y,z
    MMVII_INTERNAL_ASSERT_bench(aA1.x()==1,"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench((aA2.x()==1)&&(aA2.y()==2),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench((aA3.x()==1)&&(aA3.y()==2)&&(aA3.z()==3),"Bench_0000_Ptxd");

    MMVII_INTERNAL_ASSERT_bench(aA1==cPt1dr(1),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench(aA1!=cPt1dr(1.1),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench(aA1==cPt1dr(8)+cPt1dr(-7),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench(aA1==cPt1dr(6)-cPt1dr(5),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench((aA1*2.0)==cPt1dr(2),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench((2.5*aA1)==cPt1dr(2.5),"Bench_0000_Ptxd");


    MMVII_INTERNAL_ASSERT_bench(aA2==cPt2dr(2,1)+cPt2dr(-1,1),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench(aA2==cPt2dr(2,3)-cPt2dr(1,1),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench(aA2!=cPt2dr(1,1),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench(aA2!=cPt2dr(2,2),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench((aA2*2.0)==cPt2dr(2,4),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench((2.5*aA2)==cPt2dr(2.5,5.0),"Bench_0000_Ptxd");


    MMVII_INTERNAL_ASSERT_bench(aA3==cPt3dr(10,10,10)+cPt3dr(-9,-8,-7),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench(aA3==cPt3dr(10,10,10)-cPt3dr(9,8,7),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench(aA3!=cPt3dr(1.1,2,3),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench(aA3!=cPt3dr(1,2.1,3),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench(aA3!=cPt3dr(1,2,3.1),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench(aA3*3.0==cPt3dr(3,6,9),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench(3.0*aA3==cPt3dr(3,6,9),"Bench_0000_Ptxd");

    MMVII_INTERNAL_ASSERT_bench(cPt2dr(1,1) == cPt2dr(1,1),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench(cPt2dr(1,1) != cPt2dr(1,2),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench(cPt2dr(1,1) != cPt2dr(2,1),"Bench_0000_Ptxd");

    cRect2 aR(cPt2di(10,20),cPt2di(50,40));
    MMVII_INTERNAL_ASSERT_bench(aR.Proj(cPt2di(11,22)) == cPt2di(11,22),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench(aR.Proj(cPt2di(1,2)) == cPt2di(10,20),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench(aR.Proj(cPt2di(1,22)) == cPt2di(10,22),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench(aR.Proj(cPt2di(11,19)) == cPt2di(11,20),"Bench_0000_Ptxd");
    MMVII_INTERNAL_ASSERT_bench(aR.Proj(cPt2di(100,100)) == cPt2di(49,39),"Bench_0000_Ptxd");



    StdOut() << "done Bench_0000_Ptxd \n";
}

};

