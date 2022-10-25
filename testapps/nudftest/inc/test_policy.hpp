

#ifndef __TEST_POLICY_HPP__
#define __TEST_POLICY_HPP__

#include <gtest\gtest.h>

#include <nudf\exception.hpp>
#include <nudf\path.hpp>
#include <nxrmpkg.hpp>


extern const std::wstring sign_pub_cert;
extern const std::wstring sign_pri_cert;
extern const std::wstring exch_pub_cert;
extern const std::wstring exch_pri_cert;

class PolicySuite : public testing::Test
{
protected:
   static void SetUpTestCase()
   {
       std::cout << std::endl;
       std::cout << "TestSuite: Policy Engine" << std::endl;

       nudf::win::CModulePath mod(NULL);
       _bundlefile = mod.GetParentDir();
       _bundlefile+= L"\\policybundle.xml";
   }

   static void TearDownTestCase()
   {
   }

   static std::wstring _bundlefile;
};

std::wstring PolicySuite::_bundlefile;


bool AddUser(nxrm::engine::pkg::policy::CBundle* bundle, const wchar_t* sid, const std::set<unsigned int>& groups)
{
    nxrm::engine::pkg::policy::CUserGroups* user = new nxrm::engine::pkg::policy::CUserGroups(sid, groups);
    if(NULL == user) {
        return false;
    }
    bundle->AddUserGroup(user);
    return true;
}

bool AddRight(nxrm::engine::pkg::policy::CBundle* bundle, unsigned int id, const wchar_t* name, const wchar_t* display_name, bool custom)
{
    nxrm::engine::pkg::policy::CRight* right = new nxrm::engine::pkg::policy::CRight(id, name, display_name, custom);
    if(NULL == right) {
        return false;
    }
    bundle->AddRight(right);
    return true;
}

bool AddRule(nxrm::engine::pkg::policy::CBundle* bundle, unsigned int id, nxrm::engine::pkg::policy::RULETYPE type, const std::vector<nxrm::engine::pkg::policy::CPattern>& props, const wchar_t* comment)
{
    nxrm::engine::pkg::policy::CRule* rule = new nxrm::engine::pkg::policy::CRule(id, type, props, comment);
    if(NULL == rule) {
        return false;
    }
    bundle->AddRule(rule);
    return true;
}

nxrm::engine::pkg::policy::CPolicy* AddPolicy(nxrm::engine::pkg::policy::CBundle* bundle, const wchar_t* name, unsigned int id, unsigned int usergroup, unsigned __int64 rights, unsigned __int64 custom_rights)
{
    nxrm::engine::pkg::policy::CPolicy* policy = new nxrm::engine::pkg::policy::CPolicy(id, name, usergroup, rights, custom_rights);
    if(NULL == policy) {
        return NULL;
    }
    bundle->AddPolicy(policy);
    return policy;
}

TEST_F(PolicySuite, PolicyCreateBundle)
{
    HRESULT hr = S_OK;
    bool result = false;
    nxrm::engine::pkg::policy::CBundle bundle;

    // push user-groups
    nxrm::engine::pkg::policy::CRight* right = NULL;
    nxrm::engine::pkg::policy::CRule* rule = NULL;
    nxrm::engine::pkg::policy::CPolicy* policy = NULL;

    unsigned __int64 rights = 0;

    // User
    const std::wstring gye_sid(L"S-1-5-21-2242368633-1437193937-1178672470-1001");
    const unsigned int gye_group_ids[10] = {0U, 1U};
    const std::set<unsigned int> gye_group(gye_group_ids, gye_group_ids+1);
    const std::wstring john_sid(L"S-1-5-21-2242368633-1437193937-1178672470-1002");
    const unsigned int john_group_ids[10] = {0U};
    const std::set<unsigned int> john_group(john_group_ids, john_group_ids+1);
    const std::wstring admin_sid(L"S-1-5-21-2242368633-1437193937-1178672470-1003");
    const unsigned int admin_group_ids[10] = {0U, 1U, 2U};
    const std::set<unsigned int> admin_group(admin_group_ids, admin_group_ids+2);

    // User Pattern
    const nxrm::engine::pkg::policy::CPattern usr_pattern_bae_business(L"baesystemsintellectualpropertyaccessrights", L"intellectual property - business authorization category - identifier");
    const nxrm::engine::pkg::policy::CPattern usr_pattern_bae_internal(L"baesystemsintellectualpropertyaccessrights", L"intellectual property - internal authorization category - identifier");
    const nxrm::engine::pkg::policy::CPattern usr_pattern_bae_contractor(L"baesystemsintellectualpropertyaccessrights", L"intellectual property - contract authorization category - identifier");
    // Application Pattern
    const nxrm::engine::pkg::policy::CPattern app_pattern_word(L"path", L"**\\WINWORD.EXE");
    const nxrm::engine::pkg::policy::CPattern app_pattern_excel(L"path", L"**\\EXCEL.EXE");
    const nxrm::engine::pkg::policy::CPattern app_pattern_powerpnt(L"path", L"**\\POWERPNT.EXE");
    const nxrm::engine::pkg::policy::CPattern app_pattern_acrord(L"path", L"**\\ACRORD32.EXE");
    const nxrm::engine::pkg::policy::CPattern app_pattern_notepad(L"path", L"**\\NOTEPAD.EXE");
    const nxrm::engine::pkg::policy::CPattern app_pattern_wordpad(L"path", L"**\\WORDPAD.EXE");
    const nxrm::engine::pkg::policy::CPattern app_pattern_pub_msft(L"publisher", L"Microsoft Corporation");
    const nxrm::engine::pkg::policy::CPattern app_pattern_pub_adobe(L"publisher", L"Adobe Systems, Incorporated");
    const nxrm::engine::pkg::policy::CPattern app_pattern_pub_nextlabs(L"publisher", L"NextLabs, Inc");
    // Location Pattern
    const nxrm::engine::pkg::policy::CPattern loc_pattern_host_nextlabs(L"host", L"*.nextlabs.com");
    // Resource Pattern
    const nxrm::engine::pkg::policy::CPattern res_pattern_itar(L"itar", L"yes", nxrm::engine::pkg::policy::PropTag);
    const nxrm::engine::pkg::policy::CPattern res_pattern_pr01(L"project", L"PR-01", nxrm::engine::pkg::policy::PropTag);
    const nxrm::engine::pkg::policy::CPattern res_pattern_pr02(L"project", L"PR-02", nxrm::engine::pkg::policy::PropTag);

    
    nudf::crypto::CPkcs12CertContext pfxcert;
    nudf::crypto::CLegacyRsaPriKeyBlob prikey;

    hr = pfxcert.CreateFromFile(sign_pri_cert.c_str(), L"123blue!");
    EXPECT_TRUE(SUCCEEDED(hr));
    if(FAILED(hr)) {
        return;
    }

    hr = pfxcert.GetPrivateKeyBlob(prikey);
    EXPECT_TRUE(SUCCEEDED(hr));
    if(FAILED(hr)) {
        return;
    }


    try {

        std::vector<nxrm::engine::pkg::policy::CPattern> patterns;

        //
        //  Add user
        //
        AddUser(&bundle, gye_sid.c_str(), gye_group);
        AddUser(&bundle, john_sid.c_str(), john_group);
        AddUser(&bundle, admin_sid.c_str(), admin_group);

        //
        //  Add rights
        //
        AddRight(&bundle, 0U, L"VIEW", L"View", false);
        AddRight(&bundle, 1U, L"EDIT", L"Edit", false);
        AddRight(&bundle, 2U, L"PRINT", L"Print", false);
        AddRight(&bundle, 3U, L"COPY", L"Copy", false);
        AddRight(&bundle, 4U, L"EXTRACT", L"Extract", false);
        AddRight(&bundle, 5U, L"ANNOTATE", L"Annotate", false);
        AddRight(&bundle, 6U, L"DECRYPT", L"Decrypt", false);
        AddRight(&bundle, 7U, L"SCREENCAP", L"Screen Capture", false);
        AddRight(&bundle, 8U, L"SEND", L"Send", false);
        AddRight(&bundle, 9U, L"CONVERT", L"Convert", false);
        AddRight(&bundle, 10U, L"CLASSIFY", L"Classify", false);
        AddRight(&bundle, 56U, L"RIGHTSASSIGN", L"RightsAssign", false);
        AddRight(&bundle, 57U, L"RIGHTSEDIT", L"RightsEdit", false);

        //
        //  Add User Rules (Total: 3)
        //

        // # 0: BAE Authorized Business User
        patterns.clear();
        patterns.push_back(usr_pattern_bae_business);
        AddRule(&bundle, 0U, nxrm::engine::pkg::policy::RULE_USER, patterns, L"User: BAE Authorized Business User");

        // # 1: BAE Authorized Internal User
        patterns.clear();
        patterns.push_back(usr_pattern_bae_internal);
        AddRule(&bundle, 1U, nxrm::engine::pkg::policy::RULE_USER, patterns, L"User: BAE Authorized Internal User");

        // # 2: BAE Authorized Contractor
        patterns.clear();
        patterns.push_back(usr_pattern_bae_contractor);
        AddRule(&bundle, 2U, nxrm::engine::pkg::policy::RULE_USER, patterns, L"User: BAE Authorized Contractor");

        //
        //  Add Application Rules (Total: 7)
        //

        // # 0: Microsoft Office Word
        patterns.clear();
        patterns.push_back(app_pattern_word);
        patterns.push_back(app_pattern_pub_msft);
        AddRule(&bundle, 0U, nxrm::engine::pkg::policy::RULE_APPLICATION, patterns, L"Application: Microsoft Office Word");

        // # 1: Microsoft Office Excel
        patterns.clear();
        patterns.push_back(app_pattern_excel);
        patterns.push_back(app_pattern_pub_msft);
        AddRule(&bundle, 1U, nxrm::engine::pkg::policy::RULE_APPLICATION, patterns, L"Application: Microsoft Office Excel");

        // # 2: Microsoft Office PowerPoint
        patterns.clear();
        patterns.push_back(app_pattern_powerpnt);
        patterns.push_back(app_pattern_pub_msft);
        AddRule(&bundle, 2U, nxrm::engine::pkg::policy::RULE_APPLICATION, patterns, L"Application: Microsoft Office PowerPoint");

        // # 3: Acrobat Reader
        patterns.clear();
        patterns.push_back(app_pattern_acrord);
        patterns.push_back(app_pattern_pub_adobe);
        AddRule(&bundle, 3U, nxrm::engine::pkg::policy::RULE_APPLICATION, patterns, L"Application: Adobe Acrobat Reader");

        // # 4: Microsoft notepad
        patterns.clear();
        patterns.push_back(app_pattern_notepad);
        patterns.push_back(app_pattern_pub_msft);
        AddRule(&bundle, 4U, nxrm::engine::pkg::policy::RULE_APPLICATION, patterns, L"Application: Microsoft Notepad");

        // # 5: Microsoft wordpad
        patterns.clear();
        patterns.push_back(app_pattern_wordpad);
        patterns.push_back(app_pattern_pub_msft);
        AddRule(&bundle, 5U, nxrm::engine::pkg::policy::RULE_APPLICATION, patterns, L"Application: Microsoft Wordpad");

        // # 6: NextLabs Software
        patterns.clear();
        patterns.push_back(app_pattern_pub_nextlabs);
        AddRule(&bundle, 6U, nxrm::engine::pkg::policy::RULE_APPLICATION, patterns, L"Application: Nextlabs Software");
        
        //
        //  Add Location Rules (Total: 1)
        //

        // # 0: Host in NextLabs Domain
        patterns.clear();
        patterns.push_back(loc_pattern_host_nextlabs);
        AddRule(&bundle, 0U, nxrm::engine::pkg::policy::RULE_LOCATION, patterns, L"Location: In Nextlabs Domain");
        
        //
        //  Add Resource Rules (Total: 1)
        //

        // # 0: Itar files
        patterns.clear();
        patterns.push_back(res_pattern_itar);
        AddRule(&bundle, 0U, nxrm::engine::pkg::policy::RULE_RESOURCE, patterns, L"Resource: Itar data");

        // # 1: Project PR-01 Data
        patterns.clear();
        patterns.push_back(res_pattern_pr01);
        AddRule(&bundle, 1U, nxrm::engine::pkg::policy::RULE_RESOURCE, patterns, L"Resource: Project PR-01 Data");

        // # 2: Project PR-02 Data
        patterns.clear();
        patterns.push_back(res_pattern_pr02);
        AddRule(&bundle, 2U, nxrm::engine::pkg::policy::RULE_RESOURCE, patterns, L"Resource: Project PR-02 Data");
        
        //
        //  Add Policies (Total: 1)
        //
        nxrm::engine::pkg::policy::CPolicy* policy = NULL;

        // # 0: Allow User Group 0 to View/Edit/Print/Copy/Extract All PR-01 document when it is in NextLabs domain
        rights = bundle.GetRightsSet()[0]->GetValue()      // View
                 + bundle.GetRightsSet()[1]->GetValue()    // Edit
                 + bundle.GetRightsSet()[2]->GetValue()    // Print
                 + bundle.GetRightsSet()[3]->GetValue()    // Copy
                 + bundle.GetRightsSet()[4]->GetValue();   // Extract
        policy = AddPolicy(&bundle,
                           L"Secure Collaboration - PR-01: Allow user group 0 to View/Edit/Print/Copy/Extract All PR-01 document when it is in NextLabs domain",
                           0,
                           0,
                           rights,
                           0);
        policy->GetRules()[nxrm::engine::pkg::policy::RULE_LOCATION].first.push_back(nxrm::engine::pkg::policy::RULEOBJ(0, bundle.GetRulesSet()[nxrm::engine::pkg::policy::RULE_LOCATION][0].get()));
        policy->GetRules()[nxrm::engine::pkg::policy::RULE_RESOURCE].first.push_back(nxrm::engine::pkg::policy::RULEOBJ(1, bundle.GetRulesSet()[nxrm::engine::pkg::policy::RULE_RESOURCE][1].get()));

        // # 1: Allow User Group 1 to View/Edit/Print/Copy/Extract/Annotate All PR-02 document when it is in NextLabs domain
        rights = bundle.GetRightsSet()[0]->GetValue()      // View
                 + bundle.GetRightsSet()[1]->GetValue()    // Edit
                 + bundle.GetRightsSet()[2]->GetValue()    // Print
                 + bundle.GetRightsSet()[3]->GetValue()    // Copy
                 + bundle.GetRightsSet()[4]->GetValue()    // Extract
                 + bundle.GetRightsSet()[5]->GetValue();   // Annotate
        policy = AddPolicy(&bundle,
                           L"Secure Collaboration - PR-02: Allow User Group 1 to View/Edit/Print/Copy/Extract/Annotate All PR-02 document when it is in NextLabs domain",
                           1,
                           1,
                           rights,
                           0);
        policy->GetRules()[nxrm::engine::pkg::policy::RULE_LOCATION].first.push_back(nxrm::engine::pkg::policy::RULEOBJ(0, bundle.GetRulesSet()[nxrm::engine::pkg::policy::RULE_LOCATION][0].get()));
        policy->GetRules()[nxrm::engine::pkg::policy::RULE_RESOURCE].first.push_back(nxrm::engine::pkg::policy::RULEOBJ(2, bundle.GetRulesSet()[nxrm::engine::pkg::policy::RULE_RESOURCE][2].get()));

        // # 2: Allow User Group 2 to View/Edit/Classify/RightsAssign/RightsEdit All document when it is in NextLabs domain
        rights = bundle.GetRightsSet()[0]->GetValue()      // View
                 + bundle.GetRightsSet()[1]->GetValue()    // Edit
                 + bundle.GetRightsSet()[10]->GetValue()   // Classify
                 + bundle.GetRightsSet()[11]->GetValue()   // RightsAssign
                 + bundle.GetRightsSet()[12]->GetValue();  // RightsEdit
        policy = AddPolicy(&bundle,
                           L"Secure Collaboration - Allow User Group 2 to View/Edit/Classify/RightsAssign/RightsEdit All document when it is in NextLabs domain",
                           2,
                           2,
                           rights,
                           0);
        policy->GetRules()[nxrm::engine::pkg::policy::RULE_LOCATION].first.push_back(nxrm::engine::pkg::policy::RULEOBJ(0, bundle.GetRulesSet()[nxrm::engine::pkg::policy::RULE_LOCATION][0].get()));

        // # 3: Allow User Group 0 to Annotate/Send/Convert/Classify All PR-01 document if application is Office Word/Excel/PowerPoint/Adobereader
        rights = bundle.GetRightsSet()[5]->GetValue()      // Annotate
                 + bundle.GetRightsSet()[8]->GetValue()    // Send
                 + bundle.GetRightsSet()[9]->GetValue()    // Convert
                 + bundle.GetRightsSet()[10]->GetValue();  // Classify
        policy = AddPolicy(&bundle,
                           L"Secure Collaboration - Allow User Group 0 to Annotate/Send/Convert/Classify All PR-01 document if application is Office Word/Excel/PowerPoint/Adobereader",
                           3,
                           0,
                           rights,
                           0);
        policy->GetRules()[nxrm::engine::pkg::policy::RULE_RESOURCE].first.push_back(nxrm::engine::pkg::policy::RULEOBJ(1, bundle.GetRulesSet()[nxrm::engine::pkg::policy::RULE_RESOURCE][1].get()));
        policy->GetRules()[nxrm::engine::pkg::policy::RULE_APPLICATION].first.push_back(nxrm::engine::pkg::policy::RULEOBJ(0, bundle.GetRulesSet()[nxrm::engine::pkg::policy::RULE_APPLICATION][0].get()));
        policy->GetRules()[nxrm::engine::pkg::policy::RULE_APPLICATION].first.push_back(nxrm::engine::pkg::policy::RULEOBJ(1, bundle.GetRulesSet()[nxrm::engine::pkg::policy::RULE_APPLICATION][1].get()));
        policy->GetRules()[nxrm::engine::pkg::policy::RULE_APPLICATION].first.push_back(nxrm::engine::pkg::policy::RULEOBJ(2, bundle.GetRulesSet()[nxrm::engine::pkg::policy::RULE_APPLICATION][2].get()));
        policy->GetRules()[nxrm::engine::pkg::policy::RULE_APPLICATION].first.push_back(nxrm::engine::pkg::policy::RULEOBJ(3, bundle.GetRulesSet()[nxrm::engine::pkg::policy::RULE_APPLICATION][3].get()));

        // # 4: Allow User Group 1 to Annotate/Send/Convert/Classify All PR-02 document if application is Office Word/Excel/PowerPoint/Adobereader
        rights = bundle.GetRightsSet()[5]->GetValue()      // Annotate
                 + bundle.GetRightsSet()[8]->GetValue()    // Send
                 + bundle.GetRightsSet()[9]->GetValue()    // Convert
                 + bundle.GetRightsSet()[10]->GetValue();  // Classify
        policy = AddPolicy(&bundle,
                           L"Secure Collaboration - Allow User Group 1 to Annotate/Send/Convert/Classify All PR-02 document if application is Office Word/Excel/PowerPoint/Adobereader",
                           4,
                           1,
                           rights,
                           0);
        policy->GetRules()[nxrm::engine::pkg::policy::RULE_RESOURCE].first.push_back(nxrm::engine::pkg::policy::RULEOBJ(2, bundle.GetRulesSet()[nxrm::engine::pkg::policy::RULE_RESOURCE][2].get()));
        policy->GetRules()[nxrm::engine::pkg::policy::RULE_APPLICATION].first.push_back(nxrm::engine::pkg::policy::RULEOBJ(0, bundle.GetRulesSet()[nxrm::engine::pkg::policy::RULE_APPLICATION][0].get()));
        policy->GetRules()[nxrm::engine::pkg::policy::RULE_APPLICATION].first.push_back(nxrm::engine::pkg::policy::RULEOBJ(1, bundle.GetRulesSet()[nxrm::engine::pkg::policy::RULE_APPLICATION][1].get()));
        policy->GetRules()[nxrm::engine::pkg::policy::RULE_APPLICATION].first.push_back(nxrm::engine::pkg::policy::RULEOBJ(2, bundle.GetRulesSet()[nxrm::engine::pkg::policy::RULE_APPLICATION][2].get()));
        policy->GetRules()[nxrm::engine::pkg::policy::RULE_APPLICATION].first.push_back(nxrm::engine::pkg::policy::RULEOBJ(3, bundle.GetRulesSet()[nxrm::engine::pkg::policy::RULE_APPLICATION][3].get()));

        // # 5: Nextlabs Application has all rights on all documents
        rights = 0xFFFFFFFFFFFFFFFF;  // All Rights
        policy = AddPolicy(&bundle,
                           L"Secure Collaboration - Nextlabs Application has all rights on all documents",
                           5,
                           0,
                           rights,
                           0);
        policy->GetRules()[nxrm::engine::pkg::policy::RULE_APPLICATION].first.push_back(nxrm::engine::pkg::policy::RULEOBJ(6, bundle.GetRulesSet()[nxrm::engine::pkg::policy::RULE_APPLICATION][6].get()));

        // # 6: BAE Internal has all rights on all documents
        rights = 0xFFFFFFFFFFFFFFFF;  // All Rights
        policy = AddPolicy(&bundle,
                           L"Secure Collaboration - BAE Internal has all rights on all documents",
                           6,
                           -1,
                           rights,
                           0);
        policy->GetRules()[nxrm::engine::pkg::policy::RULE_USER].first.push_back(nxrm::engine::pkg::policy::RULEOBJ(1, bundle.GetRulesSet()[nxrm::engine::pkg::policy::RULE_USER][1].get()));

        // # 7: BAE Contractor has VIEW rights on all documents
        rights = bundle.GetRightsSet()[0]->GetValue()      // View
                 + bundle.GetRightsSet()[1]->GetValue()    // Edit
                 + bundle.GetRightsSet()[2]->GetValue()    // Print
                 + bundle.GetRightsSet()[3]->GetValue()    // Copy
                 + bundle.GetRightsSet()[4]->GetValue();   // Extract
        policy = AddPolicy(&bundle,
                           L"Secure Collaboration - BAE Contractor has VIEW rights on all documents",
                           7,
                           -1,
                           rights,
                           0);
        policy->GetRules()[nxrm::engine::pkg::policy::RULE_USER].first.push_back(nxrm::engine::pkg::policy::RULEOBJ(2, bundle.GetRulesSet()[nxrm::engine::pkg::policy::RULE_USER][2].get()));

        // # 8: BAE Business User has VIEW / EDIT / SEND rights on all documents
        rights = bundle.GetRightsSet()[0]->GetValue()      // View
                 + bundle.GetRightsSet()[1]->GetValue()    // Edit
                 + bundle.GetRightsSet()[8]->GetValue();   // Send
        policy = AddPolicy(&bundle,
                           L"Secure Collaboration - BAE Business User has VIEW / EDIT / SEND rights on all documents",
                           8,
                           -1,
                           rights,
                           0);
        policy->GetRules()[nxrm::engine::pkg::policy::RULE_USER].first.push_back(nxrm::engine::pkg::policy::RULEOBJ(0, bundle.GetRulesSet()[nxrm::engine::pkg::policy::RULE_USER][0].get()));



        bundle.ToFile(PolicySuite::_bundlefile.c_str(), L"Secure Collaboration Server 001", prikey);

        result = true;
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        bundle.Clear();
        result = false;
    }
    catch(...) {
        bundle.Clear();
        result = false;
    }

    EXPECT_TRUE(result);
}

TEST_F(PolicySuite, PolicyEvaluate)
{
    bool result = false;
    nxrm::engine::pkg::policy::CEvalBundle bundle;

    // User Properties
    const nxrm::engine::pkg::policy::CProperty user1_sid(L"id", L"S-1-5-21-2242368633-1437193937-1178672470-1001");
    const nxrm::engine::pkg::policy::CProperty user1_name(L"name", L"john.tyler@nextlabs.com");
    const nxrm::engine::pkg::policy::CProperty user2_sid(L"id", L"S-1-5-21-2242368633-1437193937-1178672470-1002");
    const nxrm::engine::pkg::policy::CProperty user2_name(L"name", L"Jimmy.Carter@nextlabs.com");
    const nxrm::engine::pkg::policy::CProperty user3_sid(L"id", L"S-1-5-21-2242368633-1437193937-1178672470-1003");
    const nxrm::engine::pkg::policy::CProperty user3_name(L"name", L"Administrator@nextlabs.com");
    const nxrm::engine::pkg::policy::CProperty user4_sid(L"id", L"S-1-5-21-2242368633-1437193937-1178672470-1004");
    const nxrm::engine::pkg::policy::CProperty user4_name(L"name", L"sherlock.holmes@bae.com");
    const nxrm::engine::pkg::policy::CProperty user4_attr(L"baesystemsintellectualpropertyaccessrights", L"intellectual property - business authorization category - identifier");
    const nxrm::engine::pkg::policy::CProperty user5_sid(L"id", L"S-1-5-21-2242368633-1437193937-1178672470-1005");
    const nxrm::engine::pkg::policy::CProperty user5_name(L"name", L"Administrator@bae.com");
    const nxrm::engine::pkg::policy::CProperty user5_attr(L"baesystemsintellectualpropertyaccessrights", L"intellectual property - internal authorization category - identifier");
    const nxrm::engine::pkg::policy::CProperty user6_sid(L"id", L"S-1-5-21-2242368633-1437193937-1178672470-1006");
    const nxrm::engine::pkg::policy::CProperty user6_name(L"name", L"edward.Kolin@contract.bae.com");
    const nxrm::engine::pkg::policy::CProperty user6_attr(L"baesystemsintellectualpropertyaccessrights", L"intellectual property - contract authorization category - identifier");

    // Application Properties
    const nxrm::engine::pkg::policy::CProperty app0_path(L"path", L"C:\\Program Files (x86)\\Microsoft Office\\Office15\\WINWORD.EXE");
    const nxrm::engine::pkg::policy::CProperty app0_pub(L"publisher", L"Microsoft Corporation");
    const nxrm::engine::pkg::policy::CProperty app1_path(L"path", L"C:\\Program Files (x86)\\Microsoft Office\\Office15\\EXCEL.EXE");
    const nxrm::engine::pkg::policy::CProperty app1_pub(L"publisher", L"Microsoft Corporation");
    const nxrm::engine::pkg::policy::CProperty app2_path(L"path", L"C:\\Program Files (x86)\\Microsoft Office\\Office15\\POWERPNT.EXE");
    const nxrm::engine::pkg::policy::CProperty app2_pub(L"publisher", L"Microsoft Corporation");
    const nxrm::engine::pkg::policy::CProperty app3_path(L"path", L"C:\\Program Files (x86)\\Adobe\\Reader 11.0\\Reader\\AcroRd32.exe");
    const nxrm::engine::pkg::policy::CProperty app3_pub(L"publisher", L"Adobe Systems, Incorporated");
    const nxrm::engine::pkg::policy::CProperty app4_path(L"path", L"C:\\Program Files (x86)\\Microsoft Office\\Office15\\NOTEPAD.EXE");
    const nxrm::engine::pkg::policy::CProperty app4_pub(L"publisher", L"Microsoft Corporation");
    const nxrm::engine::pkg::policy::CProperty app5_path(L"path", L"C:\\Program Files (x86)\\Microsoft Office\\Office15\\WORDPAD.EXE");
    const nxrm::engine::pkg::policy::CProperty app5_pub(L"publisher", L"Microsoft Corporation");
    const nxrm::engine::pkg::policy::CProperty app6_path(L"path", L"C:\\Program Files\\NextLabs\\Rights Management\\bin\\nxrmserv.exe");
    const nxrm::engine::pkg::policy::CProperty app6_pub(L"publisher", L"NextLabs, Inc");
    const nxrm::engine::pkg::policy::CProperty app7_path(L"path", L"C:\\Program Files (x86)\\Notepad++\\notepad++.exe");

    // Location Properties
    const nxrm::engine::pkg::policy::CProperty loc0_host(L"host", L"segula.nextlabs.com");
    const nxrm::engine::pkg::policy::CProperty loc1_host(L"host", L"bayarea.test.nextlabs.com");
    const nxrm::engine::pkg::policy::CProperty loc2_host(L"host", L"abbu");
    const nxrm::engine::pkg::policy::CProperty loc3_host(L"host", L"shasta.bae.com");

    // Resource Properties
    const nxrm::engine::pkg::policy::CProperty res0_path(L"path", L"C:\\Users\\john.tyler\\Documents\\itar\\design.docx");
    const nxrm::engine::pkg::policy::CProperty res0_date(L"itar", L"yes", nxrm::engine::pkg::policy::PropTag);
    const nxrm::engine::pkg::policy::CProperty res1_path(L"path", L"C:\\Users\\john.tyler\\Documents\\project\\01\\datasheet.xlsx");
    const nxrm::engine::pkg::policy::CProperty res1_project(L"project", L"pr-01", nxrm::engine::pkg::policy::PropTag);
    const nxrm::engine::pkg::policy::CProperty res2_path(L"path", L"C:\\Users\\john.tyler\\Documents\\project\\02\\datasheet.xlsx");
    const nxrm::engine::pkg::policy::CProperty res2_project(L"project", L"pr-02", nxrm::engine::pkg::policy::PropTag);
    const nxrm::engine::pkg::policy::CProperty res3_path(L"path", L"C:\\Users\\john.tyler\\Documents\\BAE\\data\\design.dxf");
    const nxrm::engine::pkg::policy::CProperty res3_owner(L"owner", L"BAE Systems", nxrm::engine::pkg::policy::PropTag);
    const nxrm::engine::pkg::policy::CProperty res3_pwneridp(L"owner_idp", L"https://www.baesystems.com/authenticate.aspx", nxrm::engine::pkg::policy::PropTag);

    try {

        bundle.LoadFromFile(_bundlefile.c_str());

        const nxrm::engine::pkg::policy::POLICYGROUP& policygroup = bundle.GetPolicySet();


        nxrm::engine::pkg::policy::CPolicySet  eval_policyset;
        nxrm::engine::pkg::policy::CEvalObject eval_object;
        nxrm::engine::pkg::policy::CEvalResult eval_result;
        std::wstring    wsRights;
        std::wstring    wsCustomRights;

        // Create PolicySet
        //    - Use all policies
        for(nxrm::engine::pkg::policy::POLICYGROUP::const_iterator it=policygroup.begin(); it!=policygroup.end(); ++it) {
            eval_policyset.GetPolicySet().push_back((*it).get());
        }


        //
        //  TEST 1
        //    - 
        //
        eval_object.Clear();
        eval_result.Clear();
        wsRights = L"";
        wsCustomRights = L"";
        bundle.GetGroupIdSet(user1_sid.GetValue(), BUILTIN_NEXTLABS_TENANTID, eval_object.GetUserGroups());
        eval_object.GetProperty(nxrm::engine::pkg::policy::RULE_USER)[L"id"] = user1_sid;
        eval_object.GetProperty(nxrm::engine::pkg::policy::RULE_USER)[L"name"] = user1_name;
        eval_object.GetProperty(nxrm::engine::pkg::policy::RULE_APPLICATION)[L"path"] = app0_path;
        eval_object.GetProperty(nxrm::engine::pkg::policy::RULE_APPLICATION)[L"publisher"] = app0_pub;
        eval_object.GetProperty(nxrm::engine::pkg::policy::RULE_LOCATION)[L"host"] = loc0_host;
        eval_object.GetProperty(nxrm::engine::pkg::policy::RULE_RESOURCE)[L"path"] = res1_path;
        eval_object.GetProperty(nxrm::engine::pkg::policy::RULE_RESOURCE)[L"project"] = res1_project;
        eval_policyset.Evaluate(eval_object, eval_result);
        wsRights = bundle.RightsToString(eval_result.GetRights(), false);
        wsCustomRights = bundle.RightsToString(eval_result.GetCustomRights(), true);
        printf("Evaluate 00:\n");
        printf("  - Rights: %S\n", wsRights.c_str());
        printf("  - Custom Rights: %S\n", wsCustomRights.c_str());
        printf("  - Policies:\n", eval_result.GetCustomRights());
        for(std::vector<nxrm::engine::pkg::policy::CEvalPolicyResult>::const_iterator it=eval_result.GetPolicies().begin(); it!=eval_result.GetPolicies().end(); ++it) {
            wsRights = bundle.RightsToString((*it).GetRights(), false);
            wsCustomRights = bundle.RightsToString((*it).GetCustomRights(), true);
            printf("      * #%d - %S\n", (*it).GetId(), (*it).GetName().c_str());
            printf("         > Rights: %S\n", wsRights.c_str());
            printf("         > Custom Rights: %S\n", wsCustomRights.c_str());
        }
        EXPECT_TRUE(eval_result.GetRights() != 0);

        result = true;
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        bundle.Clear();
        result = false;
    }
    catch(...) {
        bundle.Clear();
        result = false;
    }

    EXPECT_TRUE(result);

}


#endif