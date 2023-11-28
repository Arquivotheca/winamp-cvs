/*****************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2005)
                               All Rights Reserved

   $Id: mathlib.h,v 1.1 2009/04/28 20:17:41 audiodsp Exp $
   project:              mathematical library
   Initial author:       W. Schildbach
   contents/description: Mathlib V2.0 header file

   This software and/or program is protected by copyright law and
   international treaties. Any reproduction or distribution of this 
   software and/or program, or any portion of it, may result in severe 
   civil and criminal penalties, and will be prosecuted to the maximum 
   extent possible under law.

   Conventions used:
   - all routine names end with the type they act upon, i.e. addFLOAT adds
     float vectors.
   - if routines change their input parameters, it will be the last parameter
     (excluding vector increment and length parameters).
     read addFLOAT(X,Y,Z,n) as X+Y->Z (of dimension n)
   - scalar parameters (excluding vector increment and length) are the first
     parameters.
   - increments immediately follow the vectors they act upon.

   - Vectors are written in capitals, scalars in lower-case.
   - FCOMPLEX numbers are simply two adjoining FLOATs, DCOMPLEX two adjoining
     DOUBLEs.

   All routines assume that input and output vectors either
   - do not overlap at all
   - or are identical.

   Special routines for in-place manipulation will follow; they will be
   named opTYPEip, i.e. addFLOATip(X,Z,n) adds vector X to Z.


******************************************************************************/

#ifndef _mathlib_h
#define _mathlib_h

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct {
    char date[20];
    char versionNo[20];
  } MathLibInfo;

void MathGetLibInfo (MathLibInfo* libInfo);
void InitMathOpt(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

/**@name Mathlib

   Mathlib is a collection of routines acting on vectors.
   (Almost) all subroutine names follow a simple pattern.
   \begin{itemize}
   \item The prefix indicates the operation performed, e.g. #add#, #sub#, ...
   \item The middle part of the name indicates the type the function acts upon,
     i.e. #int#, #float#, ... Only in rare cases do functions accept
     parameters with different types. Notable exceptions are
     \Ref{norm2FCOMPLEX} and \Ref{quantFLOATtoUINT}.
   \item A postfix indicates a variant of the function. Variants come in two
     flavours: #flex# and #ip#. The former accepts vectors with elements
     indices separated by more than 1 (see \Ref{Vector increments}) while the
     latter only works in-place. In some very rare cases this might bring some
     performance wins, especially on DSP platforms.
   \item if routines change their input parameters, it will be the last
     parameter (excluding vector increment and length parameters).
     Read #addFLOAT(X,Y,Z,n)# as $\vec X+\vec Y\to\vec Z$ (of dimension n)
   \item scalar parameters (excluding vector increment and length) are the
     first parameters.
   \item increments immediately follow the vectors they act upon.

   \item Vectors are written in capitals, scalars in lower-case.
   \item #FCOMPLEX# numbers are simply two adjoining #float#s, #DCOMPLEX# two
     adjoining #DOUBLE#s.
   \end{itemize}

   These types of operations are supported:
   \begin{itemize}
   \item \Ref{Simple vector operations}
   \item \Ref{Vector comparison operations}
   \item \Ref{Vector quantizing operations}
   \item \Ref{Transcendent vector operations}
   \item \Ref{Utility routines}
   \item \Ref{Random numbers generators}
   \end{itemize}

   @author Wolfgang Schildbach (#sdb@iis.fhg.de#)
   @version V2.0beta
*/

/**@name Simple vector operations
   All these routines also exist in flex flavour, accepting \Ref{Vector increments}.
  */
/*@{*/


#ifdef __cplusplus
extern "C" {
#endif
  
/* detects CPU features */
void InitMathOpt(void);

int CFFTN(float *afftData,int len, int isign);
int CFFTNRI(float *afftDataReal,float *afftDataImag,int len, int isign);

int CFFTN_NI(float *InRealData,
              float *InImagData,
              float *OutRealData,
              float *OurImagData,
              int len, int isign);

int RFFTN(float *afftData, const float* trigPtr, int len, int isign);

float* CreateSineTable(int len);
void   DestroySineTable(float* trigPtr);

int FreqToBandWithRounding(float freq, float fs, int numOfBands, 
                           const int *bandStartOffset);

int BandToFreqWithRounding(int band, float fs, int numOfLines,
                           const int *bandOffsetTable);

int FreqToBandCut (float freq, float fs, int numOfBands, 
                   const int *bandStartOffset);

void BuildIdxUp (float *inData, int *idx, int elements);

extern float sumFLOAT(const float* X, int n); 
/** $\vec Z=\vec X+\vec Y$. */
extern  void   addFLOAT   (const float X[], const float Y[], float Z[], int n) ; 
extern  void   addFLOATflex  (const float X[], int incX, const float Y[], int incY, float Z[], int incZ, int n) ; 
extern  void   saddFLOAT(float b, const float* X, float* Z, int n);


/** $\vec Z=\vec X-\vec Y$. */
extern  void   subFLOAT   (const float X[], const float Y[], float Z[], int n) ; 
extern  void   subFLOATflex  (const float X[], int incX, const float Y[], int incY, float Z[], int incZ, int n) ; 
/** $Z_i=X_i\cdot Y_i$. */
extern  void   multFLOAT   (const float X[], const float Y[], float Z[], int n) ; 
extern  void   multFLOATflex  (const float X[], int incX, const float Y[], int incY, float Z[], int incZ, int n) ;
/** $Z_i=\frac{X_i}{Y_i}$.
    @see Exception handling */
extern  void   divFLOAT   (const float X[], const float Y[], float Z[], int n) ; 
extern  void   divFLOATflex  (const float X[], int incX, const float Y[], int incY, float Z[], int incZ, int n) ; 

/** $\vec Z=\vec X$. */
extern  void   copyFLOAT   (const float X[], float Z[], int n) ; 
extern  void   copyFLOATflex  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** $\vec Z=a\vec X$. */
extern  void   smulFLOAT   (float a, const float X[], float Z[], int n) ; 
extern  void   smulFLOATflex  (float a, const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i\equiv a$ */
extern void setFLOAT(float a, float X[], int n);
extern void setFLOATflex(float a, float X[], int incX, int n);

/** $z=\vec X \vec Y^T$. */
extern float dotFLOAT(const float X[], const float Y[], int n);
extern float dotFLOATflex(const float X[], int incX, const float Y[], int incY, int n);
/** $z=\left\|X-Y\right\|^2$. */
extern float dist2FLOAT(const float X[], const float Y[], int n);
extern float dist2FLOATflex(const float X[], int incX, const float Y[], int incY, int n);
/** $z=\left\|X\right\|^2$. */
extern float norm2FLOAT(const float X[], int n);
extern void norm2FCOMPLEX(const float* X, float* Z, int n);
/** $Z_i=\left(X_{2 i}\right)^2+\left(X_{2 i+1}\right)^2$. */
extern void rad2FCOMPLEX(const float X[], float Z[], int n);

/* simple vector operations, type int */

/** $\vec Z=\vec X+\vec Y$. */
extern  void   addINT   (const int X[], int const Y[], int Z[], int n) ; 
extern  void   addINTflex  (const int X[], int incX, int const Y[], int incY, int Z[], int incZ, int n) ; 
/** $\vec Z=\vec X-\vec Y$. */
extern  void   subINT   (const int X[], int const Y[], int Z[], int n) ; 
extern  void   subINTflex  (const int X[], int incX, int const Y[], int incY, int Z[], int incZ, int n) ; 
/** $Z_i=X_i\cdot Y_i$. */
extern  void   multINT   (const int X[],int const Y[], int Z[], int n) ; 
extern  void   multINTflex  (const int X[],int incX, int const Y[], int incY, int Z[], int incZ, int n) ; 
/** $Z_i=\frac{X_i}{Y_i}$.
@see Exception handling */


    
extern  void   divINT   (const int X[], int const Y[], int Z[], int n) ;
extern  void   divINTflex  (const int X[], int incX, int const Y[], int incY, int Z[], int incZ, int n) ; 
/** $\vec Z=a\vec X$. */
extern  void   smulINT   (int a, const int X[], int Z[], int n) ; 
extern  void   smulINTflex  (int a, const int X[], int incX, int Z[], int incZ, int n) ; 
/** $\vec Z\gets a\vec Z$.
  This routine (only) works in-place.*/
extern void smultFLOATip(float a, float *X, int n);
/** $\vec Z\gets a\vec Z$.
  This routine (only) works in-place.*/
extern void smultINTip(int a, int *X, int n);
/** $\vec Z=\vec X$. */
extern  void   copyINT   (const int X[], int Z[], int n) ; 
extern  void   copyINTflex  (const int X[], int incX, int Z[], int incZ, int n) ; 
/** $Z_i\equiv a$ */
extern void setINT(int a, int X[], int n);
extern void setINTflex(int a, int X[], int incX, int n);

/*@}*/

/* simple vector operations, type byte */
/** $\vec Z=\vec X$. */
extern void copyCHAR(const char X[], char Z[], int n);

/**@name Vector comparison operations
   All these routines also exist in flex flavour, accepting \Ref{Vector increments}.
  */
/*@{*/

/* type float */
/** $Z_i=\min\left(X_i,Y_i\right)$ */
extern  void   minFLOAT   (const float X[], const float Y[], float Z[], int n) ; 
extern  void   minFLOATflex  (const float X[], int incX, const float Y[], int incY, float Z[], int incZ, int n) ;
extern  float  findminFLOAT(const float* X, int n);
/** $Z_i=\max\left(X_i,Y_i\right)$ */
extern  void   maxFLOAT   (const float X[], const float Y[], float Z[], int n) ; 
extern  void   maxFLOATflex  (const float X[], int incX, const float Y[], int incY, float Z[], int incZ, int n) ; 
extern  float findmaxFLOAT(const float* X, int n);
/** $Z_i=\left\|X_i\right\|$ */
extern  void   absFLOAT   (const float X[], float Z[], int n) ; 
extern  void   absFLOATflex  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** limit the elements of $\vec Z$ to the range $[a,b]$.
    $Z_i=\left\{\begin{array}{ll}a& \mbox{if}\quad X_i<a,\\b& \mbox{if}\quad X_i>b,\\X_i &{\rm otherwise} \end{array}\right.$ */
extern  void   limitFLOAT   (float a, float b, const float X[], float Z[], int n) ; 
extern  void   limitFLOATflex  (float a, float b, const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i={\rm sgn}\left(X_i\right)$.
    $Z_i=\left\{\begin{array}{ll}-1& \mbox{if}\quad X_i<0,\\+1&{\rm otherwise} \end{array}\right.$ */
extern  void   signFLOAT   (const float X[], float Z[], int n) ; 
extern  void   signFLOATflex  (const float X[], int incX, float Z[], int incZ, int n) ; 

/* type int */
/** $Z_i=\min\left(X_i,Y_i\right)$ */
extern  void   minINT   (const int X[], const int Y[], int Z[], int n) ; 
extern  void   minINTflex  (const int X[], int incX, const int Y[], int incY, int Z[], int incZ, int n) ; 
/** $Z_i=\max\left(X_i,Y_i\right)$ */
extern  void   maxINT   (const int X[], const int Y[], int Z[], int n) ; 
extern  void   maxINTflex  (const int X[], int incX, const int Y[], int incY, int Z[], int incZ, int n) ; 
/** $Z_i=\left\|X_i\right\|$ */
extern  void   absINT   (const int X[], int Z[], int n) ; 
extern  void   absINTflex  (const int X[], int incX, int Z[], int incZ, int n) ; 
/** limit the elements of $\vec Z$ to the range $[a,b]$.
    $Z_i=\left\{\begin{array}{ll}a& \mbox{if}\quad X_i<a,\\b& \mbox{if}\quad X_i>b,\\X_i &{\rm otherwise} \end{array}\right.$ */
extern  void   limitINT   (int a, int b, const int X[], int Z[], int n) ; 
extern  void   limitINTflex  (int a, int b, const int X[],int incX,  int Z[], int incZ, int n) ; 
/** $Z_i={\rm sgn}\left(X_i\right)$.
    $Z_i=\left\{\begin{array}{ll}-1& \mbox{if}\quad X_i<0,\\+1&{\rm otherwise} \end{array}\right.$ */
extern  void   signINT   (const int X[], int Z[], int n) ;
extern  void   signINTflex  (const int X[], int incX, int Z[], int incZ, int n) ; 
/*@}*/

/**@name Vector quantizing operations
   All these routines also exist in flex flavour, accepting
   \Ref{Vector increments}. These operations only operate on type float (and
   possibly DOUBLE), with the exception of \Ref{quantFLOATtoUINT}
  */
/*@{*/

/** $Z_i=\left\lfloor X_i\right\rfloor$ */
extern  void   floorFLOAT   (const float X[], float Z[], int n) ; 
extern  void   floorFLOATflex  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i=\left\lceil X_i\right\rceil$ */
extern  void   ceilFLOAT   (const float X[], float Z[], int n) ; 
extern  void   ceilFLOATflex  (const float X[], int incX, float Z[], int incZ, int n) ; 
extern   int   ceillog2( int a );
/** round $Z_i$ to nearest integer. $Z_i=\left\lfloor X_i+\frac{1}{2}\right\rfloor$ */
extern  void   nintFLOAT   (const float X[], float Z[], int n) ; 
extern  void   nintFLOATflex  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** truncate fractional part of $Z_i$ */
extern  void   truncFLOAT   (const float X[], float Z[], int n) ; 
extern  void   truncFLOATflex  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i=\left\lfloor a+b X_i\right\rfloor$. */
extern void quantFLOATtoUINT(float a, float b, const float X[], unsigned int Z[], int n);
/*@}*/

extern void roundFLOAT2FLOAT16(const float A[], float B[], int n);
extern void roundFLOAT2INT(const float A[], int B[], int n);
extern void roundFLOAT2SHORT(const float A[], signed short B[], int n);
/**@name Transcendent vector operations
   All these routines also exist in flex flavour, accepting \Ref{Vector increments}.
  */
/*@{*/

/** $Z_i=\sin\left(X_i\right)$. */
extern  void   sinFLOAT   (const float X[], float Z[], int n) ; 
extern  void   sinFLOATflex  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i=\cos\left(X_i\right)$. */
extern  void   cosFLOAT   (const float X[], float Z[], int n) ; 
extern  void   cosFLOATflex  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i=e^{X_i}$. */
extern  void   expFLOAT   (const float X[], float Z[], int n) ; 
extern  void   expFLOATflex  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i=X_i^a$. */
extern  void   spowFLOAT   (float a, const float X[], float Z[], int n) ; 
extern  void   spowFLOATflex  (float a, const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i=\log\left(X_i\right)$. */
extern  void   logFLOAT   (const float X[], float Z[], int n) ; 
extern  void   logFLOATflex  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i=\log_2\left(X_i\right)$. */
extern  void   log2FLOAT   (const float X[], float Z[], int n) ; 
extern  void   log2FLOATflex  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i=\log_{10}\left(X_i\right)$. */
extern  void   log10FLOAT   (const float X[], float Z[], int n) ; 
extern  void   log10FLOATflex  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i=a\log\left(b X_i\right)$. */
extern  void   alogbFLOAT   (float a, float b, const float X[], float Z[], int n) ; 
extern  void   alogbFLOATflex  (float a, float b, const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i=\sqrt{X_i}$. */
extern  void   sqrtFLOAT   (const float X[], float Z[], int n) ; 
extern  void   sqrtFLOATflex  (const float X[], int incX, float Z[], int incZ, int n) ; 
/*@}*/



  /* optimised versions - depending form the platform */
extern float sumFLOAT_Opt(const float* X, int n);  
  /** $\vec Z=\vec X+\vec Y$. */
extern  void   addFLOAT_Opt   (const float X[], const float Y[], float Z[], int n) ; 
extern  void   addFLOATflex_Opt  (const float X[], int incX, const float Y[], int incY, float Z[], int incZ, int n) ; 
extern  void   saddFLOAT_Opt(float b, const float* X, float* Z, int n);

/** $\vec Z=\vec X-\vec Y$. */
extern  void   subFLOAT_Opt   (const float X[], const float Y[], float Z[], int n) ; 
extern  void   subFLOATflex_Opt  (const float X[], int incX, const float Y[], int incY, float Z[], int incZ, int n) ; 
/** $Z_i=X_i\cdot Y_i$. */
extern  void   multFLOAT_Opt   (const float X[], const float Y[], float Z[], int n) ; 
extern  void   multFLOATflex_Opt  (const float X[], int incX, const float Y[], int incY, float Z[], int incZ, int n) ; 
/** $Z_i=\frac{X_i}{Y_i}$.
    @see Exception handling */
extern  void   divFLOAT_Approx   (const float X[], const float Y[], float Z[], int n) ; 
extern  void   divFLOATflex_Opt  (const float X[], int incX, const float Y[], int incY, float Z[], int incZ, int n) ; 

/** $\vec Z=\vec X$. */
extern  void   copyFLOAT_Opt   (const float X[], float Z[], int n) ; 
extern  void   copyFLOATflex_Opt  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** $\vec Z=a\vec X$. */
extern  void   smulFLOAT_Opt   (float a, const float X[], float Z[], int n) ; 
extern  void   smulFLOATflex_Opt  (float a, const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i\equiv a$ */
extern void setFLOAT_Opt(float a, float X[], int n);
extern void setFLOATflex_Opt(float a, float X[], int incX, int n);

/** $z=\vec X \vec Y^T$. */
extern float dotFLOAT_Opt(const float X[], const float Y[], int n);
extern float dotFLOATflex_Opt(const float X[], int incX, const float Y[], int incY, int n);
/** $z=\left\|X-Y\right\|^2$. */
extern float dist2FLOAT_Opt(const float X[], const float Y[], int n);
extern float dist2FLOATflex_Opt(const float X[], int incX, const float Y[], int incY, int n);
/** $z=\left\|X\right\|^2$. */
extern float norm2FLOAT_Opt(const float X[], int n);
extern void norm2FCOMPLEX_Opt(const float* X, float* Z, int n);
/** $Z_i=\left(X_{2 i}\right)^2+\left(X_{2 i+1}\right)^2$. */
extern void rad2FCOMPLEX_Opt(const float X[], float Z[], int n);

/* simple vector operations, type int */

/** $\vec Z=\vec X+\vec Y$. */
extern  void   addINT_Opt   (const int X[], int const Y[], int Z[], int n) ; 
extern  void   addINTflex_Opt  (const int X[], int incX, int const Y[], int incY, int Z[], int incZ, int n) ; 
/** $\vec Z=\vec X-\vec Y$. */
extern  void   subINT_Opt   (const int X[], int const Y[], int Z[], int n) ; 
extern  void   subINTflex_Opt  (const int X[], int incX, int const Y[], int incY, int Z[], int incZ, int n) ; 
/** $Z_i=X_i\cdot Y_i$. */
extern  void   multINT_Opt   (const int X[],int const Y[], int Z[], int n) ; 
extern  void   multINTflex_Opt  (const int X[],int incX, int const Y[], int incY, int Z[], int incZ, int n) ; 
/** $Z_i=\frac{X_i}{Y_i}$.
@see Exception handling */


    
extern  void   divINT_Opt   (const int X[], int const Y[], int Z[], int n) ; 
extern  void   divINTflex_Opt  (const int X[], int incX, int const Y[], int incY, int Z[], int incZ, int n) ; 
/** $\vec Z=a\vec X$. */
extern  void   smulINT_Opt   (int a, const int X[], int Z[], int n) ; 
extern  void   smulINTflex_Opt  (int a, const int X[], int incX, int Z[], int incZ, int n) ; 
/** $\vec Z\gets a\vec Z$.
  This routine (only) works in-place.*/
extern void smultFLOATip_Opt(float a, float *X, int n);
/** $\vec Z\gets a\vec Z$.
  This routine (only) works in-place.*/
extern void smultINTip_Opt(int a, int *X, int n);
/** $\vec Z=\vec X$. */
extern  void   copyINT_Opt   (const int X[], int Z[], int n) ; 
extern  void   copyINTflex_Opt  (const int X[], int incX, int Z[], int incZ, int n) ; 
/** $Z_i\equiv a$ */
extern void setINT_Opt(int a, int X[], int n);
extern void setINTflex_Opt(int a, int X[], int incX, int n);

/*@}*/

/* simple vector operations, type byte */
/** $\vec Z=\vec X$. */
extern void copyCHAR_Opt(const char X[], char Z[], int n);

/**@name Vector comparison operations
   All these routines also exist in flex flavour, accepting \Ref{Vector increments}.
  */
/*@{*/

/* type float */
/** $Z_i=\min\left(X_i,Y_i\right)$ */
extern  void   minFLOAT_Opt   (const float X[], const float Y[], float Z[], int n) ; 
extern  void   minFLOATflex_Opt  (const float X[], int incX, const float Y[], int incY, float Z[], int incZ, int n) ;
extern  float  findminFLOAT_Opt(const float* X, int n);

/** $Z_i=\max\left(X_i,Y_i\right)$ */
extern  void   maxFLOAT_Opt   (const float X[], const float Y[], float Z[], int n) ; 
extern  void   maxFLOATflex_Opt  (const float X[], int incX, const float Y[], int incY, float Z[], int incZ, int n) ;
extern  float  findmaxFLOAT_Opt(const float* X, int n);
/** $Z_i=\left\|X_i\right\|$ */
extern  void   absFLOAT_Opt   (const float X[], float Z[], int n) ; 
extern  void   absFLOATflex_Opt  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** limit the elements of $\vec Z$ to the range $[a,b]$.
    $Z_i=\left\{\begin{array}{ll}a& \mbox{if}\quad X_i<a,\\b& \mbox{if}\quad X_i>b,\\X_i &{\rm otherwise} \end{array}\right.$ */
extern  void   limitFLOAT_Opt   (float a, float b, const float X[], float Z[], int n) ; 
extern  void   limitFLOATflex_Opt  (float a, float b, const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i={\rm sgn}\left(X_i\right)$.
    $Z_i=\left\{\begin{array}{ll}-1& \mbox{if}\quad X_i<0,\\+1&{\rm otherwise} \end{array}\right.$ */
extern  void   signFLOAT_Opt   (const float X[], float Z[], int n) ; 
extern  void   signFLOATflex_Opt  (const float X[], int incX, float Z[], int incZ, int n) ; 

/* type int */
/** $Z_i=\min\left(X_i,Y_i\right)$ */
extern  void   minINT_Opt   (const int X[], const int Y[], int Z[], int n) ; 
extern  void   minINTflex_Opt  (const int X[], int incX, const int Y[], int incY, int Z[], int incZ, int n) ; 
/** $Z_i=\max\left(X_i,Y_i\right)$ */
extern  void   maxINT_Opt   (const int X[], const int Y[], int Z[], int n) ; 
extern  void   maxINTflex_Opt  (const int X[], int incX, const int Y[], int incY, int Z[], int incZ, int n) ; 
/** $Z_i=\left\|X_i\right\|$ */
extern  void   absINT_Opt   (const int X[], int Z[], int n) ; 
extern  void   absINTflex_Opt  (const int X[], int incX, int Z[], int incZ, int n) ; 
/** limit the elements of $\vec Z$ to the range $[a,b]$.
    $Z_i=\left\{\begin{array}{ll}a& \mbox{if}\quad X_i<a,\\b& \mbox{if}\quad X_i>b,\\X_i &{\rm otherwise} \end{array}\right.$ */
extern  void   limitINT_Opt   (int a, int b, const int X[], int Z[], int n) ; 
extern  void   limitINTflex_Opt  (int a, int b, const int X[],int incX,  int Z[], int incZ, int n) ; 
/** $Z_i={\rm sgn}\left(X_i\right)$.
    $Z_i=\left\{\begin{array}{ll}-1& \mbox{if}\quad X_i<0,\\+1&{\rm otherwise} \end{array}\right.$ */
extern  void   signINT_Opt   (const int X[], int Z[], int n) ; 
extern  void   signINTflex_Opt  (const int X[], int incX, int Z[], int incZ, int n) ; 
/*@}*/

/**@name Vector quantizing operations
   All these routines also exist in flex flavour, accepting
   \Ref{Vector increments}. These operations only operate on type float (and
   possibly DOUBLE), with the exception of \Ref{quantFLOATtoUINT}
  */
/*@{*/

/** $Z_i=\left\lfloor X_i\right\rfloor$ */
extern  void   floorFLOAT_Opt   (const float X[], float Z[], int n) ; 
extern  void   floorFLOATflex_Opt  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i=\left\lceil X_i\right\rceil$ */
extern  void   ceilFLOAT_Opt   (const float X[], float Z[], int n) ; 
extern  void   ceilFLOATflex_Opt  (const float X[], int incX, float Z[], int incZ, int n) ; 
extern   int   ceillog2_Opt( int a );
/** round $Z_i$ to nearest integer. $Z_i=\left\lfloor X_i+\frac{1}{2}\right\rfloor$ */
extern  void   nintFLOAT_Opt   (const float X[], float Z[], int n) ; 
extern  void   nintFLOATflex_Opt  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** truncate fractional part of $Z_i$ */
extern  void   truncFLOAT_Opt   (const float X[], float Z[], int n) ; 
extern  void   truncFLOATflex_Opt  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i=\left\lfloor a+b X_i\right\rfloor$. */
extern void quantFLOATtoUINT_Opt(float a, float b, const float X[], unsigned int Z[], int n);
/*@}*/
/** 

*/
extern void roundFLOAT2FLOAT16_Opt(const float A[], float B[], int n);
extern void roundFLOAT2INT_Opt(const float A[], int B[], int n);
extern void roundFLOAT2SHORT_Opt(const float A[], signed short B[], int n);
/**@name Transcendent vector operations
All these routines also exist in flex flavour, accepting \Ref{Vector increments}.
*/
/*@{*/

/** $Z_i=\sin\left(X_i\right)$. */
extern  void   sinFLOAT_Opt   (const float X[], float Z[], int n) ; 
extern  void   sinFLOATflex_Opt  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i=\cos\left(X_i\right)$. */
extern  void   cosFLOAT_Opt   (const float X[], float Z[], int n) ; 
extern  void   cosFLOATflex_Opt  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i=e^{X_i}$. */
extern  void   expFLOAT_Opt   (const float X[], float Z[], int n) ; 
extern  void   expFLOATflex_Opt  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i=X_i^a$. */
extern  void   spowFLOAT_Opt   (float a, const float X[], float Z[], int n) ; 
extern  void   spowFLOATflex_Opt  (float a, const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i=\log\left(X_i\right)$. */
extern  void   logFLOAT_Opt   (const float X[], float Z[], int n) ; 
extern  void   logFLOATflex_Opt  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i=\log_2\left(X_i\right)$. */
extern  void   log2FLOAT_Opt   (const float X[], float Z[], int n) ; 
extern  void   log2FLOATflex_Opt  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i=\log_{10}\left(X_i\right)$. */
extern  void   log10FLOAT_Opt   (const float X[], float Z[], int n) ; 
extern  void   log10FLOATflex_Opt  (const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i=a\log\left(b X_i\right)$. */
extern  void   alogbFLOAT_Opt   (float a, float b, const float X[], float Z[], int n) ; 
extern  void   alogbFLOATflex_Opt  (float a, float b, const float X[], int incX, float Z[], int incZ, int n) ; 
/** $Z_i=\sqrt{X_i}$. */
extern  void   sqrtFLOAT_Opt   (const float X[], float Z[], int n) ; 
extern  void   sqrtFLOATflex_Opt  (const float X[], int incX, float Z[], int incZ, int n) ; 
/*@}*/

/*SSE2*/

extern void setINT_SSE2(int a, int X[], int n);
extern void roundFLOAT2FLOAT16_SSE2(const float A[], float B[], int n);
extern void roundFLOAT2INT_SSE2(const float A[], int B[], int n);
extern void roundFLOAT2SHORT_SSE2(const float A[], signed short B[], int n);

/*ALTIVEC*/
extern void roundFLOAT2FLOAT16_ALTIVEC(const float X[], float Y[], int n);

/**@name Vector increments
   Functions that accept vector increments can access column vectors of
   matrices. Instead of accessing $X_i$ they will access elements
   $X_{i\cdot \mbox{incX}}$.
  */

/* random generator functions*/
extern void randFLOAT(float Z[], int n);
extern void randDOUBLE(double Z[], int n);
extern void randseed(int seed);
extern void randsave(double mem[]);
extern void randrestore(double mem[]);

/**@name Utility routines
   These routines allow enabling and disabling floating point exceptions and
   other hardware-dependent features.
  */
/*@{*/
/** Enable floating-point exceptions. If a signal handler mechanism exists, set
    it to #SIG_ABORT#. */
extern void enableFPEs(void);
/** Disable floating-point exceptions. */
extern void disableFPEs(void);
#ifdef _MSC_VER
/** Round to nearest integer. This function is part of all linux RTLs I know.
  However, it does not seem to exist in MSVCs RTL. So, we have an implementation
  here. */
extern double rint(double);
#endif
/*@}*/


#ifdef __cplusplus
           }
#endif


/* Makros to determine the smaller/bigger value of two integers or doubles or floats.
   Watch the expanding process: min(rst++, xyz) will cause problems. */
#ifndef __cplusplus
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#endif


#ifndef ABS
#define ABS(x) ((x) > 0 ? (x) : -(x))
#endif 
 
#if defined(macintosh) && defined(__MRC__)
#include <float.h>
#undef FLT_MIN
#undef FLT_MAX
#define FLT_MIN 1.175494351e-38F
#define FLT_MAX 3.402823466e+38F
#endif

#endif
