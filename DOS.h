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

    int mkdir(string);

    int del_r(IndexNode&);
    
    int cd(string);
    int ls();
};

