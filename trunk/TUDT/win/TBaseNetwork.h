#pragma once
#include <set>
#include "tudt.h"
#include "MonitorGroup.h"
#include "../src/common.h"

class TBaseNetwork
{
public:
	TBaseNetwork(int iUDPPort);
	TBaseNetwork(const sockaddr* name, int namelen);
	~TBaseNetwork(void);

	int GetPort();
	//设置网络消息侦听器
	void SetNetworkListener(UDT::INetworkListener *pListener);
	//向某个地址尝试连接（连接如果成功，会触发 NL_CODE_NEWCONN 消息）
	void ShootTo(const sockaddr* name, int namelen);
	//发送数据。
	//这是异步发送。
	//发送成功会触发NL_CODE_SENDSUCCEED信息，否则触发NL_CODE_SENDFAILED消息。
	//消息的param就是iMsgID。
	int SendTo(int iConn,int iLen, const char *pData);
	//关闭连接
	void CloseConn(int iConn);	
	//将外部创建的socket放入这里管理
	void AddConn(UDTSOCKET u);
	//获取所有连接
	void GetConns(std::set<UDTSOCKET> &setConns);

	//===============================
	void AddConnection(UDTSOCKET socketNew);
	void RemoveConnection(UDTSOCKET socketNew);
	void ThreadWaitConn();
	void ThreadCheckData();
	UDTSOCKET CreateSocket(bool bRendezvous=true);
private:
	int m_iServicePort;
	sockaddr_in m_addrService;

	UDTSOCKET m_ServerSocket;
	UDT::INetworkListener *m_pListener;
	volatile bool m_bExiting;
	uintptr_t m_threadWaitConnection;
	uintptr_t m_threadCheckDataStart;

	CGuardMutex m_ConnMutex;
	//所有建立了连接的socket列表
	std::set<UDTSOCKET> m_setConns;

private:
	void Init();

};
