/******************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2006)
                               All Rights Reserved

 project:              utility library
 Initial author:       O. Kunz
 contents/description: - Configuration and commandline parsing tool
                       - part of the interface to utillib
                       - do not include this file, include utillib.h instead

 This software and/or program is protected by copyright law and
 international treaties. Any reproduction or distribution of this
 software and/or program, or any portion of it, may result in severe
 civil and criminal penalties, and will be prosecuted to the maximum
 extent possible under law.

 $Id: uci.h,v 1.1 2009/04/28 20:17:48 audiodsp Exp $

******************************************************************************/

#ifndef __UCI_H__
#define __UCI_H__

#ifdef __cplusplus
extern "C" {
#endif

  typedef enum {
    UCI_NO_ERROR,
    PARAMETER_NOT_REQUESTED,
    RANGE_CHECK_FAILED,
    TOO_FEW_ARGUMENTS,
    TOO_MANY_ARGUMENTS,
    CONFIGFILE_NOT_FOUND,
    TOO_MANY_PARAMETERS,
    EXTENDED_HELP_PROCESSED,
    NO_ENTRY_FOUND,
    UCI_OUT_OF_MEMORY,
    HELP_MODE_ACTIVE,
    DEST_STRING_TOO_SHORT
  } T_UCI_STATUS;

  T_UCI_STATUS UciInit(void);

  void         UciReset(void);

  T_UCI_STATUS AllowParameterSubset(const char *const allowedParameters[],
                                    int numOfAllowedParameters);

  T_UCI_STATUS ParseCommandline(int argc, char *argv[]);

  T_UCI_STATUS ParseConfigFile(char *filename);

  int SetLayerId(int newId, int numOfExistingLayers);

  T_UCI_STATUS SetParameter(const char *tag, char *argument);

  T_UCI_STATUS GetIntParameter(const char *tag, int numberOfValues, int minValue, int maxValue,
                               const char *help, const char *extendedHelp, int *value);

  T_UCI_STATUS GetFloatParameter(const char *tag, int numberOfValues, float minValue,
                                 float maxValue, const char *help,
                                 const char *extendedHelp, float *value);

  T_UCI_STATUS GetLayeredIntParameter(const char *tag, int minValue, int maxValue,
                                      const char *help, const char *extendedHelp, int *value);

  T_UCI_STATUS GetLayeredFloatParameter(const char *tag, float minValue,
                                        float maxValue, const char *help,
                                        const char *extendedHelp, float *value);

  T_UCI_STATUS GetCharParameter(const char *tag, const char *help, const char *extendedHelp,
                                char *value, unsigned int stringLen);

  T_UCI_STATUS GetSwitch(const char *tag, const char *help, const char *extendedHelp,
                         int *switchActivated);

  T_UCI_STATUS SetExpertFlags (int urs, int sw_visible);

  T_UCI_STATUS ExistencyCheck(void);
  T_UCI_STATUS GetUciHelpMode(void);

  void SetSingleSwitch(char clSwitch[], char* infoOutput) ;
  void SetSingleInt (char* clSwitch, int value, char* infoOutput) ;
  void SetSingleFloat (char* clSwitch, float value, char* infoOutput) ;
  void SetSingleString (char* clSwitch, char* value, char* infoOutput) ;
  void SetLayeredInt (char* clSwitch,int numValues ,int *value, char* infoOutput) ;
#define INFOOUTPUTLEN 2000
#ifdef __cplusplus
           }
#endif

#endif
