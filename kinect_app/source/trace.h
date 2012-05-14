#ifndef __kinekt_app_h__
#define __kinekt_app_h__

#include <stdio.h>
#include <string>
#include <sstream>

#define KINECT_TRACE_OUT(x, level) { using namespace kinect_app::detail; std::stringstream out; out << KGetCurrentThreadId() << "\t" << GetLevelStr(level) << "\t" << __FUNCTION__ << "\t" << x << "\n"; Trace(out.str(), level); }

#define KINECT_TRACE_ERR(x) KINECT_TRACE_OUT(x, kinect_app::detail::trace_level::Error)

#define KINECT_TRACE_INF(x) KINECT_TRACE_OUT(x, kinect_app::detail::trace_level::Information)

#define KINECT_TRACE_DBG(x) KINECT_TRACE_OUT(x, kinect_app::detail::trace_level::Debug)


namespace kinect_app {
namespace detail {

namespace trace_level
{
	enum
	{
		Error,
		Information,
		Debug
	};
	typedef size_t Type;
}

inline const char* GetLevelStr(trace_level::Type level)
{
	static const char* Err = "Err";
	static const char* Inf = "Inf";
	static const char* Dbg = "Dbg";

	return (level == trace_level::Error)? Err: (level == trace_level::Information)? Inf: Dbg;
}

unsigned long KGetCurrentThreadId();

inline void Trace(const std::string& out, trace_level::Type level)
{
#ifndef _DEBUG
	if (level == trace_level::Debug) // @todo: replace with configurable settings
		return;
#endif
	printf(out.c_str());
}

}
}

#endif // __kinekt_app_h__
