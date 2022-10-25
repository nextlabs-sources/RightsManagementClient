

#include <windows.h>

#include <iostream>

#include "nxpolicyengine.h"
#include "policy_impl.hpp"
#include "policy_parser.hpp"


static const wchar_t* PQL_STR = L"(user.GROUP has 14 AND ((application.path = \"**\\\\Excel.exe\" AND application.publisher = \"Microsoft Corporation\") OR (application.path = \"**\\\\AcroRd32.exe\" AND application.publisher = \"Adobe Systems, Incorporated\") OR (application.path = \"**\\\\POWERPNT.EXE\" AND application.publisher = \"Microsoft Corporation\") OR (application.path = \"**\\\\WINWORD.EXE\" AND application.publisher = \"Microsoft Corporation\") OR (application.path = \"**\\\\VEViewer.exe\" AND application.publisher = \"sap ag\") OR application.name = \"RMS\"))";

static const wchar_t* PQL_STR2 = L"(TRUE AND ((ENVIRONMENT.REMOTE_ACCESS = 0 AND ENVIRONMENT.TIME_SINCE_LAST_HEARTBEAT > 6000 AND CURRENT_TIME.identity >= \"Apr 12, 2016 1:24:00 PM\" AND CURRENT_TIME.identity <= \"Aug 31, 2016 1:24:48 PM\" AND CURRENT_TIME.time >= \"1:24:00 AM\" AND CURRENT_TIME.time <= \"6:24:08 PM\" AND (CURRENT_TIME.weekday = \"monday\" OR CURRENT_TIME.weekday = \"tuesday\" OR CURRENT_TIME.weekday = \"wednesday\" OR CURRENT_TIME.weekday = \"thursday\" OR CURRENT_TIME.weekday = \"friday\")) AND user.name = resource.fso.owner_name))";


static const wchar_t* PQL_STR3 = L"((resource.fso.name = \"\\\\fre_filer02**\" OR resource.fso.name = \"\\\\fre_filer03**\" OR resource.fso.name = \"\\\\PTWNFILE01**\" OR resource.fso.name = \"\\\\PTWNFILE02**\" OR resource.fso.name = \"\\\\PTWNFILE03**\" OR resource.fso.name = \"\\\\PTWNFILE05**\" OR resource.fso.name = \"\\\\PJPNFILE01**\" OR resource.fso.name = \"\\\\PJPNFILE03**\" OR resource.fso.name = \"\\\\pjpnfileyo**\" OR resource.fso.name = \"\\\\PSINFILE01**\" OR resource.fso.name = \"\\\\PKORFILE01**\" OR resource.fso.name = \"\\\\PTWNFILE04**\" OR resource.fso.name = \"\\\\PINDFILE01**\" OR resource.fso.name = \"\\\\BAINFS1**\" OR resource.fso.name = \"\\\\ZAPHOD**\" OR resource.fso.name = \"\\\\freebie\\modeling\\reports**\" OR resource.fso.name = \"\\\\appserver.metryx.local**\" OR resource.fso.name = \"\\\\server.metryx.local**\" OR resource.fso.name = \"\\\\softwareserver.metryx.local**\" OR resource.fso.name = \"\\\\terminalserver.metryx.local**\" OR resource.fso.name = \"\\\\windchill.metryx.local**\") AND NOT ((resource.fso.name = \"\\\\PTWNFILE02\\print$\\**\" OR resource.fso.name = \"\\\\PJPNFILE01\\print$\\**\" OR resource.fso.name = \"\\\\PKORFILE01\\print$\\**\" OR resource.fso.name = \"\\\\PTWNFILE03\\print$\\**\" OR resource.fso.name = \"\\\\PTWNFILE04\\print$\\**\" OR resource.fso.name = \"\\\\fre_itsupport\\sw_dist\\policyrule\\safesender.txt\" OR resource.fso.name = \"\\\\ZAPHOD\\shared5**\")))";

static const wchar_t* PQL_STR4 = L"(resource.fso.name = \"test.docx\" AND NOT (resource.fso.tag.name.not_a_book = \"this AND OR NOT hell\"))";


void main()
{
    NXPOLICY_IMPL::condition_map    cmap;
    std::shared_ptr<NXPOLICY_IMPL::condition_object> sp = NXPOLICY_IMPL::parse_pql(PQL_STR3, cmap);

    if (sp != nullptr) {
        std::wstring s = sp->serialize();

        std::wcout << L"Policy is:" << std::endl;
        std::wcout << s << std::endl;
    }
    else {
        std::wcout << L"failed:" << std::endl;
    }

    try {
        std::shared_ptr<NXPOLICY_IMPL::policy_bundle> spbundle = NXPOLICY_IMPL::parse_pql_bundle_file(L"d:\\lam.out.txt");
        if (spbundle != nullptr) {
            std::wstring bundle_str = spbundle->serialize();
            std::wcout << L"Policy Bundle is:" << std::endl;
            std::wcout << bundle_str << std::endl;
        }
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }

}