#ifndef __kinect_app_critical_section_h__
#define __kinect_app_critical_section_h__

#include <Windows.h>

namespace kinect_app
{

class CriticalSection
{
public:
	CriticalSection()
	{
		::InitializeCriticalSection(&m_section);
	}
	~CriticalSection()
	{
		::LeaveCriticalSection(&m_section);
	}

	void Lock()
	{
		::EnterCriticalSection(&m_section);
	}
	void Unlock()
	{
		::LeaveCriticalSection(&m_section);
	}

	operator PCRITICAL_SECTION ()
	{
		return &m_section; 
	}
private:
	CriticalSection(const CriticalSection&);
	CriticalSection& operator = (const CriticalSection&);

private:
	CRITICAL_SECTION m_section;
};

template <class T> class ScopedLock
{
public:
	explicit ScopedLock(T& lock): m_inner(lock)
	{
		m_inner.Lock();
	}

	~ScopedLock()
	{
		m_inner.Unlock();
	}

private:
	T& m_inner;
};

class ConditionVariable
{
public:
	ConditionVariable()
	{
		::InitializeConditionVariable(&m_conditionVariable);
	}

	void Sleep(CriticalSection& cs)
	{
		::SleepConditionVariableCS(&m_conditionVariable, cs, INFINITE); // @todo: finite!
	}

	void Wake()
	{
		::WakeConditionVariable(&m_conditionVariable);
	}

	void WakeAll()
	{
		::WakeAllConditionVariable(&m_conditionVariable);
	}

private:
	CONDITION_VARIABLE m_conditionVariable;
};

}

#endif __kinect_app_critical_section_h__