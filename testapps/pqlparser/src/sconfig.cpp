

#include <Windows.h>


#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <nudf\exception.hpp>
#include <nudf\crypto.hpp>


#include "sconfig.hpp"


std::string NX::sconfig::load(const std::string& file, const std::vector<unsigned char>& key)
{
    std::ifstream fs;

    if (key.size() != 16 && key.size() != 32) {
        throw std::invalid_argument("wrong key size");
    }

    fs.open(file, std::ifstream::binary);
    if (!fs.is_open() || !fs.good()) {
        throw std::exception("fail to open file");
    }

    // get length of file:
    fs.seekg(0, fs.end);
    size_t length = fs.tellg();
    fs.seekg(0, fs.beg);

    if (0 == length) {
        return std::string();
    }

    if (0 != length%key.size()) {
        throw std::invalid_argument("file size is not aligned with key size");
    }

    std::vector<char> s;
    s.resize(length + 1);
    fs.read(&s[0], length);
    if (fs.eof() || fs.bad() || fs.fail()) {
        throw std::exception("fail to read all the data");
    }

    nudf::crypto::CAesKeyBlob ak;
    ak.SetKey(&key[0], (ULONG)key.size());
    if (!nudf::crypto::AesDecrypt(ak, &s[0], (ULONG)length, 0)) {
        return std::string();
    }

    return &s[0];
}

void NX::sconfig::save(const std::string& file, const std::string& data, const std::vector<unsigned char>& key)
{
    std::ofstream fs;

    if (key.size() != 16 && key.size() != 32) {
        throw std::invalid_argument("wrong key size");
    }

    fs.open(file, std::ofstream::binary | std::ofstream::trunc);
    if (!fs.is_open() || !fs.good()) {
        throw std::exception("fail to open file");
    }

    if (data.length() == 0) {
        return;
    }

    std::vector<char> s;
    size_t length = ((data.length() + 1 + (key.size() - 1)) / key.size()) * key.size();

    s.resize(length, 0);
    memcpy(&s[0], &data[0], data.length());

    nudf::crypto::CAesKeyBlob ak;
    ak.SetKey(&key[0], (ULONG)key.size());
    if (!nudf::crypto::AesEncrypt(ak, &s[0], (ULONG)length, 0)) {
        throw std::exception("fail to encrypt content");
    }

    fs.write(&s[0], length);
    if (fs.fail() || fs.bad()) {
        throw std::exception("fail to write content");
    }
}