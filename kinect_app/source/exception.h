#ifndef __kinect_app_exception_h__
#define __kinect_app_exception_h__

#define KINECT_APP_EXPECT_HR(expr, code)\
	{\
		if (FAILED(expr))\
			throw code;\
	}

#define KINECT_APP_EXPECT(expr, code)\
	{\
		if (!(expr))\
			throw code;\
	}

namespace kinect_app
{
	// @todo:
}

#endif //__kinect_app_exception_h__

