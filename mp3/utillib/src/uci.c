/******************************** MPEG Audio Encoder **************************

                       (C) copyright Fraunhofer-IIS (1997-2006)
                               All Rights Reserved

   Initial author:       O. Kunz
   contents/description: Configuration and commandline parsing tool

   This software and/or program is protected by copyright law and
   international treaties. Any reproduction or distribution of this
   software and/or program, or any portion of it, may result in severe
   civil and criminal penalties, and will be prosecuted to the maximum
   extent possible under law.

   $Id: uci.c,v 1.1 2009/04/28 20:17:48 audiodsp Exp $

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "IISutillib/ngsalloc.h"
#include "IISutillib/errorhnd.h"
#include "IISutillib/uci.h"

typedef int BOOL;
#define FALSE 0
#define TRUE (!FALSE)

#define MAX_ARGS 1000          /*max. number of configuration parameters*/
#define MAX_TAG_LENGTH 100    /*max. length of tag name including SUPER_TAGS*/
#define MAX_VALUE_LENGTH 1024  /*max. length of argument string*/
#define MAX_LINE_LENGTH MAX_VALUE_LENGTH + 100   /*max: length of line in configuration file*/

static const int NUM_OF_INTERNAL_PARAMETERS = 80;

typedef enum {
  INVISIBLE,
  INTERNAL,
  EXTERNAL
} T_VISIBILITY;

typedef enum {
  NO_HELP,
  HELP,
  EXTENDED_HELP
} T_HELP_TYPE;

typedef struct {
  char *tag;                    /*parameter name*/
  char *value;                  /*parameter arguments*/
  BOOL helpProcessed;           /*info about help processing*/
  BOOL addedOnlyforHelp;        /*info about help processing*/
  BOOL extendedHelpRequested;   /*extended help printout?*/
  char *extendedHelpText;       /*extended help text*/
  T_UCI_STATUS statusInfo;      /*processing status of entry*/
  BOOL external;   /*value set by real commandline parameter ? if not: set by setparameter() internaly*/
} T_CONFIG_ENTRY;

static const char SUPER_TAG_BEGIN_ID = ':';   /*separator for tag and super-tag*/
static const char SUPER_TAG_END_ID[] = "END"; /*end id of hierarchical section
                                                in parameter file*/
static const char COMMENT_BEGIN_ID = '#';     /*comment identifier*/

/*static variables used during initialization*/
static int cntEntries = -1;
static int currentArgc = -1;
static T_HELP_TYPE helpMode = NO_HELP;
static T_CONFIG_ENTRY *configData;
static char **allowedParameterList;
static int allowedListLength = 0;
static char **internallyVisibleParameterList;
static int intVisibleListLength = 0;
static int layerId = 0;
static int numOfLayers = 1;

/*****************************************************************************

    functionname: Get next element
    description:  retrieves next from commandline
    returns:      ptr to string
    input:        commandline contents
    output:

*****************************************************************************/
static char *
GetNextElement(int argc, char *argv[]) {
  currentArgc++;
  if(currentArgc < argc)
	return(argv[currentArgc]);
  else
	return NULL;
}


/*****************************************************************************

    functionname: AddItemToList
    description:  adds item to list of detected configuration elements
    returns:
    input:        tag of parameter, parameter arguments, flag if extended help
                  is requested
    output:

*****************************************************************************/
static T_UCI_STATUS
AddItemToList(const char *tag, char *value, BOOL extendedHelp,BOOL external) {
  if(configData != 0) {
    configData[cntEntries].tag = (char*)ngsCalloc(strlen(tag)+1, sizeof(char));
    assert(configData[cntEntries].tag != 0);
    if(configData[cntEntries].tag == 0) {
      return(UCI_OUT_OF_MEMORY);
    }
    strcpy(configData[cntEntries].tag, tag);
    configData[cntEntries].value = (char*)ngsCalloc(strlen(value)+1, sizeof(char));
    assert(configData[cntEntries].value != 0);
    if(configData[cntEntries].value == 0) {
      return(UCI_OUT_OF_MEMORY);
    }
    strcpy(configData[cntEntries].value, value);
    configData[cntEntries].helpProcessed = FALSE;
    configData[cntEntries].addedOnlyforHelp= FALSE;
    configData[cntEntries].extendedHelpRequested = extendedHelp;
    configData[cntEntries].extendedHelpText = 0;
    configData[cntEntries].statusInfo = PARAMETER_NOT_REQUESTED;
    configData[cntEntries].external = external;
    cntEntries++;
  }
  return(UCI_NO_ERROR);
}

/*****************************************************************************

    functionname: FindMatchingItem
    description:  checks wether a specific tag has been given via configuration
    returns:      pointer if found, 0 otherwise
    input:        ptr to tag name
    output:

*****************************************************************************/
static T_CONFIG_ENTRY *
FindMatchingItem(const char *tag, T_VISIBILITY visibility) {

  int i;

  for(i=0; i<cntEntries; i++) {
    if(strncmp(configData[i].tag, tag, MAX_TAG_LENGTH) == 0) {
      if (!((visibility== INTERNAL) && (configData[i].external==TRUE)))
        return(&configData[i]);
    }
  }
  return(0);
}

/*****************************************************************************

    functionname: SetLayerId
    description:  changes layer number used in GetLayeredIntParameter()
                  and GetLayeredFloatParameter()
    returns:      current layer id
    input:
    output:

*****************************************************************************/
int SetLayerId(int newId, int numOfExistingLayers) {
  layerId = newId;
  if (numOfExistingLayers>0)
    numOfLayers = numOfExistingLayers;
  WARNIF(layerId >= numOfLayers, "layerID too high");
  return(layerId);
}


/*****************************************************************************

    functionname: UciInit
    description:  allocates memory for storage of configuration parameters
    returns:      Status info
    input:
    output:

*****************************************************************************/
T_UCI_STATUS UciInit() {

  configData = 0;
  if(cntEntries ==-1) { /*initial condition*/
    /*by this it can be assured that no two applications try to use UCI simultaneously*/
    cntEntries = 0;
    configData = (T_CONFIG_ENTRY*)ngsCalloc(MAX_ARGS, sizeof(T_CONFIG_ENTRY));
  }
  assert(configData != NULL);

  if(configData == 0) {
    cntEntries = -1; /*restore initial condition*/
    return(UCI_OUT_OF_MEMORY);
  } else {
    /*
      allocate storage for parameters set by the application, not by the user
      This is needed to properly use these parameters even when their use from
      outside is not permitted
    */
    allowedListLength = 0;
    allowedParameterList = NULL;
    internallyVisibleParameterList = (char**)ngsMalloc(NUM_OF_INTERNAL_PARAMETERS * sizeof(char*));
    if (!internallyVisibleParameterList) return UCI_OUT_OF_MEMORY;
    return(UCI_NO_ERROR);
  }
}

/*****************************************************************************

    functionname: CheckIfParameterIsAllowed
    description:  internally used to check whether or not parameter may be
                  used in current configuration
    returns:      TRUE if allowed
    input:        tag name to check
    output:

*****************************************************************************/
static T_VISIBILITY CheckParameterVisibility(const char *tagName) {

  int i;
  unsigned int length;

  if(allowedListLength == 0) {
    return(EXTERNAL);
  } else {
    for(i=0; i<allowedListLength; i++) {
      length = strlen(tagName);
      if(strncmp(tagName, allowedParameterList[i], length) == 0 &&
         allowedParameterList[i][length] == 0) {
        return(EXTERNAL);
      }
    }
    for(i=0; i<intVisibleListLength; i++) {
      length = strlen(tagName);
      if(strncmp(tagName, internallyVisibleParameterList[i], length) == 0 &&
         internallyVisibleParameterList[i][length] == 0) {
        return(INTERNAL);
      }
    }
  }
  return(INVISIBLE);
}

/*****************************************************************************

    functionname: AllowParameterSubset
    description:  externally used to explicitly specify the allowed parameter
                  tags the are allowed to be used
    returns:      Status Info
    input:        ptr to array of allowed tag names, num of tags given
    output:

*****************************************************************************/
T_UCI_STATUS AllowParameterSubset(const char *const allowedParameters[],
                                  int numOfAllowedParameters)
{
  int i, j;
  unsigned int tagLength;

  j=0;
  allowedParameterList = (char**)ngsMalloc(numOfAllowedParameters * sizeof(char*));
  if (!allowedParameterList) return UCI_OUT_OF_MEMORY;
  for(i=0; i<numOfAllowedParameters; i++) {
    if(allowedParameters[i] != 0) {
      tagLength = strlen(allowedParameters[i]);
      allowedParameterList[j] = (char*)ngsMalloc((tagLength+1) * sizeof(char));
      if(allowedParameterList[j] != 0) {
        strncpy(allowedParameterList[j], allowedParameters[i], tagLength+1);
        j++;
      } else {
        return(UCI_OUT_OF_MEMORY);
      }
    }
  }
  allowedListLength = j;
  return(UCI_NO_ERROR);
}


/*****************************************************************************

    functionname: CheckNumberOfArguments
    description:  checks whether the number of arguments requested matches
                  the number of arguments give to parameter
    returns:      status
    input:        argumentString: ptr to argument string,
                  tag: parameter name the argument string belongs to
                       (only for warning text)
    output:

    in/out:       numOfArgs: number of arguments expected
                             will be reduced if argumentString carries less
                             arguments than expected

*****************************************************************************/
static T_UCI_STATUS
CheckNumberOfArguments(char *argumentString, int *numOfArgs, const char *tag) {

  char parameterCopy[MAX_VALUE_LENGTH];
  char warningText[255];
  char *singleValue;
  int i;
  T_UCI_STATUS returnCode = UCI_NO_ERROR;

  /*pre compose warning text*/
  sprintf(warningText, "At parameter %s: ", tag);

  /*check for correct number of arguments*/
  strcpy(parameterCopy, argumentString);
  singleValue = strtok(parameterCopy, " ");
  for(i=0; i<*numOfArgs; i++) {
    /*less arguments than expected*/
    if(singleValue == NULL) {
      returnCode = TOO_FEW_ARGUMENTS;
      *numOfArgs = i-1; /*patch number of arguments to the number really existing*/
      strcat(warningText, "Too few arguments in configuration");
      WARN(warningText);
      break;
    }
    singleValue = strtok(NULL, " ");
  }

  if(singleValue != NULL) {
    /*more arguments than expected*/
    returnCode = TOO_MANY_ARGUMENTS;
    strcat(warningText, "Too many arguments in configuration");
    WARN(warningText);
  }

return(returnCode);
}



/*****************************************************************************

    functionname: ParseCommandline
    description:  reads commandline and builds internal representation for
                  processing of requests
    returns:      Status info
    input:        commandline
    output:

*****************************************************************************/
T_UCI_STATUS
ParseCommandline(int argc, char *argv[]) {

  char *argToParse;
  char tmpTag[MAX_TAG_LENGTH];
  char tmpValue[MAX_VALUE_LENGTH];
  T_UCI_STATUS returnValue = UCI_NO_ERROR;

  /*check for availability of config-item data structure*/
  if(configData == 0) {
    fprintf(stderr, "No internal list available, call UciInit() first!!)");
    exit(1);
  }

  argToParse = GetNextElement(argc, argv);
  while((cntEntries <= MAX_ARGS)  && (argToParse != 0))
  {
    if(*argToParse == '-' && !isdigit((int)argToParse[1])) {
      /*switch*/
      argToParse++;
      if(strcmp(argToParse, "h") == 0) {
        /*help mode*/
        helpMode = HELP;
        argToParse = GetNextElement(argc, argv);
        while((argToParse != 0) && (*argToParse != '-')) {
          /*check for parameters with extended help required*/
          strcpy(tmpTag, argToParse); /*store tag for insertion into list*/
          returnValue = AddItemToList(tmpTag, "\0", TRUE,TRUE);
          helpMode = EXTENDED_HELP;
          argToParse = GetNextElement(argc, argv);
        }
      } else {
        /*ordinary switch*/
        strcpy(tmpTag, argToParse); /*store tag for insertion into list*/
        /*check for possible arguments to switch*/
        argToParse = GetNextElement(argc, argv);
        tmpValue[0] = '\0';
        while((argToParse != 0) && (*argToParse != '-' || isdigit((int)argToParse[1]))) {
          /*collect arguments for current switch*/
          strncat(tmpValue, argToParse, MAX_VALUE_LENGTH - strlen(tmpValue));
          argToParse = GetNextElement(argc, argv);
          if ((argToParse != 0) && (*argToParse != '-' || isdigit((int)argToParse[1]))) {

            /*guarantee blank between multiple arguments*/
            strncat(tmpValue, " ",MAX_VALUE_LENGTH - strlen(tmpValue));
          }
        }
        returnValue = AddItemToList(tmpTag, tmpValue, FALSE,TRUE);
      }
    } else {
      /*hidden tag, e.g. input-file-name*/
      sprintf(tmpTag, "%d", currentArgc);
      returnValue = AddItemToList(tmpTag, argToParse, FALSE,TRUE);
      argToParse = GetNextElement(argc, argv);
    }
  }

  if(cntEntries > MAX_ARGS) {
    return(TOO_MANY_PARAMETERS);
  }

  if(helpMode != NO_HELP) {
    return(HELP_MODE_ACTIVE);
  } else {
    return(returnValue);
  }
}

/*****************************************************************************

    functionname: ParseConfigFile
    description:  reads config file and builds up internal representation for
                  processing of requests
    returns:      Status info
    input:        name of config file
    output:

*****************************************************************************/
T_UCI_STATUS
ParseConfigFile(char *fileName) {

  FILE *fileDescriptor;
  char *pLine;
  char lineInFile[MAX_LINE_LENGTH];
  char *tmpElement, *lastButOneColon;
  char tmpTag[MAX_TAG_LENGTH];
  char tmpSuperTag[MAX_TAG_LENGTH];
  char tmpArgument[MAX_VALUE_LENGTH];
  int i;
  T_UCI_STATUS returnValue = UCI_NO_ERROR;

  /*check for availability of config-item data structure*/
  if(configData == 0) {
    fprintf(stderr, "No internal list available, call UciInit() first!!\n");
    exit(1);
  }

  fileDescriptor = 0;
  if( strlen( fileName ) != 0 )
  {
    fileDescriptor = fopen(fileName,"r");
  }
  if(fileDescriptor == 0) {
    /*fprintf(stderr, "Error opening Config file.\nExecution aborted!\n");*/
    return(CONFIGFILE_NOT_FOUND);
  }

  /*scan config file and parse each line separately*/
  i=0;
  tmpSuperTag[0] = '\0';
  while (feof(fileDescriptor) == 0) {
    pLine = fgets(lineInFile, MAX_LINE_LENGTH -1, fileDescriptor);
    tmpElement = strtok(pLine, "\t \n");
    if(tmpElement != NULL) { /*guarantee valid tag*/
      /*check for comment delimiter, if found here, ignore complete line*/
      if(strncmp(tmpElement, &COMMENT_BEGIN_ID, 1) != 0) {
        if(strcmp(tmpElement, SUPER_TAG_END_ID) == 0) {
          /* end of innermost super tag region*/
          /*cut last colon*/
          tmpSuperTag[strlen(tmpSuperTag) - 1] = '\0';
          /*find last but one colon and replace it by '\0' to end the string*/
          lastButOneColon = strrchr(tmpSuperTag, (int)SUPER_TAG_BEGIN_ID);
          if(lastButOneColon != NULL) {
            *lastButOneColon = '\0';
          } else {
            *tmpSuperTag = '\0';
          }
        } else if(tmpElement[strlen(tmpElement) -1] == SUPER_TAG_BEGIN_ID) {
          /*super tag found, add to tmpSuperTag*/
          strcat(tmpSuperTag, tmpElement);
        } else {
          /*normal tag found, collect argument string and add to parameter list*/
          strcpy(tmpTag, tmpSuperTag);
          strcat(tmpTag, tmpElement);
          tmpElement = strtok(NULL, "#\n");
          if(tmpElement == NULL) {
            *tmpArgument = '\0';
          } else {
            strcpy(tmpArgument, tmpElement);
          }
          returnValue = AddItemToList(tmpTag, tmpArgument, FALSE,TRUE);
        }
      }
    }
  }
  fclose(fileDescriptor);
  return(returnValue);
}


/*****************************************************************************

    functionname: SetParameter
    description:  allows the emulation of a parameter set on commadline.
                  BE CAREFUL when using this because it can do harm to your
                  application. Using this requires in depth knowledge about
                  UCI internals. Parameters can only be set before being
                  requested. Keep this in mind.

    returns:      Status info
    input:        parameter tag, argument text
    output:       -

*****************************************************************************/
T_UCI_STATUS
SetParameter(const char *tag, char *argument) {
  unsigned int tagLength;
  T_UCI_STATUS returnValue = UCI_NO_ERROR;

  WARNIF(intVisibleListLength == NUM_OF_INTERNAL_PARAMETERS, "Overflow in internal parameter list");
  if(tag != 0 && (intVisibleListLength < NUM_OF_INTERNAL_PARAMETERS)) {
    returnValue = AddItemToList(tag, argument, FALSE,FALSE);
    tagLength = strlen(tag);
    internallyVisibleParameterList[intVisibleListLength] = (char*)ngsMalloc((tagLength+1) * sizeof(char));
    if (!internallyVisibleParameterList[intVisibleListLength]) {
      return UCI_OUT_OF_MEMORY; /* out of memory */
    }
    strncpy(internallyVisibleParameterList[intVisibleListLength],
            tag, tagLength+1);
    intVisibleListLength++;
  }
  return(returnValue);
}


void SetSingleSwitch(char clSwitch[], char* infoOutput) {

  SetParameter(clSwitch,"");
  assert(strlen(infoOutput)+strlen(clSwitch)<INFOOUTPUTLEN);
  strcat(infoOutput,"-");
  strcat(infoOutput,clSwitch);
  strcat(infoOutput," ");
}

void SetSingleInt (char* clSwitch, int value, char* infoOutput) {
  char valueText[10];
  sprintf(valueText,"%d",value);
  SetParameter(clSwitch,valueText);
  assert(strlen(infoOutput)+strlen(valueText)+strlen(clSwitch)+3<INFOOUTPUTLEN);
  strcat(infoOutput,"-");
  strcat(infoOutput,clSwitch);
  strcat(infoOutput," ");
  strcat(infoOutput,valueText);
  strcat(infoOutput," ");
}

void SetSingleFloat (char* clSwitch, float value, char* infoOutput) {
  char valueText[10];
  sprintf(valueText,"%5.2f",value);
  SetParameter(clSwitch,valueText);
  assert(strlen(infoOutput)+strlen(valueText)+strlen(clSwitch)+3<INFOOUTPUTLEN);
  strcat(infoOutput,"-");
  strcat(infoOutput,clSwitch);
  strcat(infoOutput," ");
  strcat(infoOutput,valueText);
  strcat(infoOutput," ");
}
void SetSingleString (char* clSwitch, char* value, char* infoOutput) {
  SetParameter(clSwitch,value);
  assert(strlen(infoOutput)+strlen(value)+strlen(clSwitch)+3<INFOOUTPUTLEN);
  strcat(infoOutput,"-");
  strcat(infoOutput,clSwitch);
  strcat(infoOutput," ");
  strcat(infoOutput,value);
  strcat(infoOutput," ");
}

void SetLayeredInt (char* clSwitch,int numValues ,int *value, char* infoOutput) {
  int i;
  char valueText[100];
  char tmpText[100];

  sprintf(valueText," ");
  for (i=0;i<numValues;i++){
    sprintf(tmpText,"%d ",value[i]);
    strcat(valueText,tmpText);
  }
  SetParameter(clSwitch,valueText);
  assert(strlen(infoOutput)+strlen(valueText)+strlen(clSwitch)+3<INFOOUTPUTLEN);
  strcat(infoOutput,"-");
  strcat(infoOutput,clSwitch);
  strcat(infoOutput," ");
  strcat(infoOutput,valueText);
  strcat(infoOutput," ");
}



/*****************************************************************************

    functionname: GetIntParameter
    description:  retrieves integer arguments belonging to a parameter
                  eventually given via commandline of config file
    returns:      Status info
    input:        parameter tag, number of arguments to read, min value of
                  arguments, max value of arguments, help text, extended
                  help text
    output:       values retrieved via commandline or configfile (if
                  parameter was given

*****************************************************************************/
T_UCI_STATUS
GetIntParameter(const char *tag, int numberOfValues, int minValue, int maxValue,
                const char *help, const char *extendedHelp, int *value) {

  T_CONFIG_ENTRY *config;
  char *singleValue;
  int convertedValue;
  int i;
  T_UCI_STATUS returnCode = UCI_NO_ERROR;
  char warningText[255];
  T_VISIBILITY visibility;

  /*pre compose warning text*/
  sprintf(warningText, "At parameter %s: ", tag);

  visibility = CheckParameterVisibility(tag);
  if(visibility == INVISIBLE) {
    return (NO_ENTRY_FOUND);
  }

  config = FindMatchingItem(tag,visibility);
  /*tag found ?*/

  if((helpMode == HELP) && (visibility == EXTERNAL) &&
    ((config == 0) || (config->helpProcessed == FALSE))) {
    if(config == 0) {
      returnCode = AddItemToList(tag, "", FALSE,FALSE);
      if(returnCode == UCI_NO_ERROR) {
        config = FindMatchingItem(tag,visibility);
        if (config) {
          config->statusInfo = UCI_NO_ERROR;
          config->helpProcessed = TRUE;
          config = 0;
        }
      }
    } else {
      config->helpProcessed = TRUE;
    }
/*     if(visibility == INTERNAL) { */
      if (help) {
	fprintf(stderr,
		"-%s %d*int\t\t",
		tag, numberOfValues);
	fprintf(stderr,"%s\n", help);
      }
/*     } */
  }

  if((config != 0)) {
    char localValue[MAX_VALUE_LENGTH];

    /*extract and check arguments*/
    if((config->extendedHelpRequested == FALSE) && (config->value[0]!='\0') && (helpMode == NO_HELP)) {
      /*check for correct number of arguments*/
      returnCode = CheckNumberOfArguments(config->value, &numberOfValues, tag);
      strcpy(localValue, config->value);
      singleValue = strtok(localValue, " ");
      while( (strlen(singleValue)>1) && (singleValue[0]=='0') ) singleValue++;
      for(i=0; i<numberOfValues; i++) {
        convertedValue = strtol(singleValue,NULL,0);
        if((maxValue == minValue) ||
           ((convertedValue >= minValue) && (convertedValue <= maxValue))) {
          value[i] = convertedValue;
        } else {
          /*if a range error in the parameters is encountered, force extendedHelp printout*/
          config->extendedHelpRequested = TRUE;
          returnCode = RANGE_CHECK_FAILED;
          strcat(warningText, "Range Check failed");
          WARN(warningText);
          break;
        }
        singleValue = strtok(NULL, " ");
      }

    }

    config->statusInfo = returnCode; /*store info on processing effort*/

    if((config->extendedHelpRequested == TRUE) && extendedHelp) {
      /*copy extended help text to list*/
      config->extendedHelpText = (char*)ngsMalloc(strlen(extendedHelp)+1);
      strcpy(config->extendedHelpText, extendedHelp);
    }
  } else {
    returnCode = NO_ENTRY_FOUND;
  }
  return(returnCode);
}


/*****************************************************************************

    functionname: GetLayeredIntParameter
    description:  retrieves *a single* integer argument belonging to a parameter
                  eventually given via commandline of config file The position of
                  the argument in the argument list is determined by "layerId"
                  which can be set via SetLayerId()
    returns:      Status info
    input:        parameter tag, min value of
                  arguments, max value of arguments, help text, extended
                  help text
    output:       value retrieved via commandline or configfile (if
                  parameter was given)

*****************************************************************************/
T_UCI_STATUS
GetLayeredIntParameter(const char *tag, int minValue, int maxValue,
                       const char *help, const char *extendedHelp, int *value) {

  T_CONFIG_ENTRY *config;
  char *singleValue;
  int convertedValue;
  int i;
  int numberOfValues=1;
  T_UCI_STATUS returnCode = UCI_NO_ERROR;
  char warningText[255];
  T_VISIBILITY visibility;

  /*pre compose warning text*/
  sprintf(warningText, "At parameter %s: ", tag);

  visibility = CheckParameterVisibility(tag);
  if(visibility == INVISIBLE) {
    return (NO_ENTRY_FOUND);
  }

  config = FindMatchingItem(tag,visibility);
  /*tag found ?*/

  if((helpMode == HELP) && (visibility == EXTERNAL) &&
    ((config == 0) || (config->helpProcessed == FALSE))) {
    if(config == 0) {
      returnCode = AddItemToList(tag, "", FALSE,FALSE);
      if(returnCode == UCI_NO_ERROR) {
        config = FindMatchingItem(tag,visibility);
        if (config) {
          config->statusInfo = UCI_NO_ERROR;
          config->helpProcessed = TRUE;
          config = 0;
        }
      }
    } else {
      config->helpProcessed = TRUE;
    }
    if (help) {
      fprintf(stderr,
	      "-%s int\t\t",
	      tag);
      fprintf(stderr,"%s\n", help);
    }
  }

  if((config != 0)) {
    /*extract and check arguments*/
    if((config->extendedHelpRequested == FALSE) && (config->value[0]!='\0') && (helpMode == NO_HELP)) {
      char confValuCopy[MAX_VALUE_LENGTH];
      strcpy(confValuCopy, config->value);

      /*check for correct number of arguments*/
      numberOfValues = numOfLayers;
      returnCode = CheckNumberOfArguments(confValuCopy, &numberOfValues, tag);

      singleValue = strtok(confValuCopy, " ");
      while( (strlen(singleValue)>1) && (singleValue[0]=='0') ) singleValue++;
      for(i=0; i<numberOfValues; i++) {
        if (i == layerId) {
          convertedValue = strtol(singleValue,NULL,0);
          if((maxValue == minValue) ||
             ((convertedValue >= minValue) && (convertedValue <= maxValue))) {
            *value = convertedValue;
          } else {
            /*if a range error in the parameters is encountered, force extendedHelp printout*/
            config->extendedHelpRequested = TRUE;
            returnCode = RANGE_CHECK_FAILED;
            strcat(warningText, "Range Check failed");
            WARN(warningText);
            break;
          }
        }
        singleValue = strtok(NULL, " ");
      }

    }

    config->statusInfo = returnCode; /*store info on processing effort*/

    if((config->extendedHelpRequested == TRUE) && extendedHelp) {
      /*copy extended help text to list*/
      config->extendedHelpText = (char*)ngsMalloc(strlen(extendedHelp)+1);
      strcpy(config->extendedHelpText, extendedHelp);
    }
  } else {
    returnCode = NO_ENTRY_FOUND;
  }
  return(returnCode);
}


/*****************************************************************************

    functionname: GetFloatParameter
    description:  retrieves floating point arguments belonging to a parameter
                  eventually given via commandline of config file
    returns:      Status info
    input:        parameter tag, number of arguments to read, min value of
                  arguments, max value of arguments, help text, extended
                  help text
    output:       values retrieved via commandline or configfile (if
                  parameter was given

*****************************************************************************/
T_UCI_STATUS
GetFloatParameter(const char *tag, int numberOfValues, float minValue, float maxValue,
                  const char *help, const char *extendedHelp, float *value) {

  T_CONFIG_ENTRY *config;
  char *singleValue;
  float convertedValue;
  int i;
  T_UCI_STATUS returnCode = UCI_NO_ERROR;
  char warningText[255];
  T_VISIBILITY visibility;

  /*pre compose warning text*/
  sprintf(warningText, "At parameter %s: ", tag);

  visibility = CheckParameterVisibility(tag);
  if(visibility == INVISIBLE) {
    return (NO_ENTRY_FOUND);
  }

  config = FindMatchingItem(tag,visibility);
  /*tag found ?*/

  if((helpMode == HELP) && (visibility == EXTERNAL) &&
    ((config == 0) || (config->helpProcessed == FALSE))) {
    if(config == 0) {
      returnCode = AddItemToList(tag, "", FALSE,FALSE);
      if(returnCode == UCI_NO_ERROR) {
        config = FindMatchingItem(tag,visibility);
        if (config) {
          config->statusInfo = UCI_NO_ERROR;
          config->helpProcessed = TRUE;
          config = 0;
        }
      }
    } else {
      config->helpProcessed = TRUE;
    }
    if (help) {
      fprintf(stderr,
	      "-%s %d*float\t\t",
	      tag, numberOfValues);
      fprintf(stderr,"%s\n", help);
    }
  }

  /*tag found ?*/
  if((config != 0)) {
    /*extract and check arguments*/
    if((config->extendedHelpRequested == FALSE) && (config->value[0]!='\0') && (helpMode == NO_HELP)) {
      /*check for correct number of arguments*/
      returnCode = CheckNumberOfArguments(config->value, &numberOfValues, tag);
      /*extract first value from string*/
      singleValue = strtok(config->value, " ");
      for(i=0; i<numberOfValues; i++) {
        convertedValue = (float)atof(singleValue);
        if((convertedValue >= minValue) && (convertedValue <= maxValue)) {
          value[i] = convertedValue;
        } else {
          /*if an error in the parameters is encountered, force extendedHelp printout*/
          config->extendedHelpRequested = TRUE;
          returnCode = RANGE_CHECK_FAILED;
          strcat(warningText, "Range Check failed");
          WARN(warningText);
          break;
        }
        singleValue = strtok(NULL, " ");
      }
    }

    config->statusInfo = returnCode; /*store info on processing effort*/

    if((config->extendedHelpRequested == TRUE) && extendedHelp) {
      /*copy extended help text to list*/
      config->extendedHelpText = (char*)ngsMalloc(strlen(extendedHelp)+1);
      strcpy(config->extendedHelpText, extendedHelp);
    }
  } else {
    returnCode = NO_ENTRY_FOUND;
  }
  return(returnCode);
}


/*****************************************************************************

    functionname: GetLayeredFloatParameter
    description:  retrieves *a single* integer argument belonging to a parameter
                  eventually given via commandline of config file The position of
                  the argument in the argument list is determined by "layerId"
                  which can be set via SetLayerId()
    returns:      Status info
    input:        parameter tag, min value of
                  arguments, max value of arguments, help text, extended
                  help text
    output:       value retrieved via commandline or configfile (if
                  parameter was given)

*****************************************************************************/
T_UCI_STATUS
GetLayeredFloatParameter(const char *tag, float minValue, float maxValue,
                  const char *help, const char *extendedHelp, float *value) {

  T_CONFIG_ENTRY *config;
  char *singleValue;
  float convertedValue;
  int i;
  int numberOfValues;
  T_UCI_STATUS returnCode = UCI_NO_ERROR;
  char warningText[255];
  T_VISIBILITY visibility;

  /*pre compose warning text*/
  sprintf(warningText, "At parameter %s: ", tag);

  visibility = CheckParameterVisibility(tag);
  if(visibility == INVISIBLE) {
    return (NO_ENTRY_FOUND);
  }

  config = FindMatchingItem(tag,visibility);
  /*tag found ?*/

  if((helpMode == HELP) && (visibility == EXTERNAL) &&
    ((config == 0) || (config->helpProcessed == FALSE))) {
    if(config == 0) {
      returnCode = AddItemToList(tag, "", FALSE,FALSE);
      if(returnCode == UCI_NO_ERROR) {
        config = FindMatchingItem(tag,visibility);
        if (config) {
          config->statusInfo = UCI_NO_ERROR;
          config->helpProcessed = TRUE;
          config = 0;
        }
      }
    } else {
      config->helpProcessed = TRUE;
    }
    if (help) {
      fprintf(stderr,
	      "-%s float\t\t",
	      tag);
      fprintf(stderr,"%s\n", help);
    }
  }

  /*tag found ?*/
  if((config != 0)) {
    /*extract and check arguments*/
    if((config->extendedHelpRequested == FALSE) && (config->value[0]!='\0') && (helpMode == NO_HELP)) {
      char confValuCopy[MAX_VALUE_LENGTH];

      strcpy(confValuCopy, config->value);
      /*check for correct number of arguments*/
      numberOfValues = numOfLayers;
      returnCode = CheckNumberOfArguments(confValuCopy, &numberOfValues, tag);

      /*extract first value from string*/
      singleValue = strtok(confValuCopy, " ");
      for(i=0; i<numberOfValues; i++) {
        if (i == layerId) {
          convertedValue = (float)atof(singleValue);
          if((convertedValue >= minValue) && (convertedValue <= maxValue)) {
            *value = convertedValue;
          } else {
            /*if an error in the parameters is encountered, force extendedHelp printout*/
            config->extendedHelpRequested = TRUE;
            returnCode = RANGE_CHECK_FAILED;
            strcat(warningText, "Range Check failed");
            WARN(warningText);
            break;
          }
        }
        singleValue = strtok(NULL, " ");
      }
    }

    config->statusInfo = returnCode; /*store info on processing effort*/

    if((config->extendedHelpRequested == TRUE) && extendedHelp) {
      /*copy extended help text to list*/
      config->extendedHelpText = (char*)ngsMalloc(strlen(extendedHelp)+1);
      strcpy(config->extendedHelpText, extendedHelp);
    }
  } else {
    returnCode = NO_ENTRY_FOUND;
  }
  return(returnCode);
}


/*****************************************************************************

    functionname: GetCharParameter
    description:  retrieves character argument belonging to a parameter
                  eventually given via commandline of config file
    returns:      Status info
    input:        parameter tag, help text, extended help text
    output:       string retrieved via commandline or configfile (if
                  parameter was given

*****************************************************************************/
T_UCI_STATUS
GetCharParameter(const char *tag, const char *help, const char *extendedHelp, char *value, unsigned int stringLen) {

  T_CONFIG_ENTRY *config;
  T_UCI_STATUS returnCode = UCI_NO_ERROR;
  T_VISIBILITY visibility;

  visibility = CheckParameterVisibility(tag);
  if(visibility == INVISIBLE) {
    return (NO_ENTRY_FOUND);
  }


  config = FindMatchingItem(tag,visibility);

  if((helpMode == HELP) && (visibility == EXTERNAL) &&
    ((config == 0) || (config->helpProcessed == FALSE))) {
    if(config == 0) {
      returnCode = AddItemToList(tag, "", FALSE,FALSE);
      if(returnCode == UCI_NO_ERROR) {
         config = FindMatchingItem(tag,visibility);
        if (config) {
          config->statusInfo = UCI_NO_ERROR;
          config->helpProcessed = TRUE;
          config = 0;
        }
      }
    } else {
      config->helpProcessed = TRUE;
    }
    if (help) {
      fprintf(stderr,
	      "-%s string\t\t",
	      tag);
      fprintf(stderr,"%s\n", help);
    }
  }

  /*tag found ?*/
  if((config != 0)) {
/*     if((config->extendedHelpRequested == FALSE) && (helpMode == NO_HELP)) { */
      strncpy(value, config->value, stringLen-1);
      value[stringLen-1] = '\0';
      if(strlen(config->value) > (unsigned)stringLen) {
        returnCode = DEST_STRING_TOO_SHORT;
      }
/*     } */
    config->statusInfo = returnCode; /*store info on processing effort*/

    if(config->extendedHelpRequested == TRUE && extendedHelp) {
      /*copy extended help text to list*/
      config->extendedHelpText = (char*)ngsMalloc(strlen(extendedHelp)+1);
      strcpy(config->extendedHelpText, extendedHelp);
    }
  } else {
    returnCode = NO_ENTRY_FOUND;
  }
  return(returnCode);
}

/*****************************************************************************

    functionname: GetSwitch
    description:  checks existence of a parameter on commandline or in
                  config file
    returns:      Status info
    input:        parameter tag, help text, extended help text
    output:       flag signalling if parameter was given (!=0)

*****************************************************************************/
T_UCI_STATUS
GetSwitch(const char *tag, const char *help, const char *extendedHelp, int *switchActivated) {

  T_CONFIG_ENTRY *config;
  T_UCI_STATUS returnCode = UCI_NO_ERROR;
  int zero=0; /*needed only for check of number of arguments*/
  T_VISIBILITY visibility;

  *switchActivated = FALSE;

  visibility = CheckParameterVisibility(tag);
  if(visibility == INVISIBLE) {
    return(NO_ENTRY_FOUND);
  }

  config = FindMatchingItem(tag,visibility);

  if((helpMode == HELP) && (visibility == EXTERNAL) &&
    ((config == 0) || (config->helpProcessed != TRUE))) {
    if(config == 0) {
      returnCode = AddItemToList(tag, "", FALSE,FALSE);
      if(returnCode == UCI_NO_ERROR) {
        config = FindMatchingItem(tag,visibility);
        if (config) {
          config->statusInfo = UCI_NO_ERROR;
          config->helpProcessed = TRUE;
          config = 0;
        }
      }
    } else {
      config->helpProcessed = TRUE;
      config->addedOnlyforHelp=FALSE;
    }
    if (help) {
      fprintf(stderr,
	      "-%s\t\t\t",
	      tag);
      fprintf(stderr,"%s\n", help);
    }
  }

  /*tag found ?*/
  if((config != 0) && (config->addedOnlyforHelp==FALSE) ) {
    returnCode = CheckNumberOfArguments(config->value, &zero, tag);
    *switchActivated = TRUE;

    if(config->extendedHelpRequested == TRUE && extendedHelp) {
      /*copy extended help text to list*/
      config->extendedHelpText = (char*)ngsMalloc(strlen(extendedHelp)+1);
      strcpy(config->extendedHelpText, extendedHelp);
    }
    config->statusInfo = returnCode; /*store info on processing effort*/

  } else {
    returnCode = NO_ENTRY_FOUND;
  }
  return(returnCode);
}

/*****************************************************************************

    functionname: UciReset
    description:  release memory allocated by uci

*****************************************************************************/

void
UciReset(void)
{
  int i;
  if (allowedParameterList) {
    for(i=0; i<allowedListLength; i++) {
      ngsFree(allowedParameterList[i]);
    }
    ngsFree(allowedParameterList);
    allowedParameterList = 0;
    allowedListLength = 0; /*initial value*/
  }
  if (internallyVisibleParameterList) {
    for(i=0; i<intVisibleListLength; i++) {
      ngsFree(internallyVisibleParameterList[i]);
    }
    ngsFree(internallyVisibleParameterList);
    internallyVisibleParameterList = 0;
    intVisibleListLength = 0; /*initial value*/
  }
  if (configData) {
    for(i=0; i<cntEntries; i++) {
      if(configData[i].tag)   ngsFree(configData[i].tag);
      if(configData[i].value) ngsFree(configData[i].value);
      if(configData[i].extendedHelpText) ngsFree(configData[i].extendedHelpText);
    }
    ngsFree(configData);
    configData = 0;
  }
  cntEntries = -1; /*initial value*/
  currentArgc = -1; /*initial value*/
}

static int f_urs=0;
static int f_sw_visible=0;

T_UCI_STATUS
SetExpertFlags (int urs, int sw_visible) {

  T_UCI_STATUS status = UCI_NO_ERROR;

  f_urs=urs;
  f_sw_visible=sw_visible;
  return status;
}

/*****************************************************************************

    functionname: ExistencyCheck
    description:  responsible for output of extended help text, checks
                  whether parameters were given but never requested (illegal
                  parameter)
                  restores initial conditions on all static variables to facilitate
                  multiinstanciability
    returns:      Status info
    input:
    output:

*****************************************************************************/
T_UCI_STATUS
ExistencyCheck(void)
{
  int i;
  T_UCI_STATUS status = UCI_NO_ERROR;
  T_CONFIG_ENTRY *config;

  char infoOutput[INFOOUTPUTLEN];

  strcpy(infoOutput,"actually used settings: ");

  /*begin check at second element, because first element holds executable name.
    This may not be requested (would cause an error)*/
  for(i=1; i<cntEntries; i++) {

    char tmpstring[1024]="";

    /*output extended help where requested*/
    if(configData[i].extendedHelpRequested == TRUE) {
      if (configData[i].extendedHelpText) {
        fprintf(stderr, "Extended Help for commandline argument -%s:\n%s\n\n",
                configData[i].tag, configData[i].extendedHelpText);
        status = EXTENDED_HELP_PROCESSED;
      } else {
        fprintf(stderr,
                "No extended help available for commandline argument -%s\n",
                configData[i].tag);
      }
    }
    if(configData[i].statusInfo == PARAMETER_NOT_REQUESTED) {
      /*search for first parameter with the same tag*/
      config = FindMatchingItem(configData[i].tag,EXTERNAL);
      /*
         check whether status was caused by multiple appearance of parameter
         in configuration array
      */
      if(config->statusInfo == PARAMETER_NOT_REQUESTED) {
        fprintf(stderr,
                "***\nIllegal parameter in configuration, no request to %s \"%s\".\n***\n",
                configData[i].tag,configData[i].value);
        status = PARAMETER_NOT_REQUESTED;
      }
    }
    else
      if (strncmp(configData[i].tag,"urs",3)!=0 &&
          strncmp(configData[i].tag,"sw-visible",10)!=0 &&
          helpMode==NO_HELP){
        strcat(tmpstring,"-");
        strcat(tmpstring, configData[i].tag);
        strcat(tmpstring, " ");
        strcat(tmpstring, configData[i].value);
        strcat(tmpstring, " ");
        assert(strlen(infoOutput)+strlen(tmpstring)<INFOOUTPUTLEN);
        strcat(infoOutput,tmpstring);
      }
  }
  strcat(infoOutput,"\n");
  if (f_sw_visible && f_urs && helpMode==NO_HELP)
    fprintf(stderr, infoOutput);


  /*reset all UCI related information*/
  UciReset();

  if(helpMode != NO_HELP) {
    helpMode = NO_HELP;
    /*fprintf("Behaviour of UCI changed; exit should be done by calling application"); */
    return(UCI_NO_ERROR); /*should we do this or leave it to calling application???*/
  }

  return(status);
}


/*****************************************************************************

    functionname: GetUciHelpMode
    description:  Get help mode status.

    returns:      help mode status
    input:
    output:

*****************************************************************************/

T_UCI_STATUS
GetUciHelpMode(void)
{
   T_UCI_STATUS status = UCI_NO_ERROR;

   if(helpMode != NO_HELP) {
     status=HELP_MODE_ACTIVE;
   }
   return status;
}
