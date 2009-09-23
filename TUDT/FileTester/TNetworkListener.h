#pragma once
#include <set>
#include "udt.h"

class TNetworkListener :
	public UDT::INetworkListener
{
public:
	TNetworkListener(void);
	~TNetworkListener(void);
	
	//重载父类接口，接受消息
	void OnConnectionMsg(int iConn,enNetworkCode code,int param);
	void OnData(int iConn,int iLen, const char *pData);
private:
	std::set<unsigned int> m_setConns;
};
