

#include <Windows.h>
#include <Http.h>

#include <iostream>

#include <nudf\eh.hpp>
#include <nudf\debug.hpp>
#include <nudf\filesys.hpp>
#include <nudf\crypto.hpp>
#include <nudf\string.hpp>
#include <nudf\http_client.hpp>
#include <nudf\http_service.hpp>
#include <nudf\conversion.hpp>
#include <nudf\xml.hpp>



extern NX::fs::module_path  current_image;
extern NX::fs::dos_filepath current_image_dir;
extern NX::fs::dos_filepath current_work_dir;

//class rmc_client;
//class rmc_register_request;
//class rmc_register_response;
//class rmc_heartbeat_request;

static void test_quert_ssl_conf();

bool test_http(void)
{
    bool result = true;

    try {

        test_quert_ssl_conf();

        //rmc_client  c;
        //rmc_register_response res_regiter;

        //// register
        //try {

        //    rmc_register_request req_register;
        //    c.send_request(&req_register, &res_regiter);

        //    if (res_regiter.status_code() != 200) {
        //        std::cout << "Bad status code (registration): " << (short)res_regiter.status_code() << std::endl;
        //        return;
        //    }
        //}
        //catch (const NX::exception& e) {
        //    std::cout << "Fail to register agent: " << e.what() << std::endl;
        //    throw e;
        //}

        //int i = 0;
        //// heart beat
        //while (i++ < 50) {

        //    try {

        //        rmc_heartbeat_request req_heartbeat;
        //        NX::http::string_response res_heartbeat;
        //        c.send_request(&req_heartbeat, &res_heartbeat);

        //        if (res_heartbeat.status_code() != 200) {
        //            std::cout << "Bad status code (heartbeat): " << (short)res_heartbeat.status_code() << std::endl;
        //            return;
        //        }

        //        Sleep(1000);
        //    }
        //    catch (const NX::exception& e) {
        //        std::cout << "Fail to do heart beat: " << e.what() << std::endl;
        //        throw e;
        //    }
        //}

    }
    catch (std::exception& e) {
        std::cout << "    Exception: " << e.what() << std::endl;
        result = false;
    }

    return result;
}

void test_quert_ssl_conf()
{
    HttpInitialize(HTTPAPI_VERSION_1, HTTP_INITIALIZE_CONFIG, NULL);
    const std::vector<NX::http::ssl_conf>& confs = NX::http::query_ssl_conf_all();
    HttpTerminate(HTTP_INITIALIZE_CONFIG, NULL);
    std::wcout << L"Succeed" << std::endl;
}

//class rmc_client : public NX::http::client
//{
//public:
//    rmc_client() : NX::http::client(L"https://devtestrms.qapf1.qalab01.nextlabs.com:8443", false)
//    {
//    }
//
//    virtual ~rmc_client()
//    {
//    }
//};
//class rmc_register_request : public NX::http::string_request
//{
//public:
//    rmc_register_request() : NX::http::string_request(NX::http::methods::POST,
//        L"/RMS/service/RegisterAgent",
//        { { NX::http::header_names::user_agent, L"NXRMC/8.2.900" },
//        { L"X-NXL-S-CERT", L"MIIDmzCCAoOgAwIBAgIEI8+FCTANBgkqhkiG9w0BAQsFADB+MQswCQYDVQQGEwJVUzELMAkGA1UECBMCQ0ExEjAQBgNVBAcTCVNhbiBNYXRlbzERMA8GA1UEChMITmV4dExhYnMxGjAYBgNVBAsTEVJpZ2h0cyBNYW5hZ2VtZW50MR8wHQYDVQQDExZSaWdodHMgTWFuYWdlbWVudCBUZW1wMB4XDTE1MDUyNzE0MDI0MVoXDTI1MDUyNDE0MDI0MVowfjELMAkGA1UEBhMCVVMxCzAJBgNVBAgTAkNBMRIwEAYDVQQHEwlTYW4gTWF0ZW8xETAPBgNVBAoTCE5leHRMYWJzMRowGAYDVQQLExFSaWdodHMgTWFuYWdlbWVudDEfMB0GA1UEAxMWUmlnaHRzIE1hbmFnZW1lbnQgVGVtcDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAPUFAKjEmwL/oWQpafK5vOz8/dOEf9mmcjKawhxNhJJ5R+4olKHF9ZaKMve542ehSKngA53buaPsvP96ix7j8O8E2DHnYqbR5I9jNfZarIWjAwfO94TDvQovuIFBCWMnJXsRUtVm36cF6WpQdqctIbgvSbjGbgYacqGZ6QaaWrySGxupjyB8lfD6dYBG5lXFRcFA7QQVbNAGM7Xis2S3sPZOch4VJK7faX2xRyW6sIKL0FU8W9HCbm2PjG+XBr+dmsP3lk6HOqlSEy55HRYldMI/KCSlTGIcUHjH0qpiBxceSHILgY+YOqJ3l6/d8k9ui3MK2XGUhNFgwwLYFruk1l8CAwEAAaMhMB8wHQYDVR0OBBYEFGKcZJB9ZVJ6q/T2DNJIUoQEqx6sMA0GCSqGSIb3DQEBCwUAA4IBAQA9oFoR9GYVvba1WTdq2sl7kqTxqTPkUtD5LGi5A7q1yxMkAwsR2kW00L5dbRmADT7PjE3x42V2ZHHuYhDjGg/zm+2xHVrUWl2ZxHodmHz6+qDbdAZ3+9U4Zz7nt2oxDFghp/eE1adXa2kfAIZzn8VVamD6TS9O0R/KyXToYgpjLmz6QD9GFsz5wGbVsnJGWTxfiNjX3LnFIkqJU8rHn1DcMyB3/xd3ytUJzKrAnD8f46JpfR1amJOQAxiDy5+kW1OnclGBImS9iisvCmwU3+UNixbFAAxymBA9VvAO90sw0tHcLN7M1NSpenVlAnJTHhGuLSepk8gv4jAEsa9+DPKR" }
//        },
//        L"<RegisterAgentRequest><RegistrationData>"
//        L"<host>devtestrmc01.qapf1.qalab01.nextlabs.com</host>"
//        L"<type>DESKTOP</type>"
//        L"<version><major>8</major><minor>2</minor><maintenance>0</maintenance><patch>0</patch><build>900</build></version>"
//        L"<osInformation><osType>Windows</osType><osVersion>8.1</osVersion></osInformation>"
//        L"<hardwareDetails>"
//        L"<cpu>Intel i7</cpu>"
//        L"<memoryAmount>2048</memoryAmount>"
//        L"<memoryUnits>MB</memoryUnits>"
//        L"<macAddressList>00-50-56-20-BA-15</macAddressList>"
//        L"<deviceId>0</deviceId>"
//        L"</hardwareDetails>"
//        L"<groupInformation>"
//        L"<groupId>Unknown</groupId>"
//        L"<groupName>Unknown</groupName>"
//        L"</groupInformation>"
//        L"<tenantId>Unknown</tenantId>"
//        L"</RegistrationData></RegisterAgentRequest>"
//        )
//    {
//    }
//
//    virtual ~rmc_register_request()
//    {
//    }
//};
//class rmc_register_response : public NX::http::string_response
//{
//public:
//    rmc_register_response() {}
//    virtual ~rmc_register_response() {}
//
//    inline const std::wstring& cert() const { return _cert; }
//    inline const std::wstring& agent_id() const { return _agent_id; }
//    inline const std::wstring& registration_id() const { return _registration_id; }
//    inline const std::wstring& agent_profile_name() const { return _agent_profile_name; }
//    inline const std::wstring& agent_profile_create_time() const { return _agent_profile_create_time; }
//    inline const std::wstring& agent_profile_modify_time() const { return _agent_profile_modify_time; }
//    inline const std::wstring& comm_profile_name() const { return _comm_profile_name; }
//    inline const std::wstring& comm_profile_create_time() const { return _comm_profile_create_time; }
//    inline const std::wstring& comm_profile_modify_time() const { return _comm_profile_modify_time; }
//
//    virtual void complete_write()
//    {
//        if (status_code() == 200) {
//            if (!body().empty()) {
//                NX::xml_document doc;
//                try {
//                    // load xml
//                    doc.load_from_string(body());
//                    // <RegisterAgentResponse>
//                    std::shared_ptr<NX::xml_node> root = doc.document_root();
//                    if (root == nullptr) {
//                        throw ERROR_MSG("runtime", "fail to find root");
//                    }
//                    if (0 != _wcsicmp(root->get_name().c_str(), L"RegisterAgentResponse")) {
//                        throw ERROR_MSG("runtime", "fail to find <RegisterAgentResponse>");
//                    }
//                    // <certificate>
//                    std::shared_ptr<NX::xml_node> node_cert = root->find_child_element(L"certificate");
//                    if (node_cert == nullptr) {
//                        _cert = L"MIIDkTCCAnmgAwIBAgIEUUgJizANBgkqhkiG9w0BAQsFADB5MQswCQYDVQQGEwJVUzELMAkGA1UECBMCQ0ExEjAQBgNVBAcTCVNhbiBNYXRlbzERMA8GA1UEChMITmV4dExhYnMxGjAYBgNVBAsTEVJpZ2h0cyBNYW5hZ2VtZW50MRowGAYDVQQDExFSaWdodHMgTWFuYWdlbWVudDAeFw0xNTA1MjcxNDAzNTBaFw0yNTA1MjQxNDAzNTBaMHkxCzAJBgNVBAYTAlVTMQswCQYDVQQIEwJDQTESMBAGA1UEBxMJU2FuIE1hdGVvMREwDwYDVQQKEwhOZXh0TGFiczEaMBgGA1UECxMRUmlnaHRzIE1hbmFnZW1lbnQxGjAYBgNVBAMTEVJpZ2h0cyBNYW5hZ2VtZW50MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAl5HKn8BZ0yyY0JxpvQT0IkRPfKeXnvR53L0y5Nu1DkdAwc8mcd4H7ePUe3QiToKgzVZUJatUQ";
//                    }
//                    else {
//                        _cert = node_cert->get_text();
//                    }
//                    // <StartupConfiguration>
//                    std::shared_ptr<NX::xml_node> node_conf = root->find_child_element(L"StartupConfiguration");
//                    if (node_conf == nullptr) {
//                        throw ERROR_MSG("runtime", "fail to find <StartupConfiguration>");
//                    }
//                    // <id>
//                    std::shared_ptr<NX::xml_node> node_id = node_conf->find_child_element(L"id");
//                    if (node_id == nullptr) {
//                        throw ERROR_MSG("runtime", "fail to find <StartupConfiguration>");
//                    }
//                    _agent_id = node_id->get_text();
//                    // <registrationId>
//                    std::shared_ptr<NX::xml_node> node_reg_id = node_conf->find_child_element(L"registrationId");
//                    if (node_reg_id == nullptr) {
//                        throw ERROR_MSG("runtime", "fail to find <registrationId>");
//                    }
//                    _registration_id = node_reg_id->get_text();
//                    // <commProfile>
//                    std::shared_ptr<NX::xml_node> node_comm_profile = node_conf->find_child_element(L"commProfile");
//                    if (node_comm_profile == nullptr) {
//                        throw ERROR_MSG("runtime", "fail to find <commProfile>");
//                    }
//                    else {
//                        std::shared_ptr<NX::xml_node> node_comm_profile_name = node_comm_profile->find_child_element(L"name");
//                        if (node_comm_profile_name == nullptr) {
//                            throw ERROR_MSG("runtime", "fail to find <name>");
//                        }
//                        _comm_profile_name = node_comm_profile_name->get_text();
//                        std::shared_ptr<NX::xml_node> node_comm_profile_create_time = node_comm_profile->find_child_element(L"createdDate");
//                        if (node_comm_profile_create_time == nullptr) {
//                            throw ERROR_MSG("runtime", "fail to find <createDate>");
//                        }
//                        _comm_profile_create_time = node_comm_profile_create_time->get_text();
//                        std::shared_ptr<NX::xml_node> node_comm_profile_modify_time = node_comm_profile->find_child_element(L"modifiedDate");
//                        if (node_comm_profile_modify_time == nullptr) {
//                            throw ERROR_MSG("runtime", "fail to find <createDate>");
//                        }
//                        _comm_profile_modify_time = node_comm_profile_modify_time->get_text();
//                    }
//                    // <agentProfile>
//                    std::shared_ptr<NX::xml_node> node_agent_profile = node_conf->find_child_element(L"agentProfile");
//                    if (node_agent_profile == nullptr) {
//                        throw ERROR_MSG("runtime", "fail to find <agentProfile>");
//                    }
//                    else {
//                        std::shared_ptr<NX::xml_node> node_agent_profile_name = node_agent_profile->find_child_element(L"name");
//                        if (node_agent_profile_name == nullptr) {
//                            throw ERROR_MSG("runtime", "fail to find <name>");
//                        }
//                        _agent_profile_name = node_agent_profile_name->get_text();
//                        std::shared_ptr<NX::xml_node> node_agent_profile_create_time = node_agent_profile->find_child_element(L"createdDate");
//                        if (node_agent_profile_create_time == nullptr) {
//                            throw ERROR_MSG("runtime", "fail to find <createDate>");
//                        }
//                        _agent_profile_create_time = node_agent_profile_create_time->get_text();
//                        std::shared_ptr<NX::xml_node> node_agent_profile_modify_time = node_agent_profile->find_child_element(L"modifiedDate");
//                        if (node_agent_profile_modify_time == nullptr) {
//                            throw ERROR_MSG("runtime", "fail to find <createDate>");
//                        }
//                        _agent_profile_modify_time = node_agent_profile_modify_time->get_text();
//                    }
//                }
//                catch (const NX::exception& e) {
//                    UNREFERENCED_PARAMETER(e);
//                }
//            }
//        }
//    }
//
//private:
//    std::wstring _cert;
//    std::wstring _agent_id;
//    std::wstring _registration_id;
//    std::wstring _agent_profile_name;
//    std::wstring _agent_profile_create_time;
//    std::wstring _agent_profile_modify_time;
//    std::wstring _comm_profile_name;
//    std::wstring _comm_profile_create_time;
//    std::wstring _comm_profile_modify_time;
//};
//
//class rmc_heartbeat_request : public NX::http::string_request
//{
//public:
//    rmc_heartbeat_request() : NX::http::string_request(NX::http::methods::POST,
//        L"/RMS/service/HeartBeat",
//        { { NX::http::header_names::user_agent, L"NXRMC/8.2.900" },
//        { L"X-NXL-S-CERT", L"MIIDkTCCAnmgAwIBAgIEUUgJizANBgkqhkiG9w0BAQsFADB5MQswCQYDVQQGEwJVUzELMAkGA1UECBMCQ0ExEjAQBgNVBAcTCVNhbiBNYXRlbzERMA8GA1UEChMITmV4dExhYnMxGjAYBgNVBAsTEVJpZ2h0cyBNYW5hZ2VtZW50MRowGAYDVQQDExFSaWdodHMgTWFuYWdlbWVudDAeFw0xNTA1MjcxNDAzNTBaFw0yNTA1MjQxNDAzNTBaMHkxCzAJBgNVBAYTAlVTMQswCQYDVQQIEwJDQTESMBAGA1UEBxMJU2FuIE1hdGVvMREwDwYDVQQKEwhOZXh0TGFiczEaMBgGA1UECxMRUmlnaHRzIE1hbmFnZW1lbnQxGjAYBgNVBAMTEVJpZ2h0cyBNYW5hZ2VtZW50MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAl5HKn8BZ0yyY0JxpvQT0IkRPfKeXnvR53L0y5Nu1DkdAwc8mcd4H7ePUe3QiToKgzVZUJatUQ+bOlCDIQB/bgBO88Pi5ozNGEk4iobpIR0NYi5PHuYqNlsiUXD3IARle1XpiE3m4JU6lk1p9y8YimX3dE+B05lS58MEVya998Q1yh2C0NYOfECkQv4w8FtbPJbn4VTfqanokKjcx0L5aSpS/q06AG24SruwU4sX6pMspHME8fOUbBTm+VnhpArZ35FyJj1o+BoCVt4Pp0UG+lIiBMj3E50m/Ii/YnuWrtozsUZhd6w7/QIfpTn51ZehKOhHAhsRgWjeDvp7EOp45WwIDAQABoyEwHzAdBgNVHQ4EFgQUWnITlK2cVYTVHF2J1mEns/BOP38wDQYJKoZIhvcNAQELBQADggEBAAndNhd3Shu6b1sk69LsLMccxttFP+43gfapDsyefdqdybwQwdw8vgJOikbMgJFhPHNR+0HX0Oh/u3nIqYxjGTwE5jFy6fu/P1iAyinYECuGmj1MCpsjEtLjIClaWDRKETlk5mGeRxtHtexMHITkJv/6Qunh7rYC+RvwBGAAvgIlQQbMzMRAzAY4ttnscmIVPaU+ofED8YOE+pR8MWkYLG9EJViDIVDKZO9w9PLSqFu2wvLOg58Bg7sEfHXEvikYR0SQdm2Teouw0iQCYJipbMtCl6EwzgjuZtKGEAy2w5WareoTkvQdHr8Gp7lkofRwPjdUWHqfI7GOVLZwiKLdkhg=" }
//        },
//        L"<HeartBeatRequest>"
//        L"<agentId>1517</agentId>"
//        L"<heartbeat>"
//        L"<profileStatus>"
//        L"<lastCommittedAgentProfileName>Default_Profile</lastCommittedAgentProfileName>"
//        L"<lastCommittedAgentProfileTimestamp>2015-06-01T21:20:48.501+00:00</lastCommittedAgentProfileTimestamp>"
//        L"<lastCommittedCommProfileName>Desktop Enforcer Default Profile</lastCommittedCommProfileName>"
//        L"<lastCommittedCommProfileTimestamp>2015-06-08T23:52:31.125+00:00</lastCommittedCommProfileTimestamp>"
//        L"</profileStatus>"
//        L"<policyAssemblyStatus agentHost=\"devtestrmc01.qapf1.qalab01.nextlabs.com\"  agentType=\"DESKTOP\" timestamp=\"2000-06-08T23:52:31.125+00:00\">"
//        L"<policyUsers><userSubjectType>windowsSid</userSubjectType><systemId>S-1-5-21-2018228179-1005617703-974104760-188059</systemId></policyUsers>"
//        L"<policyUsers><userSubjectType>windowsSid</userSubjectType><systemId>S-1-5-21-2018228179-1005617703-974104760-187941</systemId></policyUsers>"
//        L"<policyUsers><userSubjectType>windowsSid</userSubjectType><systemId>S-1-5-21-2018228179-1005617703-974104760-188025</systemId></policyUsers>"
//        L"<policyUsers><userSubjectType>windowsSid</userSubjectType><systemId>S-1-5-21-4129145287-642207298-4120045319-500</systemId></policyUsers>"
//        L"</policyAssemblyStatus>"
//        L"</heartbeat>"
//        L"</HeartBeatRequest>"
//        )
//    {
//    }
//
//    virtual ~rmc_heartbeat_request()
//    {
//    }
//};