#ifndef __kinekt_app_api_h__
#define __kinekt_app_api_h__

#include <string>

namespace kinect_app
{
namespace detail
{

std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const& s);
std::string createChallenge(const char* key);

}
}

#endif //__kinekt_app_api_h__
