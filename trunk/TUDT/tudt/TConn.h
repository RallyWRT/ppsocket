#pragma once
#include "tudt.h"
#include "JMutex.h"
#include <set>
#include <map>
#include <iostream>
using namespace std;
class ConnMng:
	public UDT::INetworkListener
{
public:
	ConnMng(UDT::TConnectMng *pParentMng,int iPort);
	virtual ~ConnMng();

	void AddConn(UDT::TConnection *pConn);
	void RemoveConn(UDT::TConnection *pConn);

	void OnConnectionMsg(int iConn,enNetworkCode code,int param);
	void OnData(int iConn,int iLen, const char *pData);
	UDT::P2PNetworkBase *GetBase();

	//virtual Connection *ConnFactory(UDTSOCKET u);

private:
	JMutex m_Sock2ConnMutex;
	map<UDTSOCKET,UDT::TConnection *> m_mapSock2Conn;

	UDT::P2PNetworkBase *m_pBase;
	UDT::TConnectMng *m_pParentMng;

};