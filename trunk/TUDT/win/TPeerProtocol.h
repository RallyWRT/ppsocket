#pragma once
#include <winsock.h>
#include <string>

#pragma warning(disable:4200) //΢��Ǳ�׼��չ
#pragma pack(1)
// ��������
// Ԥ�������ݳ���
typedef unsigned __int8  t_uchar;	// ����Ϊһ�ֽڵ��޷�������
typedef unsigned __int16 t_ushort;	// ����Ϊ���ֽڵ��޷�������
typedef unsigned __int32 t_uint;	// ����Ϊ���ֽڵ��޷�������
typedef unsigned __int64 t_uword;	// ����Ϊ���ֽڵ��޷�������

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


// IPv4�ĵ�ַ��
struct KIpv4Addr
{
	t_uint uiIp;		// IP��ַ
	t_ushort usPort;	// �˿ں�

public:
	//=================================================
	//���캯��
	KIpv4Addr();
	KIpv4Addr(t_uint uiIp,t_ushort usPort);
	KIpv4Addr(char *strIp,t_ushort usPort);
	KIpv4Addr(SOCKADDR_IN addr);
	KIpv4Addr(const KIpv4Addr &addr);

	//����õ�ַ�ĸ��ֱ�ʾ��ʽ
	void GetAddr(SOCKADDR_IN &addr) const;
	void GetAddr(t_uint &uiIp,t_ushort &usPort) const;
	void GetAddr(std::string &strIp,t_ushort &usPort) const;
	std::string ToString() const;

	//������
	bool operator == (const KIpv4Addr &addr) const;
	bool operator != (const KIpv4Addr &addr) const;
	const KIpv4Addr & operator = (const KIpv4Addr &addr);
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
	static TBasePack *Factory(t_uint uiUserId,const KIpv4Addr &myAddr,
		TPackSendId::emProtoType proto=PROTO_UDT,t_uint magic_code=0xFFFF);
	static int GetTotalLen();

	t_uint uiMagicCode; //ħ�������Խ��м򵥵�У��
	t_uint uiUserId;    //�û�ID
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
	static TBasePack *Factory(t_uint idPeer);
	static int GetTotalLen();

	t_uint uiPeerID;
};

//��ѯ�û���ַ
struct TPackAddrReply
{
	static TBasePack *Factory(t_uint idPeer,const KIpv4Addr &addr);
	static int GetTotalLen();

	t_uint uiPeerID;    //�����ݰ��������peer
	t_uint uiPeerType;  //�������ͣ���Ϊ�գ�
	KIpv4Addr addrMap;  //��ַΪ����˵���Է�������
};

//���ض��û����ӣ����Ӹ�����
struct TPackConnect
{
	static TBasePack *Factory(t_uint idPeer);
	static int GetTotalLen();

	t_uint uiPeerID;
};

//�������
struct TPackShoot
{
	static TBasePack *Factory(t_uint idPeer,const KIpv4Addr &addrMap,const KIpv4Addr &addrLocal);
	static int GetTotalLen();

	t_uint uiPeerID;
	KIpv4Addr addrMap;

	KIpv4Addr addrLocal;
};

#pragma pack () //�ֽڶ��뷽ʽ���Ĭ��ֵ 