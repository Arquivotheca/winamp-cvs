/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under AppleÕs copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
          OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "npplat.h"

extern NPNetscapeFuncs NPNFuncs;

void NPN_Version(int* plugin_major, int* plugin_minor, int* netscape_major, int* netscape_minor)
{
  *plugin_major   = NP_VERSION_MAJOR;
  *plugin_minor   = NP_VERSION_MINOR;
  *netscape_major = HIBYTE(NPNFuncs.version);
  *netscape_minor = LOBYTE(NPNFuncs.version);
}

NPError NPN_GetURLNotify(NPP instance, const char *url, const char *target, void* notifyData)
{
	int navMinorVers = NPNFuncs.version & 0xFF;
  NPError rv = NPERR_NO_ERROR;

  if( navMinorVers >= NPVERS_HAS_NOTIFICATION )
		rv = CallNPN_GetURLNotifyProc(NPNFuncs.geturlnotify, instance, url, target, notifyData);
	else
		rv = NPERR_INCOMPATIBLE_VERSION_ERROR;

  return rv;
}

NPError NPN_GetURL(NPP instance, const char *url, const char *target)
{
  NPError rv = CallNPN_GetURLProc(NPNFuncs.geturl, instance, url, target);
  return rv;
}

NPError NPN_PostURLNotify(NPP instance, const char* url, const char* window, uint32 len, const char* buf, NPBool file, void* notifyData)
{
	int navMinorVers = NPNFuncs.version & 0xFF;
  NPError rv = NPERR_NO_ERROR;

	if( navMinorVers >= NPVERS_HAS_NOTIFICATION )
		rv = CallNPN_PostURLNotifyProc(NPNFuncs.posturlnotify, instance, url, window, len, buf, file, notifyData);
	else
		rv = NPERR_INCOMPATIBLE_VERSION_ERROR;

  return rv;
}

NPError NPN_PostURL(NPP instance, const char* url, const char* window, uint32 len, const char* buf, NPBool file)
{
  NPError rv = CallNPN_PostURLProc(NPNFuncs.posturl, instance, url, window, len, buf, file);
  return rv;
} 

NPError NPN_RequestRead(NPStream* stream, NPByteRange* rangeList)
{
  NPError rv = CallNPN_RequestReadProc(NPNFuncs.requestread, stream, rangeList);
  return rv;
}

NPError NPN_NewStream(NPP instance, NPMIMEType type, const char* target, NPStream** stream)
{
	int navMinorVersion = NPNFuncs.version & 0xFF;

  NPError rv = NPERR_NO_ERROR;

	if( navMinorVersion >= NPVERS_HAS_STREAMOUTPUT )
		rv = CallNPN_NewStreamProc(NPNFuncs.newstream, instance, type, target, stream);
	else
		rv = NPERR_INCOMPATIBLE_VERSION_ERROR;

  return rv;
}

int32 NPN_Write(NPP instance, NPStream *stream, int32 len, void *buffer)
{
	int navMinorVersion = NPNFuncs.version & 0xFF;
  int32 rv = 0;

  if( navMinorVersion >= NPVERS_HAS_STREAMOUTPUT )
		rv = CallNPN_WriteProc(NPNFuncs.write, instance, stream, len, buffer);
	else
		rv = -1;

  return rv;
}

NPError NPN_DestroyStream(NPP instance, NPStream* stream, NPError reason)
{
	int navMinorVersion = NPNFuncs.version & 0xFF;
  NPError rv = NPERR_NO_ERROR;

  if( navMinorVersion >= NPVERS_HAS_STREAMOUTPUT )
		rv = CallNPN_DestroyStreamProc(NPNFuncs.destroystream, instance, stream, reason);
	else
		rv = NPERR_INCOMPATIBLE_VERSION_ERROR;

  return rv;
}

void NPN_Status(NPP instance, const char *message)
{
  CallNPN_StatusProc(NPNFuncs.status, instance, message);
}

const char* NPN_UserAgent(NPP instance)
{
  const char * rv = NULL;
  rv = CallNPN_UserAgentProc(NPNFuncs.uagent, instance);
  return rv;
}

void* NPN_MemAlloc(uint32 size)
{
  void * rv = NULL;
  rv = CallNPN_MemAllocProc(NPNFuncs.memalloc, size);
  return rv;
}

void NPN_MemFree(void* ptr)
{
  CallNPN_MemFreeProc(NPNFuncs.memfree, ptr);
}

uint32 NPN_MemFlush(uint32 size)
{
  uint32 rv = CallNPN_MemFlushProc(NPNFuncs.memflush, size);
  return rv;
}

void NPN_ReloadPlugins(NPBool reloadPages)
{
  CallNPN_ReloadPluginsProc(NPNFuncs.reloadplugins, reloadPages);
}

#ifdef OJI
JRIEnv* NPN_GetJavaEnv(void)
{
  JRIEnv * rv = NULL;
	rv = CallNPN_GetJavaEnvProc(NPNFuncs.getJavaEnv);
  return rv;
}

jref NPN_GetJavaPeer(NPP instance)
{
  jref rv;
  rv = CallNPN_GetJavaPeerProc(NPNFuncs.getJavaPeer, instance);
  return rv;
}
#endif

NPError NPN_GetValue(NPP instance, NPNVariable variable, void *value)
{
  NPError rv = CallNPN_GetValueProc(NPNFuncs.getvalue, instance, variable, value);
  return rv;
}

NPError NPN_SetValue(NPP instance, NPPVariable variable, void *value)
{
  NPError rv = CallNPN_SetValueProc(NPNFuncs.setvalue, instance, variable, value);
  return rv;
}

void NPN_InvalidateRect(NPP instance, NPRect *invalidRect)
{
  CallNPN_InvalidateRectProc(NPNFuncs.invalidaterect, instance, invalidRect);
}

void NPN_InvalidateRegion(NPP instance, NPRegion invalidRegion)
{
  CallNPN_InvalidateRegionProc(NPNFuncs.invalidateregion, instance, invalidRegion);
}

void NPN_ForceRedraw(NPP instance)
{
  CallNPN_ForceRedrawProc(NPNFuncs.forceredraw, instance);
}

/* NPError		NPP_New(NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData* saved);
NPError		NPP_Destroy(NPP instance, NPSavedData** save);
NPError		NPP_SetWindow(NPP instance, NPWindow* window);
NPError		NPP_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype);
NPError		NPP_DestroyStream(NPP instance, NPStream* stream, NPReason reason);
int32		NPP_WriteReady(NPP instance, NPStream* stream);
int32		NPP_Write(NPP instance, NPStream* stream, int32 offset, int32 len, void* buffer);
void		NPP_StreamAsFile(NPP instance, NPStream* stream, const char* fname);
void		NPP_Print(NPP instance, NPPrint* platformPrint);
int16		NPP_HandleEvent(NPP instance, void* event);
void		NPP_URLNotify(NPP instance, const char* URL, NPReason reason, void* notifyData);
NPError		NPP_GetValue(NPP instance, NPPVariable variable, void *value);
NPError		NPP_SetValue(NPP instance, NPNVariable variable, void *value);
*/

NPError NP_Initialize(NPNetscapeFuncs* browserFuncs)
{
  //browser = browserFuncs;  
  return NPERR_NO_ERROR;
}

/*
NPError NP_Shutdown(void)
{

  return NPERR_NO_ERROR;
}
*/

#pragma export on
int main(NPNetscapeFuncs *browserFuncs, NPPluginFuncs *pluginFuncs, NPP_ShutdownUPP* unloadUpp);
#pragma export off

#ifdef __LITTLE_ENDIAN__

// For compatibility with CFM browsers.
int main(NPNetscapeFuncs *browserFuncs, NPPluginFuncs *pluginFuncs, NPP_ShutdownUPP* unloadUpp)
{
  int navMinorVers = browserFuncs->version & 0xFF;
  
  NPNFuncs.size = browserFuncs->size;
  NPNFuncs.version = browserFuncs->version;
  
  // Since this is a mach-o plug-in and the browser is CFM because it is calling main, translate
  // our function points into TVectors so the browser can call them.
  NPNFuncs.geturl = browserFuncs->geturl;
  NPNFuncs.posturl = browserFuncs->posturl;
  NPNFuncs.requestread = browserFuncs->requestread;
  NPNFuncs.newstream = browserFuncs->newstream;
  NPNFuncs.write = browserFuncs->write;
  NPNFuncs.destroystream = browserFuncs->destroystream;
  NPNFuncs.status = browserFuncs->status;
  NPNFuncs.uagent = browserFuncs->uagent;
  NPNFuncs.memalloc = browserFuncs->memalloc;
  NPNFuncs.memfree = browserFuncs->memfree;
  NPNFuncs.memflush = browserFuncs->memflush;
  NPNFuncs.reloadplugins = browserFuncs->reloadplugins;
  NPNFuncs.geturlnotify = browserFuncs->geturlnotify;
  NPNFuncs.posturlnotify = browserFuncs->posturlnotify;
  NPNFuncs.getvalue = browserFuncs->getvalue;
  NPNFuncs.setvalue = browserFuncs->setvalue;
  NPNFuncs.invalidaterect = browserFuncs->invalidaterect;
  NPNFuncs.invalidateregion = browserFuncs->invalidateregion;
  NPNFuncs.forceredraw = browserFuncs->forceredraw;
  NPNFuncs.getJavaEnv = browserFuncs->getJavaEnv;
  NPNFuncs.getJavaPeer = browserFuncs->getJavaPeer;
  /*     
    // These functions are not yet supported in CFM browers like Netscape.
    // In the future, the versions of the vectors should be checked before accessing symbols.
    NPNFuncs->releasevariantvalue = browserFuncs->releasevariantvalue;
  NPNFuncs->getstringidentifier = browserFuncs->getstringidentifier;
  NPNFuncs->getstringidentifiers = browserFuncs->getstringidentifiers;
  NPNFuncs->getintidentifier = browserFuncs->getintidentifier;
  NPNFuncs->identifierisstring = browserFuncs->identifierisstring;
  NPNFuncs->utf8fromidentifier = browserFuncs->utf8fromidentifier;
  NPNFuncs->createobject = browserFuncs->createobject;
  NPNFuncs->retainobject = browserFuncs->retainobject;
  NPNFuncs->releaseobject = browserFuncs->releaseobject;
  NPNFuncs->invoke = browserFuncs->invoke;
  NPNFuncs->evaluate = browserFuncs->evaluate;
  NPNFuncs->getproperty = browserFuncs->getproperty;
  NPNFuncs->setproperty = browserFuncs->setproperty;
  NPNFuncs->removeproperty = browserFuncs->removeproperty;
  NPNFuncs->setexception = browserFuncs->setexception;
  */
  
  pluginFuncs->version = 11;
  pluginFuncs->size = sizeof(pluginFuncs);
  pluginFuncs->newp = NPP_New;
  pluginFuncs->destroy = NPP_Destroy;
  pluginFuncs->setwindow = NPP_SetWindow;
  pluginFuncs->newstream = NPP_NewStream;
  pluginFuncs->destroystream = NPP_DestroyStream;
  pluginFuncs->asfile = NPP_StreamAsFile;
  pluginFuncs->writeready = NPP_WriteReady;
  pluginFuncs->write = NPP_Write;
  pluginFuncs->print = NPP_Print;
  pluginFuncs->event = NPP_HandleEvent;
  if( navMinorVers >= NPVERS_HAS_NOTIFICATION )
  {	
    pluginFuncs->urlnotify = NPP_URLNotify;
  }
  pluginFuncs->getvalue = NPP_GetValue;
  pluginFuncs->setvalue = NPP_SetValue;
  
  *unloadUpp = (void(*)()) NP_Shutdown;
  return NPERR_NO_ERROR;
}

#else
typedef void (* FunctionPointer) (void);
typedef void (* TransitionVector) (void);
static FunctionPointer functionPointerForTVector(TransitionVector);
static TransitionVector tVectorForFunctionPointer(FunctionPointer);

// glue for mapping outgoing Macho function pointers to TVectors
struct TFPtoTVGlue{
    void* glue[2];
};

struct {
    TFPtoTVGlue     newp;
    TFPtoTVGlue     destroy;
    TFPtoTVGlue     setwindow;
    TFPtoTVGlue     newstream;
    TFPtoTVGlue     destroystream;
    TFPtoTVGlue     asfile;
    TFPtoTVGlue     writeready;
    TFPtoTVGlue     write;
    TFPtoTVGlue     print;
    TFPtoTVGlue     event;
    TFPtoTVGlue     urlnotify;
    TFPtoTVGlue     getvalue;
    TFPtoTVGlue     setvalue;

    TFPtoTVGlue     shutdown;
} gPluginFuncsGlueTable;

static inline void* SetupFPtoTVGlue(TFPtoTVGlue* functionGlue, void* fp)
{
    functionGlue->glue[0] = fp;
    functionGlue->glue[1] = 0;
    return functionGlue;
}

// glue for mapping netscape TVectors to Macho function pointers
struct TTVtoFPGlue {
    uint32 glue[6];
};

struct {
    TTVtoFPGlue             geturl;
    TTVtoFPGlue             posturl;
    TTVtoFPGlue             requestread;
    TTVtoFPGlue             newstream;
    TTVtoFPGlue             write;
    TTVtoFPGlue             destroystream;
    TTVtoFPGlue             status;
    TTVtoFPGlue             uagent;
    TTVtoFPGlue             memalloc;
    TTVtoFPGlue             memfree;
    TTVtoFPGlue             memflush;
    TTVtoFPGlue             reloadplugins;
    TTVtoFPGlue             getJavaEnv;
    TTVtoFPGlue             getJavaPeer;
    TTVtoFPGlue             geturlnotify;
    TTVtoFPGlue             posturlnotify;
    TTVtoFPGlue             getvalue;
    TTVtoFPGlue             setvalue;
    TTVtoFPGlue             invalidaterect;
    TTVtoFPGlue             invalidateregion;
    TTVtoFPGlue             forceredraw;
} gNetscapeFuncsGlueTable;

static void* SetupTVtoFPGlue(TTVtoFPGlue* functionGlue, void* tvp)
{
    static const TTVtoFPGlue glueTemplate = { 0x3D800000, 0x618C0000, 0x800C0000, 0x804C0004, 0x7C0903A6, 0x4E800420 };

    memcpy(functionGlue, &glueTemplate, sizeof(TTVtoFPGlue));
    functionGlue->glue[0] |= ((UInt32)tvp >> 16);
    functionGlue->glue[1] |= ((UInt32)tvp & 0xFFFF);
    ::MakeDataExecutable(functionGlue, sizeof(TTVtoFPGlue));
    return functionGlue;
}

#define HOST_TO_PLUGIN_GLUE(name, fp) (SetupTVtoFPGlue(&gNetscapeFuncsGlueTable.name, (void*)fp))
#define PLUGIN_TO_HOST_GLUE(name, fp) (SetupFPtoTVGlue(&gPluginFuncsGlueTable.name, (void*)fp))

// For compatibility with CFM browsers.
int main(NPNetscapeFuncs *browserFuncs, NPPluginFuncs *pluginFuncs, NPP_ShutdownUPP* unloadUpp)
{
  // NPNFuncs = (NPNetscapeFuncs*)malloc(sizeof(NPNetscapeFuncs));
  
  int navMinorVers = browserFuncs->version & 0xFF;
  
  // bzero(NPNFuncs, sizeof(NPNetscapeFuncs));
  
  NPNFuncs.size = browserFuncs->size;
  NPNFuncs.version = browserFuncs->version;
  
  // Since this is a mach-o plug-in and the browser is CFM because it is calling main, translate
  // our function points into TVectors so the browser can call them.
  NPNFuncs.geturl = (NPN_GetURLUPP)HOST_TO_PLUGIN_GLUE(posturl, browserFuncs->geturl);
  NPNFuncs.posturl = (NPN_PostURLUPP)HOST_TO_PLUGIN_GLUE(geturl, browserFuncs->posturl);
  NPNFuncs.requestread = (NPN_RequestReadUPP)HOST_TO_PLUGIN_GLUE(requestread, browserFuncs->requestread);
  NPNFuncs.newstream = (NPN_NewStreamUPP)HOST_TO_PLUGIN_GLUE(newstream, browserFuncs->newstream);
  NPNFuncs.write = (NPN_WriteUPP)HOST_TO_PLUGIN_GLUE(write, browserFuncs->write);
  NPNFuncs.destroystream = (NPN_DestroyStreamUPP)HOST_TO_PLUGIN_GLUE(destroystream, browserFuncs->destroystream);
  NPNFuncs.status = (NPN_StatusUPP)HOST_TO_PLUGIN_GLUE(status, browserFuncs->status);
  NPNFuncs.uagent = (NPN_UserAgentUPP)HOST_TO_PLUGIN_GLUE(uagent, browserFuncs->uagent);
  NPNFuncs.memalloc = (NPN_MemAllocUPP)HOST_TO_PLUGIN_GLUE(memalloc, browserFuncs->memalloc);
  NPNFuncs.memfree = (NPN_MemFreeUPP)HOST_TO_PLUGIN_GLUE(memfree, browserFuncs->memfree);
  NPNFuncs.memflush = (NPN_MemFlushUPP)HOST_TO_PLUGIN_GLUE(memflush, browserFuncs->memflush);
  NPNFuncs.reloadplugins = (NPN_ReloadPluginsUPP)HOST_TO_PLUGIN_GLUE(reloadplugins, browserFuncs->reloadplugins);
  NPNFuncs.geturlnotify = (NPN_GetURLNotifyUPP)HOST_TO_PLUGIN_GLUE(geturlnotify, browserFuncs->geturlnotify);
  NPNFuncs.posturlnotify = (NPN_PostURLNotifyUPP)HOST_TO_PLUGIN_GLUE(posturlnotify, browserFuncs->posturlnotify);
  NPNFuncs.getvalue = (NPN_GetValueUPP)HOST_TO_PLUGIN_GLUE(getvalue, browserFuncs->getvalue);
  NPNFuncs.setvalue = (NPN_SetValueUPP)HOST_TO_PLUGIN_GLUE(setvalue, browserFuncs->setvalue);
  NPNFuncs.invalidaterect = (NPN_InvalidateRectUPP)HOST_TO_PLUGIN_GLUE(invalidaterect, browserFuncs->invalidaterect);
  NPNFuncs.invalidateregion = (NPN_InvalidateRegionUPP)HOST_TO_PLUGIN_GLUE(invalidateregion, browserFuncs->invalidateregion);
  NPNFuncs.forceredraw = (NPN_ForceRedrawUPP)HOST_TO_PLUGIN_GLUE(forceredraw, browserFuncs->forceredraw);
  NPNFuncs.getJavaEnv = (NPN_GetJavaEnvUPP)HOST_TO_PLUGIN_GLUE(getJavaEnv, browserFuncs->getJavaEnv);
  NPNFuncs.getJavaPeer = (NPN_GetJavaPeerUPP)HOST_TO_PLUGIN_GLUE(getJavaPeer, browserFuncs->getJavaPeer);
  /*     
    // These functions are not yet supported in CFM browers like Netscape.
    // In the future, the versions of the vectors should be checked before accessing symbols.
    NPNFuncs->releasevariantvalue = (NPN_ReleaseVariantValueProcPtr)functionPointerForTVector((TransitionVector)browserFuncs->releasevariantvalue);
  NPNFuncs->getstringidentifier = (NPN_GetStringIdentifierProcPtr)functionPointerForTVector((TransitionVector)browserFuncs->getstringidentifier);
  NPNFuncs->getstringidentifiers = (NPN_GetStringIdentifiersProcPtr)functionPointerForTVector((TransitionVector)browserFuncs->getstringidentifiers);
  NPNFuncs->getintidentifier = (NPN_GetIntIdentifierProcPtr)functionPointerForTVector((TransitionVector)browserFuncs->getintidentifier);
  NPNFuncs->identifierisstring = (NPN_IdentifierIsStringProcPtr)functionPointerForTVector((TransitionVector)browserFuncs->identifierisstring);
  NPNFuncs->utf8fromidentifier = (NPN_UTF8FromIdentifierProcPtr)functionPointerForTVector((TransitionVector)browserFuncs->utf8fromidentifier);
  NPNFuncs->createobject = (NPN_CreateObjectProcPtr)functionPointerForTVector((TransitionVector)browserFuncs->createobject);
  NPNFuncs->retainobject = (NPN_RetainObjectProcPtr)functionPointerForTVector((TransitionVector)browserFuncs->retainobject);
  NPNFuncs->releaseobject = (NPN_ReleaseObjectProcPtr)functionPointerForTVector((TransitionVector)browserFuncs->releaseobject);
  NPNFuncs->invoke = (NPN_InvokeProcPtr)functionPointerForTVector((TransitionVector)browserFuncs->invoke);
  NPNFuncs->evaluate = (NPN_EvaluateProcPtr)functionPointerForTVector((TransitionVector)browserFuncs->evaluate);
  NPNFuncs->getproperty = (NPN_GetPropertyProcPtr)functionPointerForTVector((TransitionVector)browserFuncs->getproperty);
  NPNFuncs->setproperty = (NPN_SetPropertyProcPtr)functionPointerForTVector((TransitionVector)browserFuncs->setproperty);
  NPNFuncs->removeproperty = (NPN_RemovePropertyProcPtr)functionPointerForTVector((TransitionVector)browserFuncs->removeproperty);
  NPNFuncs->setexception = (NPN_SetExceptionProcPtr)functionPointerForTVector((TransitionVector)browserFuncs->setexception);
  */
  
  pluginFuncs->version = 11;
  pluginFuncs->size = sizeof(pluginFuncs);
  pluginFuncs->newp = NewNPP_NewProc(PLUGIN_TO_HOST_GLUE(newp, NPP_New));
  pluginFuncs->destroy = NewNPP_DestroyProc(PLUGIN_TO_HOST_GLUE(destroy, NPP_Destroy));
  pluginFuncs->setwindow = NewNPP_SetWindowProc(PLUGIN_TO_HOST_GLUE(setwindow, NPP_SetWindow));
  pluginFuncs->newstream = NewNPP_NewStreamProc(PLUGIN_TO_HOST_GLUE(newstream, NPP_NewStream));
  pluginFuncs->destroystream = NewNPP_DestroyStreamProc(PLUGIN_TO_HOST_GLUE(destroystream, NPP_DestroyStream));
  pluginFuncs->asfile = NewNPP_StreamAsFileProc(PLUGIN_TO_HOST_GLUE(asfile, NPP_StreamAsFile));
  pluginFuncs->writeready = NewNPP_WriteReadyProc(PLUGIN_TO_HOST_GLUE(writeready, NPP_WriteReady));
  pluginFuncs->write = NewNPP_WriteProc(PLUGIN_TO_HOST_GLUE(write, NPP_Write));
  pluginFuncs->print = NewNPP_PrintProc(PLUGIN_TO_HOST_GLUE(print, NPP_Print));
  pluginFuncs->event = NewNPP_HandleEventProc(PLUGIN_TO_HOST_GLUE(event, NPP_HandleEvent));
  if( navMinorVers >= NPVERS_HAS_NOTIFICATION )
  {	
    pluginFuncs->urlnotify = NewNPP_URLNotifyProc(PLUGIN_TO_HOST_GLUE(urlnotify, NPP_URLNotify));
  }
  pluginFuncs->getvalue = NewNPP_GetValueProc(PLUGIN_TO_HOST_GLUE(getvalue, NPP_GetValue));
  pluginFuncs->setvalue = NewNPP_SetValueProc(PLUGIN_TO_HOST_GLUE(setvalue, NPP_SetValue));
  
  //*shutdown = (NPP_ShutdownProcPtr)tVectorForFunctionPointer((FunctionPointer)NP_Shutdown);
  *unloadUpp = NewNPP_ShutdownProc(PLUGIN_TO_HOST_GLUE(shutdown, NP_Shutdown));
  return NPERR_NO_ERROR;
}

// function pointer converters
#ifdef XP_MACOSX
FunctionPointer functionPointerForTVector(TransitionVector tvp)
{
    const uint32 temp[6] = {0x3D800000, 0x618C0000, 0x800C0000, 0x804C0004, 0x7C0903A6, 0x4E800420};
    uint32 *newGlue = NULL;
    
    if (tvp != NULL) {
        newGlue = (uint32 *)malloc(sizeof(temp));
        if (newGlue != NULL) {
            unsigned i;
            for (i = 0; i < 6; i++) newGlue[i] = temp[i];
            newGlue[0] |= ((UInt32)tvp >> 16);
            newGlue[1] |= ((UInt32)tvp & 0xFFFF);
            MakeDataExecutable(newGlue, sizeof(temp));
        }
    }
    
    return (FunctionPointer)newGlue;
}

TransitionVector tVectorForFunctionPointer(FunctionPointer fp)
{
    FunctionPointer *newGlue = NULL;
    if (fp != NULL) {
        newGlue = (FunctionPointer *)malloc(2 * sizeof(FunctionPointer));
        if (newGlue != NULL) {
            newGlue[0] = fp;
            newGlue[1] = NULL;
        }
    }
    return (TransitionVector)newGlue;
}
#endif
#endif

