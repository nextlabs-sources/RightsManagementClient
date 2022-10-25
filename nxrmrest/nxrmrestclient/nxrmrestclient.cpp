

#include <Windows.h>
#include <assert.h>
#include <stdio.h>

#include <nudf\exception.hpp>
#include <nxrmrest\nxrmrest.hpp>

#include "client_register.hpp"


int main(int argc, char** argv)
{
    int nRet = 0;

    try {

        CltRegister();

    }
    catch(const nudf::CException& e) {
        nRet = -1;
        printf("Exception:\n");
        printf("  Error:%d, Function:%s, File:%s, Line:%d\n", e.GetCode(), e.GetFunction(), e.GetFile(), e.GetLine());
    }

    return nRet;
}