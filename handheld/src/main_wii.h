#ifndef MAIN_WII_H__
#define MAIN_WII_H__

#include "App.h"
#include "NinecraftApp.h"
#include <cstdio>
#include <sys/stat.h>

static bool wiiDirectoryExists(const char* path) {
    struct stat sb;
    return stat(path, &sb) == 0 && (sb.st_mode & S_IFDIR);
}


static std::string chooseWiiStorageRoot() {
    if (wiiDirectoryExists("sd:/")) return "sd:/";
    if (wiiDirectoryExists("usb:/")) return "usb:/";
    return "";
}

int main(int argc, char** argv) {
    AppContext appContext;
    appContext.doRender = false;
    appContext.platform = new AppPlatform();

    MAIN_CLASS* app = new MAIN_CLASS();
    const std::string storageRoot = chooseWiiStorageRoot();

    app->externalStoragePath = storageRoot;
    app->externalCacheStoragePath = storageRoot;

    app->init(appContext);

    while (!app->wantToQuit()) {
        app->update();
    }

    delete app;
    delete appContext.platform;

    return 0;
}

#endif
