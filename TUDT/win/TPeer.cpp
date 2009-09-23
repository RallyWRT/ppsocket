#include ".\tpeer.h"
#include "../src/debug.h"
#include "TPeerProtocol.h"
#include <iostream>

using namespace UDT;

#pragma warning(disable:4100)
const int PEER_ID_NOTDEFINED = 0;

//�Եȵ������
TPeerMng::TPeerMng(unsigned int iMyId,int iLocalPort)  //ϵͳΨһ�Ľڵ�id����ڷ�������ַ
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
//�Լ���id���ڿ�ʼ��ʱ�������0��ע�⣺IDֻ�ܸ���һ��
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

//�ص�����
TPeer *TPeerMng::PeerFactory(UDTSOCKET u,int iPeerId){//������������
	u;iPeerId;
	return NULL;
};      
//void TPeerMng::OnNATDetected(int iNetworkType){ //NAT���ͼ�����
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

	//ע���Լ���������ղ���peerЭ����Ϣ
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

	//����������Ͳ�ѯ��ַ�����ѯ������Զ��ص�OnAddressReturn����
	ASSERT(GetPeerMng());
	ASSERT(GetPeerMng()->GetServerPeer());
	if(!GetPeerMng()->GetServerPeer()->PeerSendConnectHelp(m_iPeerId))
	{
		return false;
	}

	//�ȴ�ָ����ʱ�䣬�����ӳɹ���
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

//���ӽ�������û��ȷ��ID
void  TPeer::OnConnected(bool bSuccess)
{
	//���Ѿ�logon�ɹ��ˣ��ŵ���OnConnect��
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

	//�Ϸ��Լ��
	if (pBasePack->flag!=TBasePack::PEERPACK_FLAG)
	{
		//flag����
		//TConnection::Close();
		Close();
		return;
	}

	//δ�������֮ǰ�Ĵ���ʽ
	if (!m_bLogon)
	{
		if (pBasePack->pack_type!=TBasePack::PT_SENDID)
		{
			//��һ������������SendID����������Ϊ�Ͽ�����
			//�����ʹ���
			//TConnection::Close();
			Close();
			return;
		}
		TPackSendId *pSendID = (TPackSendId *)pBasePack->data;

		//��鱾���Ƿ��Ѿ�������û�������
		TPeer *pOldPeer = m_pPeerMng->FindPeer(pSendID->uiUserId);
		if (pOldPeer && (pOldPeer!=this))
		{
			if (pOldPeer->IsReady())  //�����������ӣ�����������������ô����Ҫ��������ӡ�
			{
				Close(); //������ڴ�й©��������֪�����TPeerָ���Ƿ���Ӧ�ò������ã�����ɾ��
				//delete this;
				return;
			}
			else //�������Ӵ����ˣ�����û���ӳɹ������µ�socket����ɵġ�
			{
				UDTSOCKET u = this->GetSocket();
				SetSocket(UDT::INVALID_SOCK);
				Close();  //������ڴ�й©��������֪�����TPeerָ���Ƿ���Ӧ�ò������ã�����ɾ��
				//delete this;

				pOldPeer->SetSocket(u);
				pOldPeer->SetLogon(true);

				m_pPeerMng->RemovePeer(pSendID->uiUserId,this);
				m_pPeerMng->AddPeer(pSendID->uiUserId,pOldPeer);

				return;
			}
		}

		if (0==m_iPeerId) //û��Ԥ��ID����ô��������ID����
		{
			m_iPeerId = pSendID->uiUserId;
		}
		else
		{
			if (pSendID->uiUserId != (t_uint)m_iPeerId)
			{
				//�������ڴ���ID
				//TConnection::Close();
				Close();
				return;
			}
		}

		//���ӳɹ�
		SetLogon(true);
		m_pPeerMng->AddPeer(GetPeerId(),this);

		//����id���ݰ�
		PeerSendID();

		//��ȡӳ���ַ
		PeerSendGetAddrPack(GetPeerMng()->GetId());

		//������Ϣ
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
	case TBasePack::PT_ADDR_GETMAP: //��ѯpeer�ĵ�ַ
		{
			TPackGetAddr *pGetAddr = (TPackGetAddr *)pBasePack->data;
			if (0==pGetAddr->uiPeerID) //peer id����Ϊ0
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

			//����id���ݰ�
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
				std::cout<<"ӳ���ַ��"<<pAddrReply->addrMap.ToString()<<std::endl;
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
			//�յ���������������ô��˫������shoot���
			TPackConnect *pConnectHelp = (TPackConnect *)pBasePack->data;
			ASSERT(pConnectHelp->uiPeerID>0);
			ASSERT((pConnectHelp->uiPeerID!=GetPeerId()));
			PeerSendShootCmd(pConnectHelp->uiPeerID,GetPeerId());
			PeerSendShootCmd(GetPeerId(),pConnectHelp->uiPeerID);
			return;
		}
	case TBasePack::PT_STUN_SHOOT:
		{
			//�յ�������
			TPackShoot *pShoot = (TPackShoot *)pBasePack->data;
			std::cout<<"���Ŀ�꣺"<<pShoot->uiPeerID<<","<<pShoot->addrMap.ToString()<<std::endl;
			ASSERT(pShoot->uiPeerID!=GetPeerMng()->GetId());

			//����������ߣ���ô������ȥ����
			TPeer *pPeer = GetPeerMng()->FindPeer(pShoot->uiPeerID);
			if (!pPeer)
			{
				//û�У��򴴽�һ���µ�������
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
	//����id���ݰ�
	TBasePack *pPack = TPackSendId::Factory(m_pPeerMng->GetId(),KIpv4Addr());
	int iRes = TConnection::Send((char *)pPack,TPackSendId::GetTotalLen());
	ASSERT(TPackSendId::GetTotalLen()==iRes);
	delete[] pPack;
}

bool TPeer::PeerSendGetAddrPack(unsigned int peer_id)
{
	//����id���ݰ�
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
	//����id���ݰ�
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
	if (pPeer2) //pPeer2�����ڣ�Ҳ����pPeer1�������ݣ��հ����ݣ�
	{
		UDTSOCKET u = pPeer2->GetSocket();
		sockaddr sa;
		int len;
		UDT::getpeername(u,&sa,&len);
		addr = KIpv4Addr(*(SOCKADDR_IN*)&sa);
	}

	//����id���ݰ�
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

	//��������Ѿ����������Դ���Ϣ
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
	//�������̻�������OnAddressReturnһ��

	ASSERT(pData);
	ASSERT(ilen>0);

	TPackShoot *pShoot = (TPackShoot *)((TBasePack *)pData)->data;
	if(GetPeerId()!=pShoot->uiPeerID)
	{
		ASSERT(FALSE);
		return;
	}

	//��������Ѿ����������Դ���Ϣ
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
//�����������������˺����ᱻ�����Բ���һ��TConnection����
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

//ע�⣬���iPeerId��Ӧ��pPeer���ڣ���ô��������ɹ������һ�ASSERT
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

//�ص�����
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