#include <string>
#include "common.h"
#include "os.h"

string OS::convertPath(string path) {
    
    if (path == "") return path;
    if (path.at(0) == '.') return path;
    if (path.at(0) != '/') return path;
    
    
    
    string target = "/usr/tmp";
    size_t pos = path.find(target,0);
    string newPath;
    if (pos != string::npos) {
        return path.replace(0, target.length(), "/tmp");
    }
    target = "/tmp";
    pos = path.find(target, 0);
    if (pos != string::npos) {
        return path.replace(0, target.length(), "/tmp");
    }
    
    
    return rootPath + path;

}

void OS::createPid() {
    static int pid_max;
    pid = 12240 + (++pid_max);
}