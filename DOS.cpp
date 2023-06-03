#include "DOS.h"

DOS::DOS(string fname) :disk(fname)
{
	diskDirList = disk.readIndex();//��ȡ����Ŀ¼
	//�������Ŀ¼Ϊ�գ�������Ŀ¼
	if (diskDirList.empty()) {
		cout << "No index file found, creating a new one..." << endl;
		IndexNode root("", 'D', -1, -1);
		diskDirList.push_back(root);
		disk.writeIndex(diskDirList);
	}
	//��ӡĿ¼
	curDir = 0;//��ǰĿ¼ָ�룬ָ���Ŀ¼
	//_curDir = diskDirList.begin();
	//cout << "Current directory: _" << curDir->name << "/" << endl;
}

DOS::~DOS() {
	disk.writeIndex(diskDirList);
	disk.readIndex();
	disk.writeIndex(diskDirList);
}
//��Ա����---------------------------------------------------------
string DOS::fullPath() {
	if (diskDirList.size() <= curDir) {
		cerr << "curr out of range!" << endl;
	}
	string tmp = "/" + string(diskDirList.at(curDir).name);
	long pathPtr = diskDirList.at(curDir).father;
	while (pathPtr > 0) {
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

int DOS::cd(string childDirName) {
	if (strcmp(childDirName.c_str(), "..") == 0) {
		this->curDir = diskDirList.at(curDir).father;
		if (curDir < 0) {
			curDir = 0;
		}
		return 0;
	}
	if (strcmp(childDirName.c_str(), ".") == 0) {
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
	curDir = childAbsNum;
	return 0;
}

int DOS::mkdir(string name) {
	//������пո�,б�ܵȷ��ţ�����
	if (
		name.find(' ') != string::npos ||
		name.find('\\') != string::npos ||
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

	//���ҵ�ǰĿ¼���Ƿ��ͬ���ļ���Ŀ¼
	for (long i = 0; i < diskDirList.at(curDir).children.size(); i++) {
		if (strcmp(diskDirList.at(diskDirList.at(curDir).children.at(i)).name, name.c_str()) == 0) {
			cout << "Same name file or directory exists! (name: " << name << " )\n" << endl;
			return -1;
		}
	}
	//������Ŀ¼
	IndexNode newDir(name.c_str(), 'D', -1, curDir);
	diskDirList.push_back(newDir);
	if (!diskDirList.empty()) {
		diskDirList.at(curDir).children.push_back(long(diskDirList.size() - 1));
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
		cout << "None file or directory named " << trashName << endl;
		return -1;
	}

	//��¼������Ҫ�ݹ�ɾ����Ԫ�أ�������ȣ�
	vector<long> toDel;
	toDel.push_back(trash);
	long rP = 0;

	while (rP < toDel.size()) {
		for (long i = 0; i < diskDirList.at(toDel.at(rP)).children.size(); i++) {
			toDel.push_back(diskDirList.at(toDel.at(rP)).children.at(i));
		}
		rP++;
	}

	//ִ��ɾ��
	while (!toDel.empty()) {
		long dP = toDel.back();
		toDel.pop_back();
		//���diskDirList.at(dP)���ļ����ͷ��ļ���ռ�
		if (diskDirList.at(dP).type == 'F') {
			disk.clearBlocks(diskDirList.at(dP).fBlock);
		}
		/*
		//diskDirList.at(dP)֮�������Ԫ��{father����dP���Լ�,children�е�����Ԫ���Լ�}
		for (unsigned long i = dP + 1; i < diskDirList.size(); i++) {
			for (unsigned long j = 0; j < diskDirList.at(i).children.size(); j++) {
				diskDirList.at(i).children.at(j)--;
			}
			if (diskDirList.at(i).father > dP) { diskDirList.at(i).father--; }
		}
		*/

		//ɾ��diskDirList.at(diskDirList.at(dP).father).children�е�dP��¼
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

		//ɾ��diskDirList.at(dP)��¼
		/*
		cout << "\n\ner:" << dP<<" name:" << diskDirList.at(dP).name<<" siz:"<< diskDirList.size() << endl;
		diskDirList.erase(diskDirList.begin() + dP);
		cout << " siz2:" << diskDirList.size() << endl;
		*/
	}
	//�����º��Ŀ¼д�����
	disk.writeIndex(diskDirList);
	disk.readIndex();
	return 0;
}

int DOS::cat_r(string fName) {
	long fNum = -1;
	bool exist = false;
	for (long i = 0; i < diskDirList.at(curDir).children.size(); i++) {
		if (strcmp(diskDirList.at(diskDirList.at(curDir).children.at(i)).name, fName.c_str()) == 0) {
			fNum = diskDirList.at(curDir).children.at(i);
			exist = true;
			break;
		}
	}
	//
	if (!exist) {
		cout << "File Non Exist! " << fName << endl;
		return -1;
	}
	else if (diskDirList.at(fNum).type != 'F') {
		cout << fName << " is Not a File!" << endl;
		return -1;
	}
	vector<char> temp = disk.readFile(diskDirList.at(fNum).fBlock);
	//���temp����
	cout << "File name: " << fName << "\tFile Content: " << endl;
	cout << "----------File-Start-----------" << endl;
	for (vector<char>::iterator it = temp.begin(); it != temp.end(); it++) { cout << *it; }
	cout << endl << "------------File-END------------" << endl << endl;
	return 0;
}

int DOS::cat_w(string fName, vector<char>& cont) {
	//�ж��ļ��Ƿ����
	long fNum = -1;
	bool exist = false;
	for (long i = 0; i < diskDirList.at(curDir).children.size(); i++) {
		if (strcmp(diskDirList.at(diskDirList.at(curDir).children.at(i)).name, fName.c_str()) == 0) {
			fNum = diskDirList.at(curDir).children.at(i);
			exist = true;
			break;
		}
	}
	//
	if (!exist) {
		cout << "File Non Exist��create a new one." << endl;
		//�������ļ�
		//������пո�,б�ܵȷ��ţ�����
		if (
			fName.find(' ') != string::npos ||
			fName.find('\\') != string::npos ||
			fName.find('/') != string::npos ||
			fName.find('-') != string::npos ||
			fName.find('|') != string::npos ||
			fName.find('#') != string::npos ||
			fName.find('@') != string::npos ||
			fName.find('!') != string::npos
			) {
			cout << "Invalid name! Do not use special char in name." << endl;
			return -1;
		}
		//������Ŀ¼��¼
		IndexNode newDir(fName.c_str(), 'F', disk.minAvailable, curDir);
		BlockHead th('F', -1, -1);
		disk.writeBlock(Block(th), disk.minAvailable);
		diskDirList.push_back(newDir);
		if (!diskDirList.empty()) {
			diskDirList.at(curDir).children.push_back(long(diskDirList.size() - 1));
			while (disk.blockStatus[disk.minAvailable] != 'N' && disk.minAvailable < blockNum) {
				disk.minAvailable++;
			}
		}
		else {
			cerr << "Error: diskDirList is empty!" << endl;
			return -1;
		}
		cout << "New file created! " << fName << endl << endl;
		//�����º��Ŀ¼д�����
		disk.writeIndex(diskDirList);
		disk.readIndex();
		fNum = diskDirList.size() - 1;
	}
	//���fName���ڣ��ж��Ƿ�Ϊ�ļ�
	else if (diskDirList.at(fNum).type != 'F') {
		cout << fName << " is Not a File!" << endl;
		return -1;
	}
	//д���ļ�����
	cout << "Write to file: " << fName << endl;
	disk.writeFile(cont, diskDirList.at(fNum).fBlock);
	return 0;
}

int DOS::cat_a(string fName, vector<char>& cont) {
	long fNum = -1;
	bool exist = false;
	for (long i = 0; i < diskDirList.at(curDir).children.size(); i++) {
		if (strcmp(diskDirList.at(diskDirList.at(curDir).children.at(i)).name, fName.c_str()) == 0) {
			fNum = diskDirList.at(curDir).children.at(i);
			exist = true;
			break;
		}
	}
	//
	if (!exist) {
		cout << "File Non Exist��create a new one." << endl;
		//�������ļ�
		//������пո�,б�ܵȷ��ţ�����
		if (
			fName.find(' ') != string::npos ||
			fName.find('\\') != string::npos ||
			fName.find('/') != string::npos ||
			fName.find('-') != string::npos ||
			fName.find('|') != string::npos ||
			fName.find('#') != string::npos ||
			fName.find('@') != string::npos ||
			fName.find('!') != string::npos
			) {
			cout << "Invalid name! Do not use special char in name." << endl;
			return -1;
		}
		//������Ŀ¼��¼
		IndexNode newDir(fName.c_str(), 'F', disk.minAvailable, curDir);
		BlockHead th('F', -1, -1);
		disk.writeBlock(Block(th), disk.minAvailable);
		diskDirList.push_back(newDir);
		if (!diskDirList.empty()) {
			diskDirList.at(curDir).children.push_back(long(diskDirList.size() - 1));
			while (disk.blockStatus[disk.minAvailable] != 'N' && disk.minAvailable < blockNum) {
				disk.minAvailable++;
			}
		}
		else {
			cerr << "Error: diskDirList is empty!" << endl;
			return -1;
		}
		cout << "New file created! " << fName << endl << endl;
		//�����º��Ŀ¼д�����
		disk.writeIndex(diskDirList);
		disk.readIndex();
		fNum = diskDirList.size() - 1;
	}
	else if (diskDirList.at(fNum).type != 'F') {
		cout << fName << " is Not a File!" << endl;
		return -1;
	}
	vector<char> temp = disk.readFile(diskDirList.at(fNum).fBlock);
	//���temp����
	cout << "File name: " << fName << "\tFile Content: " << endl;
	cout << "----------File-Start-----------" << endl;
	for (vector<char>::iterator it = temp.begin(); it != temp.end(); it++) { cout << *it; }
	cout << endl << "------------File-END------------" << endl;
	cout << "New content appended:" << endl;
	for (vector<char>::iterator it = cont.begin(); it != cont.end(); it++) {
		cout << *it;
		temp.push_back(*it);
	}
	cout << endl << "-----------Append-END-----------" << endl << endl ;
	//д���ļ�����
	//temp.insert(temp.end(), cont.begin(), cont.end());
	disk.writeFile(temp, diskDirList.at(fNum).fBlock);
	return 0;
}