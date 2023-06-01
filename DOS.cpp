#include "DOS.h"

DOS::DOS(string fname) :disk(fname)
{
	diskDirList = disk.readIndex();//��ȡ����Ŀ¼
	//�������Ŀ¼Ϊ�գ�������Ŀ¼
	if (diskDirList.empty()) {
		cout << "No index file found, creating a new one..." << endl;
		IndexNode root("", 'D', -1, nullptr);
		diskDirList.push_back(root);
		disk.writeIndex(diskDirList);
	}
	//��ӡĿ¼

	curDir = &diskDirList[0];//��ǰĿ¼ָ�룬ָ���Ŀ¼
	//_curDir = diskDirList.begin();
	cout << "Current directory: _" << curDir->name << "/" << endl;
}

DOS::~DOS() {
	disk.writeIndex(diskDirList);
	disk.readIndex();
	disk.writeIndex(diskDirList);
}
//��Ա����---------------------------------------------------------
int DOS::help(string command) {
	//Ĭ��Ϊ""(���ַ���)
	if (command == "") {
		cout << "help: show all commands" << endl;
		cout << "ls: list all files and directories" << endl;
		cout << "cd: change directory" << endl;
		cout << "mkdir: make a new directory" << endl;
		cout << "rm: remove a file or directory" << endl;
		cout << "cat: create a new file" << endl;
		cout << "exit: exit the system" << endl;
	}
	//help ls
	else if (command == "ls") {
		cout << "ls: list all files and directories" << endl;
	}
	//help cd
	else if (command == "cd") {
		cout << "cd: change directory" << endl;
	}
	//help mkdir
	else if (command == "mkdir") {
		cout << "mkdir: make a new directory" << endl;
	}
	//help rm
	else if (command == "rm") {
		cout << "rm: remove a file or directory" << endl;
	}
	//help cat
	else if (command == "cat") {
		cout << "cat: create, modify(append) or read a file" << endl;
	}
	//help exit
	else if (command == "exit") {
		cout << "exit: exit the system" << endl;
	}
	else {
		cout << "No such command!" << endl;
		return -1;
	}
	return 0;
};


int DOS::mkdir(string name) {
	//������пո񣬱���
	if (name.find(' ') != string::npos) {
		cout << "Invalid name! Do not use space in name." << endl;
		return -1;
	}
	//����Ŀ¼��¼���ҵ���ǰĿ¼�����
	unsigned long curDirNo = 0;
	for (unsigned long i = 0; i < diskDirList.size(); i++) {
		if (diskDirList.at(i).father == curDir->father) {
			if (strcmp(diskDirList[i].name, curDir->name) == 0) {
				curDirNo = i;
				cout << "curDirNo:" << curDirNo << endl;
				break;
			}
		}
	}

	//���ҵ�ǰĿ¼���Ƿ��ͬ���ļ���Ŀ¼
	for (long i = 0; i < diskDirList.at(curDirNo).children.size(); i++) {
		cout << "childSize:" << diskDirList.at(curDirNo).children.size()
			<< " which: " << i << " .name: " << diskDirList.at(curDirNo).children.at(i)->name << ", yourName:" << name.c_str()
			<< " cmp:" << strcmp(diskDirList.at(curDirNo).children.at(i)->name, name.c_str()) << ":" << endl;;

		if (strcmp(diskDirList.at(curDirNo).children.at(i)->name, name.c_str()) == 0) {
			cout << "Same name file or directory exists! (name: " << name << " )\n" << endl;
			return -1;
		}
	}
	//������Ŀ¼
	IndexNode newDir(name.c_str(), 'D', -1, curDir);
	diskDirList.push_back(newDir);
	if (!diskDirList.empty()) {
		//cout << curDir->children.size()<<" " << curDir->children.max_size() << endl;
		IndexNode* newDirP = &diskDirList.back();
		//(*curDir).children.push_back(newDirP);// bug:��дȨ�޲���
		diskDirList.at(curDirNo).children.push_back(newDirP);
		curDir = &diskDirList.at(curDirNo);
	}
	else {
		cerr << "Error: diskDirList is empty!" << endl;
		return -1;
	}

	cout << "New directory created! " << name << endl << endl;
	//�����º��Ŀ¼д�����

	disk.writeIndex(diskDirList);
	disk.readIndex();
	return 0;
}

int DOS::rm(string trashName) {
	//���ҵ�ǰĿ¼���Ƿ���ڸ��ļ���Ŀ¼
	IndexNode* trash = nullptr;
	bool exist = false;
	for (long i = 0; i < curDir->children.size(); i++) {
		if (strcmp(curDir->children.at(i)->name, trashName.c_str()) == 0) {
			trash = curDir->children.at(i);
			exist = true;
			break;
		}
	}
	//
	if (!exist) {
		cout << "None file or directory named " << trash << endl;
		return -1;
	}
	//��¼������Ҫ�ݹ�ɾ����Ԫ�أ�������ȣ�
	vector<IndexNode*> toDel;
	toDel.push_back(trash);
	IndexNode* rP = toDel[0];
	for (int i = 0; i < rP->children.size(); i++) {
		toDel.push_back(rP->children.at(i));
	}
	while (rP != toDel.back()) {
		rP++;
		for (int i = 0; i < rP->children.size(); i++) {
			toDel.push_back(rP->children.at(i));
		}
	}
	//ִ��ɾ��
	while (!toDel.empty()) {
		auto dP = toDel.back();
		toDel.pop_back();
		//��������Ŀ¼(vector�ڴ治һ��������������ֱ��Ѱַ)
		for (long i = 0; i < diskDirList.size(); i++) {
			if (diskDirList[i].father != dP->father) { continue; }
			if (string(diskDirList[i].name) != string(dP->name)) { continue; }
			//Ԫ��ɾ���б������һλ��¼֮�������Ԫ�ص��ϲ�Ŀ¼ָ���Լ�
			for (
				vector<IndexNode>::iterator itDec = diskDirList.end() - 1;
				itDec != diskDirList.begin() + i; itDec--) {
				itDec->father--;
			}
			//������ļ����ͷŴ��̿ռ�
			if (dP->type == 'F' && dP->fBlock > 0) {
				disk.clearBlocks(dP->fBlock);
			}
			diskDirList.erase(diskDirList.begin() + i);
			break;
		}
	}
	//�����º��Ŀ¼д�����
	disk.writeIndex(diskDirList);
	return 0;
}

