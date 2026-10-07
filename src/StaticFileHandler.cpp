#include "server/StaticFileHandler.hpp"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <map>

namespace fs = std::filesystem;

StaticFileHandler::StaticFileHandler(const std::string& documentRoot) {
    std::error_code ec;
    root_ = fs::canonical(documentRoot, ec);  // resolves symlinks and ".."
    if (ec) root_ = fs::absolute(documentRoot);
}

bool StaticFileHandler::isInsideRoot(const fs::path& resolved) const {
    // Compare path components: every part of root_ must start `resolved`.
    auto result = std::mismatch(root_.begin(), root_.end(), resolved.begin(), resolved.end());
    return result.first == root_.end();
}

HttpResponse StaticFileHandler::serve(const std::string& urlPath) const {
    // Drop every leading '/', otherwise "root / '/etc/passwd'" would replace root.
    std::string relative = urlPath;
    relative.erase(0, relative.find_first_not_of('/'));

    fs::path candidate = root_ / relative;
    std::error_code ec;

    if (fs::is_directory(candidate, ec)) {
        candidate /= "index.html";
    } else if (!fs::exists(candidate, ec) && candidate.extension().empty()) {
        candidate += ".html";  // "/about" -> about.html
    }

    // canonical() follows symlinks and removes "..", so the check below sees the
    // file that would REALLY be opened. This is what stops path traversal.
    fs::path resolved = fs::canonical(candidate, ec);
    if (ec) return HttpResponse::error(404);
    if (!isInsideRoot(resolved)) return HttpResponse::error(403);
    if (!fs::is_regular_file(resolved, ec)) return HttpResponse::error(404);

    std::ifstream file(resolved, std::ios::binary);
    if (!file) return HttpResponse::error(403);

    std::string body((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return HttpResponse::make(200, std::move(body), mimeType(resolved));
}

std::string StaticFileHandler::mimeType(const fs::path& file) {
    static const std::map<std::string, std::string> types = {
        {".html", "text/html; charset=utf-8"},
        {".htm", "text/html; charset=utf-8"},
        {".css", "text/css; charset=utf-8"},
        {".js", "text/javascript; charset=utf-8"},
        {".mjs", "text/javascript; charset=utf-8"},
        {".json", "application/json; charset=utf-8"},
        {".txt", "text/plain; charset=utf-8"},
        {".xml", "application/xml; charset=utf-8"},
        {".svg", "image/svg+xml"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".webp", "image/webp"},
        {".ico", "image/x-icon"},
        {".woff", "font/woff"},
        {".woff2", "font/woff2"},
        {".pdf", "application/pdf"},
    };
    auto found = types.find(file.extension().string());
    return found == types.end() ? "application/octet-stream" : found->second;
}
