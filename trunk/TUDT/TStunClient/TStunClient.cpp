// FileTester.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "tudt.h"
#include "debug.h"
#include "StrFuns.h"
#include "TClientPeer.h"
#include "TPeerProtocol.h"

//#include "NPeerMng.h"

T_PEERID g_iRemotePeerID = UDT::INVALID_PEER_ID;
//
//void OnConnect(int iRes)
//{
//	cout<<"OnConnect:"<<iRes<<endl;
//}

//TStunServ的功能如下：
//1.侦听端口，接受所有的连接；
//2.UDT连接建立之后，执行交换ID流程，确认TPEER连接，并且保持这个长连接；
//3.维护映射表：peer_id->可能的地址列表;
//4.接受用户的保活消息，当用户断线的时候删除连接(UDT自带此功能，不用自己做）
//5.接受用户的地址查询请求，返回地址列表
//6.接受用户的打孔请求，向客户端发送打孔命令
int _tmain(int argc, _TCHAR* argv[])
{
	int iMyID = 10005;
	int iMyPort = 0;
	char *strServerIp = "127.0.0.1";
	short int iServerPort = 0;
	int iServerID = 5000;
	if(argc==3)
	{
		iMyID = atoi(argv[1]);
		iMyPort = atoi(argv[2]);
	}
	else if(argc==6)
	{
		iMyID = atoi(argv[1]);
		iMyPort = atoi(argv[2]);
		strServerIp = argv[3];
		iServerPort = atoi(argv[4]);
		iServerID = atoi(argv[5]);
	}
	else
	{
		cout<<"参数个数错误："<<argc<<endl;
		cout<<"用法：TStunClient <myID> <myPort> <server_ip> <server_port> <server_id>"<<endl;
		return 0;
	}
	try
	{

		//UDT::TPeerMngTemplate<TClientPeer> mng(iMyID,0,NULL);
		KIpv4Addr addr((t_uint)0,iMyPort);
		SOCKADDR_IN si;
		addr.GetAddr(si);
		UDT::TPeerMngTemplate<TClientPeer> mng(iMyID,(sockaddr *)&si,sizeof(si));

		cout<<"Stun client 启动完成 id="<<mng.GetId()<<endl;

		//cout<<"正在连接STUN服务器，请稍候……"<<endl;

		//与服务器建立连接
		sockaddr_in dist;
		dist.sin_family = AF_INET;
		dist.sin_addr.s_addr = inet_addr(strServerIp);
		dist.sin_port = htons(iServerPort);
		if (dist.sin_port!=0)  //服务器端口号为零即不必启动服务器
		{
			
			cout<<"正在尝试连接STUN服务器："<< KIpv4Addr(dist).ToString()<<endl;
			mng.ConnectToServer(iServerID,(sockaddr *)&dist,sizeof(dist));
			for (int i=0;i<100;i++)
			{
				if(mng.IsPeerReady(iServerID))
				{
					break;
				}
				Sleep(600);
			}
			if (!mng.IsPeerReady(iServerID))
			{
				cout<<"连接STUN服务器超时。"<<endl;
				return 0;
			}
		}

		string strCmd;
		const int c_iBufLen = 4096;
		char buf[c_iBufLen];

		//TClientPeer *g_pCurrPeer = NULL;
		while(strCmd!="exit")
		{
			cin.getline(buf,c_iBufLen-1);

			vector<string> vecParams;
			StrFuns::Tokenize(buf,vecParams," ");
			if(vecParams.empty())
			{
				continue;
			}
			if(vecParams[0]=="exit")
			{
				break;
			}
			else if (vecParams[0]=="sendandexit")
			{
				int iPeer = atoi(vecParams[1].c_str());
				if (iPeer<=0)
				{
					cout<<"peer id error."<<endl;
					continue;
				}
				//TClientPeer *pPeer = (TClientPeer *)mng.FindPeer(iPeer);

				if (!mng.IsPeerReady(iPeer))
				{
					cout<<"peer not found."<<endl;
					continue;
				}

				buf[strlen(buf)+1] = '\0';

				int iCmdLen = (int)vecParams[0].size() + (int)vecParams[1].size() + 2;
				int iTotalLen = (int)strlen(buf)-iCmdLen+1;
				for (int i=0;i<1000;)
				{
					char sendbuf[1024*4];
					sprintf_s(sendbuf,1024*4,"MSG %d:\t%s\n",i,buf + iCmdLen);
					if(mng.SendPeerMsg(iPeer,sendbuf,strlen(sendbuf)+1)>0)
					{
						cout<<(int)strlen(buf)-4<<" bytes sent."<<endl;
						i++;
					}
					else
					{
						cout<<"send error."<<endl;
						Sleep(100);
					}
				}
				//return 0;
				continue;
			}
			else if (vecParams[0]=="send")
			{
				int iPeer = atoi(vecParams[1].c_str());
				if (iPeer<=0)
				{
					cout<<"peer id error."<<endl;
					continue;
				}
				//TClientPeer *pPeer = (TClientPeer *)mng.FindPeer(iPeer);

				if (!mng.IsPeerReady(iPeer))
				{
					cout<<"peer not found."<<endl;
					continue;
				}

				buf[strlen(buf)+1] = '\0';

				int iCmdLen = (int)vecParams[0].size() + (int)vecParams[1].size() + 2;
				int iTotalLen = (int)strlen(buf)-iCmdLen+1;
				if(mng.SendPeerMsg(iPeer,buf + iCmdLen,iTotalLen)==iTotalLen)
				{
					cout<<(int)strlen(buf)-4<<" bytes sent."<<endl;
				}
				else
				{
					cout<<"send error."<<endl;
				}
			}
			else if(vecParams[0]=="peer")
			{
				if(vecParams.size()!=2)
				{
					cout<<"param count should be 1."<<endl;
					continue;
				}
				T_PEERID uiPeer = atoi(vecParams[1].c_str());
				if (uiPeer<=0)
				{
					cout<<"id error:"<<uiPeer<<endl;
					continue;
				}
				if (uiPeer==mng.GetId())
				{
					cout<<"您输入的是自己的ID:"<<uiPeer<<endl;
					continue;
				}

				if (mng.IsPeerExist(uiPeer))
				{
					cout<<"连接已经存在:"<<uiPeer<<endl;
					g_iRemotePeerID = uiPeer;
					if (!mng.IsPeerReady(uiPeer))
					{
						cout<<"连接存在但处于断开状态,尝试重新连接……"<<endl;
						if(mng.PeerConnect(uiPeer,5000))
						{
							cout<<"连接成功"<<endl;
						}
						else
						{
							cout<<"连接失败"<<endl;
						}
					}
					continue;
				}
				if (g_iRemotePeerID!=UDT::INVALID_PEER_ID)
				{
					//NPeer *pOld = mng.ClosePeer(g_iRemotePeerID);
					//if (NULL!=pOld)
					//{
					//	delete pOld;
					//}
					mng.ClosePeer(g_iRemotePeerID);
				}
				g_iRemotePeerID = uiPeer;

				//NPeer *pPeer = new NPeer(&mng,g_iRemotePeerID);
				UDT::TPeerPtr pPeer = mng.GetPeerInstance(g_iRemotePeerID);

				int iWaitTime = 5000;
				if(!pPeer->PeerConnect(iWaitTime))
				{
					cout<<"连接失败"<<endl;
					continue;
				}
				cout<<"连接成功，发送测试数据。"<<endl;
				if(!pPeer->SendPeerMsg("Hello,triger echo!",19))
				{
					cout<<"send error"<<endl;
					continue;
				}
				//cout<<"等待10秒……"<<endl;
				//Sleep(10*1000);

				//conn.Close(); //可以由析构函数自动调用
				//cout<<"测试完成，连接断开"<<endl;
			}
			else if(vecParams[0]=="connect")
			{
				if(vecParams.size()!=4)
				{
					cout<<"param count should be 3."<<endl;
					continue;
				}
				cout<<"connecting to :"<<vecParams[1]<<endl;
				//ShootTo(*pNetbase,vecParams[1],atoi(vecParams[2].c_str()));

				sockaddr_in dist;
				dist.sin_family = AF_INET;
				dist.sin_addr.s_addr = inet_addr(vecParams[1].c_str());
				dist.sin_port = htons(atoi(vecParams[2].c_str()));
				int iPeerID = atoi(vecParams[3].c_str());

				UDT::TPeerPtr ptrPeer = mng.GetPeerInstance(iPeerID);
				if(!ptrPeer->PeerConnect((sockaddr *)&dist,sizeof(dist)))
				{
					cout<<"connect failed."<<endl;
					continue;
				}
				for(int i=0;i<300;i++)
				{
					if(!ptrPeer->IsReady())
					{
						Sleep(10);
					}
				}

				if(!ptrPeer->IsReady())
				{
					cout<<"连接失败"<<endl;
					continue;
				}
				cout<<"发送触发数据"<<endl;
				if(!ptrPeer->SendPeerMsg("Hello,triger echo!",19))
				{
					cout<<"send error"<<endl;
					continue;
				}
				cout<<"等待10秒……"<<endl;
				Sleep(10*1000);

				//conn.Close(); //可以由析构函数自动调用
				cout<<"测试完成，连接断开"<<endl;
				mng.ClosePeer(iPeerID);

			}

			else
			{
				cout<<"Unknow cmd:"<<vecParams[0]<<endl;
				cout<<"cmd list:"<<endl;
				cout<<"\tsend <message>"<<endl;
				cout<<"\tpeer <id>"<<endl;
				cout<<"\tconnect <ip> <port>"<<endl;
				cout<<"\texit"<<endl;
			}

		}
	}
	catch (std::exception &e)
	{
		cout<<"程序异常退出，请检查端口是否已被占用:"<<e.what()<<endl;
	}
	catch (...) {

		cout<<"程序异常退出，请检查端口是否已被占用。"<<endl;
	}
	return 0;
}

