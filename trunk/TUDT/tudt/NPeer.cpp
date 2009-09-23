#include ".\npeer.h"
#include "debug.h"
#include "NPeerMng.h"
#include "TPeerProtocol.h"
#include <iostream>
using namespace std;
using namespace UDT;

const T_PEERID UDT::INVALID_PEER_ID = 0;

TPeer::TPeer(TPeerMng *pMng,T_PEERID iPeerId)
{
	ASSERT(NULL!=pMng);
	m_pMng = pMng;
	m_iPeerId = iPeerId;
	m_currSocket = UDT::INVALID_SOCK;

	//tianzuo,2009-6-19.��Щ�����ƶ���mng��Ĺ���������
	//��Mngע���Լ�
	//if(!m_pMng->AddPeer(iPeerId,this))
	//{
	//	throw std::bad_exception("TPeer existed.");
	//}
}
TPeer::~TPeer(void)
{
	//tianzuo,2009-6-19.��Щ�����ƶ���MNG�������������
	//�÷�����ɾ��������ڵ���ؼ�¼������������RemovePeer��
	//m_pMng->ClosePeer(m_iPeerId);
	//ASSERT(!m_pMng->FindPeer(m_iPeerId));
}

//====�������ӷ�ʽ====
//ͨ��STUN��������ѯ��ַ����
bool  TPeer::PeerConnect(int iWaitTime)   
{
	if (IsReady())
	{
		return true;
	}
	return m_pMng->PeerConnect(m_iPeerId,iWaitTime);
}
//֪����ַ��ֱ������
bool  TPeer::PeerConnect(const sockaddr* nameKnown, int namelen,bool bTrySTUN)
{
	if (IsReady())
	{
		return true;
	}
	return m_pMng->PeerConnect(m_iPeerId,nameKnown,namelen,bTrySTUN);
}
//֪����ַ�͸����û�id��ֱ������
bool  TPeer::PeerConnect(const sockaddr* nameKnown, int namelen,int iHelperId,bool bTrySTUN)
{
	if (IsReady())
	{
		return true;
	}
	return m_pMng->PeerConnect(m_iPeerId,nameKnown,namelen,iHelperId,bTrySTUN);
}

T_PEERID TPeer::GetPeerId()
{
	return m_iPeerId;
}
int TPeer::GetSpeed(int &iUpSpeed,int &iDownSpeed)
{
	iUpSpeed;iDownSpeed;
	return 0;
}
int  TPeer::SendPeerMsg(const char* pData,int iLen)
{
	if (IsReady())
	{
		TBasePack *pBasePack = TPackData::Factory(pData,iLen);
		int bRes = SendPack(pBasePack,TPackData::GetTotalLen(iLen));
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

int  TPeer::SendPack(const TBasePack *pPack,int iLen)
{
	return UDT::sendmsg(m_currSocket,(char *)pPack,iLen,-1,true);
}

bool TPeer::IsReady()
{
	//return UDT::INVALID_SOCK!=m_currSocket;
	return m_pMng->IsPeerReady(m_iPeerId);
}
TPeerMng *TPeer::GetPeerMng()
{
	return m_pMng;
}

void TPeer::Close()
{
	//Ĭ��ɾ������
	m_pMng->ClosePeer(m_iPeerId);
}

void  TPeer::OnPeerConnected(bool bSuccess)
{
	if (bSuccess)
	{
		//char buf[1024];
		//sprintf(buf,"%d Online\n",m_iPeerId);
		//std::cout<<buf;
	}
	else
	{
		//char buf[1024];
		//sprintf(buf,"%d failed\n",m_iPeerId);
		//std::cout<<buf;
	}
	
}
void  TPeer::OnPeerDisConnected()
{
	//char buf[1024];
	//sprintf(buf,"%d disconnected\n",m_iPeerId);
	//std::cout<<buf;
	Close();
}
void  TPeer::OnPeerData(const char* pData, int ilen)
{
	pData;ilen;
	//char buf[1024];
	//sprintf(buf,"%d said(%d):%s\n",m_iPeerId,ilen, pData);
	//std::cout<<buf;

	//std::cout<<m_iPeerId<<" ˵��"<<pData<<endl;
}

void TPeer::SetCurrentSocket(UDTSOCKET u)
{
	if (m_currSocket==u)
	{
		return;
	}


	UDTSOCKET uOld = m_currSocket;
	m_currSocket = u;

	//socket�Ӳ����ñ�Ϊ���ã�˵�����ӳɹ�
	if (uOld==UDT::INVALID_SOCK && u!=UDT::INVALID_SOCK)
	{
		m_pMng->m_pPeerMngImpl->OnPeerConnected(m_iPeerId,true);
		OnPeerConnected(true);
	}
	//socket�ӿ��ñ�Ϊ�����ã�˵�����ӶϿ�
	else if (uOld!=UDT::INVALID_SOCK && u==UDT::INVALID_SOCK)
	{
		m_pMng->m_pPeerMngImpl->OnPeerDisConnected(m_iPeerId);
		OnPeerDisConnected();
	}

}
UDTSOCKET TPeer::GetCurrentSocket()
{
	return m_currSocket;
}