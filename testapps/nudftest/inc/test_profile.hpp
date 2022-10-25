

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
extern std::wstring gDir;
extern std::wstring gProfile;

class ProfileSuite : public testing::Test
{
protected:
   static void SetUpTestCase()
   {
       std::cout << std::endl;
       std::cout << "TestSuite: Policy Engine" << std::endl;
   }

   static void TearDownTestCase()
   {
   }
};



bool AddUser(nxrm::pkg::CPolicyProfile* profile, const wchar_t* sid, const std::set<unsigned int>& groups)
{
    nxrm::pkg::policy::CUserGroups* user = new nxrm::pkg::policy::CUserGroups(sid, groups);
    if(NULL == user) {
        return false;
    }
    profile->AddUserGroup(user);
    return true;
}

bool AddRight(nxrm::pkg::CPolicyProfile* profile, unsigned int id, const wchar_t* name, const wchar_t* display_name, bool custom)
{
    nxrm::pkg::policy::CRight* right = new nxrm::pkg::policy::CRight(id, name, display_name, custom);
    if(NULL == right) {
        return false;
    }
    profile->AddRight(right);
    return true;
}

bool AddRule(nxrm::pkg::CPolicyProfile* profile, unsigned int id, nxrm::pkg::policy::RULETYPE type, const std::vector<nxrm::pkg::policy::CPattern>& props, const wchar_t* comment)
{
    nxrm::pkg::policy::CRule* rule = new nxrm::pkg::policy::CRule(id, type, props, comment);
    if(NULL == rule) {
        return false;
    }
    profile->AddRule(rule);
    return true;
}

nxrm::pkg::policy::CPolicy* AddPolicy(nxrm::pkg::CPolicyProfile* profile, const wchar_t* name, unsigned int id, unsigned int usergroup, unsigned __int64 rights, unsigned __int64 custom_rights)
{
    nxrm::pkg::policy::CPolicy* policy = new nxrm::pkg::policy::CPolicy(id, name, usergroup, rights, custom_rights);
    if(NULL == policy) {
        return NULL;
    }
    profile->AddPolicy(policy);
    return policy;
}

TEST_F(ProfileSuite, Create)
{
    HRESULT hr = S_OK;
    bool result = false;
    nxrm::pkg::CProfileBundle profile;

    // Agent Profile
    profile.GetAgentProfile().SetId(L"1");
    profile.GetAgentProfile().SetName(L"Common Agent Profile");
    profile.GetAgentProfile().SetTimestamp(L"2015-03-25T10:12:48Z");
    profile.GetAgentProfile().SetAgentId(L"8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942");
    profile.GetAgentProfile().SetAgentType(L"DESKTOP");

    // Communication Profile
    profile.GetCommProfile().SetId(L"1");
    profile.GetCommProfile().SetName(L"Common Communication Profile");
    profile.GetCommProfile().SetTimestamp(L"2015-03-25T10:12:48Z");
    profile.GetCommProfile().SetServHost(L"www.nextlabs.com");
    profile.GetCommProfile().SetServPort(8443);
    profile.GetCommProfile().SetServPath(L"/rms");
    profile.GetCommProfile().SetHeartBeatInterval(86400);
    profile.GetCommProfile().SetLogInterval(60);
    profile.GetCommProfile().SetLogSize(1048576);

    // Classify Profile

    // Key Profile
    profile.GetKeyProfile().InsertKey(L"2E4823829738FF6E0FA3EC2B500EBB68198AF80E", L"E62FE37691DB8A6FEEDFCC773C5F16A9230725E15DD7F497D9620486A9CC7977");
    profile.GetKeyProfile().InsertKey(L"E32A3EE3C6D9DB8C8FB2FEA0945E3B93BA5F2E82", L"F9B2B0EF90EFBAE6E1CCE00FAA834E10C00E4FD7040E8E7024E166EF8A6A89B1");
    profile.GetKeyProfile().InsertKey(L"E8839F1EA9362B7274E3D0DB37EF3AB86E59DA10", L"D438AEE2FF0EA8FA09B9E23A73A6E5BCAF285110F400B59A0BE092848613C21E");
    profile.GetKeyProfile().InsertKey(L"8EA8C7963E8CA026B0A58C5D06803CE6D69B4F43", L"492CCA229A996079697432727A6900B41D3F8F1D8B74FB93A75EB5BF824954A9");
    profile.GetKeyProfile().InsertKey(L"5AAA0CFE188544210EF8C2F4016A5A0701B756F0", L"59AB7B2C52DFC13B573C263BA94A99B96945303CC77B8BB8FF3DC2E0933B1370");
    profile.GetKeyProfile().InsertKey(L"CE91ADA0D5C92F5243511F0B23087EE4B50FE42D", L"5DCA82D6D013BC284894A40045571CE0765E369EDFC634E1BAF8F650FF4EC427");
    profile.GetKeyProfile().InsertKey(L"6C039D046966AFB34EA8C96F5107B8246EEA4412", L"155E06021E6CC7A514F724AFCEFC3BFE1D0D24A49C31643FB91CBB345AABF9D1");
    profile.GetKeyProfile().InsertKey(L"C77F02A82E6E8A46FE182F8B39B4576D66D17F08", L"A5A23A5AF9A115F5511D09EFB0A7DDF221E7686F545710FFAF71CCFCF5489BB4");
    profile.GetKeyProfile().InsertKey(L"729EDF94781049F3613EAE44738E40151EDE6EB6", L"46620858ED08A2C60738FBDB5A92C0667E67B7D15796B467A7B794CB5D6E8102");
    profile.GetKeyProfile().InsertKey(L"EEFCBAB37F049DA6514BC1B16422CF28C72F2B69", L"BE60D1A49F913CBC59165712EA6BFEAC3BB1A9E6E7769D8DC024CCD00BB7DB82");
    profile.GetKeyProfile().UpdateChecksum();

    // Policy Profile
    profile.GetPolicyProfile().SetTimestamp(L"2015-03-25T22:30:15Z");

    // push user-groups
    nxrm::pkg::policy::CRight* right = NULL;
    nxrm::pkg::policy::CRule* rule = NULL;
    nxrm::pkg::policy::CPolicy* policy = NULL;

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
    const nxrm::pkg::policy::CPattern usr_pattern_bae_business(L"baesystemsintellectualpropertyaccessrights", L"intellectual property - business authorization category - identifier");
    const nxrm::pkg::policy::CPattern usr_pattern_bae_internal(L"baesystemsintellectualpropertyaccessrights", L"intellectual property - internal authorization category - identifier");
    const nxrm::pkg::policy::CPattern usr_pattern_bae_contractor(L"baesystemsintellectualpropertyaccessrights", L"intellectual property - contract authorization category - identifier");
    // Application Pattern
    const nxrm::pkg::policy::CPattern app_pattern_word(L"path", L"**\\WINWORD.EXE");
    const nxrm::pkg::policy::CPattern app_pattern_excel(L"path", L"**\\EXCEL.EXE");
    const nxrm::pkg::policy::CPattern app_pattern_powerpnt(L"path", L"**\\POWERPNT.EXE");
    const nxrm::pkg::policy::CPattern app_pattern_acrord(L"path", L"**\\ACRORD32.EXE");
    const nxrm::pkg::policy::CPattern app_pattern_notepad(L"path", L"**\\NOTEPAD.EXE");
    const nxrm::pkg::policy::CPattern app_pattern_wordpad(L"path", L"**\\WORDPAD.EXE");
    const nxrm::pkg::policy::CPattern app_pattern_pub_msft(L"publisher", L"Microsoft Corporation");
    const nxrm::pkg::policy::CPattern app_pattern_pub_adobe(L"publisher", L"Adobe Systems, Incorporated");
    const nxrm::pkg::policy::CPattern app_pattern_pub_nextlabs(L"publisher", L"NextLabs, Inc");
    // Location Pattern
    const nxrm::pkg::policy::CPattern loc_pattern_host_nextlabs(L"host", L"*.nextlabs.com");
    // Resource Pattern
    const nxrm::pkg::policy::CPattern res_pattern_itar(L"itar", L"yes", nxrm::pkg::policy::PropTag);
    const nxrm::pkg::policy::CPattern res_pattern_pr01(L"project", L"PR-01", nxrm::pkg::policy::PropTag);
    const nxrm::pkg::policy::CPattern res_pattern_pr02(L"project", L"PR-02", nxrm::pkg::policy::PropTag);

    
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

        std::vector<nxrm::pkg::policy::CPattern> patterns;

        //
        //  Add user
        //
        AddUser(&profile.GetPolicyProfile(), gye_sid.c_str(), gye_group);
        AddUser(&profile.GetPolicyProfile(), john_sid.c_str(), john_group);
        AddUser(&profile.GetPolicyProfile(), admin_sid.c_str(), admin_group);

        //
        //  Add rights
        //
        AddRight(&profile.GetPolicyProfile(), 0U, L"VIEW", L"View", false);
        AddRight(&profile.GetPolicyProfile(), 1U, L"EDIT", L"Edit", false);
        AddRight(&profile.GetPolicyProfile(), 2U, L"PRINT", L"Print", false);
        AddRight(&profile.GetPolicyProfile(), 3U, L"COPY", L"Copy", false);
        AddRight(&profile.GetPolicyProfile(), 4U, L"EXTRACT", L"Extract", false);
        AddRight(&profile.GetPolicyProfile(), 5U, L"ANNOTATE", L"Annotate", false);
        AddRight(&profile.GetPolicyProfile(), 6U, L"DECRYPT", L"Decrypt", false);
        AddRight(&profile.GetPolicyProfile(), 7U, L"SCREENCAP", L"Screen Capture", false);
        AddRight(&profile.GetPolicyProfile(), 8U, L"SEND", L"Send", false);
        AddRight(&profile.GetPolicyProfile(), 9U, L"CONVERT", L"Convert", false);
        AddRight(&profile.GetPolicyProfile(), 10U, L"CLASSIFY", L"Classify", false);
        AddRight(&profile.GetPolicyProfile(), 56U, L"RIGHTSASSIGN", L"RightsAssign", false);
        AddRight(&profile.GetPolicyProfile(), 57U, L"RIGHTSEDIT", L"RightsEdit", false);

        //
        //  Add User Rules (Total: 3)
        //

        // # 0: BAE Authorized Business User
        patterns.clear();
        patterns.push_back(usr_pattern_bae_business);
        AddRule(&profile.GetPolicyProfile(), 0U, nxrm::pkg::policy::RULE_USER, patterns, L"User: BAE Authorized Business User");

        // # 1: BAE Authorized Internal User
        patterns.clear();
        patterns.push_back(usr_pattern_bae_internal);
        AddRule(&profile.GetPolicyProfile(), 1U, nxrm::pkg::policy::RULE_USER, patterns, L"User: BAE Authorized Internal User");

        // # 2: BAE Authorized Contractor
        patterns.clear();
        patterns.push_back(usr_pattern_bae_contractor);
        AddRule(&profile.GetPolicyProfile(), 2U, nxrm::pkg::policy::RULE_USER, patterns, L"User: BAE Authorized Contractor");

        //
        //  Add Application Rules (Total: 7)
        //

        // # 0: Microsoft Office Word
        patterns.clear();
        patterns.push_back(app_pattern_word);
        patterns.push_back(app_pattern_pub_msft);
        AddRule(&profile.GetPolicyProfile(), 0U, nxrm::pkg::policy::RULE_APPLICATION, patterns, L"Application: Microsoft Office Word");

        // # 1: Microsoft Office Excel
        patterns.clear();
        patterns.push_back(app_pattern_excel);
        patterns.push_back(app_pattern_pub_msft);
        AddRule(&profile.GetPolicyProfile(), 1U, nxrm::pkg::policy::RULE_APPLICATION, patterns, L"Application: Microsoft Office Excel");

        // # 2: Microsoft Office PowerPoint
        patterns.clear();
        patterns.push_back(app_pattern_powerpnt);
        patterns.push_back(app_pattern_pub_msft);
        AddRule(&profile.GetPolicyProfile(), 2U, nxrm::pkg::policy::RULE_APPLICATION, patterns, L"Application: Microsoft Office PowerPoint");

        // # 3: Acrobat Reader
        patterns.clear();
        patterns.push_back(app_pattern_acrord);
        patterns.push_back(app_pattern_pub_adobe);
        AddRule(&profile.GetPolicyProfile(), 3U, nxrm::pkg::policy::RULE_APPLICATION, patterns, L"Application: Adobe Acrobat Reader");

        // # 4: Microsoft notepad
        patterns.clear();
        patterns.push_back(app_pattern_notepad);
        patterns.push_back(app_pattern_pub_msft);
        AddRule(&profile.GetPolicyProfile(), 4U, nxrm::pkg::policy::RULE_APPLICATION, patterns, L"Application: Microsoft Notepad");

        // # 5: Microsoft wordpad
        patterns.clear();
        patterns.push_back(app_pattern_wordpad);
        patterns.push_back(app_pattern_pub_msft);
        AddRule(&profile.GetPolicyProfile(), 5U, nxrm::pkg::policy::RULE_APPLICATION, patterns, L"Application: Microsoft Wordpad");

        // # 6: NextLabs Software
        patterns.clear();
        patterns.push_back(app_pattern_pub_nextlabs);
        AddRule(&profile.GetPolicyProfile(), 6U, nxrm::pkg::policy::RULE_APPLICATION, patterns, L"Application: Nextlabs Software");
        
        //
        //  Add Location Rules (Total: 1)
        //

        // # 0: Host in NextLabs Domain
        patterns.clear();
        patterns.push_back(loc_pattern_host_nextlabs);
        AddRule(&profile.GetPolicyProfile(), 0U, nxrm::pkg::policy::RULE_LOCATION, patterns, L"Location: In Nextlabs Domain");
        
        //
        //  Add Resource Rules (Total: 1)
        //

        // # 0: Itar files
        patterns.clear();
        patterns.push_back(res_pattern_itar);
        AddRule(&profile.GetPolicyProfile(), 0U, nxrm::pkg::policy::RULE_RESOURCE, patterns, L"Resource: Itar data");

        // # 1: Project PR-01 Data
        patterns.clear();
        patterns.push_back(res_pattern_pr01);
        AddRule(&profile.GetPolicyProfile(), 1U, nxrm::pkg::policy::RULE_RESOURCE, patterns, L"Resource: Project PR-01 Data");

        // # 2: Project PR-02 Data
        patterns.clear();
        patterns.push_back(res_pattern_pr02);
        AddRule(&profile.GetPolicyProfile(), 2U, nxrm::pkg::policy::RULE_RESOURCE, patterns, L"Resource: Project PR-02 Data");
        
        //
        //  Add Policies (Total: 1)
        //
        nxrm::pkg::policy::CPolicy* policy = NULL;

        // # 0: Allow User Group 0 to View/Edit/Print/Copy/Extract All PR-01 document when it is in NextLabs domain
        rights = profile.GetPolicyProfile().GetRightsSet()[0]->GetValue()      // View
                 + profile.GetPolicyProfile().GetRightsSet()[1]->GetValue()    // Edit
                 + profile.GetPolicyProfile().GetRightsSet()[2]->GetValue()    // Print
                 + profile.GetPolicyProfile().GetRightsSet()[3]->GetValue()    // Copy
                 + profile.GetPolicyProfile().GetRightsSet()[4]->GetValue();   // Extract
        policy = AddPolicy(&profile.GetPolicyProfile(),
                           L"Secure Collaboration - PR-01: Allow user group 0 to View/Edit/Print/Copy/Extract All PR-01 document when it is in NextLabs domain",
                           0,
                           0,
                           rights,
                           0);
        policy->GetRules()[nxrm::pkg::policy::RULE_LOCATION].first.push_back(nxrm::pkg::policy::RULEOBJ(0, profile.GetPolicyProfile().GetRulesSet()[nxrm::pkg::policy::RULE_LOCATION][0].get()));
        policy->GetRules()[nxrm::pkg::policy::RULE_RESOURCE].first.push_back(nxrm::pkg::policy::RULEOBJ(1, profile.GetPolicyProfile().GetRulesSet()[nxrm::pkg::policy::RULE_RESOURCE][1].get()));

        // # 1: Allow User Group 1 to View/Edit/Print/Copy/Extract/Annotate All PR-02 document when it is in NextLabs domain
        rights = profile.GetPolicyProfile().GetRightsSet()[0]->GetValue()      // View
                 + profile.GetPolicyProfile().GetRightsSet()[1]->GetValue()    // Edit
                 + profile.GetPolicyProfile().GetRightsSet()[2]->GetValue()    // Print
                 + profile.GetPolicyProfile().GetRightsSet()[3]->GetValue()    // Copy
                 + profile.GetPolicyProfile().GetRightsSet()[4]->GetValue()    // Extract
                 + profile.GetPolicyProfile().GetRightsSet()[5]->GetValue();   // Annotate
        policy = AddPolicy(&profile.GetPolicyProfile(),
                           L"Secure Collaboration - PR-02: Allow User Group 1 to View/Edit/Print/Copy/Extract/Annotate All PR-02 document when it is in NextLabs domain",
                           1,
                           1,
                           rights,
                           0);
        policy->GetRules()[nxrm::pkg::policy::RULE_LOCATION].first.push_back(nxrm::pkg::policy::RULEOBJ(0, profile.GetPolicyProfile().GetRulesSet()[nxrm::pkg::policy::RULE_LOCATION][0].get()));
        policy->GetRules()[nxrm::pkg::policy::RULE_RESOURCE].first.push_back(nxrm::pkg::policy::RULEOBJ(2, profile.GetPolicyProfile().GetRulesSet()[nxrm::pkg::policy::RULE_RESOURCE][2].get()));

        // # 2: Allow User Group 2 to View/Edit/Classify/RightsAssign/RightsEdit All document when it is in NextLabs domain
        rights = profile.GetPolicyProfile().GetRightsSet()[0]->GetValue()      // View
                 + profile.GetPolicyProfile().GetRightsSet()[1]->GetValue()    // Edit
                 + profile.GetPolicyProfile().GetRightsSet()[10]->GetValue()   // Classify
                 + profile.GetPolicyProfile().GetRightsSet()[11]->GetValue()   // RightsAssign
                 + profile.GetPolicyProfile().GetRightsSet()[12]->GetValue();  // RightsEdit
        policy = AddPolicy(&profile.GetPolicyProfile(),
                           L"Secure Collaboration - Allow User Group 2 to View/Edit/Classify/RightsAssign/RightsEdit All document when it is in NextLabs domain",
                           2,
                           2,
                           rights,
                           0);
        policy->GetRules()[nxrm::pkg::policy::RULE_LOCATION].first.push_back(nxrm::pkg::policy::RULEOBJ(0, profile.GetPolicyProfile().GetRulesSet()[nxrm::pkg::policy::RULE_LOCATION][0].get()));

        // # 3: Allow User Group 0 to Annotate/Send/Convert/Classify All PR-01 document if application is Office Word/Excel/PowerPoint/Adobereader
        rights = profile.GetPolicyProfile().GetRightsSet()[5]->GetValue()      // Annotate
                 + profile.GetPolicyProfile().GetRightsSet()[8]->GetValue()    // Send
                 + profile.GetPolicyProfile().GetRightsSet()[9]->GetValue()    // Convert
                 + profile.GetPolicyProfile().GetRightsSet()[10]->GetValue();  // Classify
        policy = AddPolicy(&profile.GetPolicyProfile(),
                           L"Secure Collaboration - Allow User Group 0 to Annotate/Send/Convert/Classify All PR-01 document if application is Office Word/Excel/PowerPoint/Adobereader",
                           3,
                           0,
                           rights,
                           0);
        policy->GetRules()[nxrm::pkg::policy::RULE_RESOURCE].first.push_back(nxrm::pkg::policy::RULEOBJ(1, profile.GetPolicyProfile().GetRulesSet()[nxrm::pkg::policy::RULE_RESOURCE][1].get()));
        policy->GetRules()[nxrm::pkg::policy::RULE_APPLICATION].first.push_back(nxrm::pkg::policy::RULEOBJ(0, profile.GetPolicyProfile().GetRulesSet()[nxrm::pkg::policy::RULE_APPLICATION][0].get()));
        policy->GetRules()[nxrm::pkg::policy::RULE_APPLICATION].first.push_back(nxrm::pkg::policy::RULEOBJ(1, profile.GetPolicyProfile().GetRulesSet()[nxrm::pkg::policy::RULE_APPLICATION][1].get()));
        policy->GetRules()[nxrm::pkg::policy::RULE_APPLICATION].first.push_back(nxrm::pkg::policy::RULEOBJ(2, profile.GetPolicyProfile().GetRulesSet()[nxrm::pkg::policy::RULE_APPLICATION][2].get()));
        policy->GetRules()[nxrm::pkg::policy::RULE_APPLICATION].first.push_back(nxrm::pkg::policy::RULEOBJ(3, profile.GetPolicyProfile().GetRulesSet()[nxrm::pkg::policy::RULE_APPLICATION][3].get()));

        // # 4: Allow User Group 1 to Annotate/Send/Convert/Classify All PR-02 document if application is Office Word/Excel/PowerPoint/Adobereader
        rights = profile.GetPolicyProfile().GetRightsSet()[5]->GetValue()      // Annotate
                 + profile.GetPolicyProfile().GetRightsSet()[8]->GetValue()    // Send
                 + profile.GetPolicyProfile().GetRightsSet()[9]->GetValue()    // Convert
                 + profile.GetPolicyProfile().GetRightsSet()[10]->GetValue();  // Classify
        policy = AddPolicy(&profile.GetPolicyProfile(),
                           L"Secure Collaboration - Allow User Group 1 to Annotate/Send/Convert/Classify All PR-02 document if application is Office Word/Excel/PowerPoint/Adobereader",
                           4,
                           1,
                           rights,
                           0);
        policy->GetRules()[nxrm::pkg::policy::RULE_RESOURCE].first.push_back(nxrm::pkg::policy::RULEOBJ(2, profile.GetPolicyProfile().GetRulesSet()[nxrm::pkg::policy::RULE_RESOURCE][2].get()));
        policy->GetRules()[nxrm::pkg::policy::RULE_APPLICATION].first.push_back(nxrm::pkg::policy::RULEOBJ(0, profile.GetPolicyProfile().GetRulesSet()[nxrm::pkg::policy::RULE_APPLICATION][0].get()));
        policy->GetRules()[nxrm::pkg::policy::RULE_APPLICATION].first.push_back(nxrm::pkg::policy::RULEOBJ(1, profile.GetPolicyProfile().GetRulesSet()[nxrm::pkg::policy::RULE_APPLICATION][1].get()));
        policy->GetRules()[nxrm::pkg::policy::RULE_APPLICATION].first.push_back(nxrm::pkg::policy::RULEOBJ(2, profile.GetPolicyProfile().GetRulesSet()[nxrm::pkg::policy::RULE_APPLICATION][2].get()));
        policy->GetRules()[nxrm::pkg::policy::RULE_APPLICATION].first.push_back(nxrm::pkg::policy::RULEOBJ(3, profile.GetPolicyProfile().GetRulesSet()[nxrm::pkg::policy::RULE_APPLICATION][3].get()));

        // # 5: Nextlabs Application has all rights on all documents
        rights = 0xFFFFFFFFFFFFFFFF;  // All Rights
        policy = AddPolicy(&profile.GetPolicyProfile(),
                           L"Secure Collaboration - Nextlabs Application has all rights on all documents",
                           5,
                           0,
                           rights,
                           0);
        policy->GetRules()[nxrm::pkg::policy::RULE_APPLICATION].first.push_back(nxrm::pkg::policy::RULEOBJ(6, profile.GetPolicyProfile().GetRulesSet()[nxrm::pkg::policy::RULE_APPLICATION][6].get()));

        // # 6: BAE Internal has all rights on all documents
        rights = 0xFFFFFFFFFFFFFFFF;  // All Rights
        policy = AddPolicy(&profile.GetPolicyProfile(),
                           L"Secure Collaboration - BAE Internal has all rights on all documents",
                           6,
                           -1,
                           rights,
                           0);
        policy->GetRules()[nxrm::pkg::policy::RULE_USER].first.push_back(nxrm::pkg::policy::RULEOBJ(1, profile.GetPolicyProfile().GetRulesSet()[nxrm::pkg::policy::RULE_USER][1].get()));

        // # 7: BAE Contractor has VIEW rights on all documents
        rights = profile.GetPolicyProfile().GetRightsSet()[0]->GetValue()      // View
                 + profile.GetPolicyProfile().GetRightsSet()[1]->GetValue()    // Edit
                 + profile.GetPolicyProfile().GetRightsSet()[2]->GetValue()    // Print
                 + profile.GetPolicyProfile().GetRightsSet()[3]->GetValue()    // Copy
                 + profile.GetPolicyProfile().GetRightsSet()[4]->GetValue();   // Extract
        policy = AddPolicy(&profile.GetPolicyProfile(),
                           L"Secure Collaboration - BAE Contractor has VIEW rights on all documents",
                           7,
                           -1,
                           rights,
                           0);
        policy->GetRules()[nxrm::pkg::policy::RULE_USER].first.push_back(nxrm::pkg::policy::RULEOBJ(2, profile.GetPolicyProfile().GetRulesSet()[nxrm::pkg::policy::RULE_USER][2].get()));

        // # 8: BAE Business User has VIEW / EDIT / SEND rights on all documents
        rights = profile.GetPolicyProfile().GetRightsSet()[0]->GetValue()      // View
                 + profile.GetPolicyProfile().GetRightsSet()[1]->GetValue()    // Edit
                 + profile.GetPolicyProfile().GetRightsSet()[8]->GetValue();   // Send
        policy = AddPolicy(&profile.GetPolicyProfile(),
                           L"Secure Collaboration - BAE Business User has VIEW / EDIT / SEND rights on all documents",
                           8,
                           -1,
                           rights,
                           0);
        policy->GetRules()[nxrm::pkg::policy::RULE_USER].first.push_back(nxrm::pkg::policy::RULEOBJ(0, profile.GetPolicyProfile().GetRulesSet()[nxrm::pkg::policy::RULE_USER][0].get()));

        profile.UpdateProfileId();

        profile.ToFile(gProfile.c_str(), prikey);

        result = true;
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        profile.Clear();
        result = false;
    }
    catch(...) {
        profile.Clear();
        result = false;
    }

    EXPECT_TRUE(result);
}

TEST_F(ProfileSuite, Evaluate)
{
    bool result = false;

    nudf::crypto::CPkcs12CertContext pfxcert;
    nudf::crypto::CLegacyRsaPriKeyBlob prikey;
    nudf::crypto::CRsaPubKeyBlob pubkey;

    // User Properties
    const nxrm::pkg::policy::CProperty user1_sid(L"id", L"S-1-5-21-2242368633-1437193937-1178672470-1001");
    const nxrm::pkg::policy::CProperty user1_name(L"name", L"john.tyler@nextlabs.com");
    const nxrm::pkg::policy::CProperty user2_sid(L"id", L"S-1-5-21-2242368633-1437193937-1178672470-1002");
    const nxrm::pkg::policy::CProperty user2_name(L"name", L"Jimmy.Carter@nextlabs.com");
    const nxrm::pkg::policy::CProperty user3_sid(L"id", L"S-1-5-21-2242368633-1437193937-1178672470-1003");
    const nxrm::pkg::policy::CProperty user3_name(L"name", L"Administrator@nextlabs.com");
    const nxrm::pkg::policy::CProperty user4_sid(L"id", L"S-1-5-21-2242368633-1437193937-1178672470-1004");
    const nxrm::pkg::policy::CProperty user4_name(L"name", L"sherlock.holmes@bae.com");
    const nxrm::pkg::policy::CProperty user4_attr(L"baesystemsintellectualpropertyaccessrights", L"intellectual property - business authorization category - identifier");
    const nxrm::pkg::policy::CProperty user5_sid(L"id", L"S-1-5-21-2242368633-1437193937-1178672470-1005");
    const nxrm::pkg::policy::CProperty user5_name(L"name", L"Administrator@bae.com");
    const nxrm::pkg::policy::CProperty user5_attr(L"baesystemsintellectualpropertyaccessrights", L"intellectual property - internal authorization category - identifier");
    const nxrm::pkg::policy::CProperty user6_sid(L"id", L"S-1-5-21-2242368633-1437193937-1178672470-1006");
    const nxrm::pkg::policy::CProperty user6_name(L"name", L"edward.Kolin@contract.bae.com");
    const nxrm::pkg::policy::CProperty user6_attr(L"baesystemsintellectualpropertyaccessrights", L"intellectual property - contract authorization category - identifier");

    // Application Properties
    const nxrm::pkg::policy::CProperty app0_path(L"path", L"C:\\Program Files (x86)\\Microsoft Office\\Office15\\WINWORD.EXE");
    const nxrm::pkg::policy::CProperty app0_pub(L"publisher", L"Microsoft Corporation");
    const nxrm::pkg::policy::CProperty app1_path(L"path", L"C:\\Program Files (x86)\\Microsoft Office\\Office15\\EXCEL.EXE");
    const nxrm::pkg::policy::CProperty app1_pub(L"publisher", L"Microsoft Corporation");
    const nxrm::pkg::policy::CProperty app2_path(L"path", L"C:\\Program Files (x86)\\Microsoft Office\\Office15\\POWERPNT.EXE");
    const nxrm::pkg::policy::CProperty app2_pub(L"publisher", L"Microsoft Corporation");
    const nxrm::pkg::policy::CProperty app3_path(L"path", L"C:\\Program Files (x86)\\Adobe\\Reader 11.0\\Reader\\AcroRd32.exe");
    const nxrm::pkg::policy::CProperty app3_pub(L"publisher", L"Adobe Systems, Incorporated");
    const nxrm::pkg::policy::CProperty app4_path(L"path", L"C:\\Program Files (x86)\\Microsoft Office\\Office15\\NOTEPAD.EXE");
    const nxrm::pkg::policy::CProperty app4_pub(L"publisher", L"Microsoft Corporation");
    const nxrm::pkg::policy::CProperty app5_path(L"path", L"C:\\Program Files (x86)\\Microsoft Office\\Office15\\WORDPAD.EXE");
    const nxrm::pkg::policy::CProperty app5_pub(L"publisher", L"Microsoft Corporation");
    const nxrm::pkg::policy::CProperty app6_path(L"path", L"C:\\Program Files\\NextLabs\\Rights Management\\bin\\nxrmserv.exe");
    const nxrm::pkg::policy::CProperty app6_pub(L"publisher", L"NextLabs, Inc");
    const nxrm::pkg::policy::CProperty app7_path(L"path", L"C:\\Program Files (x86)\\Notepad++\\notepad++.exe");

    // Location Properties
    const nxrm::pkg::policy::CProperty loc0_host(L"host", L"segula.nextlabs.com");
    const nxrm::pkg::policy::CProperty loc1_host(L"host", L"bayarea.test.nextlabs.com");
    const nxrm::pkg::policy::CProperty loc2_host(L"host", L"abbu");
    const nxrm::pkg::policy::CProperty loc3_host(L"host", L"shasta.bae.com");

    // Resource Properties
    const nxrm::pkg::policy::CProperty res0_path(L"path", L"C:\\Users\\john.tyler\\Documents\\itar\\design.docx");
    const nxrm::pkg::policy::CProperty res0_date(L"itar", L"yes", nxrm::pkg::policy::PropTag);
    const nxrm::pkg::policy::CProperty res1_path(L"path", L"C:\\Users\\john.tyler\\Documents\\project\\01\\datasheet.xlsx");
    const nxrm::pkg::policy::CProperty res1_project(L"project", L"pr-01", nxrm::pkg::policy::PropTag);
    const nxrm::pkg::policy::CProperty res2_path(L"path", L"C:\\Users\\john.tyler\\Documents\\project\\02\\datasheet.xlsx");
    const nxrm::pkg::policy::CProperty res2_project(L"project", L"pr-02", nxrm::pkg::policy::PropTag);
    const nxrm::pkg::policy::CProperty res3_path(L"path", L"C:\\Users\\john.tyler\\Documents\\BAE\\data\\design.dxf");
    const nxrm::pkg::policy::CProperty res3_owner(L"owner", L"BAE Systems", nxrm::pkg::policy::PropTag);
    const nxrm::pkg::policy::CProperty res3_pwneridp(L"owner_idp", L"https://www.baesystems.com/authenticate.aspx", nxrm::pkg::policy::PropTag);
    
    HRESULT hr = pfxcert.CreateFromFile(sign_pri_cert.c_str(), L"123blue!");
    EXPECT_TRUE(SUCCEEDED(hr));
    if(FAILED(hr)) {
        return;
    }

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

    prikey.GetPublicKeyBlob(pubkey);

    try {

        nxrm::pkg::CProfileBundle profile;

        profile.FromFile(gProfile.c_str(), pubkey);
        profile.GetPolicyProfile().ToRegex();

        const nxrm::pkg::POLICYGROUP& policygroup = profile.GetPolicyProfile().GetPolicySet();


        nxrm::pkg::CPolicySet  eval_policyset;
        nxrm::pkg::CEvalObject eval_object;
        nxrm::pkg::CEvalResult eval_result;
        std::wstring    wsRights;
        std::wstring    wsCustomRights;

        // Create PolicySet
        //    - Use all policies
        for(nxrm::pkg::POLICYGROUP::const_iterator it=policygroup.begin(); it!=policygroup.end(); ++it) {
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
        profile.GetPolicyProfile().GetGroupIdSet(user1_sid.GetValue(), BUILTIN_NEXTLABS_TENANTID, eval_object.GetUserGroups());
        eval_object.GetProperty(nxrm::pkg::policy::RULE_USER)[L"id"] = user1_sid;
        eval_object.GetProperty(nxrm::pkg::policy::RULE_USER)[L"name"] = user1_name;
        eval_object.GetProperty(nxrm::pkg::policy::RULE_APPLICATION)[L"path"] = app0_path;
        eval_object.GetProperty(nxrm::pkg::policy::RULE_APPLICATION)[L"publisher"] = app0_pub;
        eval_object.GetProperty(nxrm::pkg::policy::RULE_LOCATION)[L"host"] = loc0_host;
        eval_object.GetProperty(nxrm::pkg::policy::RULE_RESOURCE)[L"path"] = res1_path;
        eval_object.GetProperty(nxrm::pkg::policy::RULE_RESOURCE)[L"project"] = res1_project;
        eval_policyset.Evaluate(eval_object, eval_result);
        wsRights = profile.GetPolicyProfile().RightsToString(eval_result.GetRights(), false);
        wsCustomRights = profile.GetPolicyProfile().RightsToString(eval_result.GetCustomRights(), true);
        printf("Evaluate 00:\n");
        printf("  - Rights: %S\n", wsRights.c_str());
        printf("  - Custom Rights: %S\n", wsCustomRights.c_str());
        printf("  - Policies:\n", eval_result.GetCustomRights());
        for(std::vector<nxrm::pkg::CEvalPolicyResult>::const_iterator it=eval_result.GetPolicies().begin(); it!=eval_result.GetPolicies().end(); ++it) {
            wsRights = profile.GetPolicyProfile().RightsToString((*it).GetRights(), false);
            wsCustomRights = profile.GetPolicyProfile().RightsToString((*it).GetCustomRights(), true);
            printf("      * #%d - %S\n", (*it).GetId(), (*it).GetName().c_str());
            printf("         > Rights: %S\n", wsRights.c_str());
            printf("         > Custom Rights: %S\n", wsCustomRights.c_str());
        }
        EXPECT_TRUE(eval_result.GetRights() != 0);

        result = true;
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        result = false;
    }
    catch(...) {
        result = false;
    }

    EXPECT_TRUE(result);

}


#endif