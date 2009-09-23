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
	//����������Ϣ������
	void SetNetworkListener(UDT::INetworkListener *pListener);
	//��ĳ����ַ�������ӣ���������ɹ����ᴥ�� NL_CODE_NEWCONN ��Ϣ��
	void ShootTo(const sockaddr* name, int namelen);
	//�������ݡ�
	//�����첽���͡�
	//���ͳɹ��ᴥ��NL_CODE_SENDSUCCEED��Ϣ�����򴥷�NL_CODE_SENDFAILED��Ϣ��
	//��Ϣ��param����iMsgID��
	int SendTo(int iConn,int iLen, const char *pData);
	//�ر�����
	void CloseConn(int iConn);	
	//���ⲿ������socket�����������
	void AddConn(UDTSOCKET u);
	//��ȡ��������
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
	//���н��������ӵ�socket�б�
	std::set<UDTSOCKET> m_setConns;

private:
	void Init();

};
