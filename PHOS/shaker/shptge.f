*CMZ :          17/07/98  15.49.05  by  Federico Carminati
*-- Author :
      SUBROUTINE SHPTGE(KF,PT,PHI,W)
c	==============================

c	Pt generation

*KEEP,LUDAT1.
      COMMON /LUDAT1/ MSTU(200),PARU(200),MSTJ(200),PARJ(200)
      SAVE /LUDAT1/
*KEEP,SHRAND.
      COMMON /SHRAND/ PISP1(100),PISP2(100),ETASP1(100),ETASP2(100),
     +                  PROSP(100),KAOSP(100)
*KEEP,SHPHYP.
      COMMON /SHPHYP/ JWEI,NDNDY,YLIM,PTLIM,JWEAK,JPI0,JETA,JPIC,JPRO,
     +                  JKAC,JKA0,JRHO,JOME,JPHI,JPSI,JDRY
*KEEP,SHGENE.
      COMMON /SHGENE/ IEVT,NPI0,NETA,NPIC,NPRO,NKAC,NKA0,NRHO,NOME,
     +                  NPHI,NPSI,NDRY
*KEEP,SHNORM.
      COMMON /SHNORM/ PINOR,PIRAT,ETANOR,ETARAT,RHONOR,OMENOR,PHINOR,
     +                  PSINOR,DRYNOR
*KEEP,SHPRAT.
      COMMON /SHPRAT/ PI0R,ETAR,RHOR,OMER,PHIR,PSIR,DRYR
*KEND.

      CHARACTER   CODEP*16

      KFA = ABS(KF)

      IF (JWEI.EQ.0) THEN
        W = 1.
        IF (KFA.EQ.111.OR.KFA.EQ.211) THEN
          IF(RLU(0).LE.PIRAT) THEN
            CALL FUNRAN(PISP1,PT)
          ELSE
            CALL FUNRAN(PISP2,PT)
          ENDIF
c	    PT = 10.
 	  ELSE IF (KFA.EQ.221) THEN
          IF(RLU(0).LE.ETARAT) THEN
            CALL FUNRAN(ETASP1,PT)
          ELSE
            CALL FUNRAN(ETASP2,PT)
          ENDIF
C	    PT = 2.744
        ELSE IF (KFA.EQ.2212) THEN
          CALL FUNRAN(PROSP,PT)
        ELSE IF (KFA.EQ.321.OR.KFA.EQ.311) THEN
          CALL FUNRAN(KAOSP,PT)
        ELSE
          CALL LUNAME(KFA,CODEP)
          WRITE(MSTU(11),*)'ERROR:'
          WRITE(MSTU(11),*)CODEP,'NOT generated with JWEI=0'
          WRITE(MSTU(11),*)'EXECUTION STOPPED!'
          STOP
        ENDIF
      ENDIF

      IF (JWEI.EQ.1) THEN
        PT = PTLIM*RLU(0)
        IF (KFA.EQ.111.OR.KFA.EQ.211) THEN
          W = PTLIM*SHFPI(PT)*PI0R/PINOR/FLOAT(NPI0)
        ELSE IF (KFA.EQ.221) THEN
          W = PTLIM*SHFETA(PT)*ETAR/ETANOR/FLOAT(NETA)
        ELSE IF (KFA.EQ.113) THEN
            W = PTLIM*SHFRHO(PT)*RHOR/RHONOR/FLOAT(NRHO)
        ELSE IF (KFA.EQ.223) THEN
            W = PTLIM*SHFOME(PT)*OMER/OMENOR/FLOAT(NOME)
        ELSE IF (KFA.EQ.333) THEN
            W = PTLIM*SHFPHI(PT)*PHIR/PHINOR/FLOAT(NPHI)
        ELSE IF (KFA.EQ.443) THEN
            W = PTLIM*SHFPSI(PT)*PSIR/PSINOR/FLOAT(NPSI)
        ELSE
          CALL LUNAME(KFA,CODEP)
          WRITE(MSTU(11),*)'ERROR:'
          WRITE(MSTU(11),*)CODEP,'NOT generated with JWEI=1'
          WRITE(MSTU(11),*)'EXECUTION STOPPED!'
          STOP
        ENDIF
      ENDIF

      PHI = 3.14159*2*(RLU(0)-.5)

      RETURN
      END
