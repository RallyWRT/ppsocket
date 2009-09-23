
#ifndef __TUDT_H__
#define __TUDT_H__

#ifdef TUDT_EXPORTS
#define TUDT_API __declspec(dllexport)
#else
#define TUDT_API __declspec(dllimport)
#endif

//�Ƿ�ʹ��v4.4�汾
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
	//UDT����ӿڣ���������������������͸
	//====================================================
	//added by tianzuo,2009-2-5
	//�ȴ��µ����ӵ������κε�ַ�������������ӵľ��
	//UDT_API int waitconnect(UDTSOCKET u, const struct timeval* timeout);

	//====================================================
	//P2P�����ײ�ӿ�
	//�������û�ID�ͷ���ǽ��͸����
	//====================================================
	//������Ϣ������
	class TUDT_API INetworkListener
	{
	public:
		virtual ~INetworkListener(){};
		enum enNetworkCode{
			NL_CODE_NEWCONN = 0,    //���ӽ���
			NL_CODE_BREAKDOWN = 1,  //�����ж�
			NL_CODE_FAILED = 2,     //��������ʧ�ܣ���ʱiConnΪINVALIDSOCK,paramΪsockaddr*
			//NL_CODE_SENDSUCCEED = 2,//���ݷ��ͳɹ�
			//NL_CODE_SENDFAILED = 3, //���ݷ���ʧ��
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
		//����������Ϣ������
		void SetNetworkListener(INetworkListener *pListener);
		//��ĳ����ַ�������ӣ���������ɹ����ᴥ�� NL_CODE_NEWCONN ��Ϣ��
		void ShootTo(const sockaddr* name, int namelen);
		//�������ݡ�
		//�����첽���͡�
		//���ͳɹ��ᴥ��NL_CODE_SENDSUCCEED��Ϣ�����򴥷�NL_CODE_SENDFAILED��Ϣ��
		//��Ϣ��param����iMsgID��
		int SendTo(int iConn,int iLen, const char *pData);
		//�ر�����
		void CloseConn(int iConn);

		//���ⲿ������socket�����������
		void AddConn(UDTSOCKET u);
		//��ȡ��������
		void GetConns(std::set<UDTSOCKET> &setConns);

		//��������
		UDTSOCKET CreateSocket(bool bRendezvous=true);

		//�ر��˳���
		//�˺�������������ʱ���Զ�����
		void Close();
	private:
		TBaseNetwork *m_pBase;
	};

	//////////////////////////////////////////////////////////////////////////
	// TPeer��װ
	//////////////////////////////////////////////////////////////////////////
	//���Ϸ��ڵ�ID����
	TUDT_API extern const T_PEERID INVALID_PEER_ID;

	//1.TPeer�ࡣ
	//TPeer�ഴ����ʱ��Ҫ��NPeerMng����id-->TPeer���ϡ�
	//TPeer��ֱ�ӽ��У���������ͳһ����NPeerMng�ƹܡ�
	//TPeer��Ҫ������պ�ת����Ϣ��
	//TPeer����ͬʱʹ�ö�����ӣ�Ŀǰ������UDT���ӣ����Ӹ������ӷ����������ݣ���������ղ�����Ӧ�ò���������һЩ�㷨�������ظ���ע�⣬����ʹ���˶�����ӣ���ô����Ҳ���п��ܵġ�
	//�������ݣ�����ѡ����л��һ�����ӣ�����������Ϊ���á�
	//�����������жϵ�ʱ�򣬲Żᷢ�������ж���Ϣ��
	//TPeer�˳���ʱ������������Ӷ�ɾ����
	class TPeerMng;
	class TUDT_API TPeer
	{
	public:
		//������ֱ�Ӵ�����ɾ����������NPeerMng�е���غ�����
		//iPeerId��Ŀ��id��Ŀ��idΪINVALID_PEER_ID��ʾ������������peer��id�����Ӻ���ȷ��      
		TPeer(TPeerMng *pMng,T_PEERID iPeerId);
	public:

		virtual ~TPeer(void);
		//====�������ӷ�ʽ====
		//ͨ��STUN��������ѯ��ַ����
		bool  PeerConnect(int iWaitTime=0);       
		//֪����ַ��ֱ������
		bool  PeerConnect(const sockaddr* nameKnown, int namelen,bool bTrySTUN=false);
		//֪����ַ�͸����û�id��ֱ������
		bool  PeerConnect(const sockaddr* nameKnown, int namelen,int iHelperId,bool bTrySTUN=false); 

		T_PEERID GetPeerId();
		int GetSpeed(int &iUpSpeed,int &iDownSpeed);
		bool IsReady();

		int  SendPeerMsg(const char* pData,int iLen);
		int  SendPack(const TBasePack *pPack,int iLen);
		void Close();
	public:
		//�ص�����
		virtual void  OnPeerConnected(bool bSuccess);           
		virtual void  OnPeerDisConnected();
		virtual void  OnPeerData(const char* pData, int ilen);
	protected:
		friend class TPeerMng;
		friend class NPeerMng;

		//���µ�ǰ����
		//���uΪINVALID_SOCKET����ô˵�����ӶϿ�
		void SetCurrentSocket(UDTSOCKET u);
		UDTSOCKET GetCurrentSocket();
		TPeerMng *GetPeerMng();
	private:
		TPeerMng *m_pMng;
		T_PEERID m_iPeerId;
		UDTSOCKET m_currSocket;

		//�ͻ����Լ��ϱ��ĵ�ַ��������ַ��
		unsigned int m_uiLocalIp;		// IP��ַ
		unsigned short m_usLocalPort;	// �˿ں�
	};

	//Peer��Ϣ������
	class TUDT_API IPeerDataListener
	{
	public:
		//�ص�����
		virtual void  OnPeerConnected(int iPeerId,bool bSuccess){iPeerId;bSuccess;};           
		virtual void  OnPeerDisConnected(int iPeerId){iPeerId;}	;
		virtual void  OnPeerData(T_PEERID iPeerId,const char* pData, int ilen) = 0;
	};

	//ָ��TPeer������ָ��
	typedef boost::shared_ptr<UDT::TPeer> TPeerPtr;
	class TUDT_API TPeerMng
	{
	public:
		TPeerMng(T_PEERID iMyId, const sockaddr* name, int namelen);  //ϵͳΨһ�Ľڵ�id����ڷ�������ַ
		virtual ~TPeerMng();

		//�Լ���id���ڿ�ʼ��ʱ�������INVALID_PEER_ID��ע�⣺IDֻ�ܸ���һ��
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
		TPeerPtr FindPeer(T_PEERID iPeerId);
		//��ȡ�û���ַ���û��ж�����ӵģ����ص�ǰ�û���ַ
		bool GetPeerAddress( T_PEERID iPeerId,sockaddr* name, int *namelen );
		//ͬ�ϡ�����û�û�����ӣ����ؿյ�ַ
		//KIpv4Addr GetPeerAddress(T_PEERID iPeerId);

		//���û������û����ݣ��˺���Ч�ʽϵͣ��ᳫ��SendPeerPack��
		int SendPeerMsg( T_PEERID iPeerId,const char *pData, int iLen );
		//���û�����TBasePack���ݰ�
		int SendPeerPack( T_PEERID iPeerId,const TBasePack *pPack, int iLen );

		//����û�����ʵ��
		//�û�������ھͷ��أ���������ھ͵���PeerFactory��������һ��
		TPeerPtr GetPeerInstance(T_PEERID iPeerId,int iTemplateType=0);
		//�ر����ӡ��˺����ر���������������ӣ��ͷŶ�Ӧ����ָ��
		void ClosePeer(T_PEERID iPeerId);

		//ȫ����Ϣ������
		void AddPeerListener(IPeerDataListener *pListener);
		bool RemovePeerListener(const IPeerDataListener *pListener);

		//�˳���ʱ����Զ��ر�
		void Close();
	private:
		friend class TPeer;
		friend class NPeerMng;
		NPeerMng *m_pPeerMngImpl;
	protected:
		//�û����󹤳���������������ʹ��
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
