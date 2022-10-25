

#include <Windows.h>
#include <assert.h>
#include <Wincrypt.h>

#include <vector>

#include <nudf\encoding.hpp>


using namespace nudf::util::encoding;

std::string nudf::util::encoding::Base64EncodeForX509(_In_ const void* buf, _In_ unsigned long size) throw()
{
    unsigned long cb = ((size +1) / 2) * 3 + 128;
    std::vector<char> outbuf;
    outbuf.resize(cb, 0);
    if(!CryptBinaryToStringA((const BYTE*)buf, size, CRYPT_STRING_BASE64, &outbuf[0], &cb)) {
        return "";
    }
    return std::string(&outbuf[0], cb);
}