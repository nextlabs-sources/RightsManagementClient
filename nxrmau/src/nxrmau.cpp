

#include <Windows.h>

#include <nudf\uniqueguard.hpp>

#include "nxrmau.h"


int WINAPI WinMain(_In_ HINSTANCE hInstance,
                   _In_ HINSTANCE hPrevInstance,
                   _In_ LPSTR lpCmdLine,
                   _In_ int nCmdShow
                   )
{
    nudf::util::CUniqueGuard auGuard(NXRMAU_INSTANCE_NAME);

    //
    //  Make sure this is unique instance
    //
    if(!auGuard.IsValid()) {
        return (int)auGuard.GetError();
    }
    if(!auGuard.IsUnique()) {
        return ERROR_ALREADY_EXISTS;
    }

    // Get current version

    // Check remote version

    // Download latest version

    // Start installer and wait ...

    return 0L;
}
