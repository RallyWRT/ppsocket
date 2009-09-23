#include "StdAfx.h"
#include ".\tpeerconn.h"
#include "../src/debug.h"

TPeerConn::TPeerConn(UDT::TConnectMng *pMng,UDTSOCKET u)
: UDT::TConnection(pMng,u)
{
	m_iPeerId = 0;
}

TPeerConn::~TPeerConn(void)
{
}

void TPeerConn::OnConnected( bool bSuccess )
{

}

void TPeerConn::OnDisConnected()
{

}

void TPeerConn::OnData( const char* pData, int ilen )
{
	TBasePack *pPack = (TBasePack *)pData;
	if(pPack->flag!=TBasePack::PEERPACK_TYPE)
	{
		OnErrorPack(pData,ilen,"ERROR FLAG");
		return;
	}
	if (0==m_iPeerId) //初次连接，必须是PEER ID包
	{
		if(ilen != sizeof(TBasePack) + sizeof(TPackSendId))
		{
			OnErrorPack(pData,ilen,"ERROR SENDID pack size");
			return;
		}
		TPackSendId *pSendId = (TPackSendId *)pData;
		if (TPackSendId::PS_MAGIC_CODE!=pSendId->uiMagicCode)
		{
			OnErrorPack(pData,ilen,"Error SendId magic code");
			return;
		}
		m_iPeerId = pSendId->uiUserId;
		m_protoType = pSendId->proto_type;
		m_addr = pSendId->addr;
	}
}

void TPeerConn::OnErrorPack( const char* pData, int ilen ,const char *pErrorInfo)
{
	std::cout<<"OnErrorPack:"<<pErrorInfo<<",data="<<std::hex<<pData<<std::endl;
	ASSERT(FALSE);
	delete this;
}