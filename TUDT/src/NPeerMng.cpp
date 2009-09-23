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
TPeerMng::TPeerMng(unsigned int iMyId, const sockaddr* name, int namelen)  //ϵͳΨһ�Ľڵ�id����ڷ�������ַ
{
	m_pPeerMngImpl = new NPeerMng(this,iMyId,name,namelen);
}
TPeerMng::~TPeerMng()
{
	delete m_pPeerMngImpl;
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


//���������������
void TPeerMng::ConnectToServer(unsigned int uiServerID, const sockaddr* name, int namelen)
{
	m_pPeerMngImpl->ConnectToServer(uiServerID,name,namelen);
}
//�ر�������������ӣ���ʡ��������Դ��
void TPeerMng::CloseServerConnection()
{
	m_pPeerMngImpl->CloseServerConnection();
}
//����������ӵ�״̬
bool TPeerMng::IsServerConnectReady()
{
	return m_pPeerMngImpl->IsServerConnectReady();
}

//====�������ӷ�ʽ====
//ͨ��STUN��������ѯ��ַ����
bool  TPeerMng::PeerConnect(unsigned int iPeerId,int iWaitTime)
{
	return m_pPeerMngImpl->PeerConnect(iPeerId,iWaitTime);
}
//֪����ַ��ֱ������
bool  TPeerMng::PeerConnect(unsigned int iPeerId,const sockaddr* nameKnown, int namelen,bool bTrySTUN)
{
	return m_pPeerMngImpl->PeerConnect(iPeerId,nameKnown, namelen,bTrySTUN);
}
//֪����ַ�͸����û�id��ֱ������
bool  TPeerMng::PeerConnect(unsigned int iPeerId,const sockaddr* nameKnown, int namelen,unsigned int iHelperId,bool bTrySTUN)
{
	return m_pPeerMngImpl->PeerConnect(iPeerId,nameKnown, namelen,iHelperId,bTrySTUN);
}

//�û������Ƿ��Ѵ�����������û�������ϣ�
bool  TPeerMng::IsPeerExist(unsigned int iPeerId)
{
	return m_pPeerMngImpl->IsPeerExist(iPeerId);
}
//�û��Ƿ��Ѿ����ӳɹ�
bool  TPeerMng::IsPeerReady(unsigned int iPeerId)
{
	return m_pPeerMngImpl->IsPeerReady(iPeerId);
}

//����������û��б�
void TPeerMng::GetPeerSet(std::set<unsigned int> &setPeers)
{
	m_pPeerMngImpl->GetPeerSet(setPeers);
}

//�����û�ID�����û�
TPeerPtr TPeerMng::FindPeer(unsigned int iPeerId)
{
	return m_pPeerMngImpl->FindPeer(iPeerId);
}
//��ȡ�û���ַ���û��ж�����ӵģ����ص�ǰ�û���ַ
bool TPeerMng::GetPeerAddress( unsigned int iPeerId,sockaddr* name, int *namelen )
{
	return m_pPeerMngImpl->GetPeerAddress(iPeerId,name,namelen);
}
////ͬ�ϡ�����û�û�����ӣ����ؿյ�ַ
//KIpv4Addr TPeerMng::GetPeerAddress(unsigned int iPeerId)
//{
//	return m_pPeerMngImpl->GetPeerAddress(iPeerId);
//}

//���û������û����ݣ��˺���Ч�ʽϵͣ��ᳫ��SendPeerPack��
int TPeerMng::SendPeerMsg( unsigned int iPeerId,const char *pData, int iLen )
{
	return m_pPeerMngImpl->SendPeerMsg(iPeerId,pData,iLen);
}
//���û�����TBasePack���ݰ�
int TPeerMng::SendPeerPack( unsigned int iPeerId,const TBasePack *pPack, int iLen )
{
	return m_pPeerMngImpl->SendPeerPack(iPeerId,pPack,iLen);
}

//����û�����ʵ��
//�û�������ھͷ��أ���������ھ͵���PeerFactory��������һ��
TPeerPtr TPeerMng::GetPeerInstance(int iPeerId,int iTemplateType)
{
	return m_pPeerMngImpl->GetPeerInstance(iPeerId,iTemplateType);
}
//�ر����ӡ��˺����ر���������������ӣ��ͷŶ�Ӧ����ָ��
void TPeerMng::ClosePeer(unsigned int iPeerId)
{
	return m_pPeerMngImpl->ClosePeer(iPeerId);
}
//�û����󹤳���������������ʹ��
//TPeerPtr TPeerMng::PeerFactory(int iPeerId,int iTemplateType)
//{
//	return TPeerPtr(new TPeer(this,iPeerId));
//}
//ȫ����Ϣ������
void TPeerMng::AddPeerListener(IPeerDataListener *pListener)
{
	m_pPeerMngImpl->AddPeerListener(pListener);
}
bool TPeerMng::RemovePeerListener(const IPeerDataListener *pListener)
{
	return m_pPeerMngImpl->RemovePeerListener(pListener);
}
//////////////////////////////////////////////////////////////////////////

NPeerMng::NPeerMng(TPeerMng *pParentPeerMng,unsigned int iMyId, const sockaddr* name, int namelen)  //ϵͳΨһ�Ľڵ�id����ڷ�������ַ
:m_Network(name,namelen),m_iMyId(iMyId),m_pParentPeerMng(pParentPeerMng)
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

	m_iServerPeer = INVALID_PEER_ID;
}
bool NPeerMng::IsServerConnectReady()
{
	return IsPeerReady(m_iServerPeer);
}

//�ص�����
TPeerPtr NPeerMng::PeerFactory(int iPeerId,int iTemplateType)      //������������
{
	return m_pParentPeerMng->PeerFactory(iPeerId,iTemplateType);
}


TPeerPtr NPeerMng::GetPeerInstance(int iPeerId,int iTemplateType)
{
	//����Ѿ���ʵ���ˣ�ֱ��ʹ����
	TPeerPtr ptrPeer = FindPeer(iPeerId);
	if (ptrPeer)
	{
		return ptrPeer;
	}

	//û��ʵ��������һ��
	ptrPeer = PeerFactory(iPeerId,iTemplateType);

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
			if(INVALID_PEER_ID!=Socket2Peer(iConn)) //UDT���ӽ���ǰ����Ӧ����TPeer����
			{
				ASSERT(FALSE);
				return;
			}

			//////////////////////////////////////////////////////////////////////////
			//����������������ID��
			//////////////////////////////////////////////////////////////////////////

			sockaddr saPeer,saSend;  //����ip��peer�˿ں���������
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
	case NL_CODE_BREAKDOWN:  //�����ж�
		{
			//���ӶϿ��������ҵ���Ӧ�û�������ǲ��Ǹ��û��ĵ�ǰ����
			//����ǣ���ô��û�б������ӿ��ã�û�оͷ������ӶϿ���Ϣ
			unsigned int id = Socket2Peer(iConn); //��һ��������RemoveSockID.erase֮ǰ
			RemoveSockID(iConn);

			if (INVALID_PEER_ID==id)
			{
				std::cout<<"���棺�����жϣ����Ҳ������Ӷ�Ӧ��peer_id"<<std::endl;
				return;
			}
			//�ҵ���Ӧ��TPeer
			TPeerPtr ptrPeer = FindPeer(id);
			if (!ptrPeer)
			{
				std::cout<<"���棺�����жϣ����Ҳ������Ӷ�Ӧ��TPeer��id="<<id<<std::endl;
				return;
			}
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

	if (INVALID_PEER_ID==id) //û��ͨ����֤
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
		TPeerPtr ptrPeer = GetPeerInstance(pSendID->uiUserId,0);
		if (!ptrPeer)
		{
			UDT::close(iConn); //GetPeerInstance�������NULL��˵���������µ�����
			return;
		}
		//������µ����ӽ���peerʹ��
		ptrPeer->SetCurrentSocket(iConn);

		//�����û���������ַ
		pSendID->addr.GetAddr(ptrPeer->m_uiLocalIp,ptrPeer->m_usLocalPort);

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
			//�ҵ���Ӧ��TPeer
			TPeerPtr ptrPeer = FindPeer(id);
			if (!ptrPeer)
			{
				return;
			}
			ptrPeer->SetCurrentSocket(iConn); //����һ�µ�ǰsocket
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
	case TBasePack::PT_ADDR_GETMAP: //��ѯpeer�ĵ�ַ
		{
			TPackGetAddr *pGetAddr = (TPackGetAddr *)pBasePack->data;
			if (INVALID_PEER_ID==pGetAddr->uiPeerID) //peer id����Ϊ0
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

			ASSERT(pConnectHelp->uiPeerID!=INVALID_PEER_ID);
			ASSERT(pConnectHelp->uiPeerID!=iPeerId);
			ASSERT(iPeerId!=INVALID_PEER_ID);

			PeerSendShootCmd(pConnectHelp->uiPeerID,iPeerId);
			PeerSendShootCmd(iPeerId,pConnectHelp->uiPeerID);
			return;
		}
	case TBasePack::PT_STUN_SHOOT:
		{
			//�յ�������
			TPackShoot *pShoot = (TPackShoot *)pBasePack->data;

			ASSERT(pShoot->uiPeerID!=INVALID_PEER_ID);
			ASSERT(pShoot->uiPeerID!=m_iMyId);
			ASSERT(pShoot->uiPeerID!=Socket2Peer(iConn));

			//����������ߣ���ô������ȥ����
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
	ASSERT(FALSE);//��ûʵ��
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
//���û�����TBasePack���ݰ�
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
	//peer1���������Ҳ�Ͳ��ü�����
	TPeerPtr pPeer1 = FindPeer(peer1);
	if (!pPeer1)
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	//���Ŀ���ַ
	KIpv4Addr addrMap = GetPeerAddress(peer2);
	KIpv4Addr addrLocal;
	TPeerPtr pPeer2 = FindPeer(peer2);
	if (pPeer2)
	{
		addrLocal = KIpv4Addr(pPeer2->m_uiLocalIp,pPeer2->m_usLocalPort);
	}

	//////////////////////////////////////////////////////////////////////////
	//����id���ݰ�
	TBasePack *pPack = TPackShoot::Factory(peer2,addrMap,addrLocal);
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
	if(peer_id==INVALID_PEER_ID || peer_id==GetId())
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
	if(m_iMyId==pAddrReply->uiPeerID)
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

	if (pShoot->uiPeerID==INVALID_PEER_ID || pShoot->uiPeerID==m_iMyId)
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
		std::cout<<"���ʧ�ܣ��û������ߣ�"<<pShoot->uiPeerID<<std::endl;
		return;
	}
	SOCKADDR_IN sa;
	pShoot->addrMap.GetAddr(sa);
	std::cout<<"���Ŀ�꣺"<<pShoot->uiPeerID<<","<<pShoot->addrMap.ToString()<<std::endl;
	ConnectToEx((sockaddr *)&sa,sizeof(sa));

	pShoot->addrLocal.GetAddr(sa);
	std::cout<<"���Ŀ�꣺"<<pShoot->uiPeerID<<","<<pShoot->addrLocal.ToString()<<std::endl;
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
		new ThreadConnnectTo(this,INVALID_PEER_ID,m_iMyId,u,name,namelen)
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
	TPeerPtr pPeer = FindPeer(iPeerId);
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

	//ɾ��m_mapIDPeer�еļ�¼����TPeer����
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

	if(1!=m_mapSockID.erase(u)) //�����ӱ���ɾ����
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
	//ȡ�ñ��ص�ip��ַ
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