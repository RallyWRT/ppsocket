#pragma once
#include <winsock.h>
#include <string>
#include "TypeDef.h"
#include "KIpv4Addr.h"

#pragma warning(disable:4200) //微软非标准扩展
#pragma pack(1)
// 常数定义

//1字节的数据包头
struct TBasePack 
{
	//PEERPACK标志
	enum{
		PEERPACK_FLAG = 0X0A,
	};
	//数据包类型，取值范围是0～16。
	enum emPACKTYPE{
		//连接建立
		PT_SENDID = 0,        //发送ID号
		PT_KEEPLIVE = 1,      //保活数据包，15s发送一次

		//身份验证：将来扩展用
		PT_IDVERIFY_GET = 2,  //请求ID校验
		PT_IDVERIFY_REPLY = 3,//回复ID校验
		PT_IDVERIFY_RES = 4,  //ID校验结果

		//地址相关：STUN服务器用
		PT_ADDR_GETMAP = 5,   //获取映射地址命令
		PT_ADDR_REPLY = 6,    //映射地址返回
		PT_ADDR_ADDKNOWN = 7, //增加自己已知的地址

		//用户数据包
		PT_DATA = 8,

		//STUN
		PT_STUN_CONNECT = 9,  //连接请求
		PT_STUN_SHOOT = 10,   //打孔命令
		PT_STUN_RELAY = 11,   //数据中转
	};
	static TBasePack *SetDefaultVal(TBasePack *pPack,emPACKTYPE type);
	char *GetPackType();

	//数据真正开始的地方
	t_uchar flag:4;     //FLAG,必须等于PEERPACK_FLAG
	t_uchar pack_type:4;//见emPACKTYPE定义
	t_uchar data[];     //后续数据

};

struct TPackSendId 
{
	//可能的协议类型
	enum emProtoType{
		PROTO_UDT,
		PROTO_TCP,
		PROTO_HTTP,
	};
	enum{
		PS_MAGIC_CODE = 0XABAB,
	};
	static TBasePack *Factory(T_PEERID uiUserId,const KIpv4Addr &myAddr,
		TPackSendId::emProtoType proto=PROTO_UDT,t_uint magic_code=0xFFFF);
	static int GetTotalLen();

	t_uint uiMagicCode; //魔数，用以进行简单的校验
	T_PEERID uiUserId;    //用户ID
	t_uchar ucProtoType; //协议类型
	KIpv4Addr addr;     //地址
};

//保活
struct TPackKeepLive
{
	static TBasePack *Factory();
	static int GetTotalLen();
};

//传递用户数据
struct TPackData
{
	static TBasePack *Factory(const char *pData,int iLen);
	static int GetTotalLen(int iDataLen);
};

//查询用户地址
struct TPackGetAddr
{
	static TBasePack *Factory(T_PEERID idPeer);
	static int GetTotalLen();

	T_PEERID uiPeerID;
};

//查询用户地址
struct TPackAddrReply
{
	static TBasePack *Factory(T_PEERID idPeer,const KIpv4Addr &addr);
	static int GetTotalLen();

	T_PEERID uiPeerID;    //本数据包所代表的peer
	t_uint uiPeerType;  //网络类型（暂为空）
	KIpv4Addr addrMap;  //地址为空则说明对方不在线
};

//与特定用户连接（连接辅助）
struct TPackConnect
{
	static TBasePack *Factory(T_PEERID idPeer);
	static int GetTotalLen();

	T_PEERID uiPeerID;
};

//打孔命令
struct TPackShoot
{
	static TBasePack *Factory(T_PEERID idPeer,const KIpv4Addr &addrMap,const KIpv4Addr &addrLocal);
	static int GetTotalLen();

	T_PEERID uiPeerID;
	KIpv4Addr addrMap;

	KIpv4Addr addrLocal;
};

#pragma pack () //字节对齐方式设回默认值 