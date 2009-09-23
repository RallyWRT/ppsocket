#pragma once
#include "udt.h"
#include "TPeerProtocol.h"

class TPeerConn :
	public UDT::TConnection
{
public:
	TPeerConn(UDT::TConnectMng *pMng,UDTSOCKET u=UDT::INVALID_SOCK);
	virtual ~TPeerConn(void);

	void  OnConnected(bool bSuccess);
	void  OnDisConnected();
	void  OnData(const char* pData, int ilen);

	//virtual void OnPeerConnect();
private:
	int m_iPeerId;
	t_uchar m_protoType;
	KIpv4Addr m_addr;

private:
	void  OnErrorPack(const char* pData, int ilen,const char *pErrorInfo);
};
