#ifndef ISX_VERSION_H
#define ISX_VERSION_H

#define STR(s) #s

#define APP_VERSION_MAJOR    2
#define APP_VERSION_MINOR    0
#define APP_VERSION_PATCH    0
#define APP_IS_BETA          true;


#define APP_VERSION           APP_VERSION_MAJOR,APP_VERSION_MINOR

#define APP_VERSION_STR       STR(APP_VERSION_MAJOR) "." STR(APP_VERSION_MINOR) "." STR(APP_VERSION_PATCH) 


#endif // include guard