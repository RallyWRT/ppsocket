#pragma once
#include "udt.h"
#include "../win/TPeerProtocol.h"

class NPeerMng;

//1.NPeer类。
//NPeer类创建的时候，要向NPeerMng更新id-->NPeer集合。
//NPeer不直接进行，连接任务统一交给NPeerMng掌管。
//NPeer主要负责接收和转发消息。
//NPeer可以同时使用多个连接（目前仅限于UDT连接）。从各个连接发过来的数据，它都会接收并处理。应用层可自行设计一些算法来避免重复。注意，由于使用了多个连接，那么丢包也是有可能的。
//发送数据，则挑选最近有活动的一个连接，其余连接作为备用。
//当所有连接中断的时候，才会发出连接中断消息。
//NPeer退出的时候，所有相关连接都删除。
class NPeer
{
private:
	//不允许直接创建或删除对象。请用NPeerMng中的相关函数。
	//iPeerId是目的id。目的id为0表示可以连接任意peer，id等连接后再确定      
	NPeer(NPeerMng *pMng,unsigned int iPeerId);
public:

	virtual ~NPeer(void);
	//====多种连接方式====
	//通过STUN服务器查询地址连接
	bool  PeerConnect(int iWaitTime=0);       
	//知道地址，直接连接
	bool  PeerConnect(const sockaddr* nameKnown, int namelen,bool bTrySTUN=false);
	//知道地址和辅助用户id，直接连接
	bool  PeerConnect(const sockaddr* nameKnown, int namelen,int iHelperId,bool bTrySTUN=false); 

	unsigned int GetPeerId();
	int GetSpeed(int &iUpSpeed,int &iDownSpeed);
	bool IsReady();

	int  SendPeerMsg(const char* pData,int iLen);
	int  SendPack(const TBasePack *pPack,int iLen);
public:
	//回调函数
	virtual void  OnPeerConnected(bool bSuccess);           
	virtual void  OnPeerDisConnected();
	virtual void  OnPeerData(const char* pData, int ilen);
private:
	friend class NPeerMng;

	//更新当前连接
	//如果u为INVALID_SOCKET，那么说明连接断开
	void SetCurrentSocket(UDTSOCKET u);
	UDTSOCKET GetCurrentSocket();
	NPeerMng *GetPeerMng();
private:
	NPeerMng *m_pMng;
	unsigned int m_iPeerId;
	UDTSOCKET m_currSocket;
};
