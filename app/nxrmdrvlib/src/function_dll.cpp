

#include <Windows.h>

#include "function_dll.hpp"


using namespace NX;

function_item::function_item() : _id(-1), _fp(nullptr)
{
}

function_item::function_item(unsigned int id, const std::string& name) : _id(id), _name(name), _fp(nullptr)
{
}

function_item::~function_item()
{
}

function_item& function_item::operator = (const function_item& other)
{
    if (this != &other) {
        _id = other.get_id();
        _name = other.get_name();
        _fp = other.get_function_pointer();
    }
    return *this;
}

bool function_item::load(void* h)
{
    if (valid()) {
        return false;
    }
    _fp = GetProcAddress((HMODULE)h, MAKEINTRESOURCEA(_id));
    return (nullptr != _fp);
}

void function_item::clear()
{
    _fp = nullptr;
}