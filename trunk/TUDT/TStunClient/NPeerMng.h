#pragma once
#include <map>
#include <set>
#include <boost/shared_ptr.hpp>

#include "NPeer.h"
#include "../win/TPeerProtocol.h"
#include "JThreadGroup.h"
#include "jmutex.h"

//ָ��NPeer������ָ��
typedef boost::shared_ptr<NPeer> NPeerPtr;

//2.NPeerMng��
//NPeerMng���INetworkListener�м̳У��������е�������Ϣ��
//ÿһ����������֮���ȷ���id��Ȼ�����ŵ���δ��֤�������У��ȵ�ȷ���Է�id�󣬱�����socket-->id�����С��������ݵ����ʱ���ȸ���id���ҵ���ӦNPeer���ٽ����ݷ��͸�NPeer��
//���ӹ��ܣ�
//a)����IPֱ�����ӣ�
//���ȸ���IPֱ�����ӣ�����ID������ID����Ҫ����IP��IDԤ�ڱ�����Է�ID��Ԥ�ڲ�һ�����򱨸����Ӵ���
//b)�������ӣ�
//�Է����Ӻ󣬸��ݶԷ��ṩ��ID���ҵ���ӦNPeer�࣬���������ӵ�����Ϣ�����û����ӦNPeer�࣬�����Factory��������һ����
//c��������ӣ�
//�����������ͬʱʹ��a��b���̡���NPeer�յ��������ʱ���򵥷������IPֱ�����Ӽ��ɡ�������ͬʱ�������Ӷ�������
class NPeerMng
	:public UDT::INetworkListener
{
public:
	NPeerMng(unsigned int iMyId, const sockaddr* name, int namelen);  //ϵͳΨһ�Ľڵ�id����ڷ�������ַ
	virtual ~NPeerMng();

	//�Լ���id���ڿ�ʼ��ʱ�������0��ע�⣺IDֻ�ܸ���һ��
	bool UpdateId(unsigned int iMyId);
	unsigned int GetId();

	//���������������
	void ConnectToServer(unsigned int uiServerID, const sockaddr* name, int namelen);
	//�ر�������������ӣ���ʡ��������Դ��
	void CloseServerConnection();
	//����������ӵ�״̬
	bool IsServerConnectReady();

	//====�������ӷ�ʽ====
	//ͨ��STUN��������ѯ��ַ����
	bool  PeerConnect(unsigned int iPeerId,int iWaitTime=0);
	//֪����ַ��ֱ������
	bool  PeerConnect(unsigned int iPeerId,const sockaddr* nameKnown, int namelen,bool bTrySTUN=false);
	//֪����ַ�͸����û�id��ֱ������
	bool  PeerConnect(unsigned int iPeerId,const sockaddr* nameKnown, int namelen,unsigned int iHelperId,bool bTrySTUN=false); 

	//�û������Ƿ��Ѵ�����������û�������ϣ�
	bool  IsPeerExist(unsigned int iPeerId);
	//�û��Ƿ��Ѿ����ӳɹ�
	bool  IsPeerReady(unsigned int iPeerId);

	//����������û��б�
	void GetPeerSet(std::set<unsigned int> &setPeers);
	//�����û�ID�����û�
	NPeerPtr FindPeer(unsigned int iPeerId);
	//��ȡ�û���ַ���û��ж�����ӵģ����ص�ǰ�û���ַ
	bool GetPeerAddress( unsigned int iPeerId,sockaddr* name, int *namelen );
	//ͬ�ϡ�����û�û�����ӣ����ؿյ�ַ
	KIpv4Addr GetPeerAddress(unsigned int iPeerId);

	//���û������û����ݣ��˺���Ч�ʽϵͣ��ᳫ��SendPeerPack��
	int SendPeerMsg( unsigned int iPeerId,const char *pData, int iLen );
	//���û�����TBasePack���ݰ�
	int SendPeerPack( unsigned int iPeerId,const TBasePack *pPack, int iLen );

	//����û�����ʵ��
	//�û�������ھͷ��أ���������ھ͵���PeerFactory��������һ��
	NPeerPtr GetPeerInstance(int iPeerId);
	//�ر����ӡ��˺����ر���������������ӣ��ͷŶ�Ӧ����ָ��
	void ClosePeer(unsigned int iPeerId);
private:
	//�û����󹤳���������������ʹ��
	virtual NPeerPtr PeerFactory(int iPeerId); 

	//����������ʩ
	UDT::P2PNetworkBase m_Network;

	//����ID
	unsigned int m_iMyId;

	//������ID
	unsigned int m_iServerPeer;

	//�� UDTSOCKET ���û�id��ӳ���
	typedef std::map<UDTSOCKET,unsigned int> T_mapSockID;
	T_mapSockID m_mapSockID;
	JMutex m_SockIDLock;
	void AddSockID(UDTSOCKET u,unsigned int iPeerId);
	void RemoveSockID(UDTSOCKET u);

	//���û�ID��NPeer��ӳ���
	typedef std::map<unsigned int, NPeerPtr> T_mapIDPeer;
	T_mapIDPeer m_mapIDPeer;
	JMutex m_IDPeerLock;
	void RemoveIDPeer(unsigned int iPeerId);

private:
	//INetworkListener�Ļص�����
	virtual void OnConnectionMsg(int iConn,enNetworkCode code,int param);
	virtual void OnData(int iConn,int iLen, const char *pData);

	//�ڲ��ص�����
	void OnAddressReturn(const char* pData, int ilen);
	void OnShootCmd(const char* pData, int ilen);

	//����id-->NPeer��ӳ��
	bool AddPeer(unsigned int iPeerId, NPeerPtr pPeer);

	//���û������п��������У������һ������
	//���û�У�����UDT::INVALID_SOCK
	UDTSOCKET FindUserSocket(unsigned int iUserId);

	//�ҳ�����id��Ӧ���û�ID��
	unsigned int Socket2Peer(UDTSOCKET u); //�������Ӷ�Ӧ���û�id������û������ڣ��򷵻�0.

	//һЩ�������ݰ�����
	bool PeerSendShootCmd(unsigned int peer1,unsigned int peer2);
	bool PeerSendConnectHelp(unsigned int iServerPeer,unsigned int peer_id);
	bool PeerSendGetAddrPack(unsigned int iServerPeer,unsigned int peer_id);

private:
	//�첽����
	friend class ThreadConnnectTo;
	bool ConnectToEx(const sockaddr* name, int namelen);
	void OnConnectExFinish(ThreadConnnectTo *pThread,int iRes);
	JThreadGroup m_threadGroup; //���������첽���ӵ��̼߳���
};
