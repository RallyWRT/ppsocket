#pragma once
#include <map>
#include <set>
#include <boost/shared_ptr.hpp>

#include "NPeer.h"
#include "../win/TPeerProtocol.h"
#include "JThreadGroup.h"
#include "jmutex.h"

//指向NPeer的智能指针
typedef boost::shared_ptr<NPeer> NPeerPtr;

//2.NPeerMng类
//NPeerMng类从INetworkListener中继承，侦听所有的网络消息。
//每一个连接上来之后，先发送id，然后将它放到“未认证“集合中，等到确定对方id后，保存在socket-->id集合中。当有数据到达的时候，先根据id查找到相应NPeer，再将数据发送给NPeer。
//连接功能：
//a)根据IP直接连接：
//首先根据IP直接连接，发送ID，侦听ID。需要保存IP与ID预期表。如果对方ID与预期不一样，则报告连接错误。
//b)被动连接：
//对方连接后，根据对方提供的ID，找到相应NPeer类，发送新连接到达消息。如果没有相应NPeer类，则调用Factory函数创建一个。
//c）打孔连接：
//打孔连接流程同时使用a、b流程。当NPeer收到打孔命令时，简单发起根据IP直接连接即可。最多可能同时两个连接都建立。
class NPeerMng
	:public UDT::INetworkListener
{
public:
	NPeerMng(unsigned int iMyId, const sockaddr* name, int namelen);  //系统唯一的节点id和入口服务器地址
	virtual ~NPeerMng();

	//自己的id，在开始的时候可以是0。注意：ID只能更新一次
	bool UpdateId(unsigned int iMyId);
	unsigned int GetId();

	//与服务器建立连接
	void ConnectToServer(unsigned int uiServerID, const sockaddr* name, int namelen);
	//关闭与服务器的连接（节省服务器资源）
	void CloseServerConnection();
	//与服务器连接的状态
	bool IsServerConnectReady();

	//====多种连接方式====
	//通过STUN服务器查询地址连接
	bool  PeerConnect(unsigned int iPeerId,int iWaitTime=0);
	//知道地址，直接连接
	bool  PeerConnect(unsigned int iPeerId,const sockaddr* nameKnown, int namelen,bool bTrySTUN=false);
	//知道地址和辅助用户id，直接连接
	bool  PeerConnect(unsigned int iPeerId,const sockaddr* nameKnown, int namelen,unsigned int iHelperId,bool bTrySTUN=false); 

	//用户对象是否已创建（不管有没有连接上）
	bool  IsPeerExist(unsigned int iPeerId);
	//用户是否已经连接成功
	bool  IsPeerReady(unsigned int iPeerId);

	//获得已连接用户列表
	void GetPeerSet(std::set<unsigned int> &setPeers);
	//根据用户ID查找用户
	NPeerPtr FindPeer(unsigned int iPeerId);
	//获取用户地址。用户有多个连接的，返回当前用户地址
	bool GetPeerAddress( unsigned int iPeerId,sockaddr* name, int *namelen );
	//同上。如果用户没有连接，返回空地址
	KIpv4Addr GetPeerAddress(unsigned int iPeerId);

	//给用户发送用户数据（此函数效率较低，提倡用SendPeerPack）
	int SendPeerMsg( unsigned int iPeerId,const char *pData, int iLen );
	//给用户发送TBasePack数据包
	int SendPeerPack( unsigned int iPeerId,const TBasePack *pPack, int iLen );

	//获得用户对象实例
	//用户如果存在就返回，如果不存在就调用PeerFactory函数生成一个
	NPeerPtr GetPeerInstance(int iPeerId);
	//关闭连接。此函数关闭所有相关网络连接，释放对应智能指针
	void ClosePeer(unsigned int iPeerId);
private:
	//用户对象工厂函数：仅供重载使用
	virtual NPeerPtr PeerFactory(int iPeerId); 

	//基础网络设施
	UDT::P2PNetworkBase m_Network;

	//本地ID
	unsigned int m_iMyId;

	//服务器ID
	unsigned int m_iServerPeer;

	//从 UDTSOCKET 到用户id的映射表。
	typedef std::map<UDTSOCKET,unsigned int> T_mapSockID;
	T_mapSockID m_mapSockID;
	JMutex m_SockIDLock;
	void AddSockID(UDTSOCKET u,unsigned int iPeerId);
	void RemoveSockID(UDTSOCKET u);

	//从用户ID到NPeer的映射表
	typedef std::map<unsigned int, NPeerPtr> T_mapIDPeer;
	T_mapIDPeer m_mapIDPeer;
	JMutex m_IDPeerLock;
	void RemoveIDPeer(unsigned int iPeerId);

private:
	//INetworkListener的回调函数
	virtual void OnConnectionMsg(int iConn,enNetworkCode code,int param);
	virtual void OnData(int iConn,int iLen, const char *pData);

	//内部回调函数
	void OnAddressReturn(const char* pData, int ilen);
	void OnShootCmd(const char* pData, int ilen);

	//设置id-->NPeer的映射
	bool AddPeer(unsigned int iPeerId, NPeerPtr pPeer);

	//从用户的所有可用连接中，随便找一个出来
	//如果没有，返回UDT::INVALID_SOCK
	UDTSOCKET FindUserSocket(unsigned int iUserId);

	//找出连接id对应的用户ID号
	unsigned int Socket2Peer(UDTSOCKET u); //查找连接对应的用户id。如果用户不存在，则返回0.

	//一些发送数据包函数
	bool PeerSendShootCmd(unsigned int peer1,unsigned int peer2);
	bool PeerSendConnectHelp(unsigned int iServerPeer,unsigned int peer_id);
	bool PeerSendGetAddrPack(unsigned int iServerPeer,unsigned int peer_id);

private:
	//异步连接
	friend class ThreadConnnectTo;
	bool ConnectToEx(const sockaddr* name, int namelen);
	void OnConnectExFinish(ThreadConnnectTo *pThread,int iRes);
	JThreadGroup m_threadGroup; //用来进行异步连接的线程集合
};
