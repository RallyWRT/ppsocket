#pragma once
#include "udt.h"
#include "../src/common.h"
#include <map>

class TPeerMngImpl
	:public UDT::TConnectMng
{
public:
	TPeerMngImpl(UDT::TPeerMng *pPeerMngInterface,unsigned int iMyId,int iLocalPort);  //ϵͳΨһ�Ľڵ�id����ڷ�������ַ
	virtual ~TPeerMngImpl();
	//�ر�������������ӣ���ʡ��������Դ��
	void CloseServerConnection();
	void ConnectToServer(unsigned int uiServerID, const sockaddr* name, int namelen);

	//�Լ���id���ڿ�ʼ��ʱ�������0��ע�⣺IDֻ�ܸ���һ��
	bool UpdateId(unsigned int iMyId);
	unsigned int GetId();
	UDT::TPeer *GetServerPeer();

	UDT::TPeer *FindPeer(unsigned int iPeerId);
	void GetPeerSet(std::set<unsigned int> &setPeers);
	void AddPeer(unsigned int iPeerId,UDT::TPeer *pPeer);
	bool RemovePeer(unsigned int iPeerId,UDT::TPeer *pPeer);

	//�ص�����
	virtual UDT::TPeer *PeerFactory(UDTSOCKET u,int iPeerId);      //������������
	//virtual void OnNATDetected(int iNetworkType); //NAT���ͼ�����
protected:
	//�����������������˺����ᱻ�����Բ���һ��TConnection����
	UDT::TConnection *ConnFactory(UDTSOCKET u);
private:
	friend class UDT::TPeerMng;
	unsigned int m_iMyId;
	std::string m_strServer;
	UDT::TPeerMng *m_pPeerMngInterface;

	CGuardMutex m_PeerMapMutex;
	std::map<int,UDT::TPeer*> m_mapPeers;

	UDT::TPeer *m_connServer;
};