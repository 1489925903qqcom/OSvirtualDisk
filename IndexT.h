#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

struct IndexRec {//�ļ�Ŀ¼��Ŀ¼���еļ�¼��ʽ
public:
    IndexRec();
    IndexRec(long, const char*, char, long long);
    long fatherNum;//���ļ����еĸ��ڵ���
    char name[23];//�ļ���������չ����������22Byte��
    char type;//type: Dir, File��Null
    long long fBlock;//�ļ���һ�����λ��(����)����ΪĿ¼����Ч��0��
};


class IndexNode {//�ļ�Ŀ¼���ڴ��еļ�¼
public:
    char name[23];
    char type;//type: Dir, File
    long long fBlock;//�ļ���һ�����λ�ã���ΪĿ¼����Ч��0��
    vector<long> children;
     long father;

    IndexNode(const char* , char, long long,  long);
    
};

string fullPath(vector<IndexNode>& index, unsigned long curr);
int addChild(vector<IndexNode>&, IndexRec&);