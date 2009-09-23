#include ".\msgfragmonitor.h"
#include "jmutexautolock.h"

MsgFragMonitor::MsgFragMonitor(void)
{
	m_currId = 1;
}

MsgFragMonitor::~MsgFragMonitor(void)
{
}

int MsgFragMonitor::CreateMsg()
{
	JMutexAutoLock tmpGuard(m_ConnMutex);
	m_mapMsgID[m_currId] = std::set<int>();
	return m_currId++;
}

void MsgFragMonitor::AddMsgFrag( int msgId,int fragId )
{
	JMutexAutoLock tmpGuard(m_ConnMutex);
	std::set<int> &setFrag = m_mapMsgID[msgId];
	setFrag.insert(fragId);
}

std::pair<bool,int> MsgFragMonitor::OnFragACK( int fragId )
{
	JMutexAutoLock tmpGuard(m_ConnMutex);
	std::map<int, std::set<int> >::iterator it = m_mapMsgID.begin();
	for (;it!=m_mapMsgID.end();it++)
	{
		std::set<int> &setFrag = (*it).second;
		if(setFrag.erase(fragId)>0)
		{
			int id = (*it).first;
			m_mapMsgID.erase(it);
			return std::pair<bool,int>(true,id);
		}
	}
	return std::pair<bool,int>(false,0);
}