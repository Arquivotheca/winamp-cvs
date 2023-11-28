#ifndef __WASABI_CFG_H
#define __WASABI_CFG_H

#define WA5

// BETA and NIGHT mofdified by the verctrl utility depend on the build type
// Do not remove

/*#define BETA*/
/*#define NIGHT*/

#ifdef BETA
#define ERROR_FEEDBACK
#else
#ifdef NIGHT
#define ERROR_FEEDBACK
#endif
#endif

#endif





