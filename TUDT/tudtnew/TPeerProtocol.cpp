//#include "StdAfx.h"
#include ".\tpeerprotocol.h"
#include "debug.h"

TBasePack * TBasePack::SetDefaultVal( TBasePack *pPack,emPACKTYPE type )
{
	pPack->flag = PEERPACK_FLAG;
	pPack->pack_type = type;
	return pPack;
}

char * TBasePack::GetPackType()
{
	switch(pack_type)
	{
		//连接建立
	case PT_SENDID : return "发送ID号";
	case PT_KEEPLIVE: return "保活数据包，15s发送一次";

		//身份验证：将来扩展用";
	case PT_IDVERIFY_GET : return "请求ID校验";
	case PT_IDVERIFY_REPLY : return "回复ID校验";
	case PT_IDVERIFY_RES : return "ID校验结果";

		//地址相关：STUN服务器用";
	case PT_ADDR_GETMAP : return "获取映射地址命令";
	case PT_ADDR_REPLY : return "映射地址返回";
	case PT_ADDR_ADDKNOWN : return "增加自己已知的地址";

		//用户数据
	case PT_DATA : return "用户数据";

		//STUN
	case PT_STUN_CONNECT : return "连接请求";
	case PT_STUN_SHOOT : return "打孔命令";
	case PT_STUN_RELAY : return "数据中转";
	}

	ASSERT(false);
	return "未知数据";
}
TBasePack * TPackSendId::Factory( T_PEERID uiUserId,const KIpv4Addr &myAddr,
								 emProtoType proto/*=PROTO_UDT*/,t_uint magic_code/*=0xFFFF*/ )
{
	TBasePack *pPack = (TBasePack *)new char[GetTotalLen()];
	TBasePack::SetDefaultVal(pPack,TBasePack::PT_SENDID);
	TPackSendId *pSendId = (TPackSendId *)pPack->data;
	pSendId->uiMagicCode = magic_code;
	pSendId->uiUserId = uiUserId;
	pSendId->addr = myAddr;
	pSendId->ucProtoType = (t_uchar)proto;
	pSendId->uiMagicCode = magic_code;

	return pPack;
}

int TPackSendId::GetTotalLen()
{
	return sizeof(TBasePack) + sizeof(TPackSendId);
}
TBasePack * TPackKeepLive::Factory()
{
	TBasePack *pPack = (TBasePack *)new char[GetTotalLen()];
	TBasePack::SetDefaultVal(pPack,TBasePack::PT_SENDID);
	return pPack;
}

int TPackKeepLive::GetTotalLen()
{
	return sizeof(TBasePack);
}

TBasePack * TPackData::Factory( const char *pData,int iLen )
{
	TBasePack *pPack = (TBasePack *)new char[GetTotalLen(iLen)];
	TBasePack::SetDefaultVal(pPack,TBasePack::PT_DATA);
	memcpy(pPack->data,pData,iLen);
	return pPack;
}

int TPackData::GetTotalLen( int iDataLen )
{
	return sizeof(TBasePack) + iDataLen;
}

TBasePack * TPackGetAddr::Factory(T_PEERID idPeer)
{
	TBasePack *pPack = (TBasePack *)new char[GetTotalLen()];
	TBasePack::SetDefaultVal(pPack,TBasePack::PT_ADDR_GETMAP);
	TPackGetAddr *pGetAddr = (TPackGetAddr *)pPack->data;
	pGetAddr->uiPeerID = idPeer;

	return pPack;
}

int TPackGetAddr::GetTotalLen()
{
	return sizeof(TBasePack) + sizeof(TPackGetAddr);
}

TBasePack * TPackAddrReply::Factory(T_PEERID idPeer,const KIpv4Addr &addr)
{
	TBasePack *pPack = (TBasePack *)new char[GetTotalLen()];
	TBasePack::SetDefaultVal(pPack,TBasePack::PT_ADDR_REPLY);
	TPackAddrReply *pAddrReply = (TPackAddrReply *)pPack->data;
	pAddrReply->uiPeerID = idPeer;
	pAddrReply->uiPeerType = 0;
	pAddrReply->addrMap = addr;

	return pPack;
}

int TPackAddrReply::GetTotalLen()
{
	return sizeof(TBasePack) + sizeof(TPackAddrReply);
}

TBasePack * TPackConnect::Factory( T_PEERID idPeer )
{
	TBasePack *pPack = (TBasePack *)new char[GetTotalLen()];
	TBasePack::SetDefaultVal(pPack,TBasePack::PT_STUN_CONNECT);
	TPackConnect *pStunConnect= (TPackConnect *)pPack->data;
	pStunConnect->uiPeerID = idPeer;

	return pPack;
}

int TPackConnect::GetTotalLen()
{
	return sizeof(TBasePack) + sizeof(TPackConnect);
}

TBasePack * TPackShoot::Factory( T_PEERID idPeer,const KIpv4Addr &addrMap,const KIpv4Addr &addrLocal)
{
	TBasePack *pPack = (TBasePack *)new char[GetTotalLen()];
	TBasePack::SetDefaultVal(pPack,TBasePack::PT_STUN_SHOOT);
	TPackShoot *pStunShoot = (TPackShoot *)pPack->data;
	pStunShoot->uiPeerID = idPeer;
	pStunShoot->addrMap = addrMap;
	pStunShoot->addrLocal = addrLocal;

	return pPack;
}

int TPackShoot::GetTotalLen()
{
	return sizeof(TBasePack) + sizeof(TPackShoot);
}