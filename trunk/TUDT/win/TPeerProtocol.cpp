//#include "StdAfx.h"
#include ".\tpeerprotocol.h"
#include "../src/debug.h"

KIpv4Addr::KIpv4Addr(t_uint uiIp,t_ushort usPort)
{
	this->uiIp = uiIp;
	this->usPort = usPort;
}
KIpv4Addr::KIpv4Addr(char *strIp,t_ushort usPort)
{
	this->uiIp = ntohl(inet_addr(strIp));
	this->usPort = usPort;
}
KIpv4Addr::KIpv4Addr(SOCKADDR_IN addr)
{
	this->uiIp = ntohl(addr.sin_addr.S_un.S_addr);
	this->usPort = ntohs(addr.sin_port);
}
KIpv4Addr::KIpv4Addr(const KIpv4Addr &addr)
{
	this->uiIp = addr.uiIp;
	this->usPort = addr.usPort;
}

KIpv4Addr::KIpv4Addr()
{
	uiIp = 0;
	usPort = 0;
}
void KIpv4Addr::GetAddr(SOCKADDR_IN &addr) const
{
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(uiIp);
	addr.sin_port = htons(usPort);
}
void KIpv4Addr::GetAddr(t_uint &uiIp,t_ushort &usPort) const
{
	uiIp = this->uiIp;
	usPort = this->usPort;
}
void KIpv4Addr::GetAddr(std::string &strIp,t_ushort &usPort) const
{
	SOCKADDR_IN addr;
	GetAddr(addr);
	strIp = inet_ntoa(addr.sin_addr);
	usPort = this->usPort;
}
bool KIpv4Addr::operator == (const KIpv4Addr &addr) const
{
	return (this->uiIp==addr.uiIp) && (this->usPort==addr.usPort);
}

bool KIpv4Addr::operator != (const KIpv4Addr &addr) const
{
	return (this->uiIp!=addr.uiIp) || (this->usPort!=addr.usPort);
}

const KIpv4Addr & KIpv4Addr::operator = (const KIpv4Addr &addr)
{
	this->uiIp = addr.uiIp;
	this->usPort = addr.usPort;
	return *this;
}
std::string KIpv4Addr::ToString() const
{
	std::string strIp;
	t_ushort usPort;
	GetAddr(strIp,usPort);
	char buf[16];
	itoa(usPort,buf,10);
	return strIp + ":" + buf;
}

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
		//���ӽ���
	case PT_SENDID : return "����ID��";
	case PT_KEEPLIVE: return "�������ݰ���15s����һ��";

		//�����֤��������չ��";
	case PT_IDVERIFY_GET : return "����IDУ��";
	case PT_IDVERIFY_REPLY : return "�ظ�IDУ��";
	case PT_IDVERIFY_RES : return "IDУ����";

		//��ַ��أ�STUN��������";
	case PT_ADDR_GETMAP : return "��ȡӳ���ַ����";
	case PT_ADDR_REPLY : return "ӳ���ַ����";
	case PT_ADDR_ADDKNOWN : return "�����Լ���֪�ĵ�ַ";

		//�û�����
	case PT_DATA : return "�û�����";

		//STUN
	case PT_STUN_CONNECT : return "��������";
	case PT_STUN_SHOOT : return "�������";
	case PT_STUN_RELAY : return "������ת";
	}

	ASSERT(false);
	return "δ֪����";
}
TBasePack * TPackSendId::Factory( t_uint uiUserId,const KIpv4Addr &myAddr,
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

TBasePack * TPackGetAddr::Factory(t_uint idPeer)
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

TBasePack * TPackAddrReply::Factory(t_uint idPeer,const KIpv4Addr &addr)
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

TBasePack * TPackConnect::Factory( t_uint idPeer )
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

TBasePack * TPackShoot::Factory( t_uint idPeer,const KIpv4Addr &addrMap,const KIpv4Addr &addrLocal)
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