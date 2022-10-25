

#include <Windows.h>

#include <Wincrypt.h>
#define SECURITY_WIN32
#include <security.h>

#include <nudf\eh.hpp>
#include <nudf\debug.hpp>
#include <nudf\security.hpp>



using namespace NX;


secure::smemory::smemory() : _size(0)
{
}

secure::smemory::smemory(const unsigned char* data, size_t length) : _size(length)
{
    encrypt(std::vector<unsigned char>(data, data + length));
}

secure::smemory::smemory(const std::vector<unsigned char>& data) : _size(data.size())
{
    encrypt(data);
}

secure::smemory::~smemory()
{
}

secure::smemory& secure::smemory::operator = (const secure::smemory& other) noexcept
{
    if (this != &other) {
        clear();
        encrypt(other.decrypt());
    }
    return *this;
}

void secure::smemory::encrypt(const std::vector<unsigned char>& data)
{
    clear();

    const size_t data_size = data.size();


    // Buffer must be a multiple of CRYPTPROTECTMEMORY_BLOCK_SIZE
    const auto mod = data_size % CRYPTPROTECTMEMORY_BLOCK_SIZE;
    if (mod != 0) {
        _buffer.resize(data_size + CRYPTPROTECTMEMORY_BLOCK_SIZE - mod, 0);
    }
    else {
        _buffer.resize(data_size, 0);
    }
    // Copy data
    memcpy_s(_buffer.data(), _buffer.size(), data.data(), data_size);
    // Encrypt data
    if (!CryptProtectMemory(_buffer.data(), static_cast<DWORD>(_buffer.size()), CRYPTPROTECTMEMORY_SAME_PROCESS)) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }

    // succeeded
    _size = data_size;
}

std::vector<unsigned char> secure::smemory::decrypt() const
{
    std::vector<unsigned char> data = _buffer;
    if (!data.empty()) {
        if (!CryptUnprotectMemory(data.data(), static_cast<DWORD>(_buffer.size()), CRYPTPROTECTMEMORY_SAME_PROCESS)) {
            throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
        }
        data.resize(_size);
    }
    return std::move(data);
}

void secure::smemory::clear() noexcept
{
    _buffer.clear();
    _size = 0;
}