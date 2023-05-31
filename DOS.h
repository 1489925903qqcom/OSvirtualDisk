#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "Vdisk.h"

using namespace std;


class DOS {
public:
    Vdisk disk;
    vector<IndexNode> diskDirList;//�̵�Ŀ¼

    IndexNode* curDir;//��ǰĿ¼
public:
    DOS(string fname="virtDisk.vd");
    ~DOS();
    int help(string command="");
    int mkdir(string);
    int rm(string);
    int cat(string);
    int cd(string);
    int ls();
};

