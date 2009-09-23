#pragma once
#include "udt.h"
#include "../win/TPeerProtocol.h"

class NPeerMng;

//1.NPeer�ࡣ
//NPeer�ഴ����ʱ��Ҫ��NPeerMng����id-->NPeer���ϡ�
//NPeer��ֱ�ӽ��У���������ͳһ����NPeerMng�ƹܡ�
//NPeer��Ҫ������պ�ת����Ϣ��
//NPeer����ͬʱʹ�ö�����ӣ�Ŀǰ������UDT���ӣ����Ӹ������ӷ����������ݣ���������ղ�����Ӧ�ò���������һЩ�㷨�������ظ���ע�⣬����ʹ���˶�����ӣ���ô����Ҳ���п��ܵġ�
//�������ݣ�����ѡ����л��һ�����ӣ�����������Ϊ���á�
//�����������жϵ�ʱ�򣬲Żᷢ�������ж���Ϣ��
//NPeer�˳���ʱ������������Ӷ�ɾ����
class NPeer
{
private:
	//������ֱ�Ӵ�����ɾ����������NPeerMng�е���غ�����
	//iPeerId��Ŀ��id��Ŀ��idΪ0��ʾ������������peer��id�����Ӻ���ȷ��      
	NPeer(NPeerMng *pMng,unsigned int iPeerId);
public:

	virtual ~NPeer(void);
	//====�������ӷ�ʽ====
	//ͨ��STUN��������ѯ��ַ����
	bool  PeerConnect(int iWaitTime=0);       
	//֪����ַ��ֱ������
	bool  PeerConnect(const sockaddr* nameKnown, int namelen,bool bTrySTUN=false);
	//֪����ַ�͸����û�id��ֱ������
	bool  PeerConnect(const sockaddr* nameKnown, int namelen,int iHelperId,bool bTrySTUN=false); 

	unsigned int GetPeerId();
	int GetSpeed(int &iUpSpeed,int &iDownSpeed);
	bool IsReady();

	int  SendPeerMsg(const char* pData,int iLen);
	int  SendPack(const TBasePack *pPack,int iLen);
public:
	//�ص�����
	virtual void  OnPeerConnected(bool bSuccess);           
	virtual void  OnPeerDisConnected();
	virtual void  OnPeerData(const char* pData, int ilen);
private:
	friend class NPeerMng;

	//���µ�ǰ����
	//���uΪINVALID_SOCKET����ô˵�����ӶϿ�
	void SetCurrentSocket(UDTSOCKET u);
	UDTSOCKET GetCurrentSocket();
	NPeerMng *GetPeerMng();
private:
	NPeerMng *m_pMng;
	unsigned int m_iPeerId;
	UDTSOCKET m_currSocket;
};
