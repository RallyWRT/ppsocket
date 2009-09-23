#pragma once
#include <winsock.h>
#include <string>
#include "TypeDef.h"
#include "KIpv4Addr.h"

#pragma warning(disable:4200) //΢��Ǳ�׼��չ
#pragma pack(1)
// ��������

//1�ֽڵ����ݰ�ͷ
struct TBasePack 
{
	//PEERPACK��־
	enum{
		PEERPACK_FLAG = 0X0A,
	};
	//���ݰ����ͣ�ȡֵ��Χ��0��16��
	enum emPACKTYPE{
		//���ӽ���
		PT_SENDID = 0,        //����ID��
		PT_KEEPLIVE = 1,      //�������ݰ���15s����һ��

		//�����֤��������չ��
		PT_IDVERIFY_GET = 2,  //����IDУ��
		PT_IDVERIFY_REPLY = 3,//�ظ�IDУ��
		PT_IDVERIFY_RES = 4,  //IDУ����

		//��ַ��أ�STUN��������
		PT_ADDR_GETMAP = 5,   //��ȡӳ���ַ����
		PT_ADDR_REPLY = 6,    //ӳ���ַ����
		PT_ADDR_ADDKNOWN = 7, //�����Լ���֪�ĵ�ַ

		//�û����ݰ�
		PT_DATA = 8,

		//STUN
		PT_STUN_CONNECT = 9,  //��������
		PT_STUN_SHOOT = 10,   //�������
		PT_STUN_RELAY = 11,   //������ת
	};
	static TBasePack *SetDefaultVal(TBasePack *pPack,emPACKTYPE type);
	char *GetPackType();

	//����������ʼ�ĵط�
	t_uchar flag:4;     //FLAG,�������PEERPACK_FLAG
	t_uchar pack_type:4;//��emPACKTYPE����
	t_uchar data[];     //��������

};

struct TPackSendId 
{
	//���ܵ�Э������
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

	t_uint uiMagicCode; //ħ�������Խ��м򵥵�У��
	T_PEERID uiUserId;    //�û�ID
	t_uchar ucProtoType; //Э������
	KIpv4Addr addr;     //��ַ
};

//����
struct TPackKeepLive
{
	static TBasePack *Factory();
	static int GetTotalLen();
};

//�����û�����
struct TPackData
{
	static TBasePack *Factory(const char *pData,int iLen);
	static int GetTotalLen(int iDataLen);
};

//��ѯ�û���ַ
struct TPackGetAddr
{
	static TBasePack *Factory(T_PEERID idPeer);
	static int GetTotalLen();

	T_PEERID uiPeerID;
};

//��ѯ�û���ַ
struct TPackAddrReply
{
	static TBasePack *Factory(T_PEERID idPeer,const KIpv4Addr &addr);
	static int GetTotalLen();

	T_PEERID uiPeerID;    //�����ݰ��������peer
	t_uint uiPeerType;  //�������ͣ���Ϊ�գ�
	KIpv4Addr addrMap;  //��ַΪ����˵���Է�������
};

//���ض��û����ӣ����Ӹ�����
struct TPackConnect
{
	static TBasePack *Factory(T_PEERID idPeer);
	static int GetTotalLen();

	T_PEERID uiPeerID;
};

//�������
struct TPackShoot
{
	static TBasePack *Factory(T_PEERID idPeer,const KIpv4Addr &addrMap,const KIpv4Addr &addrLocal);
	static int GetTotalLen();

	T_PEERID uiPeerID;
	KIpv4Addr addrMap;

	KIpv4Addr addrLocal;
};

#pragma pack () //�ֽڶ��뷽ʽ���Ĭ��ֵ 