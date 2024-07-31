         START , 
         YREGS ,                   register equates, syslib SYS1.MACLIB 
CRTMEM   CSECT , 
CRTMEM   AMODE 31 
CRTMEM   RMODE ANY
        SYSSTATE AMODE64=NO,ARCHLVL=OSREL,OSREL=SYSSTATE 
        IEABRCX  DEFINE    convert based branches to relative
*------------------------------------------------------------------- 
* Linkage and storage obtain
*------------------------------------------------------------------- 
         BAKR  R14,0                use linkage stack 
         LARL  R12,DATCONST         setup base for CONSTANTS
         USING DATCONST,R12         "baseless" CSECT 
        STORAGE OBTAIN,LENGTH=WALEN,EXECUTABLE=NO,LOC=24,CHECKZERO=YES
         LR    R10,R1               R10 points to Working Storage 
         USING WAREA,R10            BASE FOR DSECT 
*
* Clear storage
*
         CHI   R15,X'0014'           X'14': storage zeroed
         BE    STG_WA_CLEAR
         LR    R2,R1                 system did not clear, do ourselves
         LA    R3,WALEN
         XR    R5,R5
         MVCL  R2,R4                 clear storage (pad byte zero)

STG_WA_CLEAR DS 0H
*
         MVC   SAVEA+4(4),=C'F1SA'  linkage stack convention 
         LA    R13,SAVEA            ADDRESS OF OUR SA IN R13 

*------------------------------------------------------------------- 
* application logic                                                - 
*------------------------------------------------------------------- 

*
* DCB has to be below the line
*
        STORAGE OBTAIN,LENGTH=DCBLEN,EXECUTABLE=NO,LOC=24,CHECKZERO=YES
         LR R8,R1                   R8 points to Output DCB
         USING DCBAREA,R8
*
* Clear storage
*
         CHI   R15,X'0014'           X'14': storage zeroed
         BE    STG_DCB_CLEAR
         LR    R2,R1                 system did not clear, do ourselves
         LA    R3,DCBLEN
         XR    R5,R5
         MVCL  R2,R4                 clear storage (pad byte zero)

STG_DCB_CLEAR DS 0H
*
*
* Copy the DCB template into 24-bit storage
* The OPEN_PARMS and DCBE is 31-bit to minimize below-line stg
*
LIB_OPEN  DS  0H
         MVC LIB_DCB(DCBLEN),CONST_DCB
         MVC OPEN_PARMS(OPENLEN),CONST_OPEN
        OPEN (LIB_DCB,OUTPUT),MF=(E,OPEN_PARMS),MODE=31
         CIJE R15,0,OPEN_SUCCESS
*
OPEN_FAIL DS  0H
         LR  R9,R15                put err code in R9
         B   DONE
*
OPEN_SUCCESS DS 0H

MEM_WRITE  DS  0H
         MVC   WRITE_DECB(WRITELEN),DECBMODW
         LA   R2,WRITE_BUFFER
         LA   R3,BUFFLEN
         LA   R4,CONST_BUFFER
         LR   R5,R3
         MVCL R2,R4              Copy CONST_BUFFER to WRITE_BUFFER
        WRITE WRITE_DECB,SF,LIB_DCB,WRITE_BUFFER,'S',MF=E

WRITE_SUCCESS DS 0H

MEM_CHECK  DS 0H
         CHECK WRITE_DECB             WAIT UNTIL COMPLETE

MEM_NOTE   DS 0H
         NOTE LIB_DCB                 GET TTR TO 1st (ONLY) BLOCK
         STCM  R1,14,MEM_TTR          SAVE TTR FOR USE BY STOW
*
         MVC   MEM_IFF(MEMINFOLEN),CONST_MEM_IFF
         LA    R2,MEM_DIR
         ST    R2,MEM_DIRA
         STOW  LIB_DCB,MEM_IFF,IFF     ADD TO DIRECTORY

         CIJE R15,4,STOW_SUCCESS   4 is expected - create new mem
*
STOW_FAIL DS  0H
         LR  R9,R15                put err code in R9
         B   DONE
*
STOW_SUCCESS DS 0H
         SR  R9,R9                 clear R9 to 0
*
LIB_CLOSE  DS 0H
         MVC CLOSE_PARMS(CLOSELEN),CONST_CLOSE
        CLOSE (LIB_DCB),MF=(E,CLOSE_PARMS),MODE=31
         CIJE R15,0,CLOSE_SUCCESS
*
CLOSE_FAIL DS  0H
         LR  R9,R15                put err code in R9
         B   DONE
*
CLOSE_SUCCESS DS 0H
         LA  R9,0                  CRTMEM successful put 0 in R9
*
* Free DCB storage
*
RLSE_DCB   DS 0H
        STORAGE RELEASE,ADDR=(R8),LENGTH=DCBLEN,EXECUTABLE=NO 

*------------------------------------------------------------------- 
* Linkage and storage release. set RC (reg 15)                     -
*------------------------------------------------------------------- 
DONE     DS 0H
RLSE_WA  DS 0H
        STORAGE RELEASE,ADDR=(R10),LENGTH=WALEN,EXECUTABLE=NO 
         LR    R15,R9               get saved rc into R15
         PR    ,                    return to caller 

*------------------------------------------------------------------- 
* constants and literal pool                                       - 
*------------------------------------------------------------------- 
DATCONST   DS    0D                 Doubleword alignment for LARL
CONST_DCB  DCB   DSORG=PO,MACRF=(W),DDNAME=MYDD,DCBE=CONST_DCBE
DCBLEN    EQU   *-CONST_DCB
CONST_DCBE DCBE  RMODE31=BUFF
CONST_OPEN OPEN (*-*,(OUTPUT)),MODE=31,MF=L
OPENLEN   EQU   *-CONST_OPEN
CONST_CLOSE CLOSE (*-*),MODE=31,MF=L
CLOSELEN  EQU   *-CONST_CLOSE
*
WRITE    WRITE  DECBMODW,SF,0,0,'S',MF=L
WRITELEN EQU   *-DECBMODW

CONST_BUFFER DC CL80'Hello world'
             DC CL80'Velo world'
             DC CL80'Cello world'
             DC CL80'Yellow world'
BUFFLEN EQU *-CONST_BUFFER

CONST_MEM_IFF      DS 0F
CONST_MEM_LEN      DC AL2(MEMIFFLEN)
CONST_MEM_FLAGS    DC XL1'0'
CONST_MEM_DCB24    DC XL3'0'
CONST_MEM_TIME     DC 2F'0'
CONST_MEM_DIRA     DC 1A(0)
CONST_MEM_TYPE     DC XL16'0'
CONST_MEM_CCSID    DC 1H'1047'
MEMIFFLEN EQU *-CONST_MEM_IFF

CONST_MEM_DIR      DS 0F
CONST_MEM_NAME     DC CL8'NEWMEM'
CONST_MEM_TTR      DC XL3'0'
MEMINFOLEN EQU *-CONST_MEM_IFF

         LTORG ,

*------------------------------------------------------------------- 
* DSECT                                                            - 
*------------------------------------------------------------------- 
WAREA       DSECT 
SAVEA       DS    18F 
OPEN_PARMS  DS CL(OPENLEN)
CLOSE_PARMS DS CL(CLOSELEN)
WRITE_DECB  DS CL(WRITELEN)
WRITE_BUFFER DS CL(BUFFLEN)

MEM_IFF      DS 0F
MEM_LEN      DS 1H
MEM_FLAGS    DS XL1
MEM_DCB24    DS XL3
MEM_TIME     DS 2F
MEM_DIRA     DS 1A
MEM_TYPE     DS XL16
MEM_CCSID    DS 1H

MEM_DIR      DS 0F
MEM_NAME     DS CL8
MEM_TTR      DS XL3
WALEN       EQU  *-SAVEA

DCBAREA     DSECT
LIB_DCB     DS   CL(DCBLEN)
         END   CRTMEM   
