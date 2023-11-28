/*****************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2005)
                               All Rights Reserved

   This software and/or program is protected by copyright law and
   international treaties. Any reproduction or distribution of this
   software and/or program, or any portion of it, may result in severe
   civil and criminal penalties, and will be prosecuted to the maximum
   extent possible under law.
   
************************************************************************************/
/******************************** iis-fft.h ****************************************/
#ifndef __INLCUDED_IIS_FFT_H
#define __INLCUDED_IIS_FFT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Data Types ************************************************************/
typedef struct T_IIS_FFT* HANDLE_IIS_FFT;

typedef enum {
  IIS_FFT_NO_ERROR = 0,
  IIS_FFT_INTERNAL_ERROR
} IIS_FFT_ERROR; 


typedef enum {
  IIS_FFT_FWD = -1,
  IIS_FFT_BWD =  1
} IIS_FFT_DIR; 

/*-------------------- function prototypes --------------------------------*/

/*---------------------------------------------------------------------------

    functionname: IIS_RFFT_Create
    description : allocate and initialize a new real FFT instance.
    returns:      IIS_FFT_NO_ERROR       indicates no error    
	          IIS_FFT_INTERNAL_ERROR indicates an error

  ---------------------------------------------------------------------------*/


IIS_FFT_ERROR IIS_RFFT_Create(
  HANDLE_IIS_FFT* phIisFft,     /* pointer to FFT handle */
  int len,			/* transform length  */
  int isign     	        /* sets sign of rotation coefficient:  exp(isign*2*pi/len * n*k)
                         	   i.e. +1=backward (IFFT), -1=forward transform (FFT)
				   BEWARE OF THE SIGNS !!!!!!*/
  );

/*-------------------------------------------------------------------------------
  
    functionname: IIS_CFFT_Create
    description : allocate and initialize a new complex FFT instance.     
    returns:      IIS_FFT_NO_ERROR       indicates no error    
                  IIS_FFT_INTERNAL_ERROR indicates an error

  ---------------------------------------------------------------------------*/

IIS_FFT_ERROR IIS_CFFT_Create(
  HANDLE_IIS_FFT* phIisFft, /* pointer to FFT handle */
  int len,                  /* transform length  */
  int iSign                 /* sets sign of rotation coefficient:  exp(isign*2*pi/len * n*k)
			       i.e. +1=backward (IFFT), -1=forward transform (FFT)    
			       BEWARE OF THE SIGNS !!!!!!*/
  );


/*---------------------------------------------------------------------------

    functionname: IIS_FFT_Apply_RFFT
    description:  computes Fourier backward or forward transform of a real signal 

    returns values:      IIS_FFT_NO_ERROR       indicates no error    
	                 IIS_FFT_INTERNAL_ERROR indicates an error
    
---------------------------------------------------------------------------*/

IIS_FFT_ERROR IIS_FFT_Apply_RFFT(
  HANDLE_IIS_FFT hIisFft,  /* pointer to FFT handle */
  float* pInBuffer,        /* pointer to the input array containing real values for the forward transform (FFT)
                             or packed complex values (Perm) for the backward transform (IFFT)*/  
  float* pOutBuffer        /* pointer to the output array containing real values resulted from the backward transform (IFFT)
                             or packed complex (Sperm) values reulted from the forward transform*/
								 );


/*-------------------------------------------------------------------------------

    functionname: IIS_FFT_Apply_CFFT
    description:  computes complex backward or forward fourier transform 
    returns:      IIS_FFT_NO_ERROR       indicates no error    
	          IIS_FFT_INTERNAL_ERROR indicates an error
    
-------------------------------------------------------------------------------*/

IIS_FFT_ERROR IIS_FFT_Apply_CFFT(
  HANDLE_IIS_FFT hIisFft,  /* pointer to FFT handle */
  float* pInReBuffer,      /* pointer to the input array containing real parts of the signal for the forward transform (FFT)
                             or for the backward transform (IFFT)*/  
  float* pInImBuffer,      /* pointer to the input array containing imaginary parts of the signal for the forward transform (FFT)
								                             or for the backward transform (IFFT)*/
  float* pOutReBuffer,     /* pointer to the output array containing real values resulted from the forward transform (FFT)
	                      or from the backward transform (IFFT)*/ 
  float* pOutImBuffer      /* pointer to the output array containing imaginary values resulted from the forward transform (FFT)
                              or from the backward transform (IFFT)*/ 
  );


/*---------------------------------------------------------------------------

    functionname: IIS_RFFT_Destroy
    description:  deallocate a real FFT instance
    returns:      IIS_FFT_NO_ERROR       indicates no error    
	          IIS_FFT_INTERNAL_ERROR indicates an error

  ---------------------------------------------------------------------------*/

IIS_FFT_ERROR IIS_RFFT_Destroy(
  HANDLE_IIS_FFT* phIisFft   /* pointer to FFT handle */
  );


/*---------------------------------------------------------------------------

    functionname: IIS_CFFT_Destroy
    description:  deallocate a complex FFT instance
    returns:      IIS_FFT_NO_ERROR       indicates no error    
	          IIS_FFT_INTERNAL_ERROR indicates an error

  ---------------------------------------------------------------------------*/

IIS_FFT_ERROR IIS_CFFT_Destroy(
  HANDLE_IIS_FFT* phIisFft   /* pointer to FFT handle */
  );


#ifdef __cplusplus
}
#endif

#endif
