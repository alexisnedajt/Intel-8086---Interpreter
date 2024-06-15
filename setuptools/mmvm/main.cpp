#include <stdio.h>
#include <iostream>
#include <vector>
#include "binary.h"

/* Entry Point */
int main(int argc, char *argv[]) {
    Binary binary;
    bool disassm_flg = false;
    vector<string> args;
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if(arg == "-d") {
            disassm_flg = true;
        } else if(arg == "-m") {
            binary.setDebug(true);
        } else if (arg == "-r") {
            binary.setRootPath(argv[++i]);
            if (binary.getRootPath().at(0) != '/') {
                fprintf(STDERR, "specify binary root after -r option\n");
                exit(1);
            }
        } else {
            for (; i < argc; ++i) {
                args.push_back(argv[i]);
            }
        }
    }
    
    binary.setFile(args[0]);
    
    if(disassm_flg) {
        binary.disassm();
        return 0;
    }
    
    binary.run(args);
	return 0;
}
