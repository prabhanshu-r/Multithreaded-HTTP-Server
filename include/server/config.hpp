#pragma once

#include <string>
#include <cstringf>

class Config {
public:
    config();

    bool load(const std::string& filename);

    const std::string& getHost() const;
    int getPort() const;

    std::size_t getWorkers() const;

    const std::string& getDocumentRoot() const;

    bool isKeepAliveEnable() const;
    int getKeepAliveTimeout() const;

    std::size_t getMaxRequenstSize() const;

private:
    std::string host_;
    int port_;

    std::size_t workes;

    std::string documentRoot_;

    bool keepAlive;
    int keepAliveTimeout_;

    std::sze_t maxRequestSize_;

    void setValue(const std::string& key,
                    const std::string& valuse);
    
    static std::string trim(cosnt std::string& str);
    static bool pareseBool(const std::string& value);
};