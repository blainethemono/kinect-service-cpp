#ifndef __kinect_app_event_h__
#define __kinect_app_event_h__

#include "exception.h"

#include <Windows.h>

namespace kinect_app
{

class Event
{
public:
	Event()
	{
		KINECT_APP_EXPECT(!!(m_event = ::CreateEvent(NULL, TRUE, FALSE, NULL)), 1);
	}

	~Event()
	{
		::CloseHandle(m_event);
	}

	void Set()
	{
		KINECT_APP_EXPECT(::SetEvent(m_event), 1);
	}

	void Reset()
	{
		KINECT_APP_EXPECT(::ResetEvent(m_event), 1);
	}

	DWORD Wait(DWORD timeout = INFINITE)
	{
		return ::WaitForSingleObject(m_event, timeout);
	}

	HANDLE GetWaitableHandle() const
	{
		return m_event;
	}

private:
	Event(const Event&);
	Event& operator =(const Event&);

private:
	mutable HANDLE m_event;
};

}

#endif //__kinect_app_event_h__
