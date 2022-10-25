

#include <Windows.h>

#include <string>
#include <iostream>

#include <nudf\exception.hpp>
#include <nudf\secure.hpp>
#include <nudf\sa.hpp>


template <SE_OBJECT_TYPE OBJTYPE>
void add_ace_to_object_security_descriptor(
    const std::wstring& object_name,
    const std::wstring& trustee,
    TRUSTEE_FORM trustee_form,
    unsigned long access_rights,
    ACCESS_MODE access_mode,
    unsigned long inheritance
    )
{
    PACL pOldDACL = NULL, pNewDACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    EXPLICIT_ACCESS ea;

    try {

        DWORD dwRes = 0;

        dwRes = GetNamedSecurityInfoW(object_name.c_str(),
            OBJTYPE,
            DACL_SECURITY_INFORMATION,
            NULL,
            NULL,
            &pOldDACL,
            NULL,
            &pSD);
        if (ERROR_SUCCESS != dwRes) {
            throw std::exception("fail to get security info");
        }

        // Initialize an EXPLICIT_ACCESS structure for the new ACE.
        ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
        ea.grfAccessPermissions = access_rights;
        ea.grfAccessMode = access_mode;
        ea.grfInheritance = inheritance;
        ea.Trustee.TrusteeForm = trustee_form;
        ea.Trustee.ptstrName = (LPWSTR)trustee.c_str();

        // Create a new ACL that merges the new ACE
        // into the existing DACL.
        dwRes = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
        if (ERROR_SUCCESS != dwRes) {
            throw std::exception("fail to set entry in acl");
        }

        // Attach the new ACL as the object's DACL.
        dwRes = SetNamedSecurityInfoW((LPWSTR)object_name.c_str(),
            OBJTYPE,
            DACL_SECURITY_INFORMATION,
            NULL,
            NULL,
            pNewDACL,
            NULL);
        if (ERROR_SUCCESS != dwRes) {
            throw std::exception("fail to set security info");
        }

        if (pSD != NULL) {
            LocalFree((HLOCAL)pSD);
            pSD = NULL;
        }
        if (pNewDACL != NULL) {
            LocalFree((HLOCAL)pNewDACL);
            pNewDACL = NULL;
        }
    }
    catch (const std::exception& e) {
        if (pSD != NULL) {
            LocalFree((HLOCAL)pSD);
            pSD = NULL;
        }
        if (pNewDACL != NULL) {
            LocalFree((HLOCAL)pNewDACL);
            pNewDACL = NULL;
        }
        throw e;
    }
}


int main(int argc, char** argv)
{
    NX::security_attribute  sa;
    //sa.add_acl(sid, GENERIC_ALL, TRUSTEE_IS_USER, SET_ACCESS, SUB_CONTAINERS_AND_OBJECTS_INHERIT);
    sa.add_acl_for_wellknown_group2(SECURITY_NT_AUTHORITY, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, GENERIC_ALL, SUB_CONTAINERS_AND_OBJECTS_INHERIT);    
    sa.add_acl_for_wellknown_group(SECURITY_WORLD_SID_AUTHORITY, SECURITY_WORLD_RID, GENERIC_READ, SUB_CONTAINERS_AND_OBJECTS_INHERIT);
    sa.generate();

    nudf::win::CSidEveryone sid_everyone;

    try {

        add_ace_to_object_security_descriptor<SE_FILE_OBJECT>(
            L"D:\\dev\\bitbucket\\ruby\\nudf\\v1\\nudf_test\\test.txt",
            L"Everyone",
            TRUSTEE_IS_NAME, //TRUSTEE_IS_SID,
            GENERIC_READ,
            GRANT_ACCESS,
            NO_INHERITANCE
            );
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        ; //
    }
    
    //if (!::CreateDirectoryW(L"Test", sa)) {
    //if (!SetFileSecurityW(L"test.txt", DACL_SECURITY_INFORMATION, &sa)) {
    //    std::cout << "fail to create folder (" << (int)GetLastError() << ")" << std::endl;
    //    return -1;
    //}

    std::cout << "succeeded!"  << std::endl;
    return 0;
}