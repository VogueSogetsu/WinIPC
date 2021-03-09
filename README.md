# WinIPC

WinIPC是一个Windows下的C++进程通信库  
1. 基于Windows共享内存API
2. 使用类似TCP/IP间三次握手协议，进程启动后自动完成与对方进程的适配，调用者之需要关注发送的数据即可
3. 项目构建为一个COM组件，为方便在非COM程序中使用，使用IPCPeerImpl2进行包装
4. 进程可以实现为发送方，也可以实现为接收方，或者既发送又接收，发送方和接收方可以进行多对多配对
5. 发送数据可以以广播的形式，所以配对的接收方都可以收到，也可以指定向某一个接收方发送数据

_依赖：_  
ATL/WTL  
log4cplus（如果需要打印日志）
****

# 使用示例

参考PeerDemo项目  
PeerMonitor项目可以打印当前参与通信的进程及其id  

**初始化：**  
```
CComPtr<IUnknown> spUkn;
std::vector vecTargetIDs;
vecTargetIDs.push_back(2000);
CreateIPCPeer(1000, vecTargetIDs, &spUkn);

CComQIPtr<IWinIPCPeer> spIPCPeer = spUkn;
```

**发送数据**
```
CStringA saText = "abcdefg";
// broadcast
spIPCPeer->SendData((LPBYTE)saText.GetBuffer(), saText.GetLength(), VARIANT_FALSE);

// or send to special product
//spIPCPeer->SendDataToSpecialProduct((LPBYTE)saText.GetBuffer(), saText.GetLength(), VARIANT_FALSE);
```

**接收数据**  
接收数据需要实现IWinIPCEvent
非COM程序可以继承包装类CIPCEvent
```
HRESULT STDMETHODCALLTYPE CMainDlg::OnReceiveData(  /* [in] */ ULONG lSenderID, /* [in] */ BYTE * lBuffer, /* [in] */ ULONG lSize )
{
	// do something
	return S_OK;
}
```