#ifndef VERSION_H
#define VERSION_H

#define APP_VERSION_MAJOR 1
#define APP_VERSION_MINOR 0
#define APP_VERSION_PATCH 2

#define STRINGIFY(x) #x
#define VERSION_STRING(major, minor, patch) \
STRINGIFY(major) "." STRINGIFY(minor) "." STRINGIFY(patch)

#endif // VERSION_H
