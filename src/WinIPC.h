

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0622 */
/* at Tue Jan 19 11:14:07 2038
 */
/* Compiler settings for WinIPC.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.01.0622 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __WinIPC_h__
#define __WinIPC_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IWinIPCEvent_FWD_DEFINED__
#define __IWinIPCEvent_FWD_DEFINED__
typedef interface IWinIPCEvent IWinIPCEvent;

#endif 	/* __IWinIPCEvent_FWD_DEFINED__ */


#ifndef __IWinIPCPeer_FWD_DEFINED__
#define __IWinIPCPeer_FWD_DEFINED__
typedef interface IWinIPCPeer IWinIPCPeer;

#endif 	/* __IWinIPCPeer_FWD_DEFINED__ */


#ifndef __WinIPCPeer_FWD_DEFINED__
#define __WinIPCPeer_FWD_DEFINED__

#ifdef __cplusplus
typedef class WinIPCPeer WinIPCPeer;
#else
typedef struct WinIPCPeer WinIPCPeer;
#endif /* __cplusplus */

#endif 	/* __WinIPCPeer_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_WinIPC_0000_0000 */
/* [local] */ 

typedef /* [public] */ 
enum __MIDL___MIDL_itf_WinIPC_0000_0000_0001
    {
        IPC_ET_SENDER	= 0,
        IPC_ET_RECEIVER	= ( IPC_ET_SENDER + 1 ) 
    } 	enumIPCEvenType;



extern RPC_IF_HANDLE __MIDL_itf_WinIPC_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_WinIPC_0000_0000_v0_0_s_ifspec;

#ifndef __IWinIPCEvent_INTERFACE_DEFINED__
#define __IWinIPCEvent_INTERFACE_DEFINED__

/* interface IWinIPCEvent */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_IWinIPCEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DCF17584-06B8-4d54-90E0-A0D8E545DFB0")
    IWinIPCEvent : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnConnect( 
            /* [in] */ VARIANT_BOOL vbIsSender,
            /* [in] */ ULONG lTargetID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnReceiverDisconnect( 
            /* [in] */ ULONG lTargetID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnSendFinish( 
            /* [in] */ ULONG lOffset,
            /* [in] */ ULONG lTargetID,
            /* [in] */ LONG lRet) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnReceiveData( 
            /* [in] */ ULONG lSenderID,
            /* [in] */ BYTE *lBuffer,
            /* [in] */ ULONG lSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnReceiveMsg( 
            /* [in] */ ULONG lSenderID,
            /* [in] */ LONG lParam1,
            /* [in] */ LONG lParam2) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IWinIPCEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWinIPCEvent * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWinIPCEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWinIPCEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnConnect )( 
            IWinIPCEvent * This,
            /* [in] */ VARIANT_BOOL vbIsSender,
            /* [in] */ ULONG lTargetID);
        
        HRESULT ( STDMETHODCALLTYPE *OnReceiverDisconnect )( 
            IWinIPCEvent * This,
            /* [in] */ ULONG lTargetID);
        
        HRESULT ( STDMETHODCALLTYPE *OnSendFinish )( 
            IWinIPCEvent * This,
            /* [in] */ ULONG lOffset,
            /* [in] */ ULONG lTargetID,
            /* [in] */ LONG lRet);
        
        HRESULT ( STDMETHODCALLTYPE *OnReceiveData )( 
            IWinIPCEvent * This,
            /* [in] */ ULONG lSenderID,
            /* [in] */ BYTE *lBuffer,
            /* [in] */ ULONG lSize);
        
        HRESULT ( STDMETHODCALLTYPE *OnReceiveMsg )( 
            IWinIPCEvent * This,
            /* [in] */ ULONG lSenderID,
            /* [in] */ LONG lParam1,
            /* [in] */ LONG lParam2);
        
        END_INTERFACE
    } IWinIPCEventVtbl;

    interface IWinIPCEvent
    {
        CONST_VTBL struct IWinIPCEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWinIPCEvent_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IWinIPCEvent_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IWinIPCEvent_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IWinIPCEvent_OnConnect(This,vbIsSender,lTargetID)	\
    ( (This)->lpVtbl -> OnConnect(This,vbIsSender,lTargetID) ) 

#define IWinIPCEvent_OnReceiverDisconnect(This,lTargetID)	\
    ( (This)->lpVtbl -> OnReceiverDisconnect(This,lTargetID) ) 

#define IWinIPCEvent_OnSendFinish(This,lOffset,lTargetID,lRet)	\
    ( (This)->lpVtbl -> OnSendFinish(This,lOffset,lTargetID,lRet) ) 

#define IWinIPCEvent_OnReceiveData(This,lSenderID,lBuffer,lSize)	\
    ( (This)->lpVtbl -> OnReceiveData(This,lSenderID,lBuffer,lSize) ) 

#define IWinIPCEvent_OnReceiveMsg(This,lSenderID,lParam1,lParam2)	\
    ( (This)->lpVtbl -> OnReceiveMsg(This,lSenderID,lParam1,lParam2) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IWinIPCEvent_INTERFACE_DEFINED__ */


#ifndef __IWinIPCPeer_INTERFACE_DEFINED__
#define __IWinIPCPeer_INTERFACE_DEFINED__

/* interface IWinIPCPeer */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IWinIPCPeer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("76DF9AC1-37B5-4BE7-B547-629D24070CAE")
    IWinIPCPeer : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_isSender( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_isReceiver( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_productID( 
            /* [retval][out] */ LONG *pVal) = 0;
        
        virtual /* [local][helpstring][id] */ HRESULT STDMETHODCALLTYPE AddEventHandler( 
            /* [in] */ IUnknown *pHandler) = 0;
        
        virtual /* [local][helpstring][id] */ HRESULT STDMETHODCALLTYPE RemoveEventHandler( 
            /* [in] */ IUnknown *pEventHandler) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddSenderProductID( 
            /* [in] */ LONG lProductID) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddReceiverProductID( 
            /* [in] */ LONG lProductID) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RemoveSenderProductID( 
            /* [in] */ LONG lProductID) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RemoveReceiverProductID( 
            /* [in] */ LONG lProductID) = 0;
        
        virtual /* [local][helpstring][id] */ HRESULT STDMETHODCALLTYPE SendData( 
            /* [in] */ BYTE *pBuffer,
            /* [in] */ ULONG ulSize,
            /* [in] */ VARIANT_BOOL vbSync) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ClosePeer( void) = 0;
        
        virtual /* [local][helpstring][id] */ HRESULT STDMETHODCALLTYPE SendDataToSpecialProduct( 
            /* [in] */ LONG lProductID,
            /* [in] */ BYTE *pBuffer,
            /* [in] */ ULONG ulSize,
            /* [in] */ VARIANT_BOOL vbSync) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsSenderConnected( 
            /* [in] */ LONG lProductID,
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsReceiverConnected( 
            /* [in] */ LONG lProductID,
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [local][helpstring][id] */ HRESULT STDMETHODCALLTYPE SendMsgToSpecialProduct( 
            /* [in] */ LONG lProductID,
            /* [in] */ LONG lParam1,
            /* [in] */ LONG lParam2) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IWinIPCPeerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWinIPCPeer * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWinIPCPeer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWinIPCPeer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWinIPCPeer * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWinIPCPeer * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWinIPCPeer * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWinIPCPeer * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_isSender )( 
            IWinIPCPeer * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_isReceiver )( 
            IWinIPCPeer * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_productID )( 
            IWinIPCPeer * This,
            /* [retval][out] */ LONG *pVal);
        
        /* [local][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *AddEventHandler )( 
            IWinIPCPeer * This,
            /* [in] */ IUnknown *pHandler);
        
        /* [local][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RemoveEventHandler )( 
            IWinIPCPeer * This,
            /* [in] */ IUnknown *pEventHandler);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *AddSenderProductID )( 
            IWinIPCPeer * This,
            /* [in] */ LONG lProductID);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *AddReceiverProductID )( 
            IWinIPCPeer * This,
            /* [in] */ LONG lProductID);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RemoveSenderProductID )( 
            IWinIPCPeer * This,
            /* [in] */ LONG lProductID);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RemoveReceiverProductID )( 
            IWinIPCPeer * This,
            /* [in] */ LONG lProductID);
        
        /* [local][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SendData )( 
            IWinIPCPeer * This,
            /* [in] */ BYTE *pBuffer,
            /* [in] */ ULONG ulSize,
            /* [in] */ VARIANT_BOOL vbSync);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ClosePeer )( 
            IWinIPCPeer * This);
        
        /* [local][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SendDataToSpecialProduct )( 
            IWinIPCPeer * This,
            /* [in] */ LONG lProductID,
            /* [in] */ BYTE *pBuffer,
            /* [in] */ ULONG ulSize,
            /* [in] */ VARIANT_BOOL vbSync);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsSenderConnected )( 
            IWinIPCPeer * This,
            /* [in] */ LONG lProductID,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsReceiverConnected )( 
            IWinIPCPeer * This,
            /* [in] */ LONG lProductID,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [local][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SendMsgToSpecialProduct )( 
            IWinIPCPeer * This,
            /* [in] */ LONG lProductID,
            /* [in] */ LONG lParam1,
            /* [in] */ LONG lParam2);
        
        END_INTERFACE
    } IWinIPCPeerVtbl;

    interface IWinIPCPeer
    {
        CONST_VTBL struct IWinIPCPeerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWinIPCPeer_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IWinIPCPeer_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IWinIPCPeer_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IWinIPCPeer_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IWinIPCPeer_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IWinIPCPeer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IWinIPCPeer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IWinIPCPeer_get_isSender(This,pVal)	\
    ( (This)->lpVtbl -> get_isSender(This,pVal) ) 

#define IWinIPCPeer_get_isReceiver(This,pVal)	\
    ( (This)->lpVtbl -> get_isReceiver(This,pVal) ) 

#define IWinIPCPeer_get_productID(This,pVal)	\
    ( (This)->lpVtbl -> get_productID(This,pVal) ) 

#define IWinIPCPeer_AddEventHandler(This,pHandler)	\
    ( (This)->lpVtbl -> AddEventHandler(This,pHandler) ) 

#define IWinIPCPeer_RemoveEventHandler(This,pEventHandler)	\
    ( (This)->lpVtbl -> RemoveEventHandler(This,pEventHandler) ) 

#define IWinIPCPeer_AddSenderProductID(This,lProductID)	\
    ( (This)->lpVtbl -> AddSenderProductID(This,lProductID) ) 

#define IWinIPCPeer_AddReceiverProductID(This,lProductID)	\
    ( (This)->lpVtbl -> AddReceiverProductID(This,lProductID) ) 

#define IWinIPCPeer_RemoveSenderProductID(This,lProductID)	\
    ( (This)->lpVtbl -> RemoveSenderProductID(This,lProductID) ) 

#define IWinIPCPeer_RemoveReceiverProductID(This,lProductID)	\
    ( (This)->lpVtbl -> RemoveReceiverProductID(This,lProductID) ) 

#define IWinIPCPeer_SendData(This,pBuffer,ulSize,vbSync)	\
    ( (This)->lpVtbl -> SendData(This,pBuffer,ulSize,vbSync) ) 

#define IWinIPCPeer_ClosePeer(This)	\
    ( (This)->lpVtbl -> ClosePeer(This) ) 

#define IWinIPCPeer_SendDataToSpecialProduct(This,lProductID,pBuffer,ulSize,vbSync)	\
    ( (This)->lpVtbl -> SendDataToSpecialProduct(This,lProductID,pBuffer,ulSize,vbSync) ) 

#define IWinIPCPeer_IsSenderConnected(This,lProductID,pVal)	\
    ( (This)->lpVtbl -> IsSenderConnected(This,lProductID,pVal) ) 

#define IWinIPCPeer_IsReceiverConnected(This,lProductID,pVal)	\
    ( (This)->lpVtbl -> IsReceiverConnected(This,lProductID,pVal) ) 

#define IWinIPCPeer_SendMsgToSpecialProduct(This,lProductID,lParam1,lParam2)	\
    ( (This)->lpVtbl -> SendMsgToSpecialProduct(This,lProductID,lParam1,lParam2) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IWinIPCPeer_INTERFACE_DEFINED__ */



#ifndef __WinIPCLib_LIBRARY_DEFINED__
#define __WinIPCLib_LIBRARY_DEFINED__

/* library WinIPCLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_WinIPCLib;

EXTERN_C const CLSID CLSID_WinIPCPeer;

#ifdef __cplusplus

class DECLSPEC_UUID("9C2B4EB1-F1E1-4E67-8BB8-12F0B18EC4F3")
WinIPCPeer;
#endif
#endif /* __WinIPCLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


