

#ifndef __NXRM_PKG_COMMUNICATION_PROFILE_HPP__
#define __NXRM_PKG_COMMUNICATION_PROFILE_HPP__

#include <string>
#include "profile_basic.hpp"

namespace nxrm {
namespace pkg {


#define PROFILE_COMM_NODENAME   L"COMM_PROFILE"


/*
<COMM_PROFILE id="1" name="Default Communication Profile" timestamp="2015-03-22T13:24:50Z">
    <SERVER_HOST>rms.nextlabs.com</SERVER_HOST>
    <SERVER_PORT>8443</SERVER_PORT>
    <SERVER_PATH>/Dabs</SERVER_PATH>
</COMM_PROFILE>
*/

class CCommProfile : public CProfile
{
public:
    CCommProfile();
    virtual ~CCommProfile();
    
    inline const std::wstring& GetServHost() const {return _servHost;}
    inline void SetServHost(const std::wstring& host) throw() {_servHost = host;}
    inline const std::wstring& GetServPath() const {return _servPath;}
    inline void SetServPath(const std::wstring& path) throw() {_servPath = path;}
    inline USHORT GetServPort() const {return _servPort;}
    inline void SetServPort(USHORT port) throw() {_servPort = port;}
    inline __int64 GetHeartBeatInterval() const {return _hbInterval;}
    inline void SetHeartBeatInterval(__int64 interval) throw() {_hbInterval = interval;}
    inline __int64 GetLogInterval() const {return _logInterval;}
    inline void SetLogInterval(__int64 interval) throw() {_logInterval = interval;}
    inline __int64 GetLogSize() const {return _logInterval;}
    inline void SetLogSize(__int64 size) throw() {_logInterval = size;}

    inline CCommProfile& operator =(const CCommProfile& profile) throw()
    {
        if(this != &profile) {
            ((CProfile*)this)->operator=(profile);
            _servHost = profile.GetServHost();
            _servPort = profile.GetServPort();
            _servPath = profile.GetServPath();
            _hbInterval = profile.GetHeartBeatInterval();
            _logInterval = profile.GetLogInterval();
            _logSize = profile.GetLogSize();
        }
        return *this;
    }

    inline void Clear() throw()
    {
        _servHost.clear();
        _servPath.clear();
        _servPort = 0;
        _hbInterval = 0;
        _logInterval = 0;
        _logSize = 0;
    }
        
    void FromXml(IXMLDOMNode* node);
    void ToXml(IXMLDOMDocument* doc, IXMLDOMNode* parent, IXMLDOMNode** node) const;

private:
    std::wstring    _servHost;
    USHORT          _servPort;
    std::wstring    _servPath;
    __int64         _hbInterval;
    __int64         _logInterval;
    __int64         _logSize;
};

    
}   // namespace nxrm::pkg
}   // namespace nxrm


#endif  // __NXRM_PKG_COMMUNICATION_PROFILE_HPP__