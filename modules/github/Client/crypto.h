#pragma once
#include <string>

std::string xor_crypt(const std::string& data, const std::string& key);
std::string base64_encode(const std::string& in);
std::string base64_decode(const std::string& in);
std::string encode_payload(const std::string& payload, const std::string& key);
std::string decode_payload(const std::string& payload, const std::string& key);