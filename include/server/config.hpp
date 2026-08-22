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
    int getPosrt() const;

    std::
}