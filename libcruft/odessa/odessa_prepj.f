      SUBROUTINE ODESSA_PREPJ (NEQ, Y, YH, NYH, WM, IWM, EWT, SAVF, 
     1   FTEM, PAR, F, JAC, JOPT)
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION NEQ(*), Y(*), YH(NYH,*), WM(*), IWM(*), EWT(*),
     1   SAVF(*), FTEM(*), PAR(*)
      EXTERNAL F, JAC
      PARAMETER (ZERO=0.0D0,ONE=1.0D0)
      COMMON /ODE001/ ROWND, ROWNS(173),
     2   RDUM1(37), EL0, H, RDUM2(4), TN, UROUND,
     3   IOWND(14), IOWNS(4),
     4   IDUM1(3), IERPJ, IDUM2, JCUR, IDUM3(4),
     5   MITER, IDUM4(4), N, IDUM5(2), NFE, NJE, IDUM6
C-----------------------------------------------------------------------
C ODESSA_PREPJ IS CALLED BY ODESSA_STODE TO COMPUTE AND PROCESS THE MATRIX
C P = I - H*EL(1)*J , WHERE J IS AN APPROXIMATION TO THE JACOBIAN.
C IF ISOPT = 1, ODESSA_PREPJ IS ALSO CALLED BY ODESSA_SPRIME WITH JOPT = 1.
C HERE J IS COMPUTED BY THE USER-SUPPLIED ROUTINE JAC IF
C MITER = 1 OR 4, OR BY FINITE DIFFERENCING IF MITER = 2, 3, OR 5.
C IF MITER = 3, A DIAGONAL APPROXIMATION TO J IS USED.
C J IS STORED IN WM AND REPLACED BY P.  IF MITER .NE. 3, P IS THEN
C SUBJECTED TO LU DECOMPOSITION (JOPT = 0) IN PREPARATION FOR LATER
C SOLUTION OF LINEAR SYSTEMS WITH P AS COEFFICIENT MATRIX. THIS IS
C DONE BY DGETRF IF MITER = 1 OR 2, AND BY DGBTRF IF MITER = 4 OR 5.
C
C IN ADDITION TO VARIABLES DESCRIBED PREVIOUSLY, COMMUNICATION
C WITH ODESSA_PREPJ USES THE FOLLOWING..
C Y     = ARRAY CONTAINING PREDICTED VALUES ON ENTRY.
C FTEM  = WORK ARRAY OF LENGTH N (ACOR IN ODESSA_STODE).
C SAVF  = ARRAY CONTAINING F EVALUATED AT PREDICTED Y.
C WM    = REAL WORK SPACE FOR MATRICES.  ON OUTPUT IT CONTAINS THE
C         INVERSE DIAGONAL MATRIX IF MITER = 3 AND THE LU DECOMPOSITION
C         OF P IF MITER IS 1, 2 , 4, OR 5.
C         STORAGE OF MATRIX ELEMENTS STARTS AT WM(3).
C         WM ALSO CONTAINS THE FOLLOWING MATRIX-RELATED DATA..
C         WM(1) = SQRT(UROUND), USED IN NUMERICAL JACOBIAN INCREMENTS.
C         WM(2) = H*EL0, SAVED FOR LATER USE IF MITER = 3.
C IWM   = INTEGER WORK SPACE CONTAINING PIVOT INFORMATION, STARTING AT
C         IWM(21), IF MITER IS 1, 2, 4, OR 5.  IWM ALSO CONTAINS BAND
C         PARAMETERS ML = IWM(1) AND MU = IWM(2) IF MITER IS 4 OR 5.
C EL0   = EL(1) (INPUT).
C IERPJ = OUTPUT ERROR FLAG,  = 0 IF NO TROUBLE, .GT. 0 IF
C         P MATRIX FOUND TO BE SINGULAR.
C JCUR  = OUTPUT FLAG = 1 TO INDICATE THAT THE JACOBIAN MATRIX
C         (OR APPROXIMATION) IS NOW CURRENT.
C JOPT  = INPUT JACOBIAN OPTION, = 1 IF JAC IS DESIRED ONLY.
C THIS ROUTINE ALSO USES THE COMMON VARIABLES EL0, H, TN, UROUND,
C IERPJ, JCUR, MITER, N, NFE, AND NJE.
C-----------------------------------------------------------------------
      NJE = NJE + 1
      IERPJ = 0
      JCUR = 1
      HL0 = H*EL0
      GO TO (100, 200, 300, 400, 500), MITER
C IF MITER = 1, CALL JAC AND MULTIPLY BY SCALAR. -----------------------
 100   LENP = N*N
      DO 110 I = 1,LENP
 110    WM(I+2) = ZERO
      CALL JAC (NEQ, TN, Y, PAR, 0, 0, WM(3), N)
      IF (JOPT .EQ. 1) RETURN
      CON = -HL0
      DO 120 I = 1,LENP
 120    WM(I+2) = WM(I+2)*CON
      GO TO 240
C IF MITER = 2, MAKE N CALLS TO F TO APPROXIMATE J. --------------------
 200   FAC = ODESSA_VNORM (N, SAVF, EWT)
      R0 = 1000.0D0*DABS(H)*UROUND*DBLE(N)*FAC
      IF (R0 .EQ. ZERO) R0 = ONE
      SRUR = WM(1)
      J1 = 2
      DO 230 J = 1,N
        YJ = Y(J)
        R = DMAX1(SRUR*DABS(YJ),R0/EWT(J))
        Y(J) = Y(J) + R
        FAC = -HL0/R
        CALL F (NEQ, TN, Y, PAR, FTEM)
        DO 220 I = 1,N
 220      WM(I+J1) = (FTEM(I) - SAVF(I))*FAC
        Y(J) = YJ
        J1 = J1 + N
 230    CONTINUE
      NFE = NFE + N
      IF (JOPT .EQ. 1) RETURN
C ADD IDENTITY MATRIX. -------------------------------------------------
 240   J = 3
      DO 250 I = 1,N
        WM(J) = WM(J) + ONE
 250    J = J + (N + 1)
C DO LU DECOMPOSITION ON P. --------------------------------------------
      CALL DGETRF ( N, N, WM(3), N, IWM(21), IER)
      IF (IER .NE. 0) IERPJ = 1
      RETURN
C IF MITER = 3, CONSTRUCT A DIAGONAL APPROXIMATION TO J AND P. ---------
 300   WM(2) = HL0
      R = EL0*0.1D0
      DO 310 I = 1,N
 310    Y(I) = Y(I) + R*(H*SAVF(I) - YH(I,2))
      CALL F (NEQ, TN, Y, PAR, WM(3))
      NFE = NFE + 1
      DO 320 I = 1,N
        R0 = H*SAVF(I) - YH(I,2)
        DI = 0.1D0*R0 - H*(WM(I+2) - SAVF(I))
        WM(I+2) = 1.0D0
        IF (DABS(R0) .LT. UROUND/EWT(I)) GO TO 320
        IF (DABS(DI) .EQ. ZERO) GO TO 330
        WM(I+2) = 0.1D0*R0/DI
 320    CONTINUE
      RETURN
 330   IERPJ = 1
      RETURN
C IF MITER = 4, CALL JAC AND MULTIPLY BY SCALAR. -----------------------
 400   ML = IWM(1)
      MU = IWM(2)
      ML3 = ML + 3
      MBAND = ML + MU + 1
      MEBAND = MBAND + ML
      LENP = MEBAND*N
      DO 410 I = 1,LENP
 410    WM(I+2) = ZERO
      CALL JAC (NEQ, TN, Y, PAR, ML, MU, WM(ML3), MEBAND)
      IF (JOPT .EQ. 1) RETURN
      CON = -HL0
      DO 420 I = 1,LENP
 420    WM(I+2) = WM(I+2)*CON
      GO TO 570
C IF MITER = 5, MAKE MBAND CALLS TO F TO APPROXIMATE J. ----------------
 500   ML = IWM(1)
      MU = IWM(2)
      MBAND = ML + MU + 1
      MBA = MIN0(MBAND,N)
      MEBAND = MBAND + ML
      MEB1 = MEBAND - 1
      SRUR = WM(1)
      FAC = ODESSA_VNORM (N, SAVF, EWT)
      R0 = 1000.0D0*DABS(H)*UROUND*DBLE(N)*FAC
      IF (R0 .EQ. ZERO) R0 = ONE
      DO 560 J = 1,MBA
        DO 530 I = J,N,MBAND
          YI = Y(I)
          R = DMAX1(SRUR*DABS(YI),R0/EWT(I))
 530      Y(I) = Y(I) + R
        CALL F (NEQ, TN, Y, PAR, FTEM)
        DO 550 JJ = J,N,MBAND
          Y(JJ) = YH(JJ,1)
          YJJ = Y(JJ)
          R = DMAX1(SRUR*DABS(YJJ),R0/EWT(JJ))
          FAC = -HL0/R
          I1 = MAX0(JJ-MU,1)
          I2 = MIN0(JJ+ML,N)
          II = JJ*MEB1 - ML + 2
          DO 540 I = I1,I2
 540        WM(II+I) = (FTEM(I) - SAVF(I))*FAC
 550      CONTINUE
 560    CONTINUE
      NFE = NFE + MBA
      IF (JOPT .EQ. 1) RETURN
C ADD IDENTITY MATRIX. -------------------------------------------------
 570   II = MBAND + 2
      DO 580 I = 1,N
        WM(II) = WM(II) + ONE
 580    II = II + MEBAND
C DO LU DECOMPOSITION OF P. --------------------------------------------
      CALL DGBTRF ( N, N, ML, MU, WM(3), MEBAND, IWM(21), IER)
      IF (IER .NE. 0) IERPJ = 1
      RETURN
C---------------- END OF SUBROUTINE ODESSA_PREPJ -----------------------
      END
