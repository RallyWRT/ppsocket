#pragma once
#include "udt.h"
#include "../src/common.h"
#include <map>

class TPeerMngImpl
	:public UDT::TConnectMng
{
public:
	TPeerMngImpl(UDT::TPeerMng *pPeerMngInterface,unsigned int iMyId,int iLocalPort);  //系统唯一的节点id和入口服务器地址
	virtual ~TPeerMngImpl();
	//关闭与服务器的连接（节省服务器资源）
	void CloseServerConnection();
	void ConnectToServer(unsigned int uiServerID, const sockaddr* name, int namelen);

	//自己的id，在开始的时候可以是0。注意：ID只能更新一次
	bool UpdateId(unsigned int iMyId);
	unsigned int GetId();
	UDT::TPeer *GetServerPeer();

	UDT::TPeer *FindPeer(unsigned int iPeerId);
	void GetPeerSet(std::set<unsigned int> &setPeers);
	void AddPeer(unsigned int iPeerId,UDT::TPeer *pPeer);
	bool RemovePeer(unsigned int iPeerId,UDT::TPeer *pPeer);

	//回调函数
	virtual UDT::TPeer *PeerFactory(UDTSOCKET u,int iPeerId);      //有新连接上来
	//virtual void OnNATDetected(int iNetworkType); //NAT类型检测完成
protected:
	//当有新连接上来，此函数会被调用以产生一个TConnection对象
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