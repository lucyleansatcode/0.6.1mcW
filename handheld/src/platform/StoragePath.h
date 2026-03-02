#ifndef STORAGE_PATH_H__
#define STORAGE_PATH_H__

#include <cstdio>
#include <string>

#ifndef WIN32
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#else
#include <direct.h>
#endif

namespace PlatformStorage {

inline std::string stripTrailingSlash(const std::string& path) {
    std::string result = path;
    while (result.size() > 1 && (result[result.size() - 1] == '/' || result[result.size() - 1] == '\\')) {
        result.erase(result.size() - 1);
    }
    return result;
}

inline bool directoryExists(const std::string& path) {
#ifndef WIN32
    struct stat sb;
    return stat(path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode);
#else
    struct _stat sb;
    return _stat(path.c_str(), &sb) == 0 && (sb.st_mode & _S_IFDIR) != 0;
#endif
}

inline bool createDirectoryIfMissing(const std::string& path) {
    if (path.empty()) return false;
    if (directoryExists(path)) return true;
#ifndef WIN32
    return mkdir(path.c_str(), 0755) == 0;
#else
    return _mkdir(path.c_str()) == 0;
#endif
}

inline bool ensureDirectoryTree(const std::string& path) {
    if (path.empty()) return false;
    std::string normalized = stripTrailingSlash(path);

    std::size_t index = 0;
#ifndef WIN32
    if (!normalized.empty() && normalized[0] == '/') index = 1;
#else
    if (normalized.size() > 2 && normalized[1] == ':') index = 3;
#endif

    for (; index <= normalized.size(); ++index) {
        if (index == normalized.size() || normalized[index] == '/' || normalized[index] == '\\') {
            std::string subPath = normalized.substr(0, index);
            if (subPath.empty()) continue;
#ifndef WIN32
            if (subPath == "/") continue;
#endif
            if (!createDirectoryIfMissing(subPath)) return false;
        }
    }

    return directoryExists(normalized);
}

inline bool canWriteFile(const std::string& folder) {
    std::string probePath = folder + "/.ninecraft_write_test";
    FILE* fp = fopen(probePath.c_str(), "wb");
    if (!fp) return false;
    fclose(fp);
    remove(probePath.c_str());
    return true;
}

inline std::string chooseWiiRoot() {
#ifdef WII
    if (directoryExists("sd:/")) return "sd:/";
    if (directoryExists("usb:/")) return "usb:/";
#endif
    return "";
}

inline bool initializeWritableStorage(std::string& externalRoot, std::string& cacheRoot, std::string& errorMessage) {
#ifdef WII
    if (externalRoot.empty()) externalRoot = chooseWiiRoot();
    if (cacheRoot.empty()) cacheRoot = externalRoot;
#endif

    externalRoot = stripTrailingSlash(externalRoot);
    cacheRoot = stripTrailingSlash(cacheRoot.empty() ? externalRoot : cacheRoot);

    if (externalRoot.empty()) {
        errorMessage = "No writable storage root configured.";
        return false;
    }

    const std::string gameRoot = externalRoot + "/games/com.mojang";
    const std::string worldsRoot = gameRoot + "/minecraftWorlds";
    const std::string configRoot = gameRoot + "/minecraftpe";

    if (!ensureDirectoryTree(worldsRoot) || !ensureDirectoryTree(configRoot)) {
        errorMessage = "Storage is unavailable. Please mount an SD/USB device and retry.";
        return false;
    }

    if (!canWriteFile(configRoot)) {
        errorMessage = "Storage is read-only. Saving is disabled until writable storage is available.";
        return false;
    }

    if (!cacheRoot.empty() && cacheRoot != externalRoot) {
        if (!ensureDirectoryTree(cacheRoot + "/games/com.mojang")) {
            cacheRoot = externalRoot;
        }
    }

    return true;
}

inline std::string getOptionsFilePath(const std::string& externalRoot) {
    return stripTrailingSlash(externalRoot) + "/games/com.mojang/minecraftpe/options.txt";
}

inline std::string getScreenshotDirectory(const std::string& externalRoot) {
    return stripTrailingSlash(externalRoot) + "/games/com.mojang/minecraftpe";
}

}

#endif
