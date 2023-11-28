
/****************************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2005)
                               All Rights Reserved

   This software and/or program is protected by copyright law and
   international treaties. Any reproduction or distribution of this
   software and/or program, or any portion of it, may result in severe
   civil and criminal penalties, and will be prosecuted to the maximum
   extent possible under law.

   
******************************************************************************************/

/******************************************* iis-fft.c ***********************************/
/* Includes ******************************************************************************/
#include <math.h>
#include <stdlib.h>
#include "cpuinfo.h"   /* needed for GetCPUInfo()  */ 

#ifdef USE_IPP
#define IPPAPI1(type,name,arg) extern type __STDCALL a6_##name arg; 
#define IPPAPI2(type,name,arg) extern type __STDCALL px_##name arg; 
#define IPPAPI(type,name,arg) IPPAPI1(type,name,arg) IPPAPI2(type,name,arg)
#include <ipps.h>
#endif

#ifdef USE_ALTIVEC
#include <vecLib/vDSP.h>
#endif
#include "iis_fft.h"
#include "mathlib.h"

/* Data Types ***********************************************************************************/
typedef struct T_IIS_FFT {

  int iSign;  /* Fwd or Bwd FFT */ 
  int len;    /* length */
  int shift;  
 
#if defined USE_IPP

  IIS_FFT_ERROR(*iis_fft_apply_rfft)(HANDLE_IIS_FFT hIisFft, float* pInBuffer, float* pOutBuffer);
  IIS_FFT_ERROR(*iis_fft_destroy_fft)(HANDLE_IIS_FFT* phIisFft);
  IIS_FFT_ERROR(*iis_fft_apply_cfft)(HANDLE_IIS_FFT hIisFft, float* pInReBuffer, float* pInImBuffer, float* pOutReBuffer, float* pOutImBuffer);
  
  /* pointer to FFT specification structure to be created*/
  IppsFFTSpec_R_32f* pRFFTSpec;	
  IppsDFTSpec_R_32f* pRDFTSpec;
  IppsFFTSpec_C_32f* pCFFTSpec;
  IppsDFTSpec_C_32f* pCDFTSpec;

  /* Data Buffers */
  Ipp32f* pInReBuffer;
  Ipp32f* pInImBuffer;
  Ipp32f* pOutReBuffer;
  Ipp32f* pOutImBuffer;
  /*Temporal Buffer*/
  Ipp8u*  p_FFT_Buffer; 
  
#elif defined USE_ALTIVEC
  /* Fft setup struct */
  FFTSetup vecFftSetup;     
  /*FFT Data Buffers structures*/
  DSPSplitComplex*  vecFftData; 
  DSPSplitComplex*  vecFftDataResult;
  float vecScaleFwd;
  float vecScaleBwd;
  int   vecOrder;
#else 
  float* trigPtr;
#endif

} IIS_FFT;

/* Functions *************************************************************************************************/
#if defined USE_IPP
static IIS_FFT_ERROR iis_fft_apply_rfft_a6(HANDLE_IIS_FFT hIisFft, float* pInBuffer, float* pOutBuffer);
static IIS_FFT_ERROR iis_fft_apply_rfft_px(HANDLE_IIS_FFT hIisFft, float* pInBuffer, float* pOutBuffer);
static IIS_FFT_ERROR iis_fft_destroy_rfft_a6(HANDLE_IIS_FFT* phIisFft);
static IIS_FFT_ERROR iis_fft_destroy_rfft_px(HANDLE_IIS_FFT* phIisFft);

static IIS_FFT_ERROR iis_fft_apply_cfft_a6(HANDLE_IIS_FFT hIisFft, float* pInReBuffer, float* pInImBuffer, float* pOutReBuffer, float* pOutImBuffer);
static IIS_FFT_ERROR iis_fft_apply_cfft_px(HANDLE_IIS_FFT hIisFft, float* pInReBuffer, float* pInImBuffer, float* pOutReBuffer, float* pOutImBuffer);
static IIS_FFT_ERROR iis_fft_destroy_cfft_a6(HANDLE_IIS_FFT* phIisFft);
static IIS_FFT_ERROR iis_fft_destroy_cfft_px(HANDLE_IIS_FFT* phIisFft);
#endif

/*------------------------------- Fourier transform for real signals ----------------------------------------*/

IIS_FFT_ERROR IIS_RFFT_Create(HANDLE_IIS_FFT* phIisFft, int len, int iSign){

#ifdef USE_IPP
  int ippOrder;
  int Size; 
#endif
  
  HANDLE_IIS_FFT hIisFft;
  
  /* first handle is not valid */
  *phIisFft = NULL;

  if( iSign != IIS_FFT_FWD && iSign != IIS_FFT_BWD  ){
    return IIS_FFT_INTERNAL_ERROR;   /*neither forward nor backward FFT*/
  }
 
  if( NULL == (hIisFft = (HANDLE_IIS_FFT)calloc(1,sizeof(IIS_FFT)))){  
    return IIS_FFT_INTERNAL_ERROR;
  }
    
  for( hIisFft->shift = 1; len>> hIisFft->shift != 2 && len>>hIisFft->shift != 0; hIisFft->shift++ ); 
   
  hIisFft->len   = len; 
  hIisFft->iSign = iSign;

#if defined USE_IPP

  hIisFft->pRFFTSpec = NULL;	
  hIisFft->pRDFTSpec = NULL;
  hIisFft->pInReBuffer = NULL;
  hIisFft->pOutReBuffer = NULL;
  hIisFft->p_FFT_Buffer = NULL; 
  
  if(GetCPUInfo(HAS_CPU_SSE)){
    hIisFft->iis_fft_apply_rfft = iis_fft_apply_rfft_a6; 
    hIisFft->iis_fft_destroy_fft = iis_fft_destroy_rfft_a6; 

    if( NULL == (hIisFft->pInReBuffer = a6_ippsMalloc_32f(len))){        
      return IIS_FFT_INTERNAL_ERROR;
	}
	
    if( NULL == (hIisFft->pOutReBuffer = a6_ippsMalloc_32f(len))){
      return IIS_FFT_INTERNAL_ERROR;
	}  

    if (((0x2 << hIisFft->shift) & len) == len){
      ippOrder = hIisFft->shift +1 ;     /*(int)((float)log10(len)/log2 + 0.5f);    2^n == len  */
      if (IIS_FFT_NO_ERROR != a6_ippsFFTInitAlloc_R_32f(&(hIisFft->pRFFTSpec), ippOrder, IPP_FFT_DIV_INV_BY_N, ippAlgHintNone)){
        return IIS_FFT_INTERNAL_ERROR;
	  }
      if (IIS_FFT_NO_ERROR != a6_ippsFFTGetBufSize_R_32f( hIisFft->pRFFTSpec, &Size)){
        return IIS_FFT_INTERNAL_ERROR;
	  }  
      hIisFft->p_FFT_Buffer  = (Size > 0) ? a6_ippsMalloc_8u(Size) :  NULL;
	}

    else{
      if (IIS_FFT_NO_ERROR != (a6_ippsDFTInitAlloc_R_32f)(&(hIisFft->pRDFTSpec), len, IPP_FFT_DIV_INV_BY_N, ippAlgHintNone)){
        return IIS_FFT_INTERNAL_ERROR;
	  } 
      if (IIS_FFT_NO_ERROR != (a6_ippsDFTGetBufSize_R_32f)( hIisFft->pRDFTSpec, &Size)){
        return IIS_FFT_INTERNAL_ERROR;
	  } 
      hIisFft->p_FFT_Buffer  = (Size > 0) ? a6_ippsMalloc_8u(Size) :  NULL;
	}

  }

  else{

    hIisFft->iis_fft_apply_rfft = iis_fft_apply_rfft_px;
	hIisFft->iis_fft_destroy_fft = iis_fft_destroy_rfft_px; 
 
    if( NULL == (hIisFft->pInReBuffer = px_ippsMalloc_32f(len))){        
      return IIS_FFT_INTERNAL_ERROR;
	}
	
    if( NULL == (hIisFft->pOutReBuffer = px_ippsMalloc_32f(len))){
      return IIS_FFT_INTERNAL_ERROR;
	}  

    if ( ((0x2 << hIisFft->shift) & len) == len ){
      ippOrder = hIisFft->shift +1 ;     /*(int)((float)log10(len)/log2 + 0.5f);    2^n == len  */
      if (IIS_FFT_NO_ERROR != px_ippsFFTInitAlloc_R_32f( &(hIisFft->pRFFTSpec), ippOrder, IPP_FFT_DIV_INV_BY_N, ippAlgHintNone)){
        return IIS_FFT_INTERNAL_ERROR;
	  }
      if (IIS_FFT_NO_ERROR != px_ippsFFTGetBufSize_R_32f( hIisFft->pRFFTSpec, &Size)){
        return IIS_FFT_INTERNAL_ERROR;
	  }  
      hIisFft->p_FFT_Buffer  = (Size > 0) ? px_ippsMalloc_8u(Size) :  NULL;
	}

    else{
      if (IIS_FFT_NO_ERROR != (px_ippsDFTInitAlloc_R_32f(&(hIisFft->pRDFTSpec), len, IPP_FFT_DIV_INV_BY_N, ippAlgHintNone))){
        return IIS_FFT_INTERNAL_ERROR;
	  } 
      if (IIS_FFT_NO_ERROR != (px_ippsDFTGetBufSize_R_32f( hIisFft->pRDFTSpec, &Size))){
        return IIS_FFT_INTERNAL_ERROR;
	  } 
      hIisFft->p_FFT_Buffer  = (Size > 0) ? px_ippsMalloc_8u(Size) :  NULL;
	}  

  }


#elif defined USE_ALTIVEC

  hIisFft->vecFftSetup = NULL;     
  hIisFft->vecFftData = NULL; 
  
  if ( ((0x2 << hIisFft->shift) & len) == len){
    hIisFft->vecOrder = hIisFft->shift + 1;     /*(int)((float)log10(len)/log2 + 0.5f);	*/
    
    hIisFft->vecScaleFwd = (float)1.0/2;
    hIisFft->vecScaleBwd = (float)1.0/len;

    hIisFft->vecFftSetup  = create_fftsetup(hIisFft->vecOrder, FFT_RADIX2);    

    if( NULL == (hIisFft->vecFftData =(DSPSplitComplex*)calloc(1,sizeof(DSPSplitComplex)))){
      return IIS_FFT_INTERNAL_ERROR;
    }  

    if( NULL == (hIisFft->vecFftData->realp =(float*)malloc(len/2 *sizeof(float)))){
      return IIS_FFT_INTERNAL_ERROR;
    }
  
    if( NULL == (hIisFft->vecFftData->imagp =(float*)malloc(len/2 *sizeof(float)))){
      return IIS_FFT_INTERNAL_ERROR;
    }
  }

  else{
    IIS_RFFT_Destroy(&hIisFft); 
    return IIS_FFT_INTERNAL_ERROR;
  }

#else
 
 hIisFft->trigPtr = NULL;   
 
 if( ((0x2 << hIisFft->shift) & len) == len){                                 /*RFFTN*/
    if( NULL == (hIisFft->trigPtr = CreateSineTable(len))){
      return IIS_FFT_INTERNAL_ERROR;
    }		
 }
    
 else{
   IIS_RFFT_Destroy(&hIisFft); 
   return IIS_FFT_INTERNAL_ERROR;
  }

#endif
 
 /* now handle is valid */
 *phIisFft = hIisFft; 

  return IIS_FFT_NO_ERROR;
}




IIS_FFT_ERROR IIS_FFT_Apply_RFFT(HANDLE_IIS_FFT hIisFft, float* pInBuffer, float* pOutBuffer){

  if(hIisFft != NULL){

#if defined USE_IPP
    if (IIS_FFT_NO_ERROR != hIisFft->iis_fft_apply_rfft(hIisFft, pInBuffer, pOutBuffer)){  
      return IIS_FFT_INTERNAL_ERROR;
	}

#elif defined USE_ALTIVEC
    ctoz((DSPComplex *)pInBuffer, 2, hIisFft->vecFftData, 1, hIisFft->len/2 );
    
    if(hIisFft->iSign == IIS_FFT_FWD){ 	    
      fft_zrip( hIisFft->vecFftSetup, hIisFft->vecFftData, 1, hIisFft->vecOrder, FFT_FORWARD );  
      ztoc( hIisFft->vecFftData, 1, (DSPComplex * )pOutBuffer, 2, hIisFft->len/2 );
      vsmul( pOutBuffer, 1, &(hIisFft->vecScaleFwd), pOutBuffer, 1, hIisFft->len ); 
    }
    
    else if(hIisFft->iSign == IIS_FFT_BWD){
      fft_zrip( hIisFft->vecFftSetup, hIisFft->vecFftData, 1 , hIisFft->vecOrder, FFT_INVERSE ); 
      ztoc( hIisFft->vecFftData, 1, (DSPComplex * )pOutBuffer, 2, hIisFft->len/2 );
      vsmul( pOutBuffer, 1, &(hIisFft->vecScaleBwd), pOutBuffer, 1, hIisFft->len ); 
    }

#else										      /*RFFTN*/
 
    if(pInBuffer != pOutBuffer){
	  copyFLOAT (pInBuffer, pOutBuffer, hIisFft->len);
	}
    
    if (IIS_FFT_NO_ERROR != !(RFFTN ( pOutBuffer, hIisFft->trigPtr, hIisFft->len, hIisFft->iSign))){
      return IIS_FFT_NO_ERROR;
    }
       	
#endif

    return IIS_FFT_NO_ERROR;
  }	
  else{
    return IIS_FFT_INTERNAL_ERROR;
  }
}	
	

IIS_FFT_ERROR IIS_RFFT_Destroy(HANDLE_IIS_FFT* phIisFft){

  HANDLE_IIS_FFT hIisFft;
  if(phIisFft != NULL){
    if(*phIisFft != NULL){    

#if defined USE_IPP
	    
      if (IIS_FFT_NO_ERROR != (*phIisFft)->iis_fft_destroy_fft(phIisFft)){
        return IIS_FFT_INTERNAL_ERROR;
      }
      
#elif defined USE_ALTIVEC

       
      if ((*phIisFft)->vecFftSetup != NULL){
        destroy_fftsetup((*phIisFft)->vecFftSetup);     
      }

      if ((*phIisFft)->vecFftData != NULL){      
        free((*phIisFft)->vecFftData);
      
        if ((*phIisFft)->vecFftData->realp != NULL){
          free((*phIisFft)->vecFftData->realp);
        }
        if ((*phIisFft)->vecFftData->imagp != NULL){
          free((*phIisFft)->vecFftData->imagp);
        }
      }
      
#else
      if((*phIisFft)->trigPtr != NULL){
        DestroySineTable((*phIisFft)->trigPtr);
      }
#endif
      
      hIisFft = *phIisFft;
      if(hIisFft != NULL){
        free(hIisFft);
      }
      *phIisFft = NULL;
    }
  }

  return IIS_FFT_NO_ERROR;
}


#ifdef USE_IPP


static IIS_FFT_ERROR iis_fft_apply_rfft_a6(HANDLE_IIS_FFT hIisFft, float* pInBuffer, float* pOutBuffer){

  copyFLOAT (pInBuffer, hIisFft->pInReBuffer, hIisFft->len);
    
  if(hIisFft->iSign == IIS_FFT_FWD){ 	                           /*-1=forward transform (FFT)*/
    if( ((0x2 << hIisFft->shift) & hIisFft->len) == hIisFft->len ){	/*RFFT_IPP*/
	  if (IIS_FFT_NO_ERROR != a6_ippsFFTFwd_RToPerm_32f( hIisFft->pInReBuffer, hIisFft->pOutReBuffer, hIisFft->pRFFTSpec, hIisFft->p_FFT_Buffer )){
	    return IIS_FFT_INTERNAL_ERROR;
	  }
	}
    else{											            /*RDFT_IPP*/
	  if (IIS_FFT_NO_ERROR != a6_ippsDFTFwd_RToPerm_32f( hIisFft->pInReBuffer, hIisFft->pOutReBuffer, hIisFft->pRDFTSpec, hIisFft->p_FFT_Buffer )){
	    return IIS_FFT_INTERNAL_ERROR;
	  }
    }
  }    
    
  else if(hIisFft->iSign == IIS_FFT_BWD){			                   /* +1=backward (IFFT)*/
      
	if(((0x2 << hIisFft->shift) & hIisFft->len) == hIisFft->len ){  	/*RFFT_IPP*/
      if (IIS_FFT_NO_ERROR != a6_ippsFFTInv_PermToR_32f(hIisFft->pInReBuffer, hIisFft->pOutReBuffer, hIisFft->pRFFTSpec, hIisFft->p_FFT_Buffer )){
        return IIS_FFT_INTERNAL_ERROR;
	  }
    }
    else{
      if (IIS_FFT_NO_ERROR != a6_ippsDFTInv_PermToR_32f( hIisFft->pInReBuffer, hIisFft->pOutReBuffer, hIisFft->pRDFTSpec, hIisFft->p_FFT_Buffer )){
	    return IIS_FFT_INTERNAL_ERROR;
	  }
    }
  }
   
  copyFLOAT(hIisFft->pOutReBuffer, pOutBuffer, hIisFft->len);
  return IIS_FFT_NO_ERROR;

}


static IIS_FFT_ERROR iis_fft_apply_rfft_px(HANDLE_IIS_FFT hIisFft, float* pInBuffer, float* pOutBuffer){

  copyFLOAT (pInBuffer, hIisFft->pInReBuffer, hIisFft->len);
    
  if(hIisFft->iSign == IIS_FFT_FWD){ 	                           /*-1=forward transform (FFT)*/
    if( ((0x2 << hIisFft->shift) & hIisFft->len) == hIisFft->len ){			/*RFFT_IPP*/
      if (IIS_FFT_NO_ERROR != px_ippsFFTFwd_RToPerm_32f( hIisFft->pInReBuffer, hIisFft->pOutReBuffer, hIisFft->pRFFTSpec, hIisFft->p_FFT_Buffer )){
        return IIS_FFT_INTERNAL_ERROR;
	  }
	}
    else{											               /*RDFT_IPP*/
	  if (IIS_FFT_NO_ERROR != px_ippsDFTFwd_RToPerm_32f( hIisFft->pInReBuffer, hIisFft->pOutReBuffer, hIisFft->pRDFTSpec, hIisFft->p_FFT_Buffer )){
	    return IIS_FFT_INTERNAL_ERROR;
	  }
    }
  }    
    
  else if(hIisFft->iSign == IIS_FFT_BWD){			               /* +1=backward (IFFT) */
      
    if( ((0x2 << hIisFft->shift) & hIisFft->len) == hIisFft->len ){			/*RFFT_IPP*/
	  if (IIS_FFT_NO_ERROR != px_ippsFFTInv_PermToR_32f(hIisFft->pInReBuffer, hIisFft->pOutReBuffer, hIisFft->pRFFTSpec, hIisFft->p_FFT_Buffer )){
	    return IIS_FFT_INTERNAL_ERROR;
		}
      }
    else{
      if (IIS_FFT_NO_ERROR != px_ippsDFTInv_PermToR_32f( hIisFft->pInReBuffer, hIisFft->pOutReBuffer, hIisFft->pRDFTSpec, hIisFft->p_FFT_Buffer )){
	    return IIS_FFT_INTERNAL_ERROR;
	  }
    }
}
   
  copyFLOAT(hIisFft->pOutReBuffer, pOutBuffer, hIisFft->len);
  return IIS_FFT_NO_ERROR;
}




static IIS_FFT_ERROR iis_fft_destroy_rfft_a6(HANDLE_IIS_FFT* phIisFft){

  if((*phIisFft)->pRFFTSpec != NULL){                                      
    if (IIS_FFT_NO_ERROR != a6_ippsFFTFree_R_32f((*phIisFft)->pRFFTSpec )){
	  return IIS_FFT_INTERNAL_ERROR;
    }
  }
	    
  if((*phIisFft)->pRDFTSpec != NULL){                                      
    if (IIS_FFT_NO_ERROR != a6_ippsDFTFree_R_32f((*phIisFft)->pRDFTSpec)){
	  return IIS_FFT_INTERNAL_ERROR;
	}
  }
			
  if((*phIisFft)->pInReBuffer != NULL){                                     
    a6_ippsFree((*phIisFft)->pInReBuffer);
  }
	
  if((*phIisFft)->pOutReBuffer != NULL){                                     
    a6_ippsFree((*phIisFft)->pOutReBuffer);
  }
	
  if((*phIisFft)->p_FFT_Buffer != NULL){                                      	
    a6_ippsFree((*phIisFft)->p_FFT_Buffer);
  }

  return IIS_FFT_NO_ERROR;

}



static IIS_FFT_ERROR iis_fft_destroy_rfft_px(HANDLE_IIS_FFT* phIisFft){

  if((*phIisFft)->pRFFTSpec != NULL){                                      
    if (IIS_FFT_NO_ERROR != px_ippsFFTFree_R_32f((*phIisFft)->pRFFTSpec )){
	  return IIS_FFT_INTERNAL_ERROR;
    }
  }
	    
  if((*phIisFft)->pRDFTSpec != NULL){                                      
    if (IIS_FFT_NO_ERROR != px_ippsDFTFree_R_32f((*phIisFft)->pRDFTSpec)){
	  return IIS_FFT_INTERNAL_ERROR;
	}
  }
			
  if((*phIisFft)->pInReBuffer != NULL){                                     
    px_ippsFree((*phIisFft)->pInReBuffer);
  }
	
  if((*phIisFft)->pOutReBuffer != NULL){                                     
    px_ippsFree((*phIisFft)->pOutReBuffer);
  }
	
  if((*phIisFft)->p_FFT_Buffer != NULL){                                      	
    px_ippsFree((*phIisFft)->p_FFT_Buffer);
  }

  return IIS_FFT_NO_ERROR; 

}
#endif

/*-------------------------------------------- Fourier transform for complex signals------------------------------------------*/


IIS_FFT_ERROR IIS_CFFT_Create(HANDLE_IIS_FFT* phIisFft, int len, int iSign){

#ifdef USE_IPP
  int ippOrder;
  int Size = 0; 
#endif	
	
  HANDLE_IIS_FFT hIisFft;
  
  /* first handle is not valid */
  *phIisFft = NULL;
  
  if( iSign != IIS_FFT_FWD && iSign != IIS_FFT_BWD ){
    return IIS_FFT_INTERNAL_ERROR;   /*neither forward nor backward FFT*/
  }

  if( NULL == (hIisFft = (HANDLE_IIS_FFT)calloc(1, sizeof(IIS_FFT)))){
    return IIS_FFT_INTERNAL_ERROR;
  }	

  for( hIisFft->shift = 1; len>>hIisFft->shift != 2 && len>>hIisFft->shift != 3 && len>>hIisFft->shift != 5 && len>>hIisFft->shift != 0;  hIisFft->shift++ );
  hIisFft->len   = len;
  hIisFft->iSign = iSign;


#if defined USE_IPP

  hIisFft->pCFFTSpec = NULL;
  hIisFft->pCDFTSpec = NULL;
  hIisFft->pInReBuffer = NULL;
  hIisFft->pInImBuffer = NULL;
  hIisFft->pOutReBuffer = NULL;
  hIisFft->pOutImBuffer = NULL;
  hIisFft->p_FFT_Buffer = NULL; 

  if(GetCPUInfo(HAS_CPU_SSE)){


    hIisFft->iis_fft_apply_cfft  = iis_fft_apply_cfft_a6; 
    hIisFft->iis_fft_destroy_fft = iis_fft_destroy_cfft_a6; 


    if( NULL == (hIisFft->pInReBuffer  = a6_ippsMalloc_32f(len))){
      return IIS_FFT_INTERNAL_ERROR;
	}
    if( NULL == (hIisFft->pInImBuffer  = a6_ippsMalloc_32f(len))){
      return IIS_FFT_INTERNAL_ERROR;
	}
    if( NULL == (hIisFft->pOutReBuffer = a6_ippsMalloc_32f(len))){
      return IIS_FFT_INTERNAL_ERROR;
	}	
    if( NULL == (hIisFft->pOutImBuffer = a6_ippsMalloc_32f(len))){
      return IIS_FFT_INTERNAL_ERROR;
	}
    
	if ( ((0x2 << hIisFft->shift) & len) == len){ 
      ippOrder = hIisFft->shift + 1;    /* (int)((float)log10(len)/log2+0.5f);		2^ippOrder == len */
      if (IIS_FFT_NO_ERROR != (a6_ippsFFTInitAlloc_C_32f(&(hIisFft->pCFFTSpec), ippOrder, IPP_FFT_NODIV_BY_ANY, ippAlgHintNone))){
        return IIS_FFT_INTERNAL_ERROR;
	  }  
      if (IIS_FFT_NO_ERROR != (a6_ippsFFTGetBufSize_C_32f( hIisFft->pCFFTSpec, &Size))){
        return IIS_FFT_INTERNAL_ERROR;
	  }  
      hIisFft->p_FFT_Buffer  = (Size > 0) ? a6_ippsMalloc_8u(Size) :  NULL;
	}
	
    
	else{
      if (IIS_FFT_NO_ERROR != (a6_ippsDFTInitAlloc_C_32f(&(hIisFft->pCDFTSpec), len, IPP_FFT_NODIV_BY_ANY, ippAlgHintNone))){
	    return IIS_FFT_INTERNAL_ERROR;
    }
      if (IIS_FFT_NO_ERROR!= (a6_ippsDFTGetBufSize_C_32f( hIisFft->pCDFTSpec, &Size))){
        return IIS_FFT_INTERNAL_ERROR;
	  }
      hIisFft->p_FFT_Buffer  = (Size > 0) ? a6_ippsMalloc_8u(Size) :  NULL;
	}
  }
  
  else{

    hIisFft->iis_fft_apply_cfft   = iis_fft_apply_cfft_px; 
    hIisFft->iis_fft_destroy_fft  = iis_fft_destroy_cfft_px; 


    if( NULL == (hIisFft->pInReBuffer = px_ippsMalloc_32f(len))){
      return IIS_FFT_INTERNAL_ERROR;
	}
    if( NULL == (hIisFft->pInImBuffer = px_ippsMalloc_32f(len))){
      return IIS_FFT_INTERNAL_ERROR;
	}
    if( NULL == (hIisFft->pOutReBuffer = px_ippsMalloc_32f(len))){
      return IIS_FFT_INTERNAL_ERROR;
	}	
    if( NULL == (hIisFft->pOutImBuffer = px_ippsMalloc_32f(len))){
      return IIS_FFT_INTERNAL_ERROR;
	}

    
	if( ((0x2 << hIisFft->shift) & len) == len){ 
      ippOrder = hIisFft->shift + 1;    /* (int)((float)log10(len)/log2+0.5f);		2^ippOrder == len */
      if (IIS_FFT_NO_ERROR != (px_ippsFFTInitAlloc_C_32f(&(hIisFft->pCFFTSpec), ippOrder, IPP_FFT_NODIV_BY_ANY, ippAlgHintNone))){
        return IIS_FFT_INTERNAL_ERROR;
	  }  
      if (IIS_FFT_NO_ERROR != (px_ippsFFTGetBufSize_C_32f( hIisFft->pCFFTSpec, &Size))){
        return IIS_FFT_INTERNAL_ERROR;
	  }  
      hIisFft->p_FFT_Buffer  = (Size > 0) ? px_ippsMalloc_8u(Size) :  NULL;
	}
	
    
	else{
      if(IIS_FFT_NO_ERROR != (px_ippsDFTInitAlloc_C_32f(&(hIisFft->pCDFTSpec), len, IPP_FFT_NODIV_BY_ANY, ippAlgHintNone))){
	    return IIS_FFT_INTERNAL_ERROR;
	  }
      if (IIS_FFT_NO_ERROR!= (px_ippsDFTGetBufSize_C_32f( hIisFft->pCDFTSpec, &Size))){
        return IIS_FFT_INTERNAL_ERROR;
	  }
      hIisFft->p_FFT_Buffer  = (Size > 0) ? px_ippsMalloc_8u(Size) :  NULL;
	}

  }


#elif defined USE_ALTIVEC
  
  hIisFft->vecFftSetup = NULL;     
  hIisFft->vecFftData = NULL; 
  hIisFft->vecFftDataResult = NULL;    

  if( ((0x2 << hIisFft->shift) & len) == len){       /*FFT_RADIX2*/
  
    hIisFft->vecOrder = hIisFft->shift + 1;     /*(int)((float)log10(len)/log2 + 0.5f);	*/
    /*hIisFft->vecScaleBwd = (float)1.0/len;*/

    hIisFft->vecFftSetup = create_fftsetup(hIisFft->vecOrder, FFT_RADIX2);    
  }

  else if( ((0x3 << hIisFft->shift) & len) == len ){     /*FFT_RADIX3*/
  
    hIisFft->vecOrder = hIisFft->shift;
    hIisFft->vecFftSetup = create_fftsetup(hIisFft->vecOrder, FFT_RADIX3 );    

    if( NULL == (hIisFft->vecFftDataResult = (DSPSplitComplex*)calloc(1,sizeof(DSPSplitComplex)))){
      return IIS_FFT_INTERNAL_ERROR;
    }
    if( NULL == ( hIisFft->vecFftDataResult->realp = (float*)malloc( len *sizeof(float)))){
      return IIS_FFT_INTERNAL_ERROR;
    }   
    if( NULL == ( hIisFft->vecFftDataResult->imagp = (float*)malloc( len *sizeof(float)))){
      return IIS_FFT_INTERNAL_ERROR;
    }
  }

  
  else if( ((0x5 << hIisFft->shift) & len) == len){     /*FFT_RADIX5*/
     
    hIisFft->vecOrder = hIisFft->shift;
    hIisFft->vecFftSetup = create_fftsetup(hIisFft->vecOrder, FFT_RADIX5);    
     
    if(NULL == (hIisFft->vecFftDataResult = (DSPSplitComplex*)calloc(1,sizeof(DSPSplitComplex)))){
      return IIS_FFT_INTERNAL_ERROR;
      }
    if(NULL == (hIisFft->vecFftDataResult->realp = (float*)malloc( len *sizeof(float)))){
      return IIS_FFT_INTERNAL_ERROR;
     } 
    if(NULL == (hIisFft->vecFftDataResult->imagp = (float*)malloc( len *sizeof(float)))){
      return IIS_FFT_INTERNAL_ERROR;
    }
  }
 
  else{ /*CFFTN*/
/* now handle is valid */ 
    *phIisFft = hIisFft;
    return IIS_FFT_NO_ERROR;
  }

  if( NULL == (hIisFft->vecFftData =(DSPSplitComplex*)calloc(1,sizeof(DSPSplitComplex)))){
    return IIS_FFT_INTERNAL_ERROR;
  }  

  if( NULL == (hIisFft->vecFftData->realp =(float*)malloc(len *sizeof(float)))){
    return IIS_FFT_INTERNAL_ERROR;
  }
  
  if( NULL == (hIisFft->vecFftData->imagp =(float*)malloc(len *sizeof(float)))){
    return IIS_FFT_INTERNAL_ERROR;
  }

#endif

/* now handle is valid */
  *phIisFft = hIisFft; 
  return IIS_FFT_NO_ERROR;

}


IIS_FFT_ERROR IIS_FFT_Apply_CFFT(HANDLE_IIS_FFT hIisFft, float* pInReBuffer, float* pInImBuffer, float* pOutReBuffer, float* pOutImBuffer ){


  if( hIisFft != NULL){

#if defined USE_IPP
		    
    if(IIS_FFT_NO_ERROR != hIisFft->iis_fft_apply_cfft(hIisFft, pInReBuffer, pInImBuffer, pOutReBuffer, pOutImBuffer )){  
      return IIS_FFT_INTERNAL_ERROR;
	}	  

#elif defined USE_ALTIVEC

  if( ((0x2 << hIisFft->shift) & hIisFft->len) == hIisFft->len ){     /*FFT_RADIX2*/
  
    copyFLOAT( pInReBuffer, hIisFft->vecFftData->realp, hIisFft->len );
    copyFLOAT( pInImBuffer, hIisFft->vecFftData->imagp, hIisFft->len ); 
   
    if(hIisFft->iSign == IIS_FFT_FWD){ 	 
      fft_zip( hIisFft->vecFftSetup, hIisFft->vecFftData, 1, hIisFft->vecOrder, FFT_FORWARD );
    }

    else if(hIisFft->iSign == IIS_FFT_BWD){
      fft_zip(  hIisFft->vecFftSetup, hIisFft->vecFftData, 1, hIisFft->vecOrder, FFT_INVERSE );  
      /* vsmul( hIisFft->vecFftData->realp, 1, &hIisFft->vecScaleBwd, pOutReBuffer, 1, hIisFft->len ); */
      /* vsmul( hIisFft->vecFftData->imagp, 1, &hIisFft->vecScaleBwd, pOutImBuffer, 1, hIisFft->len ); */
    }
 
    copyFLOAT( hIisFft->vecFftData->realp, pOutReBuffer, hIisFft->len );
    copyFLOAT( hIisFft->vecFftData->imagp, pOutImBuffer, hIisFft->len );
  }


  else if( ((0x3 << hIisFft->shift) & hIisFft->len) == hIisFft->len ){    /*FFT_RADIX3*/

    copyFLOAT( pInReBuffer, hIisFft->vecFftData->realp, hIisFft->len );
    copyFLOAT( pInImBuffer, hIisFft->vecFftData->imagp, hIisFft->len ); 

    if(hIisFft->iSign == IIS_FFT_FWD){
      fft3_zop( hIisFft->vecFftSetup, hIisFft->vecFftData, 1, hIisFft->vecFftDataResult, 1, hIisFft->vecOrder, FFT_FORWARD ); 
    }

    if(hIisFft->iSign == IIS_FFT_BWD){
      fft3_zop( hIisFft->vecFftSetup, hIisFft->vecFftData, 1, hIisFft->vecFftDataResult, 1, hIisFft->vecOrder, FFT_INVERSE );  
    }
    
    copyFLOAT( hIisFft->vecFftDataResult->realp, pOutReBuffer, hIisFft->len );
    copyFLOAT( hIisFft->vecFftDataResult->imagp, pOutImBuffer, hIisFft->len );
  }


  else if( ((0x5 << hIisFft->shift) & hIisFft->len) == hIisFft->len ){  /*FFT_RADIX5*/
   
    copyFLOAT( pInReBuffer, hIisFft->vecFftData->realp, hIisFft->len );
    copyFLOAT( pInImBuffer, hIisFft->vecFftData->imagp, hIisFft->len ); 

    if(hIisFft->iSign == IIS_FFT_FWD){
      fft5_zop(hIisFft->vecFftSetup, hIisFft->vecFftData, 1, hIisFft->vecFftDataResult, 1, hIisFft->vecOrder, FFT_FORWARD); 
    }

    if(hIisFft->iSign == IIS_FFT_BWD){
      fft5_zop(hIisFft->vecFftSetup, hIisFft->vecFftData, 1, hIisFft->vecFftDataResult, 1, hIisFft->vecOrder, FFT_INVERSE); 
    }
    copyFLOAT( hIisFft->vecFftDataResult->realp, pOutReBuffer, hIisFft->len );
    copyFLOAT( hIisFft->vecFftDataResult->imagp, pOutImBuffer, hIisFft->len );
  }
  
  else{
  						               /*CFFTN*/
    if (IIS_FFT_NO_ERROR != !(CFFTN_NI(pInReBuffer, pInImBuffer, pOutReBuffer, pOutImBuffer, hIisFft->len, hIisFft->iSign))){
      return IIS_FFT_INTERNAL_ERROR;
    }
  }

#else 
								       /*CFFTN*/
    if (IIS_FFT_NO_ERROR != !(CFFTN_NI(pInReBuffer, pInImBuffer, pOutReBuffer, pOutImBuffer, hIisFft->len, hIisFft->iSign))){
      return IIS_FFT_INTERNAL_ERROR;
    }


#endif	

    return IIS_FFT_NO_ERROR;
  }

  else{
    return IIS_FFT_INTERNAL_ERROR;
  }

}	


IIS_FFT_ERROR IIS_CFFT_Destroy(HANDLE_IIS_FFT* phIisFft){

  HANDLE_IIS_FFT hIisFft;
  if(phIisFft != NULL){
    if(*phIisFft != NULL){   


#ifdef USE_IPP
	
      if (IIS_FFT_NO_ERROR != (*phIisFft)->iis_fft_destroy_fft(phIisFft)){
        return IIS_FFT_INTERNAL_ERROR;
      }     


#elif defined USE_ALTIVEC

     
      if ((*phIisFft)->vecFftSetup != NULL){
        destroy_fftsetup((*phIisFft)->vecFftSetup);     
      }

      if ((*phIisFft)->vecFftData != NULL){      
        free((*phIisFft)->vecFftData);
        
        if((*phIisFft)->vecFftData->realp != NULL){
          free((*phIisFft)->vecFftData->realp);
        }        
        if((*phIisFft)->vecFftData->imagp != NULL){
          free((*phIisFft)->vecFftData->imagp);
        }
      }  

      if( (*phIisFft)->vecFftDataResult != NULL ){
        free((*phIisFft)->vecFftDataResult);
        
        if( (*phIisFft)->vecFftDataResult->realp != NULL ){
          free((*phIisFft)->vecFftDataResult->realp);
        }     
        if( (*phIisFft)->vecFftDataResult->imagp != NULL ){  
          free((*phIisFft)->vecFftDataResult->imagp);
        } 
      }

#endif
      hIisFft = *phIisFft;
      if( hIisFft != NULL ){
        free(hIisFft);
      }
      *phIisFft = NULL;
    }
  }
  return IIS_FFT_NO_ERROR;
}


#ifdef USE_IPP

static IIS_FFT_ERROR iis_fft_apply_cfft_a6(HANDLE_IIS_FFT hIisFft, float* pInReBuffer, float* pInImBuffer, float* pOutReBuffer, float* pOutImBuffer){

    copyFLOAT (pInReBuffer, hIisFft->pInReBuffer, hIisFft->len);
    copyFLOAT (pInImBuffer, hIisFft->pInImBuffer, hIisFft->len);
	
    if(hIisFft->iSign == IIS_FFT_FWD){ 	                                        /* -1=forward transform (FFT)*/
	
      if ( ((0x2 << hIisFft->shift) & hIisFft->len) == hIisFft->len ){			/*CFFT_IPP*/
        if (IIS_FFT_NO_ERROR != a6_ippsFFTFwd_CToC_32f(hIisFft->pInReBuffer, hIisFft->pInImBuffer, hIisFft->pOutReBuffer, hIisFft->pOutImBuffer,  hIisFft->pCFFTSpec, hIisFft->p_FFT_Buffer)){
          return IIS_FFT_INTERNAL_ERROR;  	
        }	
      }
      else{								/*CDFT_IPP*/
        if (IIS_FFT_NO_ERROR != a6_ippsDFTFwd_CToC_32f(hIisFft->pInReBuffer, hIisFft->pInImBuffer, hIisFft->pOutReBuffer, hIisFft->pOutImBuffer,  hIisFft->pCDFTSpec, hIisFft->p_FFT_Buffer)){
	      return IIS_FFT_INTERNAL_ERROR;
		}
      }
    }
    
    else if(hIisFft->iSign == IIS_FFT_BWD){			                /* +1=backward (IFFT)  */
     
      if ( ((0x2 << hIisFft->shift) & hIisFft->len) == hIisFft->len ){						/*CFFT_IPP*/
        if (IIS_FFT_NO_ERROR != a6_ippsFFTInv_CToC_32f(hIisFft->pInReBuffer, hIisFft->pInImBuffer, hIisFft->pOutReBuffer, hIisFft->pOutImBuffer,  hIisFft->pCFFTSpec, hIisFft->p_FFT_Buffer)){
          return IIS_FFT_INTERNAL_ERROR;  	
        }	
      }
      else{								/*CDFT_IPP*/
        if (IIS_FFT_NO_ERROR != a6_ippsDFTInv_CToC_32f(hIisFft->pInReBuffer, hIisFft->pInImBuffer, hIisFft->pOutReBuffer, hIisFft->pOutImBuffer,  hIisFft->pCDFTSpec, hIisFft->p_FFT_Buffer)){
	      return IIS_FFT_INTERNAL_ERROR;
        }
      }
    }

    copyFLOAT (hIisFft->pOutReBuffer, pOutReBuffer, hIisFft->len);
    copyFLOAT (hIisFft->pOutImBuffer, pOutImBuffer, hIisFft->len);

    return IIS_FFT_NO_ERROR;
}



static IIS_FFT_ERROR iis_fft_apply_cfft_px(HANDLE_IIS_FFT hIisFft, float* pInReBuffer, float* pInImBuffer, float* pOutReBuffer, float* pOutImBuffer){

  copyFLOAT (pInReBuffer, hIisFft->pInReBuffer, hIisFft->len);
  copyFLOAT (pInImBuffer, hIisFft->pInImBuffer, hIisFft->len);
	
  if(hIisFft->iSign == IIS_FFT_FWD){ 	                                        /* -1=forward transform (FFT)*/
	
    if ( ((0x2 << hIisFft->shift) & hIisFft->len) == hIisFft->len ){			/*CFFT_IPP*/
      if (IIS_FFT_NO_ERROR != px_ippsFFTFwd_CToC_32f( hIisFft->pInReBuffer, hIisFft->pInImBuffer, hIisFft->pOutReBuffer, hIisFft->pOutImBuffer,  hIisFft->pCFFTSpec, hIisFft->p_FFT_Buffer)){
        return IIS_FFT_INTERNAL_ERROR;  	
      }	
    }
    else{								/*CDFT_IPP*/
      if (IIS_FFT_NO_ERROR != px_ippsDFTFwd_CToC_32f( hIisFft->pInReBuffer, hIisFft->pInImBuffer, hIisFft->pOutReBuffer, hIisFft->pOutImBuffer,  hIisFft->pCDFTSpec, hIisFft->p_FFT_Buffer)){
        return IIS_FFT_INTERNAL_ERROR;
	  }
    }
  }
    
  else if(hIisFft->iSign == IIS_FFT_BWD){			                /* +1=backward (IFFT)  */
     
    if ( ((0x2 << hIisFft->shift) & hIisFft->len) == hIisFft->len ){						/*CFFT_IPP*/
      if (IIS_FFT_NO_ERROR != px_ippsFFTInv_CToC_32f( hIisFft->pInReBuffer, hIisFft->pInImBuffer, hIisFft->pOutReBuffer, hIisFft->pOutImBuffer,  hIisFft->pCFFTSpec, hIisFft->p_FFT_Buffer)){
        return IIS_FFT_INTERNAL_ERROR;  	
      }	
    }
    else{								/*CDFT_IPP*/
      if (IIS_FFT_NO_ERROR != px_ippsDFTInv_CToC_32f( hIisFft->pInReBuffer, hIisFft->pInImBuffer, hIisFft->pOutReBuffer, hIisFft->pOutImBuffer,  hIisFft->pCDFTSpec, hIisFft->p_FFT_Buffer)){
        return IIS_FFT_INTERNAL_ERROR;
      }
    }
  }

  copyFLOAT (hIisFft->pOutReBuffer, pOutReBuffer, hIisFft->len);
  copyFLOAT (hIisFft->pOutImBuffer, pOutImBuffer, hIisFft->len);

  return IIS_FFT_NO_ERROR;
}


static IIS_FFT_ERROR iis_fft_destroy_cfft_a6(HANDLE_IIS_FFT* phIisFft){

  if((*phIisFft)->pCFFTSpec != NULL){            
    if(IIS_FFT_NO_ERROR != a6_ippsFFTFree_C_32f((*phIisFft)->pCFFTSpec )){
	  return IIS_FFT_INTERNAL_ERROR;
	}  
  }
     
  if((*phIisFft)->pCDFTSpec != NULL){            
    if(IIS_FFT_NO_ERROR != a6_ippsDFTFree_C_32f((*phIisFft)->pCDFTSpec )){
      return IIS_FFT_INTERNAL_ERROR;
	}
  }	
	
  if((*phIisFft)->pInReBuffer != NULL){            
    a6_ippsFree((*phIisFft)->pInReBuffer);
  }

  if((*phIisFft)->pInImBuffer != NULL){                  
    a6_ippsFree((*phIisFft)->pInImBuffer);
  }

  if((*phIisFft)->pOutReBuffer != NULL){             	
    a6_ippsFree((*phIisFft)->pOutReBuffer);
  }
	
  if((*phIisFft)->pOutImBuffer != NULL){             	
  	a6_ippsFree((*phIisFft)->pOutImBuffer);
  }
	
  if((*phIisFft)->p_FFT_Buffer != NULL){             	
    a6_ippsFree((*phIisFft)->p_FFT_Buffer);
  }

  return IIS_FFT_NO_ERROR;

}

static IIS_FFT_ERROR iis_fft_destroy_cfft_px(HANDLE_IIS_FFT* phIisFft){

  if((*phIisFft)->pCFFTSpec != NULL){            
    if(IIS_FFT_NO_ERROR != px_ippsFFTFree_C_32f((*phIisFft)->pCFFTSpec )){
	  return IIS_FFT_INTERNAL_ERROR;
	}  
  }
     
  if((*phIisFft)->pCDFTSpec != NULL){            
    if(IIS_FFT_NO_ERROR != px_ippsDFTFree_C_32f((*phIisFft)->pCDFTSpec )){
      return IIS_FFT_INTERNAL_ERROR;
	}
  }	
	
  if((*phIisFft)->pInReBuffer != NULL){            
    px_ippsFree((*phIisFft)->pInReBuffer);
  }

  if((*phIisFft)->pInImBuffer != NULL){                  
    px_ippsFree((*phIisFft)->pInImBuffer);
  }

  if((*phIisFft)->pOutReBuffer != NULL){             	
    px_ippsFree((*phIisFft)->pOutReBuffer);
  }
	
  if((*phIisFft)->pOutImBuffer != NULL){             	
  	px_ippsFree((*phIisFft)->pOutImBuffer);
  }
	
  if((*phIisFft)->p_FFT_Buffer != NULL){             	
    px_ippsFree((*phIisFft)->p_FFT_Buffer);
  }
  
  return IIS_FFT_NO_ERROR;

}



#endif




