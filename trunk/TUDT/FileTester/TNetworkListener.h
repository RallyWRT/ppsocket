#pragma once
#include <set>
#include "udt.h"

class TNetworkListener :
	public UDT::INetworkListener
{
public:
	TNetworkListener(void);
	~TNetworkListener(void);
	
	//���ظ���ӿڣ�������Ϣ
	void OnConnectionMsg(int iConn,enNetworkCode code,int param);
	void OnData(int iConn,int iLen, const char *pData);
private:
	std::set<unsigned int> m_setConns;
};
