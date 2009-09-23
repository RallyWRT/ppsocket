#pragma once
#include <map>
#include <set>
#include <boost/shared_ptr.hpp>


#include "tudt.h"
#include "TPeerProtocol.h"
#include "JThreadGroup.h"
#include "jmutex.h"

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
	NPeerMng(UDT::TPeerMng *pParentPeerMng,T_PEERID iMyId, const sockaddr* name, int namelen);  //ϵͳΨһ�Ľڵ�id����ڷ�������ַ
	virtual ~NPeerMng();

	//�Լ���id���ڿ�ʼ��ʱ�������0��ע�⣺IDֻ�ܸ���һ��
	bool UpdateId(T_PEERID iMyId);
	T_PEERID GetId();

	//���������������
	void ConnectToServer(T_PEERID uiServerID, const sockaddr* name, int namelen);
	//�ر�������������ӣ���ʡ��������Դ��
	void CloseServerConnection();
	//����������ӵ�״̬
	bool IsServerConnectReady();

	//====�������ӷ�ʽ====
	//ͨ��STUN��������ѯ��ַ����
	bool  PeerConnect(T_PEERID iPeerId,int iWaitTime=0);
	//֪����ַ��ֱ������
	bool  PeerConnect(T_PEERID iPeerId,const sockaddr* nameKnown, int namelen,bool bTrySTUN=false);
	//֪����ַ�͸����û�id��ֱ������
	bool  PeerConnect(T_PEERID iPeerId,const sockaddr* nameKnown, int namelen,T_PEERID iHelperId,bool bTrySTUN=false); 

	//�û������Ƿ��Ѵ�����������û�������ϣ�
	bool  IsPeerExist(T_PEERID iPeerId);
	//�û��Ƿ��Ѿ����ӳɹ�
	bool  IsPeerReady(T_PEERID iPeerId);

	//����������û��б�
	void GetPeerSet(std::set<T_PEERID> &setPeers);
	//�����û�ID�����û�
	UDT::TPeerPtr FindPeer(T_PEERID iPeerId);
	//��ȡ�û���ַ���û��ж�����ӵģ����ص�ǰ�û���ַ
	bool GetPeerAddress( T_PEERID iPeerId,sockaddr* name, int *namelen );
	//ͬ�ϡ�����û�û�����ӣ����ؿյ�ַ
	KIpv4Addr GetPeerAddress(T_PEERID iPeerId);

	//���û������û����ݣ��˺���Ч�ʽϵͣ��ᳫ��SendPeerPack��
	int SendPeerMsg( T_PEERID iPeerId,const char *pData, int iLen );
	//���û�����TBasePack���ݰ�
	int SendPeerPack( T_PEERID iPeerId,const TBasePack *pPack, int iLen );

	//����û�����ʵ��
	//�û�����������ھͷ��أ���������ھ͵���PeerFactory��������һ����ʹ��iTemplateType������
	//ע�⣺���صĶ������������Ĳ�̫һ��������û������Ѵ��ڵĻ�
	UDT::TPeerPtr GetPeerInstance(T_PEERID iPeerId,int iTemplateType);
	//�ر����ӡ��˺����ر���������������ӣ��ͷŶ�Ӧ����ָ��
	void ClosePeer(T_PEERID iPeerId);

private:
	//�û����󹤳���������������ʹ��
	virtual UDT::TPeerPtr PeerFactory(T_PEERID iPeerId,int iTemplateType); 

	//����������ʩ
	UDT::P2PNetworkBase m_Network;

	//����ID
	T_PEERID m_iMyId;

	//������ID
	T_PEERID m_iServerPeer;
	sockaddr m_addrServer;

	//�� UDTSOCKET ���û�id��ӳ���
	typedef std::map<UDTSOCKET,T_PEERID> T_mapSockID;
	T_mapSockID m_mapSockID;
	JMutex m_SockIDLock;
	void AddSockID(UDTSOCKET u,T_PEERID iPeerId);
	void RemoveSockID(UDTSOCKET u);

	//���û�ID��NPeer��ӳ���
	typedef std::map<T_PEERID, UDT::TPeerPtr> T_mapIDPeer;
	T_mapIDPeer m_mapIDPeer;
	JMutex m_IDPeerLock;
	void RemoveIDPeer(T_PEERID iPeerId);

	UDT::TPeerMng *m_pParentPeerMng;

	friend class UDT::TPeerMng;
	typedef std::set<UDT::IPeerDataListener*> T_setListener;
	T_setListener m_setPeerListener;
	JMutex m_setPeerListenerLock;
	////����������ڱ����У���ô�Լ��ϵ���ɾ����ֱ�ӽ��С��ȷŵ�����ȱ��������ִ�С�
	//std::vector<std::pair<bool,UDT::IPeerDataListener*> m_vecSetModified;
	//bool m_bUsingListenerSet;
	//ȫ����Ϣ������
	void AddPeerListener(UDT::IPeerDataListener *pListener);
	bool RemovePeerListener(const UDT::IPeerDataListener *pListener);
private:
	//INetworkListener�Ļص�����
	virtual void OnConnectionMsg(int iConn,enNetworkCode code,int param);
	virtual void OnData(int iConn,int iLen, const char *pData);

	//�ڲ��ص�����
	void OnAddressReturn(const char* pData, int ilen);
	void OnShootCmd(const char* pData, int ilen);

	friend class UDT::TPeer;
	//TPeer�ص�����
	void  OnPeerConnected(T_PEERID idPeer,bool bSuccess);
	void  OnPeerDisConnected(T_PEERID idPeer);

	//����id-->NPeer��ӳ��
	bool AddPeer(T_PEERID iPeerId, UDT::TPeerPtr pPeer);

	//���û������п��������У������һ������
	//���û�У�����UDT::INVALID_SOCK
	UDTSOCKET FindUserSocket(T_PEERID iUserId);

	//�ҳ�����id��Ӧ���û�ID��
	T_PEERID Socket2Peer(UDTSOCKET u); //�������Ӷ�Ӧ���û�id������û������ڣ��򷵻�0.

	//һЩ�������ݰ�����
	bool PeerSendShootCmd(T_PEERID peer1,T_PEERID peer2);
	bool PeerSendConnectHelp(T_PEERID iServerPeer,T_PEERID peer_id);
	bool PeerSendGetAddrPack(T_PEERID iServerPeer,T_PEERID peer_id);

private:
	//�첽����
	friend class ThreadConnnectTo;
	bool ConnectToEx(const sockaddr* name, int namelen);
	void OnConnectExFinish(ThreadConnnectTo *pThread,int iRes);
	JThreadGroup m_threadGroup; //���������첽���ӵ��̼߳���

private:
	bool GetServAddress( sockaddr *name, int &iLen);
};
