
#ifndef __TUDT_H__
#define __TUDT_H__

#ifdef TUDT_EXPORTS
#define TUDT_API __declspec(dllexport)
#else
#define TUDT_API __declspec(dllimport)
#endif

//是否使用v4.4版本
//#define UDT_VERSION_4_4

#include "udt.h"
#include "TypeDef.h"

#include <boost/shared_ptr.hpp>
class TBaseNetwork;	
class Connection;
class ConnMng;
class NPeerMng;
struct TBasePack;
namespace UDT
{
	//====================================================
	//UDT扩充接口，方便做服务器和内网穿透
	//====================================================
	//added by tianzuo,2009-2-5
	//等待新的连接到来（任何地址）。返回新连接的句柄
	//UDT_API int waitconnect(UDTSOCKET u, const struct timeval* timeout);

	//====================================================
	//P2P网络层底层接口
	//不考虑用户ID和防火墙穿透功能
	//====================================================
	//网络消息侦听者
	class TUDT_API INetworkListener
	{
	public:
		virtual ~INetworkListener(){};
		enum enNetworkCode{
			NL_CODE_NEWCONN = 0,    //连接建立
			NL_CODE_BREAKDOWN = 1,  //连接中断
			NL_CODE_FAILED = 2,     //尝试连接失败，此时iConn为INVALIDSOCK,param为sockaddr*
			//NL_CODE_SENDSUCCEED = 2,//数据发送成功
			//NL_CODE_SENDFAILED = 3, //数据发送失败
		};
		virtual void OnConnectionMsg(int iConn,enNetworkCode code,int param) = 0;
		virtual void OnData(int iConn,int iLen, const char *pData) = 0;
	};
	class TUDT_API P2PNetworkBase
	{
	public:
		P2PNetworkBase(int iUDPPort);
		P2PNetworkBase(const sockaddr* name, int namelen);
		~P2PNetworkBase();
		int GetPort();
		//设置网络消息侦听器
		void SetNetworkListener(INetworkListener *pListener);
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

		//辅助函数
		UDTSOCKET CreateSocket(bool bRendezvous=true);

		//关闭退出。
		//此函数会在析构的时候自动调用
		void Close();
	private:
		TBaseNetwork *m_pBase;
	};

	//////////////////////////////////////////////////////////////////////////
	// TPeer封装
	//////////////////////////////////////////////////////////////////////////
	//不合法节点ID定义
	TUDT_API extern const T_PEERID INVALID_PEER_ID;

	//1.TPeer类。
	//TPeer类创建的时候，要向NPeerMng更新id-->TPeer集合。
	//TPeer不直接进行，连接任务统一交给NPeerMng掌管。
	//TPeer主要负责接收和转发消息。
	//TPeer可以同时使用多个连接（目前仅限于UDT连接）。从各个连接发过来的数据，它都会接收并处理。应用层可自行设计一些算法来避免重复。注意，由于使用了多个连接，那么丢包也是有可能的。
	//发送数据，则挑选最近有活动的一个连接，其余连接作为备用。
	//当所有连接中断的时候，才会发出连接中断消息。
	//TPeer退出的时候，所有相关连接都删除。
	class TPeerMng;
	class TUDT_API TPeer
	{
	public:
		//不允许直接创建或删除对象。请用NPeerMng中的相关函数。
		//iPeerId是目的id。目的id为INVALID_PEER_ID表示可以连接任意peer，id等连接后再确定      
		TPeer(TPeerMng *pMng,T_PEERID iPeerId);
	public:

		virtual ~TPeer(void);
		//====多种连接方式====
		//通过STUN服务器查询地址连接
		bool  PeerConnect(int iWaitTime=0);       
		//知道地址，直接连接
		bool  PeerConnect(const sockaddr* nameKnown, int namelen,bool bTrySTUN=false);
		//知道地址和辅助用户id，直接连接
		bool  PeerConnect(const sockaddr* nameKnown, int namelen,int iHelperId,bool bTrySTUN=false); 

		T_PEERID GetPeerId();
		int GetSpeed(int &iUpSpeed,int &iDownSpeed);
		bool IsReady();

		int  SendPeerMsg(const char* pData,int iLen);
		int  SendPack(const TBasePack *pPack,int iLen);
		void Close();
	public:
		//回调函数
		virtual void  OnPeerConnected(bool bSuccess);           
		virtual void  OnPeerDisConnected();
		virtual void  OnPeerData(const char* pData, int ilen);
	protected:
		friend class TPeerMng;
		friend class NPeerMng;

		//更新当前连接
		//如果u为INVALID_SOCKET，那么说明连接断开
		void SetCurrentSocket(UDTSOCKET u);
		UDTSOCKET GetCurrentSocket();
		TPeerMng *GetPeerMng();
	private:
		TPeerMng *m_pMng;
		T_PEERID m_iPeerId;
		UDTSOCKET m_currSocket;

		//客户端自己上报的地址（内网地址）
		unsigned int m_uiLocalIp;		// IP地址
		unsigned short m_usLocalPort;	// 端口号
	};

	//Peer消息侦听器
	class TUDT_API IPeerDataListener
	{
	public:
		//回调函数
		virtual void  OnPeerConnected(int iPeerId,bool bSuccess){iPeerId;bSuccess;};           
		virtual void  OnPeerDisConnected(int iPeerId){iPeerId;}	;
		virtual void  OnPeerData(T_PEERID iPeerId,const char* pData, int ilen) = 0;
	};

	//指向TPeer的智能指针
	typedef boost::shared_ptr<UDT::TPeer> TPeerPtr;
	class TUDT_API TPeerMng
	{
	public:
		TPeerMng(T_PEERID iMyId, const sockaddr* name, int namelen);  //系统唯一的节点id和入口服务器地址
		virtual ~TPeerMng();

		//自己的id，在开始的时候可以是INVALID_PEER_ID。注意：ID只能更新一次
		bool UpdateId(T_PEERID iMyId);
		T_PEERID GetId();

		//与服务器建立连接
		void ConnectToServer(T_PEERID uiServerID, const sockaddr* name, int namelen);
		//关闭与服务器的连接（节省服务器资源）
		void CloseServerConnection();
		//与服务器连接的状态
		bool IsServerConnectReady();

		//====多种连接方式====
		//通过STUN服务器查询地址连接
		bool  PeerConnect(T_PEERID iPeerId,int iWaitTime=0);
		//知道地址，直接连接
		bool  PeerConnect(T_PEERID iPeerId,const sockaddr* nameKnown, int namelen,bool bTrySTUN=false);
		//知道地址和辅助用户id，直接连接
		bool  PeerConnect(T_PEERID iPeerId,const sockaddr* nameKnown, int namelen,T_PEERID iHelperId,bool bTrySTUN=false); 

		//用户对象是否已创建（不管有没有连接上）
		bool  IsPeerExist(T_PEERID iPeerId);
		//用户是否已经连接成功
		bool  IsPeerReady(T_PEERID iPeerId);

		//获得已连接用户列表
		void GetPeerSet(std::set<T_PEERID> &setPeers);
		//根据用户ID查找用户
		TPeerPtr FindPeer(T_PEERID iPeerId);
		//获取用户地址。用户有多个连接的，返回当前用户地址
		bool GetPeerAddress( T_PEERID iPeerId,sockaddr* name, int *namelen );
		//同上。如果用户没有连接，返回空地址
		//KIpv4Addr GetPeerAddress(T_PEERID iPeerId);

		//给用户发送用户数据（此函数效率较低，提倡用SendPeerPack）
		int SendPeerMsg( T_PEERID iPeerId,const char *pData, int iLen );
		//给用户发送TBasePack数据包
		int SendPeerPack( T_PEERID iPeerId,const TBasePack *pPack, int iLen );

		//获得用户对象实例
		//用户如果存在就返回，如果不存在就调用PeerFactory函数生成一个
		TPeerPtr GetPeerInstance(T_PEERID iPeerId,int iTemplateType=0);
		//关闭连接。此函数关闭所有相关网络连接，释放对应智能指针
		void ClosePeer(T_PEERID iPeerId);

		//全局消息侦听器
		void AddPeerListener(IPeerDataListener *pListener);
		bool RemovePeerListener(const IPeerDataListener *pListener);

		//退出的时候会自动关闭
		void Close();
	private:
		friend class TPeer;
		friend class NPeerMng;
		NPeerMng *m_pPeerMngImpl;
	protected:
		//用户对象工厂函数：仅供重载使用
		virtual UDT::TPeerPtr PeerFactory(T_PEERID iPeerId,int iTemplateType)=0; 
	};

	template<class T_Conn>
	class TPeerMngTemplate
		:public TPeerMng
	{
	public:
		TPeerMngTemplate(T_PEERID iMyId, const sockaddr* name, int namelen):TPeerMng(iMyId,name,namelen){};
		virtual ~TPeerMngTemplate(){};
		UDT::TPeerPtr PeerFactory(T_PEERID iPeerId,int iTemplateType)
		{
			return UDT::TPeerPtr(new T_Conn(this,iPeerId));
		}
	};
}

#endif
