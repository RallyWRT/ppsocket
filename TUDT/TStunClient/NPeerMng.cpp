#include ".\npeermng.h"
#include "../win/TPeerProtocol.h"
#include "../src/debug.h"
#include "ThreadConnnectTo.h"
#include <process.h>    /* _beginthread, _endthread */
#include <iostream>

#include "jmutexautolock.h"

NPeerMng::NPeerMng(unsigned int iMyId, const sockaddr* name, int namelen)  //ϵͳΨһ�Ľڵ�id����ڷ�������ַ
:m_Network(name,namelen),m_iMyId(iMyId)
{
	m_Network.SetNetworkListener(this);
}
NPeerMng::~NPeerMng()
{
	m_Network.SetNetworkListener(NULL);
}

//�Լ���id���ڿ�ʼ��ʱ�������0��ע�⣺IDֻ�ܸ���һ��
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

//���������������
void NPeerMng::ConnectToServer(unsigned int uiServerID, const sockaddr* name, int namelen)
{
	m_iServerPeer = uiServerID;

	ConnectToEx(name,namelen);
}
//�ر�������������ӣ���ʡ��������Դ��
void NPeerMng::CloseServerConnection()
{
	ClosePeer(m_iServerPeer);

	m_iServerPeer = 0;
}
bool NPeerMng::IsServerConnectReady()
{
	return IsPeerReady(m_iServerPeer);
}

//�ص�����
NPeerPtr NPeerMng::PeerFactory(int iPeerId)      //������������
{
	return NPeerPtr(new NPeer(this,iPeerId));
}


NPeerPtr NPeerMng::GetPeerInstance(int iPeerId)
{
	//����Ѿ���ʵ���ˣ�ֱ��ʹ����
	NPeerPtr ptrPeer = FindPeer(iPeerId);
	if (ptrPeer)
	{
		return ptrPeer;
	}

	//û��ʵ��������һ��
	ptrPeer = PeerFactory(iPeerId);

	//��peer���뵽��������
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
	case NL_CODE_NEWCONN:    //���ӽ���
		{
			if(0!=Socket2Peer(iConn))
			{
				ASSERT(FALSE);
				return;
			}

			//����������������ID��
			//����id���ݰ�
			sockaddr sa;
			int len;
			UDT::getsockname(iConn,&sa,&len);
			TBasePack *pPack = TPackSendId::Factory(m_iMyId,KIpv4Addr(*(SOCKADDR_IN *)&sa));
			int iRes = UDT::sendmsg(iConn,(char *)pPack,TPackSendId::GetTotalLen());
			ASSERT(TPackSendId::GetTotalLen()==iRes);
			delete[] pPack;
		}
		break;
	case NL_CODE_BREAKDOWN:  //�����ж�
		{
			//���ӶϿ��������ҵ���Ӧ�û�������ǲ��Ǹ��û��ĵ�ǰ����
			//����ǣ���ô��û�б������ӿ��ã�û�оͷ������ӶϿ���Ϣ
			unsigned int id = Socket2Peer(iConn); //��һ��������RemoveSockID.erase֮ǰ
			RemoveSockID(iConn);

			//�ҵ���Ӧ��NPeer
			NPeerPtr ptrPeer = FindPeer(id);
			//�ǲ��ǵ�ǰ����
			//������ǵ�ǰ���ӣ��Ͽ��ͶϿ��ˣ����ù���
			if (ptrPeer->GetCurrentSocket()==iConn) 
			{
				//ע������Ҳ�������socket���Զ���INVALID_SOCKET��ȥ��
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

	//�Ϸ��Լ��
	if (pBasePack->flag!=TBasePack::PEERPACK_FLAG)
	{
		ASSERT(FALSE); //���ݰ����Ϸ���ɾ��֮��ע���Ժ���Զ��õ����ӶϿ���Ϣ
		UDT::close(iConn);
		return;
	}

	//�����ݵ���ʱ�����û���û��ͨ�������֤
	//�Ѿ�ͨ���˵ģ�ֱ��ת�����ݣ�
	//û��ͨ���ģ�����������ݰ������Ϸ��������Ͽ�����
	//��ͨ����֤
	unsigned int id = Socket2Peer(iConn);

	if (0==id) //û��ͨ����֤
	{
		//��ʱ����һ���������Ƿ���ID�������򲻷���Э��淶
		if (pBasePack->pack_type!=TBasePack::PT_SENDID)
		{
			ASSERT(FALSE); //���ݰ����Ϸ���ɾ��֮��ע���Ժ���Զ��õ����ӶϿ���Ϣ
			UDT::close(iConn);
			return;
		}
		TPackSendId *pSendID = (TPackSendId *)pBasePack->data;

		//���û���û�ж�Ӧ�����Ӷ�����ڣ�û���򴴽�һ��
		NPeerPtr ptrPeer = GetPeerInstance(pSendID->uiUserId);
		if (!ptrPeer)
		{
			UDT::close(iConn); //GetPeerInstance�������NULL��˵���������µ�����
			return;
		}
		//������µ����ӽ���peerʹ��
		ptrPeer->SetCurrentSocket(iConn);

		//��������-->peerӳ���ϵ
		AddSockID(iConn,pSendID->uiUserId);

		std::cout<<iConn<<"�����ӵı��ص�ַ��"<<pSendID->addr.ToString()<<std::endl;

		//���ԣ���ȡ�Լ���ӳ���ַ
		//PeerSendGetAddrPack(pSendID->uiUserId,m_iMyId);
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	//�����Ѿ����������
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
			//�ҵ���Ӧ��NPeer
			NPeerPtr ptrPeer = FindPeer(id);
			if (!ptrPeer)
			{
				return;
			}
			ptrPeer->SetCurrentSocket(iConn); //����һ�µ�ǰsocket
			ptrPeer->OnPeerData((char *)pBasePack->data,iLen-sizeof(TBasePack));
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
			KIpv4Addr addr = GetPeerAddress(pGetAddr->uiPeerID);

			//���͵�ַ���ݰ�
			TBasePack *pPack = TPackAddrReply::Factory(pGetAddr->uiPeerID,addr);
			int iRes = UDT::sendmsg(iConn,(char *)pPack,TPackAddrReply::GetTotalLen());
			ASSERT(TPackAddrReply::GetTotalLen()==iRes);
			delete[] pPack;
			return;
		}
	case TBasePack::PT_ADDR_REPLY:
		{
			TPackAddrReply *pAddrReply = (TPackAddrReply *)pBasePack->data;
			std::cout<<pAddrReply->uiPeerID<<" ��ӳ���ַ��"<<pAddrReply->addrMap.ToString()<<std::endl;
			//if(pAddrReply->uiPeerID==GetId())
			//{
			//	std::cout<<"ӳ���ַ��"<<pAddrReply->addrMap.ToString()<<std::endl;
			//}
			//else
			//{
			//	std::cout<<"׼�����ӵ�ַ��"<<pAddrReply->addrMap.ToString()<<std::endl;
			//	OnAddressReturn(pData,iLen);
			//}
			return;
		}

	case TBasePack::PT_STUN_CONNECT:
		{
			//�յ���������������ô��˫������shoot���
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
			//�յ�������
			TPackShoot *pShoot = (TPackShoot *)pBasePack->data;

			std::cout<<"���Ŀ�꣺"<<pShoot->uiPeerID<<","<<pShoot->addrMap.ToString()<<std::endl;
			ASSERT(pShoot->uiPeerID!=0);
			ASSERT(pShoot->uiPeerID!=m_iMyId);
			ASSERT(pShoot->uiPeerID!=Socket2Peer(iConn));

			//����������ߣ���ô������ȥ����
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
//====�������ӷ�ʽ====
//ͨ��STUN��������ѯ��ַ����
bool  NPeerMng::PeerConnect(unsigned int iPeerId,int iWaitTime)
{
	if (!IsPeerReady(m_iServerPeer)) //��������û�����ӣ�
	{
		return false;
	}
	if(IsPeerReady(iPeerId))         //Ŀ���Ѿ����ӣ�
	{
		return false;
	}

	//����������Ͳ�ѯ��ַ�����ѯ������Զ��ص�OnAddressReturn����
	if(!PeerSendConnectHelp(m_iServerPeer,iPeerId))
	{
		return false;
	}

	//�ȴ�ָ����ʱ�䣬�����ӳɹ���
	DWORD dwBgn = GetTickCount();
	while (!IsPeerReady(iPeerId) && GetTickCount()<(dwBgn+iWaitTime))
	{
		Sleep(10);
	}
	return IsPeerReady(iPeerId);
}
//֪����ַ��ֱ������
bool  NPeerMng::PeerConnect(unsigned int iPeerId,const sockaddr* nameKnown, int namelen,bool bTrySTUN)
{
	return ConnectToEx(nameKnown,namelen);
}
//֪����ַ�͸����û�id��ֱ������
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
//���û�����TBasePack���ݰ�
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

	//����id���ݰ�
	TBasePack *pPack = TPackShoot::Factory(peer2,addr);
	int iRes = pPeer1->SendPack(pPack,TPackShoot::GetTotalLen());
	delete[] pPack;
	return TPackShoot::GetTotalLen()==iRes;
}

bool NPeerMng::PeerSendGetAddrPack(unsigned int iServerPeer,unsigned int peer_id)
{
	//����id���ݰ�
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
	//����id���ݰ�
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

	//��������Ѿ����������Դ���Ϣ
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
	//�������̻�������OnAddressReturnһ��

	ASSERT(pData);
	ASSERT(ilen>0);

	TPackShoot *pShoot = (TPackShoot *)((TBasePack *)pData)->data;

	if (pShoot->uiPeerID==0 || pShoot->uiPeerID==m_iMyId)
	{
		ASSERT(FALSE);
		return;
	}

	//��������Ѿ����������Դ���Ϣ
	if (IsPeerReady(pShoot->uiPeerID))
	{
		return;
	}

	if(pShoot->addrMap == KIpv4Addr())
	{
		std::cout<<"��׵�ַΪ�գ�"<<std::endl;
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
	UDTSOCKET u = m_Network.CreateSocket(true);		//����m_uSocket�����ⱻ��Ϊ�����ѽ���

	JThread *pThread = m_threadGroup.AddThread(
		new ThreadConnnectTo(this,0,m_iMyId,u,name,namelen)
		);

	pThread->Start();
	return true;
}
void NPeerMng::OnConnectExFinish(ThreadConnnectTo *pThread,int iRes)
{
	if (iRes==0) //���ӳɹ�
	{
		m_Network.AddConn(pThread->m_socket);
		OnConnectionMsg(pThread->m_socket,INetworkListener::NL_CODE_NEWCONN,0);
	}
	else
	{
		//����ʧ�ܣ���ô֪ͨ��
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
//�ر����ӡ��˺����ر���������������ӣ��ͷŶ�Ӧ����ָ��
void NPeerMng::ClosePeer(unsigned int iPeerId)
{
	//����m_mapSockID����������ӹر�
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

	//ɾ��m_mapIDPeer�еļ�¼����NPeer����
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

	if(1!=m_mapSockID.erase(u)) //�����ӱ���ɾ����
	{
		ASSERT(FALSE);
	}
}

void NPeerMng::RemoveIDPeer( unsigned int iPeerId )
{
	JMutexAutoLock autolock(m_IDPeerLock);

	m_mapIDPeer.erase(iPeerId);
}
