




#ifndef __NXRM_REST_REGISTER_HPP__
#define __NXRM_REST_REGISTER_HPP__

#include <nudf\httputil.hpp>

#include <vector>

namespace nxrmrest {

class CResgiterRequestData;
class CResgiterResponseData;

class CRegisterRequest : public nudf::http::CRequest
{
public:
    CRegisterRequest();
    CRegisterRequest(_In_ const std::wstring& path);
    virtual ~CRegisterRequest();
};


/*
<!-- REQUEST XML -->
<?xml version="1.0" encoding="UTF-8"?>
<RegisterAgentRequest>
    <RegistrationData>
        <host></host>
        <type></type>
        <version>
            <major></major>
            <minor></minor>
            <maintenance></maintenance>
            <patch></patch>
            <build></build>
        </version>
    </RegistrationData>
</RegisterAgentRequest>
*/
class CResgiterRequestData
{
public:
    CResgiterRequestData();
    ~CResgiterRequestData();

    std::wstring ToXml();
};

/*
<!-- RESPONSE XML -->
<?xml version="1.0" encoding="UTF-8"?>
<RegisterAgentResponse>
    <StartupConfiguration>
        <id></id>
        <commProfile>
            <id></id>
            <defaultProfile></defaultProfile>
            <createdDate></createdDate>
            <modifiedDate></modifiedDate>
            <DABSLocation></DABSLocation>
            <agentType></agentType>
            <heartBeatFrequency></heartBeatFrequency>
            <logLimit></logLimit>
            <logFrequency></logFrequency>
            <pushEnabled></pushEnabled>
            <defaultPushPort></defaultPushPort>
            <passwordHash></passwordHash>
            <password></password>
            <currentActivityJournalingSettings></currentActivityJournalingSettings>
            <customActivityJournalingSettings></customActivityJournalingSettings>
        </commProfile>
        <agentProfile>
            <id></id>
            <defaultProfile></defaultProfile>
            <createdDate></createdDate>
            <modifiedDate></modifiedDate>
            <hookAllProc></hookAllProc>
            <hookSystemProc></hookSystemProc>
            <logViewingEnabled></logViewingEnabled>
            <trayIconEnabled></trayIconEnabled>
            <hookedApplications></hookedApplications>
            <protectedApplications></protectedApplications>
        </agentProfile>
        <registrationId></registrationId>
    </StartupConfiguration>
    <certificate>
    </certificate>
    <fault>
    </fault>
</RegisterAgentResponse>
*/
class CResgiterResponseData
{
public:
    CResgiterResponseData();
    ~CResgiterResponseData();

    void FromXml(const std::wstring& xml);
};


}   // namespace nxrmrest


#endif // __NXRM_REST_REGISTER_HPP__