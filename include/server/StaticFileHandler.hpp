#ifndef STATIC_FILE_HANDLER_HPP
#define STATIC_FILE_HANDLER_HPP

#include <filesystem>
#include <string>

#include "server/HttpResponse.hpp"

// Serves files from one folder and refuses to leave it.
class StaticFileHandler {
public:
    explicit StaticFileHandler(const std::string& documentRoot);

    // `urlPath` is the decoded request path, e.g. "/about" or "/css/site.css".
    HttpResponse serve(const std::string& urlPath) const;

private:
    bool isInsideRoot(const std::filesystem::path& resolved) const;
    static std::string mimeType(const std::filesystem::path& file);

    std::filesystem::path root_;
};

#endif
