#ifndef NULLSOFT_WINAMP_BUILD_TYPE_H
#define NULLSOFT_WINAMP_BUILD_TYPE_H


// DO NOT MODIFY !!!
// BETA and NIGHT mofdified by the verctrl utility depend on the build type
// Do not remove


#define INTERNAL

/*#define BETA*/
/*#define NIGHT*/
/*#define NOKIA*/

#ifdef BETA
  #define ERROR_FEEDBACK
#else
  #ifdef NIGHT
    #define ERROR_FEEDBACK
  #endif
#endif

#if !defined(BETA) && !defined(NIGHT) && !defined(INTERNAL)
#define WINAMP_FINAL_BUILD
#endif

#endif // NULLSOFT_WINAMP_BUILD_TYPE_H



