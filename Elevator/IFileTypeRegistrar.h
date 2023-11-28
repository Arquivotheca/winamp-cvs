

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Sat Apr 06 01:49:32 2013
 */
/* Compiler settings for .\IFileTypeRegistrar.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __IFileTypeRegistrar_h__
#define __IFileTypeRegistrar_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IFileTypeRegistrar_FWD_DEFINED__
#define __IFileTypeRegistrar_FWD_DEFINED__
typedef interface IFileTypeRegistrar IFileTypeRegistrar;
#endif 	/* __IFileTypeRegistrar_FWD_DEFINED__ */


#ifndef __WFileTypeRegistrar_FWD_DEFINED__
#define __WFileTypeRegistrar_FWD_DEFINED__

#ifdef __cplusplus
typedef class WFileTypeRegistrar WFileTypeRegistrar;
#else
typedef struct WFileTypeRegistrar WFileTypeRegistrar;
#endif /* __cplusplus */

#endif 	/* __WFileTypeRegistrar_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IFileTypeRegistrar_INTERFACE_DEFINED__
#define __IFileTypeRegistrar_INTERFACE_DEFINED__

/* interface IFileTypeRegistrar */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IFileTypeRegistrar;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2E74C695-8E9C-4179-B0A0-BC2EBDEB5C2B")
    IFileTypeRegistrar : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterMIMEType( 
            /* [string][in] */ LPCWSTR mimeType,
            /* [string][in] */ LPCWSTR programName,
            /* [string][in] */ LPCWSTR extension,
            /* [in] */ BOOL netscapeOnly) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterCDPlayer( 
            /* [string][in] */ LPCWSTR programName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterCDPlayer( 
            /* [string][in] */ LPCWSTR programName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterType( 
            /* [string][in] */ LPCWSTR extension,
            /* [string][in] */ LPCWSTR which_str,
            /* [string][in] */ LPCWSTR prog_name) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterType( 
            /* [string][in] */ LPCWSTR extension,
            /* [string][in] */ LPCWSTR which_str,
            /* [string][in] */ LPCWSTR prog_name,
            /* [in] */ int is_playlist) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddDirectoryContext( 
            /* [string][in] */ LPCWSTR commandLine,
            /* [string][in] */ LPCWSTR which_str,
            /* [string][in] */ LPCWSTR description) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveDirectoryContext( 
            /* [string][in] */ LPCWSTR which_str) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddAgent( 
            /* [string][in] */ LPCWSTR agentFilename) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveAgent( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterMediaPlayer( 
            /* [in] */ DWORD accessEnabled,
            /* [string][in] */ LPCWSTR programName,
            /* [string][in] */ LPCWSTR prog_name,
            /* [in] */ int iconNumber) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterMediaPlayerProtocol( 
            /* [string][in] */ LPCWSTR protocol,
            /* [string][in] */ LPCWSTR prog_name) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterMediaPlayerProtocol( 
            /* [string][in] */ LPCWSTR protocol,
            /* [string][in] */ LPCWSTR prog_name) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetupFileType( 
            /* [string][in] */ LPCWSTR programName,
            /* [string][in] */ LPCWSTR winamp_file,
            /* [string][in] */ LPCWSTR name,
            /* [in] */ int iconNumber,
            /* [string][in] */ LPCWSTR defaultShellCommand,
            /* [string][in] */ LPCWSTR iconPath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetupShell( 
            /* [string][in] */ LPCWSTR commandLine,
            /* [string][in] */ LPCWSTR winamp_file,
            /* [string][in] */ LPCWSTR description,
            /* [string][in] */ LPCWSTR commandName,
            /* [string][in] */ LPCWSTR dragAndDropGUID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveShell( 
            /* [string][in] */ LPCWSTR winamp_file,
            /* [string][in] */ LPCWSTR commandName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetupDefaultFileType( 
            /* [string][in] */ LPCWSTR winamp_file,
            /* [string][in] */ LPCWSTR defaultShellCommand) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterTypeShell( 
            /* [string][in] */ LPCWSTR programName,
            /* [string][in] */ LPCWSTR which_file,
            /* [string][in] */ LPCWSTR description,
            /* [in] */ int iconNumber,
            /* [string][in] */ LPCWSTR commandName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterGUID( 
            /* [string][in] */ LPCWSTR programName,
            /* [string][in] */ LPCWSTR guidString) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterDVDPlayer( 
            /* [string][in] */ LPCWSTR programName,
            /* [in] */ int iconNumber,
            /* [string][in] */ LPCWSTR which_file,
            /* [string][in] */ LPCWSTR commandName,
            /* [string][in] */ LPCWSTR provider,
            /* [string][in] */ LPCWSTR description) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InstallItem( 
            /* [string][in] */ LPCWSTR sourceFile,
            /* [string][in] */ LPCWSTR destinationFolder,
            /* [string][in] */ LPCWSTR destinationFilename) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteItem( 
            /* [string][in] */ LPCWSTR file) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RenameItem( 
            /* [string][in] */ LPCWSTR oldFile,
            /* [string][in] */ LPCWSTR newFile,
            /* [in] */ BOOL force) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CleanupDirectory( 
            /* [string][in] */ LPCWSTR directory) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MoveDirectoryContents( 
            /* [string][in] */ LPCWSTR oldDirectory,
            /* [string][in] */ LPCWSTR newDirectory) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WriteProKey( 
            /* [string][in] */ LPCWSTR name,
            /* [string][in] */ LPCWSTR key) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WriteClientUIDKey( 
            /* [string][in] */ LPCWSTR path,
            /* [string][in] */ LPCWSTR uid_str) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterProtocol( 
            /* [string][in] */ LPCWSTR protocol,
            /* [string][in] */ LPCWSTR command,
            /* [string][in] */ LPCWSTR icon) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterCapability( 
            /* [string][in] */ LPCWSTR programName,
            /* [string][in] */ LPCWSTR winamp_file,
            /* [string][in] */ LPCWSTR extension) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IFileTypeRegistrarVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFileTypeRegistrar * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFileTypeRegistrar * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFileTypeRegistrar * This);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterMIMEType )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR mimeType,
            /* [string][in] */ LPCWSTR programName,
            /* [string][in] */ LPCWSTR extension,
            /* [in] */ BOOL netscapeOnly);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterCDPlayer )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR programName);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterCDPlayer )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR programName);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterType )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR extension,
            /* [string][in] */ LPCWSTR which_str,
            /* [string][in] */ LPCWSTR prog_name);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterType )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR extension,
            /* [string][in] */ LPCWSTR which_str,
            /* [string][in] */ LPCWSTR prog_name,
            /* [in] */ int is_playlist);
        
        HRESULT ( STDMETHODCALLTYPE *AddDirectoryContext )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR commandLine,
            /* [string][in] */ LPCWSTR which_str,
            /* [string][in] */ LPCWSTR description);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveDirectoryContext )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR which_str);
        
        HRESULT ( STDMETHODCALLTYPE *AddAgent )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR agentFilename);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveAgent )( 
            IFileTypeRegistrar * This);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterMediaPlayer )( 
            IFileTypeRegistrar * This,
            /* [in] */ DWORD accessEnabled,
            /* [string][in] */ LPCWSTR programName,
            /* [string][in] */ LPCWSTR prog_name,
            /* [in] */ int iconNumber);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterMediaPlayerProtocol )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR protocol,
            /* [string][in] */ LPCWSTR prog_name);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterMediaPlayerProtocol )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR protocol,
            /* [string][in] */ LPCWSTR prog_name);
        
        HRESULT ( STDMETHODCALLTYPE *SetupFileType )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR programName,
            /* [string][in] */ LPCWSTR winamp_file,
            /* [string][in] */ LPCWSTR name,
            /* [in] */ int iconNumber,
            /* [string][in] */ LPCWSTR defaultShellCommand,
            /* [string][in] */ LPCWSTR iconPath);
        
        HRESULT ( STDMETHODCALLTYPE *SetupShell )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR commandLine,
            /* [string][in] */ LPCWSTR winamp_file,
            /* [string][in] */ LPCWSTR description,
            /* [string][in] */ LPCWSTR commandName,
            /* [string][in] */ LPCWSTR dragAndDropGUID);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveShell )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR winamp_file,
            /* [string][in] */ LPCWSTR commandName);
        
        HRESULT ( STDMETHODCALLTYPE *SetupDefaultFileType )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR winamp_file,
            /* [string][in] */ LPCWSTR defaultShellCommand);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterTypeShell )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR programName,
            /* [string][in] */ LPCWSTR which_file,
            /* [string][in] */ LPCWSTR description,
            /* [in] */ int iconNumber,
            /* [string][in] */ LPCWSTR commandName);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterGUID )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR programName,
            /* [string][in] */ LPCWSTR guidString);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterDVDPlayer )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR programName,
            /* [in] */ int iconNumber,
            /* [string][in] */ LPCWSTR which_file,
            /* [string][in] */ LPCWSTR commandName,
            /* [string][in] */ LPCWSTR provider,
            /* [string][in] */ LPCWSTR description);
        
        HRESULT ( STDMETHODCALLTYPE *InstallItem )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR sourceFile,
            /* [string][in] */ LPCWSTR destinationFolder,
            /* [string][in] */ LPCWSTR destinationFilename);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteItem )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR file);
        
        HRESULT ( STDMETHODCALLTYPE *RenameItem )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR oldFile,
            /* [string][in] */ LPCWSTR newFile,
            /* [in] */ BOOL force);
        
        HRESULT ( STDMETHODCALLTYPE *CleanupDirectory )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR directory);
        
        HRESULT ( STDMETHODCALLTYPE *MoveDirectoryContents )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR oldDirectory,
            /* [string][in] */ LPCWSTR newDirectory);
        
        HRESULT ( STDMETHODCALLTYPE *WriteProKey )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR name,
            /* [string][in] */ LPCWSTR key);
        
        HRESULT ( STDMETHODCALLTYPE *WriteClientUIDKey )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR path,
            /* [string][in] */ LPCWSTR uid_str);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterProtocol )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR protocol,
            /* [string][in] */ LPCWSTR command,
            /* [string][in] */ LPCWSTR icon);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterCapability )( 
            IFileTypeRegistrar * This,
            /* [string][in] */ LPCWSTR programName,
            /* [string][in] */ LPCWSTR winamp_file,
            /* [string][in] */ LPCWSTR extension);
        
        END_INTERFACE
    } IFileTypeRegistrarVtbl;

    interface IFileTypeRegistrar
    {
        CONST_VTBL struct IFileTypeRegistrarVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFileTypeRegistrar_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IFileTypeRegistrar_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IFileTypeRegistrar_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IFileTypeRegistrar_RegisterMIMEType(This,mimeType,programName,extension,netscapeOnly)	\
    ( (This)->lpVtbl -> RegisterMIMEType(This,mimeType,programName,extension,netscapeOnly) ) 

#define IFileTypeRegistrar_RegisterCDPlayer(This,programName)	\
    ( (This)->lpVtbl -> RegisterCDPlayer(This,programName) ) 

#define IFileTypeRegistrar_UnregisterCDPlayer(This,programName)	\
    ( (This)->lpVtbl -> UnregisterCDPlayer(This,programName) ) 

#define IFileTypeRegistrar_RegisterType(This,extension,which_str,prog_name)	\
    ( (This)->lpVtbl -> RegisterType(This,extension,which_str,prog_name) ) 

#define IFileTypeRegistrar_UnregisterType(This,extension,which_str,prog_name,is_playlist)	\
    ( (This)->lpVtbl -> UnregisterType(This,extension,which_str,prog_name,is_playlist) ) 

#define IFileTypeRegistrar_AddDirectoryContext(This,commandLine,which_str,description)	\
    ( (This)->lpVtbl -> AddDirectoryContext(This,commandLine,which_str,description) ) 

#define IFileTypeRegistrar_RemoveDirectoryContext(This,which_str)	\
    ( (This)->lpVtbl -> RemoveDirectoryContext(This,which_str) ) 

#define IFileTypeRegistrar_AddAgent(This,agentFilename)	\
    ( (This)->lpVtbl -> AddAgent(This,agentFilename) ) 

#define IFileTypeRegistrar_RemoveAgent(This)	\
    ( (This)->lpVtbl -> RemoveAgent(This) ) 

#define IFileTypeRegistrar_RegisterMediaPlayer(This,accessEnabled,programName,prog_name,iconNumber)	\
    ( (This)->lpVtbl -> RegisterMediaPlayer(This,accessEnabled,programName,prog_name,iconNumber) ) 

#define IFileTypeRegistrar_RegisterMediaPlayerProtocol(This,protocol,prog_name)	\
    ( (This)->lpVtbl -> RegisterMediaPlayerProtocol(This,protocol,prog_name) ) 

#define IFileTypeRegistrar_UnregisterMediaPlayerProtocol(This,protocol,prog_name)	\
    ( (This)->lpVtbl -> UnregisterMediaPlayerProtocol(This,protocol,prog_name) ) 

#define IFileTypeRegistrar_SetupFileType(This,programName,winamp_file,name,iconNumber,defaultShellCommand,iconPath)	\
    ( (This)->lpVtbl -> SetupFileType(This,programName,winamp_file,name,iconNumber,defaultShellCommand,iconPath) ) 

#define IFileTypeRegistrar_SetupShell(This,commandLine,winamp_file,description,commandName,dragAndDropGUID)	\
    ( (This)->lpVtbl -> SetupShell(This,commandLine,winamp_file,description,commandName,dragAndDropGUID) ) 

#define IFileTypeRegistrar_RemoveShell(This,winamp_file,commandName)	\
    ( (This)->lpVtbl -> RemoveShell(This,winamp_file,commandName) ) 

#define IFileTypeRegistrar_SetupDefaultFileType(This,winamp_file,defaultShellCommand)	\
    ( (This)->lpVtbl -> SetupDefaultFileType(This,winamp_file,defaultShellCommand) ) 

#define IFileTypeRegistrar_RegisterTypeShell(This,programName,which_file,description,iconNumber,commandName)	\
    ( (This)->lpVtbl -> RegisterTypeShell(This,programName,which_file,description,iconNumber,commandName) ) 

#define IFileTypeRegistrar_RegisterGUID(This,programName,guidString)	\
    ( (This)->lpVtbl -> RegisterGUID(This,programName,guidString) ) 

#define IFileTypeRegistrar_RegisterDVDPlayer(This,programName,iconNumber,which_file,commandName,provider,description)	\
    ( (This)->lpVtbl -> RegisterDVDPlayer(This,programName,iconNumber,which_file,commandName,provider,description) ) 

#define IFileTypeRegistrar_InstallItem(This,sourceFile,destinationFolder,destinationFilename)	\
    ( (This)->lpVtbl -> InstallItem(This,sourceFile,destinationFolder,destinationFilename) ) 

#define IFileTypeRegistrar_DeleteItem(This,file)	\
    ( (This)->lpVtbl -> DeleteItem(This,file) ) 

#define IFileTypeRegistrar_RenameItem(This,oldFile,newFile,force)	\
    ( (This)->lpVtbl -> RenameItem(This,oldFile,newFile,force) ) 

#define IFileTypeRegistrar_CleanupDirectory(This,directory)	\
    ( (This)->lpVtbl -> CleanupDirectory(This,directory) ) 

#define IFileTypeRegistrar_MoveDirectoryContents(This,oldDirectory,newDirectory)	\
    ( (This)->lpVtbl -> MoveDirectoryContents(This,oldDirectory,newDirectory) ) 

#define IFileTypeRegistrar_WriteProKey(This,name,key)	\
    ( (This)->lpVtbl -> WriteProKey(This,name,key) ) 

#define IFileTypeRegistrar_WriteClientUIDKey(This,path,uid_str)	\
    ( (This)->lpVtbl -> WriteClientUIDKey(This,path,uid_str) ) 

#define IFileTypeRegistrar_RegisterProtocol(This,protocol,command,icon)	\
    ( (This)->lpVtbl -> RegisterProtocol(This,protocol,command,icon) ) 

#define IFileTypeRegistrar_RegisterCapability(This,programName,winamp_file,extension)	\
    ( (This)->lpVtbl -> RegisterCapability(This,programName,winamp_file,extension) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IFileTypeRegistrar_INTERFACE_DEFINED__ */



#ifndef __ElevatorLib_LIBRARY_DEFINED__
#define __ElevatorLib_LIBRARY_DEFINED__

/* library ElevatorLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_ElevatorLib;

EXTERN_C const CLSID CLSID_WFileTypeRegistrar;

#ifdef __cplusplus

class DECLSPEC_UUID("3B29AB5C-52CB-4a36-9314-E3FEE0BA7468")
WFileTypeRegistrar;
#endif
#endif /* __ElevatorLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


