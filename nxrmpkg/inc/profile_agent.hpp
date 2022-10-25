

#ifndef __NXRM_PKG_AGENT_PROFILE_HPP__
#define __NXRM_PKG_AGENT_PROFILE_HPP__

#include <string>
#include "profile_basic.hpp"

namespace nxrm {
namespace pkg {


#define PROFILE_AGENT_NODENAME  L"AGENT_PROFILE"


/*
<AGENT_PROFILE id="1" name="Default Agent Profile" timestamp="2015-03-22T13:24:50Z">
    <AGENT_ID>100</AGENT_ID>
    <AGENT_TYPE>DESKTOP</AGENT_TYPE>
</AGENT_PROFILE>
*/

class CAgentProfile : public CProfile
{
public:
    CAgentProfile();
    virtual ~CAgentProfile();
    
    inline const std::wstring& GetAgentId() const {return _agentId;}
    inline void SetAgentId(const std::wstring& id) throw() {_agentId = id;}
    inline const std::wstring& GetAgentType() const {return _agentType;}
    inline void SetAgentType(const std::wstring& type) throw() {_agentType = type;}

    inline CAgentProfile& operator =(const CAgentProfile& profile) throw()
    {
        if(this != &profile) {
            ((CProfile*)this)->operator=(profile);
            _agentId = profile.GetAgentId();
            _agentType = profile.GetAgentType();
        }
        return *this;
    }

    inline void Clear() throw()
    {
        _agentId.clear();
        _agentType.clear();
        CProfile::Clear();
    }
        
    void FromXml(IXMLDOMNode* node);
    void ToXml(IXMLDOMDocument* doc, IXMLDOMNode* parent, IXMLDOMNode** node) const;

private:
    std::wstring    _agentId;
    std::wstring    _agentType;
};

    
}   // namespace nxrm::pkg
}   // namespace nxrm


#endif  // __NXRM_PKG_AGENT_PROFILE_HPP__