

#ifndef __NX_CIPHER_STREAM_HPP__
#define __NX_CIPHER_STREAM_HPP__

#include <string>
#include <vector>


namespace NX {
namespace sconfig {


std::string load(const std::string& file, const std::vector<unsigned char>& key);
void save(const std::string& file, const std::string& data, const std::vector<unsigned char>& key);




}   // namespace NX::sconfig
}   // namespace NX


#endif