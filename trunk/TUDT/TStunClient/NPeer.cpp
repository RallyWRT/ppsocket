#include ".\npeer.h"
#include "../src/debug.h"
#include "NPeerMng.h"
#include <iostream>
using namespace std;

NPeer::NPeer(NPeerMng *pMng,unsigned int iPeerId)
{
	ASSERT(NULL!=pMng);
	m_pMng = pMng;
	m_iPeerId = iPeerId;
	m_currSocket = UDT::INVALID_SOCK;

	//tianzuo,2009-6-19.��Щ�����ƶ���mng��Ĺ���������
	//��Mngע���Լ�
	//if(!m_pMng->AddPeer(iPeerId,this))
	//{
	//	throw std::bad_exception("NPeer existed.");
	//}
}
NPeer::~NPeer(void)
{
	//tianzuo,2009-6-19.��Щ�����ƶ���MNG�������������
	//�÷�����ɾ��������ڵ���ؼ�¼������������RemovePeer��
	//m_pMng->ClosePeer(m_iPeerId);
}

//====�������ӷ�ʽ====
//ͨ��STUN��������ѯ��ַ����
bool  NPeer::PeerConnect(int iWaitTime)   
{
	if (IsReady())
	{
		return true;
	}
	return m_pMng->PeerConnect(m_iPeerId,iWaitTime);
}
//֪����ַ��ֱ������
bool  NPeer::PeerConnect(const sockaddr* nameKnown, int namelen,bool bTrySTUN)
{
	if (IsReady())
	{
		return true;
	}
	return m_pMng->PeerConnect(m_iPeerId,nameKnown,namelen,bTrySTUN);
}
//֪����ַ�͸����û�id��ֱ������
bool  NPeer::PeerConnect(const sockaddr* nameKnown, int namelen,int iHelperId,bool bTrySTUN)
{
	if (IsReady())
	{
		return true;
	}
	return m_pMng->PeerConnect(m_iPeerId,nameKnown,namelen,iHelperId,bTrySTUN);
}

unsigned int NPeer::GetPeerId()
{
	return m_iPeerId;
}
int NPeer::GetSpeed(int &iUpSpeed,int &iDownSpeed)
{
	return 0;
}
int  NPeer::SendPeerMsg(const char* pData,int iLen)
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

int  NPeer::SendPack(const TBasePack *pPack,int iLen)
{
	return UDT::sendmsg(m_currSocket,(char *)pPack,iLen,-1,true);
}

bool NPeer::IsReady()
{
	return UDT::INVALID_SOCK!=m_currSocket;
}
NPeerMng *NPeer::GetPeerMng()
{
	return m_pMng;
}

void  NPeer::OnPeerConnected(bool bSuccess)
{
	if (bSuccess)
	{
		std::cout<<m_iPeerId<<" ���ߡ�"<<endl;
	}
	else
	{
		std::cout<<m_iPeerId<<" ����ʧ��"<<endl;
	}
	
}
void  NPeer::OnPeerDisConnected()
{
	std::cout<<m_iPeerId<<" �Ͽ����ӡ�"<<endl;

	//Ĭ��ɾ������
	m_pMng->ClosePeer(m_iPeerId);
}
void  NPeer::OnPeerData(const char* pData, int ilen)
{
	std::cout<<m_iPeerId<<" ˵��"<<pData<<endl;
}

void NPeer::SetCurrentSocket(UDTSOCKET u)
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
		OnPeerConnected(true);
	}
	//socket�ӿ��ñ�Ϊ�����ã�˵�����ӶϿ�
	else if (uOld!=UDT::INVALID_SOCK && u==UDT::INVALID_SOCK)
	{
		OnPeerDisConnected();
	}

}
UDTSOCKET NPeer::GetCurrentSocket()
{
	return m_currSocket;
}