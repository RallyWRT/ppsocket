#pragma once

#include <set>

template<class T_Monitor>
class MonitorGroup
{
public:

	//通过此接口，监视缓存数据变化的通知消息：发布端和上传模块用

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
