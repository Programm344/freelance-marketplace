#include "jwt_helper.h"
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <sstream>
#include <ctime>

const std::string JwtHelper::SECRET = "freelance-marketplace-secret-key-2024";

// Простой Base64 URL-safe энкодер
static std::string base64UrlEncode(const std::string& data) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, data.c_str(), data.size());
    BIO_flush(bio);
    
    char* encoded;
    long len = BIO_get_mem_data(bio, &encoded);
    std::string result(encoded, len);
    BIO_free_all(bio);
    
    // Base64 → Base64URL
    for (char& c : result) {
        if (c == '+') c = '-';
        if (c == '/') c = '_';
    }
    result.erase(std::find(result.begin(), result.end(), '='), result.end());
    return result;
}

static std::string base64UrlDecode(const std::string& data) {
    std::string s = data;
    for (char& c : s) {
        if (c == '-') c = '+';
        if (c == '_') c = '/';
    }
    while (s.size() % 4) s += '=';
    
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bio = BIO_new_mem_buf(s.c_str(), s.size());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    
    char decoded[1024];
    int len = BIO_read(bio, decoded, sizeof(decoded));
    BIO_free_all(bio);
    return std::string(decoded, len);
}

static std::string hmacSha256(const std::string& data, const std::string& key) {
    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int len;
    HMAC(EVP_sha256(), key.c_str(), key.size(), 
         (unsigned char*)data.c_str(), data.size(), result, &len);
    return std::string((char*)result, len);
}

std::string JwtHelper::generateToken(int userId, const std::string& role, const std::string& email) {
    // Header
    Json::Value header;
    header["alg"] = "HS256";
    header["typ"] = "JWT";
    std::string headerEncoded = base64UrlEncode(header.toStyledString());
    
    // Payload
    Json::Value payload;
    payload["user_id"] = userId;
    payload["role"] = role;
    payload["email"] = email;
    payload["exp"] = (int)time(nullptr) + 86400; // 24 часа
    std::string payloadEncoded = base64UrlEncode(payload.toStyledString());
    
    // Signature
    std::string signatureInput = headerEncoded + "." + payloadEncoded;
    std::string signature = base64UrlEncode(hmacSha256(signatureInput, SECRET));
    
    return headerEncoded + "." + payloadEncoded + "." + signature;
}

bool JwtHelper::verifyToken(const std::string& token, int& userId, std::string& role) {
    size_t firstDot = token.find('.');
    size_t secondDot = token.find('.', firstDot + 1);
    if (firstDot == std::string::npos || secondDot == std::string::npos) return false;
    
    std::string headerEncoded = token.substr(0, firstDot);
    std::string payloadEncoded = token.substr(firstDot + 1, secondDot - firstDot - 1);
    std::string signature = token.substr(secondDot + 1);
    
    // Проверяем подпись
    std::string expectedSig = base64UrlEncode(hmacSha256(headerEncoded + "." + payloadEncoded, SECRET));
    if (signature != expectedSig) return false;
    
    // Декодируем payload
    std::string payloadJson = base64UrlDecode(payloadEncoded);
    Json::Reader reader;
    Json::Value payload;
    if (!reader.parse(payloadJson, payload)) return false;
    
    // Проверяем срок
    int exp = payload["exp"].asInt();
    if (time(nullptr) > exp) return false;
    
    userId = payload["user_id"].asInt();
    role = payload["role"].asString();
    return true;
}
