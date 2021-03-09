// WinIPC.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "resource.h"
#include "WinIPC.h"
#include "WinIPCPeer.h"


class CWinIPCModule : public CAtlDllModuleT< CWinIPCModule >
{
public :
	DECLARE_LIBID(LIBID_WinIPCLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_WINIPC, "{367AC2EA-7275-42D2-BD5A-46890BCC5DAF}")
};

CWinIPCModule _AtlModule;


#ifdef _MANAGED
#pragma managed(push, off)
#endif

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	hInstance;
    return _AtlModule.DllMain(dwReason, lpReserved); 
}

#ifdef _MANAGED
#pragma managed(pop)
#endif




// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}


// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    HRESULT hr = _AtlModule.DllRegisterServer();
	return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
	return hr;
}


STDAPI CreateIPCPeer(INT nProductID, 
                     BOOL bAsSender,
                     BOOL bAsReceiver,
                     INT nAcceptSenderProductID,
                     INT nAcceptReceiverProductID,
                     BOOL bExitIfSameProductExist,
                     IWinIPCPeer** ppOut)
{
    _ATL_VALIDATE_OUT_POINTER(ppOut);

    CComObject<CWinIPCPeer>* spPeer = NULL;
    HRESULT hr = CComObject<CWinIPCPeer>::CreateInstance(&spPeer);

    if(FAILED(hr) || !spPeer)
    {
        return hr;
    }

    spPeer->AddAcceptReceiverProductID(nAcceptReceiverProductID);
    spPeer->AddAcceptSenderProductID(nAcceptSenderProductID);
    spPeer->Create(bAsSender, bAsReceiver, nProductID, FALSE, bExitIfSameProductExist);

    CComPtr<IWinIPCPeer> spIPCPeer = spPeer;
    *ppOut = spIPCPeer.Detach();
    return S_OK;
}

STDAPI CreateIPCPeerArray(INT nProductID, 
					      BOOL bAsSender,
						  BOOL bAsReceiver,
						  INT* nAcceptSenderProductID,
						  INT nSenderCount,
						  INT* nAcceptReceiverProductID,
						  INT nReceiverCount,
						  BOOL bExitIfSameProductExist,
						  IWinIPCPeer** ppOut)
{
	_ATL_VALIDATE_OUT_POINTER(ppOut);

	CComObject<CWinIPCPeer>* spPeer = NULL;
	HRESULT hr = CComObject<CWinIPCPeer>::CreateInstance(&spPeer);

	if(FAILED(hr) || !spPeer)
	{
		return hr;
	}

	INT i = 0;
	for(i = 0; i < nReceiverCount; i++)
	{
		spPeer->AddAcceptReceiverProductID(nAcceptReceiverProductID[i]);
	}

	for(i = 0; i < nSenderCount; i++)
	{
		spPeer->AddAcceptSenderProductID(nAcceptSenderProductID[i]);
	}
	
	spPeer->Create(bAsSender, bAsReceiver, nProductID, FALSE, bExitIfSameProductExist);

	CComPtr<IWinIPCPeer> spIPCPeer = spPeer;
	*ppOut = spIPCPeer.Detach();
	return S_OK;
}



