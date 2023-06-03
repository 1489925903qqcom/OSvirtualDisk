#include "DOS.h"

DOS::DOS(string fname) :disk(fname)
{
	diskDirList = disk.readIndex();//读取磁盘目录
	//如果磁盘目录为空，创建根目录
	if (diskDirList.empty()) {
		cout << "No index file found, creating a new one..." << endl;
		IndexNode root("", 'D', -1, -1);
		diskDirList.push_back(root);
		disk.writeIndex(diskDirList);
	}
	//打印目录
	curDir = 0;//当前目录指针，指向根目录
	//_curDir = diskDirList.begin();
	//cout << "Current directory: _" << curDir->name << "/" << endl;
}

DOS::~DOS() {
	disk.writeIndex(diskDirList);
	disk.readIndex();
	disk.writeIndex(diskDirList);
}
//成员函数---------------------------------------------------------
string DOS::fullPath() {
	if (diskDirList.size() <= curDir) {
		cerr << "curr out of range!" << endl;
	}
	string tmp = "/" + string(diskDirList.at(curDir).name);
	long pathPtr = diskDirList.at(curDir).father;
	while (pathPtr >= 0) {
		tmp = "/" + string(diskDirList.at(pathPtr).name) + tmp;
		pathPtr = diskDirList.at(pathPtr).father;
	}
	return tmp;
}

int DOS::ls() {
	cout << "name\ttype" << endl;
	for (long i = 0; i < diskDirList.at(curDir).children.size(); ++i) {
		cout << diskDirList.at(diskDirList.at(curDir).children.at(i)).name << '\t' <<
			diskDirList.at(diskDirList.at(curDir).children.at(i)).type << endl;
	}
	cout << endl;
	return 0;
}


int DOS::help(string command) {
	//默认为""(空字符串)
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

int DOS::cd(string childDirName) {
	if (strcmp(childDirName.c_str(), "..")) {
		this->curDir = diskDirList.at(curDir).father;
		if (curDir < 0) {
			curDir = 0;
		}
		return 0;
	}
	if (strcmp(childDirName.c_str(), ".")) {
		return 0;
	}
	long childAbsNum = -1;
	bool exist = false;
	for (long i = 0; i < diskDirList.at(curDir).children.size(); i++) {
		if (strcmp(diskDirList.at(diskDirList.at(curDir).children.at(i)).name, childDirName.c_str()) == 0) {
			childAbsNum = diskDirList.at(curDir).children.at(i);
			exist = true;
			break;
		}
	}
	//
	if (!exist) {
		cout << "None directory named " << childDirName << endl;
		return -1;
	}
	if (diskDirList.at(childAbsNum).type != 'D') {
		cout << childDirName << " is not a directory" << endl;
		return -1;
	}
	this->curDir = childAbsNum;
	return 0;
}

int DOS::mkdir(string name) {
	//如果含有空格,斜杠等符号，报错
	if (
		name.find(' ') != string::npos ||
		name.find('\\')!= string::npos ||
		name.find('/') != string::npos ||
		name.find('-') != string::npos ||
		name.find('|') != string::npos ||
		name.find('#') != string::npos ||
		name.find('@') != string::npos ||
		name.find('!') != string::npos 
	) {
		cout << "Invalid name! Do not use special char in name." << endl;
		return -1;
	}

	//查找当前目录下是否存同名文件或目录
	for (long i = 0; i < diskDirList.at(curDir).children.size(); i++) {
		if (strcmp(diskDirList.at(diskDirList.at(curDir).children.at(i)).name, name.c_str()) == 0) {
			cout << "Same name file or directory exists! (name: " << name << " )\n" << endl;
			return -1;
		}
	}
	//创建新目录
	IndexNode newDir(name.c_str(), 'D', -1, curDir);
	diskDirList.push_back(newDir);
	if (!diskDirList.empty()) {
		diskDirList.at(curDir).children.push_back(long(diskDirList.size()-1));
	}
	else {
		cerr << "Error: diskDirList is empty!" << endl;
		return -1;
	}
	cout << "New directory created! " << name << endl << endl;
	//将更新后的目录写入磁盘
	disk.writeIndex(diskDirList);
	disk.readIndex();
	return 0;
}


int DOS::rm(string trashName) {
	//查找当前目录下是否存在该文件或目录
	long trash = -1;
	bool exist = false;
	for (long i = 0; i < diskDirList.at(curDir).children.size(); i++) {
		if (strcmp(diskDirList.at(diskDirList.at(curDir).children.at(i)).name, trashName.c_str()) == 0) {
			trash = diskDirList.at(curDir).children.at(i);
			exist = true;
			break;
		}
	}
	//
	if (!exist) {
		cout << "None file or directory named " << trash << endl;
		return -1;
	}

	//记录所有需要递归删除的元素（广度优先）
	vector<long> toDel;
	toDel.push_back(trash);
	long rP = 0;
	
	while (rP < toDel.size()) {
		for (long i = 0; i < diskDirList.at(toDel.at(rP)).children.size(); i++) {
			toDel.push_back(diskDirList.at(toDel.at(rP)).children.at(i));
		}
		rP++;
	}

	//执行删除
	while (!toDel.empty()) {
		long dP = toDel.back();
		toDel.pop_back();
		//如果diskDirList.at(dP)是文件，释放文件快空间
		if (diskDirList.at(dP).type == 'F') {
			disk.clearBlocks(diskDirList.at(dP).fBlock);
		}
		/*
		//diskDirList.at(dP)之后的所有元素{father大于dP则自减,children中的所有元素自减}
		for (unsigned long i = dP + 1; i < diskDirList.size(); i++) {
			for (unsigned long j = 0; j < diskDirList.at(i).children.size(); j++) {
				diskDirList.at(i).children.at(j)--;
			}
			if (diskDirList.at(i).father > dP) { diskDirList.at(i).father--; }
		}
		*/

		//删除diskDirList.at(diskDirList.at(dP).father).children中的dP记录
		//cout << "dad before:" << diskDirList.at(diskDirList.at(dP).father).children.size() << endl;
		
		diskDirList.at(diskDirList.at(dP).father).children.erase(
			std::remove(
				diskDirList.at(diskDirList.at(dP).father).children.begin(),
				diskDirList.at(diskDirList.at(dP).father).children.end(),
				dP
			),
			diskDirList.at(diskDirList.at(dP).father).children.end()
		);

		//cout << "dad after:" << diskDirList.at(diskDirList.at(dP).father).children.size() << endl;

		//删除diskDirList.at(dP)记录
		/*
		cout << "\n\ner:" << dP<<" name:" << diskDirList.at(dP).name<<" siz:"<< diskDirList.size() << endl;
		diskDirList.erase(diskDirList.begin() + dP);
		cout << " siz2:" << diskDirList.size() << endl;
		*/
	}
	//将更新后的目录写入磁盘
	disk.writeIndex(diskDirList);
	disk.readIndex();
	return 0;
}

int DOS::cat(string) {

	return 0;
}