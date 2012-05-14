#include "trace.h"

#include <Windows.h>

namespace kinect_app {
namespace detail {

unsigned long KGetCurrentThreadId()
{
	return ::GetCurrentThreadId();
}

}
}
