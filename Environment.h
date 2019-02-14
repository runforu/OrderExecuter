#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <string>
#include "Loger.h"

struct Environment {
    static char s_module_path[256];
    static char s_doc_root[256];

    static void Log() {
        LOG("Module path: (%s)", s_module_path);
        LOG("Http document root path: (%s)", s_doc_root);
    }
};

#endif  // !_ENVIRONMENT_H_
