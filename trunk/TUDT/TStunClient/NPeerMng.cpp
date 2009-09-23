#include ".\npeermng.h"
#include "../win/TPeerProtocol.h"
#include "../src/debug.h"
#include "ThreadConnnectTo.h"
#include <process.h>    /* _beginthread, _endthread */
#include <iostream>

#include "jmutexautolock.h"

NPeerMng::NPeerMng(unsigned int iMyId, const sockaddr* name, int namelen)  //系统唯一的节点id和入口服务器地址
:m_Network(name,namelen),m_iMyId(iMyId)
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
	if (m_iMyId!=0)
	{
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

	m_iServerPeer = 0;
}
bool NPeerMng::IsServerConnectReady()
{
	return IsPeerReady(m_iServerPeer);
}

//回调函数
NPeerPtr NPeerMng::PeerFactory(int iPeerId)      //有新连接上来
{
	return NPeerPtr(new NPeer(this,iPeerId));
}


NPeerPtr NPeerMng::GetPeerInstance(int iPeerId)
{
	//如果已经有实例了，直接使用它
	NPeerPtr ptrPeer = FindPeer(iPeerId);
	if (ptrPeer)
	{
		return ptrPeer;
	}

	//没有实例，创建一个
	ptrPeer = PeerFactory(iPeerId);

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
			if(0!=Socket2Peer(iConn))
			{
				ASSERT(FALSE);
				return;
			}

			//新连接上来，发送ID号
			//发送id数据包
			sockaddr sa;
			int len;
			UDT::getsockname(iConn,&sa,&len);
			TBasePack *pPack = TPackSendId::Factory(m_iMyId,KIpv4Addr(*(SOCKADDR_IN *)&sa));
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

			//找到对应的NPeer
			NPeerPtr ptrPeer = FindPeer(id);
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

	if (0==id) //没有通过认证
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
		NPeerPtr ptrPeer = GetPeerInstance(pSendID->uiUserId);
		if (!ptrPeer)
		{
			UDT::close(iConn); //GetPeerInstance如果返回NULL，说明不接受新的连接
			return;
		}
		//把这个新的连接交给peer使用
		ptrPeer->SetCurrentSocket(iConn);

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
			//找到对应的NPeer
			NPeerPtr ptrPeer = FindPeer(id);
			if (!ptrPeer)
			{
				return;
			}
			ptrPeer->SetCurrentSocket(iConn); //更新一下当前socket
			ptrPeer->OnPeerData((char *)pBasePack->data,iLen-sizeof(TBasePack));
			return;
		}
	case TBasePack::PT_ADDR_GETMAP: //查询peer的地址
		{
			TPackGetAddr *pGetAddr = (TPackGetAddr *)pBasePack->data;
			if (0==pGetAddr->uiPeerID) //peer id不能为0
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

			ASSERT(pConnectHelp->uiPeerID>0);
			ASSERT(pConnectHelp->uiPeerID!=iPeerId);
			ASSERT(iPeerId!=0);

			PeerSendShootCmd(pConnectHelp->uiPeerID,iPeerId);
			PeerSendShootCmd(iPeerId,pConnectHelp->uiPeerID);
			return;
		}
	case TBasePack::PT_STUN_SHOOT:
		{
			//收到打洞命令
			TPackShoot *pShoot = (TPackShoot *)pBasePack->data;

			std::cout<<"打孔目标："<<pShoot->uiPeerID<<","<<pShoot->addrMap.ToString()<<std::endl;
			ASSERT(pShoot->uiPeerID!=0);
			ASSERT(pShoot->uiPeerID!=m_iMyId);
			ASSERT(pShoot->uiPeerID!=Socket2Peer(iConn));

			//如果有连接者，那么命令它去连接
			NPeerPtr pPeer = GetPeerInstance(pShoot->uiPeerID);
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
bool NPeerMng::AddPeer(unsigned int iPeerId,NPeerPtr pPeer)
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
	NPeerPtr pPeer = FindPeer(iPeerId);
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
	NPeerPtr pPeer = FindPeer(iPeerId);
	if (!pPeer)
	{
		return -1;
	}
	return pPeer->SendPeerMsg(pData,iLen);
}
//给用户发送TBasePack数据包
int NPeerMng::SendPeerPack( unsigned int iPeerId,const TBasePack *pPack, int iLen )
{
	NPeerPtr pPeer = FindPeer(iPeerId);
	if (!pPeer)
	{
		return -1;
	}
	return pPeer->SendPack(pPack,iLen);
}
NPeerPtr NPeerMng::FindPeer(unsigned int iPeerId)
{
	JMutexAutoLock autolock(m_IDPeerLock);

	T_mapIDPeer::iterator it = m_mapIDPeer.find(iPeerId);
	if (it==m_mapIDPeer.end())
	{
		return NPeerPtr();
	}
	return (*it).second;
}
unsigned int NPeerMng::Socket2Peer(UDTSOCKET u)
{
	JMutexAutoLock autolock(m_SockIDLock);

	T_mapSockID::iterator it = m_mapSockID.find(u);
	if (it==m_mapSockID.end())
	{
		return 0;
	}
	return (*it).second;
}


bool NPeerMng::PeerSendShootCmd(unsigned int peer1,unsigned int peer2)
{
	std::cout<<"send shoot cmd:"<<peer1<<"-->"<<peer2<<std::endl;

	NPeerPtr pPeer1 = FindPeer(peer1);
	if (!pPeer1)
	{
		return false;
	}

	KIpv4Addr addr = GetPeerAddress(peer2);

	//发送id数据包
	TBasePack *pPack = TPackShoot::Factory(peer2,addr);
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
	if(peer_id<=0 || peer_id==GetId())
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
	if(m_iMyId==pAddrReply->uiPeerID
		||pAddrReply==0)
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

	if (pShoot->uiPeerID==0 || pShoot->uiPeerID==m_iMyId)
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
		std::cout<<"打孔地址为空！"<<std::endl;
		return;
	}
	SOCKADDR_IN sa;
	pShoot->addrMap.GetAddr(sa);
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
		new ThreadConnnectTo(this,0,m_iMyId,u,name,namelen)
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
	NPeerPtr pPeer = FindPeer(iPeerId);
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

	//删除m_mapIDPeer中的记录，将NPeer返回
	NPeerPtr pResPeer = FindPeer(iPeerId);
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
		ASSERT(FALSE);
	}
}

void NPeerMng::RemoveIDPeer( unsigned int iPeerId )
{
	JMutexAutoLock autolock(m_IDPeerLock);

	m_mapIDPeer.erase(iPeerId);
}
