#include ".\tpeer.h"
#include "../src/debug.h"
#include "TPeerProtocol.h"
#include <iostream>

using namespace UDT;

#pragma warning(disable:4100)
const int PEER_ID_NOTDEFINED = 0;

//对等点管理器
TPeerMng::TPeerMng(unsigned int iMyId,int iLocalPort)  //系统唯一的节点id和入口服务器地址
{
	m_pPeerMngImpl = new TPeerMngImpl(this,iMyId,iLocalPort);
}
TPeerMng::~TPeerMng()
{
	delete m_pPeerMngImpl;
}
void TPeerMng::CloseServerConnection()
{
	m_pPeerMngImpl->CloseServerConnection();
}		
void TPeerMng::ConnectToServer(unsigned int uiServerID, const sockaddr* name, int namelen)
{
	m_pPeerMngImpl->ConnectToServer(uiServerID,name,namelen);
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

UDT::TPeer * TPeerMng::GetServerPeer()
{
	return m_pPeerMngImpl->GetServerPeer();
}

TPeer *TPeerMng::FindPeer(unsigned int iPeerId)
{
	return m_pPeerMngImpl->FindPeer(iPeerId);
}
void TPeerMng::GetPeerSet(std::set<unsigned int> &setPeers)
{
	m_pPeerMngImpl->GetPeerSet(setPeers);
}
void TPeerMng::AddPeer(unsigned int iPeerId,TPeer *pPeer)
{
	m_pPeerMngImpl->AddPeer(iPeerId,pPeer);
}
bool TPeerMng::RemovePeer(unsigned int iPeerId,TPeer *pPeer)
{
	return m_pPeerMngImpl->RemovePeer(iPeerId,pPeer);
}

//回调函数
TPeer *TPeerMng::PeerFactory(UDTSOCKET u,int iPeerId){//有新连接上来
	u;iPeerId;
	return NULL;
};      
//void TPeerMng::OnNATDetected(int iNetworkType){ //NAT类型检测完成
//	iNetworkType;
//};

//============================
//TPeerImpl
//============================
TPeer::TPeer( TPeerMng *pMng,int iPeerId,UDTSOCKET u)
:TConnection(pMng->m_pPeerMngImpl,u)
{
	m_iPeerId = iPeerId;
	m_bLogon = false;
	m_pPeerMng = pMng;

	//注册自己，否则接收不到peer协议消息
	//if (UDT::INVALID_SOCK != u)
	if(m_iPeerId!=0)
	{
		m_pPeerMng->AddPeer(m_iPeerId,this);
	}
}

TPeer::~TPeer()
{
	if (0!=GetPeerId())
	{
		m_pPeerMng->RemovePeer(GetPeerId(),this);
	}
}

unsigned int TPeer::GetPeerId()
{
	return m_iPeerId;
}

bool TPeer::PeerConnect(int iWaitTime)
{
	if(m_iPeerId<=0)
	{
		return false;
	}
	if(IsReady())
	{
		return false;
	}

	//向服务器发送查询地址命令。查询到后会自动回调OnAddressReturn函数
	ASSERT(GetPeerMng());
	ASSERT(GetPeerMng()->GetServerPeer());
	if(!GetPeerMng()->GetServerPeer()->PeerSendConnectHelp(m_iPeerId))
	{
		return false;
	}

	//等待指定的时间，看连接成功否
	DWORD dwBgn = GetTickCount();
	while (!IsReady() && GetTickCount()<(dwBgn+iWaitTime))
	{
		Sleep(10);
	}
	return IsReady();
}

bool TPeer::PeerConnect( const sockaddr* nameKnown, int namelen,bool bTrySTUN/*=false*/ )
{
	if (IsReady())
	{
		ASSERT(FALSE);
		return false;
	}
	return TConnection::ConnectToEx(nameKnown,namelen);
}

//bool TPeer::PeerConnect( const sockaddr* nameKnown, int namelen,int iHelperId,bool bTrySTUN/*=false*/ )
//{
//	ASSERT(FALSE);
//	return 0;
//}

int TPeer::GetSpeed( int &iUpSpeed,int &iDownSpeed )
{
	if (!IsReady())
	{
		return -1;
	}
	return 0;
}

int TPeer::SendPeerMsg( const char* pData,int iLen )
{
	if (m_bLogon)
	{
		TBasePack *pBasePack = TPackData::Factory(pData,iLen);
		int bRes = TConnection::Send((char *)pBasePack,TPackData::GetTotalLen(iLen));
		delete[] pBasePack;
		//return bRes;
		if (bRes>0)
		{
			return iLen;
		}
		return -1;
	}
	else
	{
		return -1;
	}
}

bool TPeer::Close()
{
	return TConnection::Close();
}

bool TPeer::IsReady()
{
	if (!TConnection::IsReady())
	{
		return false;
	}
	return m_bLogon;
}

void TPeer::SetLogon(bool bLogon)
{
	m_bLogon = bLogon;
}

//链接建立，但没有确认ID
void  TPeer::OnConnected(bool bSuccess)
{
	//都已经logon成功了，才调用OnConnect？
	ASSERT(!m_bLogon);

	sockaddr sa;
	int len;
	UDT::getpeername(GetSocket(),&sa,&len);
	std::cout<<"TPeer::OnConnected:"
		<<bSuccess
		<<","<<GetPeerId()<<""
		<<KIpv4Addr(*(SOCKADDR_IN*)&sa).ToString()
		<<std::endl;

	if (bSuccess)
	{
		PeerSendID();
	}
	else
	{
		OnPeerConnected(false);
	}
}
void  TPeer::OnDisConnected()
{
	OnPeerDisConnected();
};

void  TPeer::OnPeerConnected(bool bSuccess)
{
	bSuccess;
};           
void  TPeer::OnPeerDisConnected()
{
	delete this;
};
void  TPeer::OnPeerData(const char* pData, int ilen){
	pData;ilen;
};

void  TPeer::OnData(const char* pData, int ilen)
{
	TBasePack *pBasePack = (TBasePack *)pData;
	std::cout<<"OnPack:"<<pBasePack->GetPackType()<<std::endl;

	//合法性检查
	if (pBasePack->flag!=TBasePack::PEERPACK_FLAG)
	{
		//flag错误
		//TConnection::Close();
		Close();
		return;
	}

	//未完成连接之前的处理方式
	if (!m_bLogon)
	{
		if (pBasePack->pack_type!=TBasePack::PT_SENDID)
		{
			//第一个包，必须是SendID包，否则认为断开连接
			//包类型错误
			//TConnection::Close();
			Close();
			return;
		}
		TPackSendId *pSendID = (TPackSendId *)pBasePack->data;

		//检查本地是否已经有与此用户的连接
		TPeer *pOldPeer = m_pPeerMng->FindPeer(pSendID->uiUserId);
		if (pOldPeer && (pOldPeer!=this))
		{
			if (pOldPeer->IsReady())  //本地已有连接，并且连接正常，那么不需要多余的连接。
			{
				Close(); //好像会内存泄漏？？但不知道这个TPeer指针是否在应用层有引用，不敢删除
				//delete this;
				return;
			}
			else //本地连接创建了，但还没连接成功：用新的socket代替旧的。
			{
				UDTSOCKET u = this->GetSocket();
				SetSocket(UDT::INVALID_SOCK);
				Close();  //好像会内存泄漏？？但不知道这个TPeer指针是否在应用层有引用，不敢删除
				//delete this;

				pOldPeer->SetSocket(u);
				pOldPeer->SetLogon(true);

				m_pPeerMng->RemovePeer(pSendID->uiUserId,this);
				m_pPeerMng->AddPeer(pSendID->uiUserId,pOldPeer);

				return;
			}
		}

		if (0==m_iPeerId) //没有预设ID，那么允许任意ID连接
		{
			m_iPeerId = pSendID->uiUserId;
		}
		else
		{
			if (pSendID->uiUserId != (t_uint)m_iPeerId)
			{
				//不是所期待的ID
				//TConnection::Close();
				Close();
				return;
			}
		}

		//连接成功
		SetLogon(true);
		m_pPeerMng->AddPeer(GetPeerId(),this);

		//发送id数据包
		PeerSendID();

		//获取映射地址
		PeerSendGetAddrPack(GetPeerMng()->GetId());

		//传递消息
		OnPeerConnected(true);

		return;
	}

	switch(pBasePack->pack_type)
	{
	case TBasePack::PT_KEEPLIVE:
		{
			m_dwLastKeepLive = GetTickCount();
			return;
		}
	case TBasePack::PT_DATA:
		{
			OnPeerData((char *)pBasePack->data,ilen-sizeof(TBasePack));
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
			//
			TPeer *pPeer = GetPeerMng()->FindPeer(pGetAddr->uiPeerID);
			KIpv4Addr addr;
			if (pPeer)
			{
				UDTSOCKET u = pPeer->GetSocket();
				sockaddr sa;
				int len;
				UDT::getpeername(u,&sa,&len);
				addr = KIpv4Addr(*(SOCKADDR_IN*)&sa);
			}

			//发送id数据包
			TBasePack *pPack = TPackAddrReply::Factory(pGetAddr->uiPeerID,addr);
			int iRes = TConnection::Send((char *)pPack,TPackAddrReply::GetTotalLen());
			ASSERT(TPackAddrReply::GetTotalLen()==iRes);
			delete[] pPack;
			return;
		}
	case TBasePack::PT_ADDR_REPLY:
		{
			TPackAddrReply *pAddrReply = (TPackAddrReply *)pBasePack->data;
			if(pAddrReply->uiPeerID==GetPeerMng()->GetId())
			{
				std::cout<<"映射地址："<<pAddrReply->addrMap.ToString()<<std::endl;
			}
			TPeer *pPeer = GetPeerMng()->FindPeer(pAddrReply->uiPeerID);
			if (pPeer)
			{
				pPeer->OnAddressReturn(pData,ilen);
			}
			return;
		}

	case TBasePack::PT_STUN_CONNECT:
		{
			//收到辅助连接请求，那么向双发发送shoot命令。
			TPackConnect *pConnectHelp = (TPackConnect *)pBasePack->data;
			ASSERT(pConnectHelp->uiPeerID>0);
			ASSERT((pConnectHelp->uiPeerID!=GetPeerId()));
			PeerSendShootCmd(pConnectHelp->uiPeerID,GetPeerId());
			PeerSendShootCmd(GetPeerId(),pConnectHelp->uiPeerID);
			return;
		}
	case TBasePack::PT_STUN_SHOOT:
		{
			//收到打洞命令
			TPackShoot *pShoot = (TPackShoot *)pBasePack->data;
			std::cout<<"打孔目标："<<pShoot->uiPeerID<<","<<pShoot->addrMap.ToString()<<std::endl;
			ASSERT(pShoot->uiPeerID!=GetPeerMng()->GetId());

			//如果有连接者，那么命令它去连接
			TPeer *pPeer = GetPeerMng()->FindPeer(pShoot->uiPeerID);
			if (!pPeer)
			{
				//没有，则创建一个新的来连接
				pPeer = GetPeerMng()->PeerFactory(UDT::INVALID_SOCK,pShoot->uiPeerID);
			}
			if (pPeer)
			{
				pPeer->OnShootCmd(pData,ilen);
			}
			return;
		}
	case TBasePack::PT_SENDID:return;
	}

	

};

UDT::TPeerMng *TPeer::GetPeerMng()
{
	return m_pPeerMng;
}
void TPeer::PeerSendID()
{
	//发送id数据包
	TBasePack *pPack = TPackSendId::Factory(m_pPeerMng->GetId(),KIpv4Addr());
	int iRes = TConnection::Send((char *)pPack,TPackSendId::GetTotalLen());
	ASSERT(TPackSendId::GetTotalLen()==iRes);
	delete[] pPack;
}

bool TPeer::PeerSendGetAddrPack(unsigned int peer_id)
{
	//发送id数据包
	TBasePack *pPack = TPackGetAddr::Factory(peer_id);
	int iRes = TConnection::Send((char *)pPack,TPackGetAddr::GetTotalLen());
	delete[] pPack;
	return TPackGetAddr::GetTotalLen()==iRes;
}

bool TPeer::PeerSendConnectHelp(unsigned int peer_id)
{
	if(peer_id<=0 || peer_id==GetPeerMng()->GetId())
	{
		ASSERT(FALSE);
		return false;
	}
	//发送id数据包
	TBasePack *pPack = TPackConnect::Factory(peer_id);
	int iRes = TConnection::Send((char *)pPack,TPackConnect::GetTotalLen());
	delete[] pPack;
	return TPackConnect::GetTotalLen()==iRes;
}

bool TPeer::PeerSendShootCmd(unsigned int peer1,unsigned int peer2)
{
	TPeer *pPeer1 = GetPeerMng()->FindPeer(peer1);
	if (!pPeer1)
	{
		return false;
	}

	KIpv4Addr addr;
	TPeer *pPeer2 = GetPeerMng()->FindPeer(peer2);
	if (pPeer2) //pPeer2不存在，也得向pPeer1发送数据（空白数据）
	{
		UDTSOCKET u = pPeer2->GetSocket();
		sockaddr sa;
		int len;
		UDT::getpeername(u,&sa,&len);
		addr = KIpv4Addr(*(SOCKADDR_IN*)&sa);
	}

	//发送id数据包
	TBasePack *pPack = TPackShoot::Factory(peer2,addr);
	int iRes = pPeer1->Send((char *)pPack,TPackShoot::GetTotalLen());
	std::cout<<"send shoot cmd:"<<peer1<<"-->"<<peer2<<" "<<pPack->GetPackType()<<std::endl;
	delete[] pPack;
	return TPackShoot::GetTotalLen()==iRes;
}

void TPeer::OnAddressReturn(const char* pData, int ilen)
{
	ASSERT(pData);
	ASSERT(ilen>0);

	TPackAddrReply *pAddrReply = (TPackAddrReply *)((TBasePack *)pData)->data;
	if(GetPeerId()!=pAddrReply->uiPeerID)
	{
		ASSERT(FALSE);
		return;
	}

	//如果连接已经建立，忽略此消息
	if (IsReady())
	{
		return;
	}

	SOCKADDR_IN sa;
	pAddrReply->addrMap.GetAddr(sa);
	ConnectToEx((sockaddr *)&sa,sizeof(sa));
}

void TPeer::OnShootCmd(const char* pData, int ilen)
{
	//处理流程基本上与OnAddressReturn一致

	ASSERT(pData);
	ASSERT(ilen>0);

	TPackShoot *pShoot = (TPackShoot *)((TBasePack *)pData)->data;
	if(GetPeerId()!=pShoot->uiPeerID)
	{
		ASSERT(FALSE);
		return;
	}

	//如果连接已经建立，忽略此消息
	if (IsReady())
	{
		return;
	}

	SOCKADDR_IN sa;
	pShoot->addrMap.GetAddr(sa);
	ConnectToEx((sockaddr *)&sa,sizeof(sa));
}


//============================
//TPeerMngImpl
//============================
TPeerMngImpl::TPeerMngImpl(UDT::TPeerMng *pPeerMngInterface,unsigned int iMyId,int iLocalPort )
:TConnectMng(iLocalPort)
{
	m_iMyId = iMyId;
	m_pPeerMngInterface = pPeerMngInterface;
	m_connServer = NULL;
}

TPeerMngImpl::~TPeerMngImpl()
{
	delete m_connServer;
}

void TPeerMngImpl::CloseServerConnection()
{

}

bool TPeerMngImpl::UpdateId( unsigned int iMyId )
{
	if(PEER_ID_NOTDEFINED != m_iMyId)
	{
		return false;
	}
	m_iMyId = iMyId;
	return true;
}

unsigned int TPeerMngImpl::GetId()
{
	return m_iMyId;
}
//当有新连接上来，此函数会被调用以产生一个TConnection对象
UDT::TConnection *TPeerMngImpl::ConnFactory(UDTSOCKET u)
{
	return PeerFactory(u,0);
}

UDT::TPeer *TPeerMngImpl::FindPeer(unsigned int iPeerId)
{
	CGuard tmpGuard(m_PeerMapMutex.GetMutex());
	std::map<int,UDT::TPeer*>::iterator it = m_mapPeers.find(iPeerId);
	if (it==m_mapPeers.end())
	{
		return NULL;
	}
	ASSERT((*it).second);
	return (*it).second;
}

void TPeerMngImpl::GetPeerSet(std::set<unsigned int> &setPeers)
{
	CGuard tmpGuard(m_PeerMapMutex.GetMutex());
	std::map<int,UDT::TPeer*>::iterator it = m_mapPeers.begin();
	for(;it!=m_mapPeers.end();it++)
	{
		setPeers.insert((*it).first);
	}
}

//注意，如果iPeerId对应的pPeer存在，那么操作不会成功，而且会ASSERT
void TPeerMngImpl::AddPeer(unsigned int iPeerId,TPeer *pPeer)
{
	ASSERT(pPeer);
	ASSERT(iPeerId>0);
	TPeer *pOld = FindPeer(iPeerId);
	if (NULL!=pOld)
	{
		if(pOld!=pPeer)
		{
			ASSERT(FALSE);
			return;
		}
		return;

	}

	CGuard tmpGuard(m_PeerMapMutex.GetMutex());
	m_mapPeers[iPeerId] = pPeer;
}
bool TPeerMngImpl::RemovePeer(unsigned int iPeerId,TPeer *pPeer)
{
	CGuard tmpGuard(m_PeerMapMutex.GetMutex());
	//ASSERT(pPeer==FindPeer(iPeerId));
	return m_mapPeers.erase(iPeerId)>0;
}

//回调函数
UDT::TPeer *TPeerMngImpl::PeerFactory(UDTSOCKET u,int iPeerId)
{
	return m_pPeerMngInterface->PeerFactory(u,iPeerId);
};
//void TPeerMngImpl::OnNATDetected(int iNetworkType){
//	return m_pPeerMngInterface->OnNATDetected(iNetworkType);
//
//};

void TPeerMngImpl::ConnectToServer(unsigned int uiServerID, const sockaddr* name, int namelen)
{
	if (m_connServer)
	{
		delete m_connServer;
	}
	m_connServer = new UDT::TPeer(m_pPeerMngInterface,uiServerID);
	m_connServer->ConnectToEx(name,namelen);
}

UDT::TPeer * TPeerMngImpl::GetServerPeer()
{
	return m_connServer;
}