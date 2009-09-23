#include ".\npeermng.h"
#include <process.h>    /* _beginthread, _endthread */
#include <iostream>
#include "../win/TPeerProtocol.h"
#include "../src/debug.h"
#include "ThreadConnnectTo.h"
#include "jmutexautolock.h"

using namespace UDT;

#pragma warning(disable:4100)

//////////////////////////////////////////////////////////////////////////
TPeerMng::TPeerMng(unsigned int iMyId, const sockaddr* name, int namelen)  //系统唯一的节点id和入口服务器地址
{
	m_pPeerMngImpl = new NPeerMng(this,iMyId,name,namelen);
}
TPeerMng::~TPeerMng()
{
	delete m_pPeerMngImpl;
}

//自己的id，在开始的时候可以是0。注意：ID只能更新一次
bool TPeerMng::UpdateId(unsigned int iMyId)
{
	return m_pPeerMngImpl->UpdateId(iMyId);
}
unsigned int TPeerMng::GetId()
{
	return m_pPeerMngImpl->GetId();
}


//与服务器建立连接
void TPeerMng::ConnectToServer(unsigned int uiServerID, const sockaddr* name, int namelen)
{
	m_pPeerMngImpl->ConnectToServer(uiServerID,name,namelen);
}
//关闭与服务器的连接（节省服务器资源）
void TPeerMng::CloseServerConnection()
{
	m_pPeerMngImpl->CloseServerConnection();
}
//与服务器连接的状态
bool TPeerMng::IsServerConnectReady()
{
	return m_pPeerMngImpl->IsServerConnectReady();
}

//====多种连接方式====
//通过STUN服务器查询地址连接
bool  TPeerMng::PeerConnect(unsigned int iPeerId,int iWaitTime)
{
	return m_pPeerMngImpl->PeerConnect(iPeerId,iWaitTime);
}
//知道地址，直接连接
bool  TPeerMng::PeerConnect(unsigned int iPeerId,const sockaddr* nameKnown, int namelen,bool bTrySTUN)
{
	return m_pPeerMngImpl->PeerConnect(iPeerId,nameKnown, namelen,bTrySTUN);
}
//知道地址和辅助用户id，直接连接
bool  TPeerMng::PeerConnect(unsigned int iPeerId,const sockaddr* nameKnown, int namelen,unsigned int iHelperId,bool bTrySTUN)
{
	return m_pPeerMngImpl->PeerConnect(iPeerId,nameKnown, namelen,iHelperId,bTrySTUN);
}

//用户对象是否已创建（不管有没有连接上）
bool  TPeerMng::IsPeerExist(unsigned int iPeerId)
{
	return m_pPeerMngImpl->IsPeerExist(iPeerId);
}
//用户是否已经连接成功
bool  TPeerMng::IsPeerReady(unsigned int iPeerId)
{
	return m_pPeerMngImpl->IsPeerReady(iPeerId);
}

//获得已连接用户列表
void TPeerMng::GetPeerSet(std::set<unsigned int> &setPeers)
{
	m_pPeerMngImpl->GetPeerSet(setPeers);
}

//根据用户ID查找用户
TPeerPtr TPeerMng::FindPeer(unsigned int iPeerId)
{
	return m_pPeerMngImpl->FindPeer(iPeerId);
}
//获取用户地址。用户有多个连接的，返回当前用户地址
bool TPeerMng::GetPeerAddress( unsigned int iPeerId,sockaddr* name, int *namelen )
{
	return m_pPeerMngImpl->GetPeerAddress(iPeerId,name,namelen);
}
////同上。如果用户没有连接，返回空地址
//KIpv4Addr TPeerMng::GetPeerAddress(unsigned int iPeerId)
//{
//	return m_pPeerMngImpl->GetPeerAddress(iPeerId);
//}

//给用户发送用户数据（此函数效率较低，提倡用SendPeerPack）
int TPeerMng::SendPeerMsg( unsigned int iPeerId,const char *pData, int iLen )
{
	return m_pPeerMngImpl->SendPeerMsg(iPeerId,pData,iLen);
}
//给用户发送TBasePack数据包
int TPeerMng::SendPeerPack( unsigned int iPeerId,const TBasePack *pPack, int iLen )
{
	return m_pPeerMngImpl->SendPeerPack(iPeerId,pPack,iLen);
}

//获得用户对象实例
//用户如果存在就返回，如果不存在就调用PeerFactory函数生成一个
TPeerPtr TPeerMng::GetPeerInstance(int iPeerId,int iTemplateType)
{
	return m_pPeerMngImpl->GetPeerInstance(iPeerId,iTemplateType);
}
//关闭连接。此函数关闭所有相关网络连接，释放对应智能指针
void TPeerMng::ClosePeer(unsigned int iPeerId)
{
	return m_pPeerMngImpl->ClosePeer(iPeerId);
}
//用户对象工厂函数：仅供重载使用
//TPeerPtr TPeerMng::PeerFactory(int iPeerId,int iTemplateType)
//{
//	return TPeerPtr(new TPeer(this,iPeerId));
//}
//全局消息侦听器
void TPeerMng::AddPeerListener(IPeerDataListener *pListener)
{
	m_pPeerMngImpl->AddPeerListener(pListener);
}
bool TPeerMng::RemovePeerListener(const IPeerDataListener *pListener)
{
	return m_pPeerMngImpl->RemovePeerListener(pListener);
}
//////////////////////////////////////////////////////////////////////////

NPeerMng::NPeerMng(TPeerMng *pParentPeerMng,unsigned int iMyId, const sockaddr* name, int namelen)  //系统唯一的节点id和入口服务器地址
:m_Network(name,namelen),m_iMyId(iMyId),m_pParentPeerMng(pParentPeerMng)
{
	m_Network.SetNetworkListener(this);
}
NPeerMng::~NPeerMng()
{
	m_Network.SetNetworkListener(NULL);
}

//自己的id，在开始的时候可以是0。注意：ID只能更新一次
bool NPeerMng::UpdateId(unsigned int iMyId)
{
	if (m_iMyId!=INVALID_PEER_ID)
	{
		ASSERT(FALSE);
		return false;
	}
	m_iMyId = iMyId;
	return true;
}
unsigned int NPeerMng::GetId()
{
	return m_iMyId;
}

//与服务器建立连接
void NPeerMng::ConnectToServer(unsigned int uiServerID, const sockaddr* name, int namelen)
{
	m_iServerPeer = uiServerID;

	ConnectToEx(name,namelen);
}
//关闭与服务器的连接（节省服务器资源）
void NPeerMng::CloseServerConnection()
{
	ClosePeer(m_iServerPeer);

	m_iServerPeer = INVALID_PEER_ID;
}
bool NPeerMng::IsServerConnectReady()
{
	return IsPeerReady(m_iServerPeer);
}

//回调函数
TPeerPtr NPeerMng::PeerFactory(int iPeerId,int iTemplateType)      //有新连接上来
{
	return m_pParentPeerMng->PeerFactory(iPeerId,iTemplateType);
}


TPeerPtr NPeerMng::GetPeerInstance(int iPeerId,int iTemplateType)
{
	//如果已经有实例了，直接使用它
	TPeerPtr ptrPeer = FindPeer(iPeerId);
	if (ptrPeer)
	{
		return ptrPeer;
	}

	//没有实例，创建一个
	ptrPeer = PeerFactory(iPeerId,iTemplateType);

	//把peer放入到管理集合里
	if(!AddPeer(iPeerId,ptrPeer))
	{
		ASSERT(FALSE);
	}

	return ptrPeer;
}

void NPeerMng::OnConnectionMsg( int iConn,enNetworkCode code,int param )
{
	switch(code)
	{
	case NL_CODE_NEWCONN:    //连接建立
		{
			if(INVALID_PEER_ID!=Socket2Peer(iConn)) //UDT连接建立前，不应当有TPeer存在
			{
				ASSERT(FALSE);
				return;
			}

			//////////////////////////////////////////////////////////////////////////
			//新连接上来，发送ID号
			//////////////////////////////////////////////////////////////////////////

			sockaddr saPeer,saSend;  //本地ip和peer端口号联合起来
			int len;
			UDT::getsockname(iConn,&saPeer,&len);
			GetServAddress(&saSend,len);
			((SOCKADDR_IN *)&saSend)->sin_port = ((SOCKADDR_IN *)&saPeer)->sin_port;

			TBasePack *pPack = TPackSendId::Factory(m_iMyId,KIpv4Addr(*(SOCKADDR_IN *)&saSend));
			int iRes = UDT::sendmsg(iConn,(char *)pPack,TPackSendId::GetTotalLen());
			ASSERT(TPackSendId::GetTotalLen()==iRes);
			delete[] pPack;
		}
		break;
	case NL_CODE_BREAKDOWN:  //连接中断
		{
			//连接断开，首先找到对应用户，检查是不是该用户的当前连接
			//如果是，那么有没有备用连接可用，没有就发送连接断开消息
			unsigned int id = Socket2Peer(iConn); //这一步必须在RemoveSockID.erase之前
			RemoveSockID(iConn);

			if (INVALID_PEER_ID==id)
			{
				std::cout<<"警告：连接中断，但找不到连接对应的peer_id"<<std::endl;
				return;
			}
			//找到对应的TPeer
			TPeerPtr ptrPeer = FindPeer(id);
			if (!ptrPeer)
			{
				std::cout<<"警告：连接中断，但找不到连接对应的TPeer。id="<<id<<std::endl;
				return;
			}
			//是不是当前连接
			//如果不是当前连接，断开就断开了，不用管它
			if (ptrPeer->GetCurrentSocket()==iConn) 
			{
				//注意如果找不到可用socket，自动传INVALID_SOCKET进去。
				ptrPeer->SetCurrentSocket(FindUserSocket(id)); 
			}

		}
		break;
	}
	
}

void NPeerMng::OnData( int iConn,int iLen, const char *pData )
{
	TBasePack *pBasePack = (TBasePack *)pData;
	std::cout<<"OnPack:"<<pBasePack->GetPackType()<<std::endl;

	//合法性检查
	if (pBasePack->flag!=TBasePack::PEERPACK_FLAG)
	{
		ASSERT(FALSE); //数据包不合法，删除之。注意稍候会自动得到连接断开消息
		UDT::close(iConn);
		return;
	}

	//当数据到达时，看用户有没有通过身份认证
	//已经通过了的，直接转发数据；
	//没有通过的，则检查身份数据包，不合法的立即断开连接
	//已通过认证
	unsigned int id = Socket2Peer(iConn);

	if (INVALID_PEER_ID==id) //没有通过认证
	{
		//此时，第一个包必须是发送ID包，否则不符合协议规范
		if (pBasePack->pack_type!=TBasePack::PT_SENDID)
		{
			ASSERT(FALSE); //数据包不合法，删除之。注意稍候会自动得到连接断开消息
			UDT::close(iConn);
			return;
		}
		TPackSendId *pSendID = (TPackSendId *)pBasePack->data;

		//该用户有没有对应的连接对象存在？没有则创建一个
		TPeerPtr ptrPeer = GetPeerInstance(pSendID->uiUserId,0);
		if (!ptrPeer)
		{
			UDT::close(iConn); //GetPeerInstance如果返回NULL，说明不接受新的连接
			return;
		}
		//把这个新的连接交给peer使用
		ptrPeer->SetCurrentSocket(iConn);

		//更新用户的内网地址
		pSendID->addr.GetAddr(ptrPeer->m_uiLocalIp,ptrPeer->m_usLocalPort);

		//设置连接-->peer映射关系
		AddSockID(iConn,pSendID->uiUserId);

		std::cout<<iConn<<"新连接的本地地址："<<pSendID->addr.ToString()<<std::endl;

		//测试：读取自己的映射地址
		//PeerSendGetAddrPack(pSendID->uiUserId,m_iMyId);
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	//连接已经建立的情况
	switch(pBasePack->pack_type)
	{
	case TBasePack::PT_KEEPLIVE:
		{
			//m_dwLastKeepLive = GetTickCount();
			ASSERT(FALSE);
			return;
		}
	case TBasePack::PT_DATA:
		{
			//找到对应的TPeer
			TPeerPtr ptrPeer = FindPeer(id);
			if (!ptrPeer)
			{
				return;
			}
			ptrPeer->SetCurrentSocket(iConn); //更新一下当前socket
			ptrPeer->OnPeerData((char *)pBasePack->data,iLen-sizeof(TBasePack));

			{
				JMutexAutoLock auto_lock(m_setPeerListenerLock);
				T_setListener::iterator it = m_setPeerListener.begin();
				for (;it!=m_setPeerListener.end();it++)
				{
					UDT::IPeerDataListener *pListener = *it;
					pListener->OnPeerData(id,(char *)pBasePack->data,iLen-sizeof(TBasePack));
				}
			}
			return;
		}
	case TBasePack::PT_ADDR_GETMAP: //查询peer的地址
		{
			TPackGetAddr *pGetAddr = (TPackGetAddr *)pBasePack->data;
			if (INVALID_PEER_ID==pGetAddr->uiPeerID) //peer id不能为0
			{
				ASSERT(FALSE);
				return;
			}
			KIpv4Addr addr = GetPeerAddress(pGetAddr->uiPeerID);

			//发送地址数据包
			TBasePack *pPack = TPackAddrReply::Factory(pGetAddr->uiPeerID,addr);
			int iRes = UDT::sendmsg(iConn,(char *)pPack,TPackAddrReply::GetTotalLen());
			ASSERT(TPackAddrReply::GetTotalLen()==iRes);
			delete[] pPack;
			return;
		}
	case TBasePack::PT_ADDR_REPLY:
		{
			TPackAddrReply *pAddrReply = (TPackAddrReply *)pBasePack->data;
			std::cout<<pAddrReply->uiPeerID<<" 的映射地址："<<pAddrReply->addrMap.ToString()<<std::endl;
			//if(pAddrReply->uiPeerID==GetId())
			//{
			//	std::cout<<"映射地址："<<pAddrReply->addrMap.ToString()<<std::endl;
			//}
			//else
			//{
			//	std::cout<<"准备连接地址："<<pAddrReply->addrMap.ToString()<<std::endl;
			//	OnAddressReturn(pData,iLen);
			//}
			return;
		}

	case TBasePack::PT_STUN_CONNECT:
		{
			//收到辅助连接请求，那么向双发发送shoot命令。
			TPackConnect *pConnectHelp = (TPackConnect *)pBasePack->data;
			unsigned int iPeerId = Socket2Peer(iConn);

			ASSERT(pConnectHelp->uiPeerID!=INVALID_PEER_ID);
			ASSERT(pConnectHelp->uiPeerID!=iPeerId);
			ASSERT(iPeerId!=INVALID_PEER_ID);

			PeerSendShootCmd(pConnectHelp->uiPeerID,iPeerId);
			PeerSendShootCmd(iPeerId,pConnectHelp->uiPeerID);
			return;
		}
	case TBasePack::PT_STUN_SHOOT:
		{
			//收到打洞命令
			TPackShoot *pShoot = (TPackShoot *)pBasePack->data;

			ASSERT(pShoot->uiPeerID!=INVALID_PEER_ID);
			ASSERT(pShoot->uiPeerID!=m_iMyId);
			ASSERT(pShoot->uiPeerID!=Socket2Peer(iConn));

			//如果有连接者，那么命令它去连接
			TPeerPtr pPeer = GetPeerInstance(pShoot->uiPeerID,0);
			if (pPeer)
			{
				OnShootCmd(pData,iLen);
			}
			return;
		}
	case TBasePack::PT_SENDID:return;
	}

}
//====多种连接方式====
//通过STUN服务器查询地址连接
bool  NPeerMng::PeerConnect(unsigned int iPeerId,int iWaitTime)
{
	if (!IsPeerReady(m_iServerPeer)) //服务器有没有连接？
	{
		return false;
	}
	if(IsPeerReady(iPeerId))         //目标已经连接？
	{
		return false;
	}

	//向服务器发送查询地址命令。查询到后会自动回调OnAddressReturn函数
	if(!PeerSendConnectHelp(m_iServerPeer,iPeerId))
	{
		return false;
	}

	//等待指定的时间，看连接成功否
	DWORD dwBgn = GetTickCount();
	while (!IsPeerReady(iPeerId) && GetTickCount()<(dwBgn+iWaitTime))
	{
		Sleep(10);
	}
	return IsPeerReady(iPeerId);
}
//知道地址，直接连接
bool  NPeerMng::PeerConnect(unsigned int iPeerId,const sockaddr* nameKnown, int namelen,bool bTrySTUN)
{
	return ConnectToEx(nameKnown,namelen);
}
//知道地址和辅助用户id，直接连接
bool  NPeerMng::PeerConnect(unsigned int iPeerId,const sockaddr* nameKnown, int namelen,unsigned int iHelperId,bool bTrySTUN)
{
	ASSERT(FALSE);//还没实现
	return false;
}


void NPeerMng::GetPeerSet(std::set<unsigned int> &setPeers)
{
	JMutexAutoLock autolock(m_IDPeerLock);

	T_mapIDPeer::iterator it = m_mapIDPeer.begin();
	for(;it!=m_mapIDPeer.end();it++)
	{
		setPeers.insert((*it).first);
	}
}
bool NPeerMng::AddPeer(unsigned int iPeerId,TPeerPtr pPeer)
{
	JMutexAutoLock autolock(m_IDPeerLock);

	if (m_mapIDPeer.find(iPeerId)!=m_mapIDPeer.end())
	{
		return false;
	}
	m_mapIDPeer[iPeerId] = pPeer;
	return true;
}

UDTSOCKET NPeerMng::FindUserSocket( unsigned int iUserId )
{
	JMutexAutoLock autolock(m_SockIDLock);

	T_mapSockID::iterator it = m_mapSockID.begin();
	for(;it!=m_mapSockID.end();it++)
	{
		if (iUserId==(*it).second)
		{
			return (*it).first;
		}
	}
	return UDT::INVALID_SOCK;
}

bool NPeerMng::GetPeerAddress( unsigned int iPeerId,sockaddr* name, int *namelen )
{
	TPeerPtr pPeer = FindPeer(iPeerId);
	if (!pPeer)
	{
		return false;
	}
	UDTSOCKET u = pPeer->GetCurrentSocket();
	if (u==UDT::INVALID_SOCK)
	{
		return false;
	}
	return 0==UDT::getpeername(u,name,namelen);
}
KIpv4Addr NPeerMng::GetPeerAddress(unsigned int iPeerId)
{
	sockaddr sa;
	int len;
	if (!GetPeerAddress(iPeerId,&sa,&len))
	{
		return KIpv4Addr();
	}
	return KIpv4Addr(*(SOCKADDR_IN *)&sa);
}

int NPeerMng::SendPeerMsg( unsigned int iPeerId,const char *pData, int iLen )
{
	TPeerPtr pPeer = FindPeer(iPeerId);
	if (!pPeer)
	{
		return -1;
	}
	return pPeer->SendPeerMsg(pData,iLen);
}
//给用户发送TBasePack数据包
int NPeerMng::SendPeerPack( unsigned int iPeerId,const TBasePack *pPack, int iLen )
{
	TPeerPtr pPeer = FindPeer(iPeerId);
	if (!pPeer)
	{
		return -1;
	}
	return pPeer->SendPack(pPack,iLen);
}
TPeerPtr NPeerMng::FindPeer(unsigned int iPeerId)
{
	ASSERT(INVALID_PEER_ID!=iPeerId);

	JMutexAutoLock autolock(m_IDPeerLock);

	T_mapIDPeer::iterator it = m_mapIDPeer.find(iPeerId);
	if (it==m_mapIDPeer.end())
	{
		return TPeerPtr();
	}
	return (*it).second;
}
unsigned int NPeerMng::Socket2Peer(UDTSOCKET u)
{
	JMutexAutoLock autolock(m_SockIDLock);

	T_mapSockID::iterator it = m_mapSockID.find(u);
	if (it==m_mapSockID.end())
	{
		return INVALID_PEER_ID;
	}
	return (*it).second;
}


bool NPeerMng::PeerSendShootCmd(unsigned int peer1,unsigned int peer2)
{
	std::cout<<"send shoot cmd:"<<peer1<<"-->"<<peer2<<std::endl;

	//////////////////////////////////////////////////////////////////////////
	//peer1如果不存在也就不用继续了
	TPeerPtr pPeer1 = FindPeer(peer1);
	if (!pPeer1)
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	//获得目标地址
	KIpv4Addr addrMap = GetPeerAddress(peer2);
	KIpv4Addr addrLocal;
	TPeerPtr pPeer2 = FindPeer(peer2);
	if (pPeer2)
	{
		addrLocal = KIpv4Addr(pPeer2->m_uiLocalIp,pPeer2->m_usLocalPort);
	}

	//////////////////////////////////////////////////////////////////////////
	//发送id数据包
	TBasePack *pPack = TPackShoot::Factory(peer2,addrMap,addrLocal);
	int iRes = pPeer1->SendPack(pPack,TPackShoot::GetTotalLen());
	delete[] pPack;
	return TPackShoot::GetTotalLen()==iRes;
}

bool NPeerMng::PeerSendGetAddrPack(unsigned int iServerPeer,unsigned int peer_id)
{
	//发送id数据包
	TBasePack *pPack = TPackGetAddr::Factory(peer_id);
	int iRes = SendPeerPack(iServerPeer,pPack,TPackGetAddr::GetTotalLen());
	delete[] pPack;
	return TPackGetAddr::GetTotalLen()==iRes;
}

bool NPeerMng::PeerSendConnectHelp(unsigned int iServerPeer,unsigned int peer_id)
{
	if(peer_id==INVALID_PEER_ID || peer_id==GetId())
	{
		ASSERT(FALSE);
		return false;
	}
	//发送id数据包
	TBasePack *pPack = TPackConnect::Factory(peer_id);
	int iRes = SendPeerPack(iServerPeer,pPack,TPackConnect::GetTotalLen());
	delete[] pPack;
	return TPackConnect::GetTotalLen()==iRes;
}

void NPeerMng::OnAddressReturn(const char* pData, int ilen)
{
	ASSERT(pData);
	ASSERT(ilen>0);

	TPackAddrReply *pAddrReply = (TPackAddrReply *)((TBasePack *)pData)->data;
	if(m_iMyId==pAddrReply->uiPeerID)
	{
		ASSERT(FALSE);
		return;
	}

	//如果连接已经建立，忽略此消息
	//if (IsReady())
	//{
	//	return;
	//}

	//SOCKADDR_IN sa;
	//pAddrReply->addrMap.GetAddr(sa);
	//ConnectToEx((sockaddr *)&sa,sizeof(sa));
}

void NPeerMng::OnShootCmd(const char* pData, int ilen)
{
	//处理流程基本上与OnAddressReturn一致

	ASSERT(pData);
	ASSERT(ilen>0);

	TPackShoot *pShoot = (TPackShoot *)((TBasePack *)pData)->data;

	if (pShoot->uiPeerID==INVALID_PEER_ID || pShoot->uiPeerID==m_iMyId)
	{
		ASSERT(FALSE);
		return;
	}

	//如果连接已经建立，忽略此消息
	if (IsPeerReady(pShoot->uiPeerID))
	{
		return;
	}

	if(pShoot->addrMap == KIpv4Addr())
	{
		std::cout<<"打孔失败，用户不在线："<<pShoot->uiPeerID<<std::endl;
		return;
	}
	SOCKADDR_IN sa;
	pShoot->addrMap.GetAddr(sa);
	std::cout<<"打孔目标："<<pShoot->uiPeerID<<","<<pShoot->addrMap.ToString()<<std::endl;
	ConnectToEx((sockaddr *)&sa,sizeof(sa));

	pShoot->addrLocal.GetAddr(sa);
	std::cout<<"打孔目标："<<pShoot->uiPeerID<<","<<pShoot->addrLocal.ToString()<<std::endl;
	ConnectToEx((sockaddr *)&sa,sizeof(sa));
}



//#ifndef WIN32
//void* ThreadTryConnect(void* s)
//#else
//unsigned int WINAPI ThreadTryConnect(LPVOID s)
//#endif
//{
//	NPeerMng *pMng = (NPeerMng *)s;
//	pMng->ThreadConnect();
//	if(1!=pMng->m_setConnectThreads.erase(GetThreadId(GetCurrentThread())))
//	{
//		ASSERT(FALSE);
//	}
//#ifndef WIN32
//	return NULL;
//#else
//	return 0;
//#endif
//}

bool NPeerMng::ConnectToEx(const sockaddr* name, int namelen)
{
	UDTSOCKET u = m_Network.CreateSocket(true);		//不用m_uSocket，避免被认为连接已建立

	JThread *pThread = m_threadGroup.AddThread(
		new ThreadConnnectTo(this,INVALID_PEER_ID,m_iMyId,u,name,namelen)
		);

	pThread->Start();
	return true;
}
void NPeerMng::OnConnectExFinish(ThreadConnnectTo *pThread,int iRes)
{
	if (iRes==0) //连接成功
	{
		m_Network.AddConn(pThread->m_socket);
		OnConnectionMsg(pThread->m_socket,INetworkListener::NL_CODE_NEWCONN,0);
	}
	else
	{
		//尝试失败，怎么通知？
	}
	m_threadGroup.RemoveThread(pThread);
}

bool NPeerMng::IsPeerReady( unsigned int iPeerId )
{
	TPeerPtr pPeer = FindPeer(iPeerId);
	if (!pPeer)
	{
		return false;
	}
	return pPeer->IsReady();
}
//关闭连接。此函数关闭所有相关网络连接，释放对应智能指针
void NPeerMng::ClosePeer(unsigned int iPeerId)
{
	//遍历m_mapSockID表，将相关连接关闭
	{
		JMutexAutoLock autolock(m_SockIDLock);

		T_mapSockID::iterator it = m_mapSockID.begin();
		for(;it!=m_mapSockID.end();it++)
		{
			if (iPeerId==(*it).second)
			{
				m_Network.CloseConn((*it).first);
				it = m_mapSockID.erase(it);
			}
		}
	}

	//删除m_mapIDPeer中的记录，将TPeer返回
	TPeerPtr pResPeer = FindPeer(iPeerId);
	if (pResPeer)
	{
		pResPeer->SetCurrentSocket(UDT::INVALID_SOCK);
	}
	RemoveIDPeer(iPeerId);

}

bool NPeerMng::IsPeerExist( unsigned int iPeerId )
{
	JMutexAutoLock autolock(m_IDPeerLock);

	T_mapIDPeer::iterator it = m_mapIDPeer.find(iPeerId);
	return it!=m_mapIDPeer.end();
}

void NPeerMng::AddSockID(UDTSOCKET u,unsigned int iPeerId)
{
	JMutexAutoLock autolock(m_SockIDLock);

	m_mapSockID[u] = iPeerId;
}
void NPeerMng::RemoveSockID(UDTSOCKET u)
{
	JMutexAutoLock autolock(m_SockIDLock);

	if(1!=m_mapSockID.erase(u)) //从连接表里删除它
	{
		//ASSERT(FALSE);
	}
}

void NPeerMng::RemoveIDPeer( unsigned int iPeerId )
{
	JMutexAutoLock autolock(m_IDPeerLock);

	m_mapIDPeer.erase(iPeerId);
}

void NPeerMng::AddPeerListener( IPeerDataListener *pListener )
{
	JMutexAutoLock auto_lock(m_setPeerListenerLock);
	m_setPeerListener.insert(pListener);
}

bool NPeerMng::RemovePeerListener( const IPeerDataListener *pListener )
{
	JMutexAutoLock auto_lock(m_setPeerListenerLock);
	return 1==m_setPeerListener.erase(const_cast<IPeerDataListener *>(pListener));
}

bool NPeerMng::GetServAddress( sockaddr *name, int &iLen )
{
	//取得本地的ip地址
	char szLocalName[256];
	if( gethostname ( szLocalName, sizeof(szLocalName)) == 0)
	{
		PHOSTENT hostinfo;
		if((hostinfo = gethostbyname(szLocalName)) != NULL)
		{
			//*name = *(struct in_addr *)*hostinfo->h_addr_list;
			//*name = *(struct sockaddr *)(hostinfo->h_addr_list[0]);
			(*(SOCKADDR_IN*)name).sin_addr = *(struct in_addr *)(hostinfo->h_addr_list[0]);
			(*(SOCKADDR_IN*)name).sin_port = htons(0);
			iLen = sizeof(sockaddr);

			//string strAddr = inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list);
			//std::cout<<"ip:"<<strAddr<<std::endl;
			return true;
		}
	}
	return false;
}