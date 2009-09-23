#pragma once

#include <set>

template<class T_Monitor>
class MonitorGroup
{
public:

	//ͨ���˽ӿڣ����ӻ������ݱ仯��֪ͨ��Ϣ�������˺��ϴ�ģ����

	int AddMonitor( T_Monitor *pMonitor )
	{
		m_setMonitors.insert(pMonitor);
		return 0;
	}

	int RemoveMonitor( const T_Monitor *pMonitor )
	{
		m_setMonitors.erase(const_cast<T_Monitor *>(pMonitor));
		return 0;
	}

	void Fire( int iBlockIndex )
	{
		std::set<T_Monitor *>::iterator it =  m_setMonitors.begin();
		for (;it!=m_setMonitors.end();it++)
		{
			T_Monitor *pMonitor = *it;
			//pMonitor->OnBufferAdded(iBlockIndex);
		}
	}

private:
	std::set<T_Monitor *> m_setMonitors;
};
