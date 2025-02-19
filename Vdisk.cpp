#include "Vdisk.h"

/*
	暂时未完成的功能：
	·磁盘满时的处理
*/

Vdisk::Vdisk(Vdisk&& disk) noexcept :
	diskFile(move(disk.diskFile)), minAvailable(disk.minAvailable)
{
	blockStatus = new char[blockNum];
	for (long long i = 0; i < blockNum; i++)
	{
		this->blockStatus[i] = disk.blockStatus[i];
	}
	disk.~Vdisk();
}

Vdisk::Vdisk(string fileName) :
	diskFile(fileName, ios::in | ios::out | ios::binary)
{
	blockStatus = new char[blockNum];
	if (!diskFile) {
		cout << "Virtual disk file NOT found, Creat a new one" << endl;
		diskFile.open(fileName, ios::out | ios::binary);
		if (diskFile) { diskFile.close(); }
		else {
			cerr << "ERROR: Could not creat file";
			system("pause");
		}
		diskFile.open(fileName, ios::in | ios::out | ios::binary);
		//初始化磁盘块状态
		BlockHead headDat('D', -1, -1);//初始化磁盘块头:目录块
		Block headBlock(headDat);//初始化磁盘块
		//初始化根目录
		IndexRec _root = { -1,"",'D',-1 };
		memcpy(
			(char*)&headBlock.data[0],
			&_root,
			sizeof(IndexRec)
		);
		IndexRec blankrec = { -1, "", 'N', -1 };
		for (int _i = 1; _i < 25; _i++) {
			memcpy(
				(char*)&headBlock.data[40 * _i],
				&blankrec,
				sizeof(IndexRec)
			);
		}
		diskFile.seekp(ios::beg);
		diskFile.write((char*)&headBlock, sizeof(Block));
		auto tmpBlock = Block();
		for (long long i = 1; i < blockNum; i++)
		{
			diskFile.write((char*)&tmpBlock, sizeof(Block));
		}
	}

	//获取磁盘块状态
	for (long long i = 0; i < blockNum; i++) {
		diskFile.seekg(i * sizeof(Block), ios::beg);
		diskFile.read((char*)&blockStatus[i], sizeof(char));
	}

	//寻找最小可用块号
	minAvailable = blockNum;
	for (long long i = 0; i < blockNum; i++) {
		if (blockStatus[i] == 'N') {
			minAvailable = i;
			break;
		}
	}
	cout << "\nVdisk mount OK!\n" << endl;
}

Vdisk::~Vdisk() {
	delete[] blockStatus;
	diskFile.close();
	cout << "\nVdisk closed!\n" << endl;
}


//-----------------------------------------------------------
//成员函数
//-----------------------------------------------------------

Block Vdisk::readBlock(long long fBlockNum) {
	if (fBlockNum < 0 || fBlockNum >= blockNum) {
		cerr << "ERROR: Block number out of range" << endl;
		system("pause");
		exit(-1);
	}
	Block temp;
	//读取编号为fBlockNum的磁盘块内容并返回
	diskFile.seekg(fBlockNum * sizeof(Block), ios::beg);
	diskFile.read((char*)&temp, sizeof(Block));
	return temp;
}

int Vdisk::writeBlock(const Block& source, long long distBlockNum) {
	if (distBlockNum < 0 || distBlockNum >= blockNum) {
		cerr << "ERROR: Block number out of range" << endl;
		system("pause");
		return -1;
	}

	//将文件指针移动到指定磁盘块位置
	diskFile.seekp(distBlockNum * sizeof(Block), ios::beg);
	//将数据写入磁盘块
	diskFile.write((char*)&source, sizeof(Block));
	return 0;
}

//Clear blocks------------------------------------------------
int Vdisk::clearBlocks(long long fBlockNum){
	if (fBlockNum < 0 || fBlockNum >= blockNum) {
		cerr << "ERROR: Block number out of range" << endl;
		system("pause");
		return -1;
	}
	long long toOpBlock = fBlockNum;
	vector<long long> trash;
	//查找后续不需要的块编号，添加到数组trash中
	while (toOpBlock != -1) {
		// 更新minAvailable
		if (minAvailable > toOpBlock) {
			minAvailable = toOpBlock;
		}
		trash.push_back(toOpBlock);
		//查找后续块编号，添加到数组trash中
		BlockHead trashHead;
		diskFile.seekg(toOpBlock * sizeof(Block), ios::beg);
		diskFile.read((char*)&trashHead, sizeof(BlockHead));
		toOpBlock = trashHead.nextLL;
	}
	//将trash数组中的块置为可用
	BlockHead nullHead('N', -1, -1);
	for (int i = 0; i < trash.size(); ++i) {
		diskFile.seekp(trash[i] * sizeof(Block), ios::beg);
		diskFile.write((char*)&nullHead, sizeof(BlockHead));
		blockStatus[trash[i]] = 'N';
	}
	return 0;
}

//Index operate------------------------------------------------
vector<IndexNode>  Vdisk::readIndex(long long fBlockNum) {
	if (fBlockNum < 0 || fBlockNum >= blockNum) {
		cerr << "ERROR: Block number out of range" << endl;
		system("pause");
		exit(-1);
	}
	vector<IndexNode> index; //文件树
	bool _flagNotEnd = true;
	long long nowBlock = fBlockNum;//当前磁盘块号

	while (_flagNotEnd) {//读取每一个目录块链的{文件,目录}记录
		BlockHead tHead;
		diskFile.seekg(nowBlock * sizeof(Block), ios::beg);
		diskFile.read((char*)&tHead, sizeof(BlockHead));
		if (tHead.nextLL <= 0) { _flagNotEnd = false; }
		//读取块中的记录
		for (int i = 0; i < 25; i++) {
			IndexRec tRec;
			diskFile.seekg(nowBlock * sizeof(Block)+ sizeof (BlockHead) + i * sizeof(IndexRec), ios::beg);
			diskFile.read((char*)&tRec, sizeof(IndexRec));
			if (tRec.type == 'N') { break; }//读到空记录则退出
			else { addChild(index, tRec); }//添加记录到文件树
		}
		nowBlock = tHead.nextLL;
	}
	return index;
}

int Vdisk::writeIndex(const vector<IndexNode>& index, long long distBlockNum) {
	if (distBlockNum < 0 || distBlockNum >= blockNum) {
		cerr << "ERROR: Block number out of range" << endl;
		system("pause");
		return -1;
	}
	//̫将文件树转换为有序磁盘目录记录
	vector<IndexRec> rec;
	rec.push_back(IndexRec(
		long(-1),
		index.at(0).name,
		index.at(0).type,
		index.at(0).fBlock
	));
	unsigned long now = 0;
	//unsigned long last = 1;
	while (now < rec.size()) {
		for (unsigned long i = 0; i < index.at(now).children.size(); ++i) {
			rec.push_back(
				//新实例化一个IndexRec对象
				IndexRec(
					now,
					index.at(index.at(now).children.at(i)).name,
					index.at(index.at(now).children.at(i)).type,
					index.at(index.at(now).children.at(i)).fBlock
				)
			);
			//last++;
		}
		now++;
	}

	// 写入文件树磁盘目录记录   
	long long toOpBlock = distBlockNum;
	long long preOpBlock = -1;
	for (int i = 0; i < rec.size(); ) {
		Block temp;
		if (toOpBlock == -1) {// 磁盘目录块不够,需要新分配一个磁盘块
			temp.head.preLL = preOpBlock;
			temp.head.nextLL = -1;
			temp.head.status[0] = 'D';

			toOpBlock = minAvailable;
			// 找到下一个可用的磁盘块
			while (blockStatus[minAvailable] != 'N' && minAvailable < blockNum) {
				minAvailable++;
			}
			blockStatus[toOpBlock] = 'D';
			// 更新上一个磁盘块的nextLL
			if (preOpBlock != -1) {
				BlockHead tempH;
				diskFile.seekg(preOpBlock * sizeof(Block), ios::beg);
				diskFile.read((char*)&tempH, sizeof(BlockHead));
				tempH.nextLL = toOpBlock;
				diskFile.seekp(preOpBlock * sizeof(Block), ios::beg);
				diskFile.write((char*)&tempH, sizeof(BlockHead));
			}
		}
		else {
			diskFile.seekg(toOpBlock * sizeof(Block), ios::beg);
			diskFile.read((char*)&temp.head, sizeof(BlockHead));
			//如果块类型不是目录块,报错
			if (temp.head.status[0] != 'D') {
				cerr << "ERROR: Block status error" << endl;
				system("pause");
				return -1;
			}
		}

		IndexRec blankrec[25];
		//cout << endl << endl;
		for (int j = 0; j < 25; ++j, ++i) {
			if (i >= rec.size()) { break; }
			blankrec[j] = rec[i];
		}
		memcpy(
			(char*)temp.data,
			blankrec,
			1000
		);

		//cout << endl << endl;
		// 更新对应的那个磁盘目录块状态
		diskFile.seekp(toOpBlock * sizeof(Block), ios::beg);
		diskFile.write((char*)&temp, sizeof(Block));
		preOpBlock = toOpBlock;
		toOpBlock = temp.head.nextLL;
	}

	// 如果文件已经写完，将最后一块的下一块块号置为 -1 并更新之后的块状态为可用
	if (toOpBlock != -1) {
		// 将最后一块的nextLL置为-1
		BlockHead tempH;
		diskFile.seekg(preOpBlock * sizeof(Block), ios::beg);
		diskFile.read((char*)&tempH, sizeof(BlockHead));
		tempH.nextLL = -1;
		diskFile.seekp(preOpBlock * sizeof(Block), ios::beg);
		diskFile.write((char*)&tempH, sizeof(BlockHead));
		// 更新之后的块状态为可用
		this->clearBlocks(toOpBlock);
	}
	return 0;
}


//File operate------------------------------------------------
vector<char> Vdisk::readFile(long long fBlockNum) {
	if (fBlockNum < 0 || fBlockNum >= blockNum) {
		cerr << "ERROR: Block number out of range" << endl;
		system("pause");
		exit(-1);
	}
	//从块中读取数据，拼接成文件内容
	vector<char> fileCont;

	//如果块内文件不足1000字节，读取到文件末尾
	long long nextBlockNum = fBlockNum;
	while (nextBlockNum != -1) {
		Block temp = readBlock(nextBlockNum);
		if (temp.head.status[0] != 'F') {//判断块状态是否为文件
			cerr << "ERROR: Block status error" << endl;
			system("pause");
			exit(1);
		}

		for (int i = 0; i < 1000; i++) {
			fileCont.push_back(temp.data[i]);
			if (temp.data[i] == '\0') { break; }
		}
		nextBlockNum = temp.head.nextLL;
	}

	return fileCont;
}

int Vdisk::writeFile(vector<char>& content, long long distBlockNum) {//write file data to blocks
	if (distBlockNum < 0 || distBlockNum >= blockNum) {
		cerr << "ERROR: Block number out of range" << endl;
		system("pause");
		return -1;
	}
	//拆分content，适应Block的data大小
	//写入content到磁盘文件块
	long long toOpBlock = distBlockNum;
	long long preOpBlock = -1;
	for (int i = 0; i < content.size(); ) {
		Block temp;
		if (toOpBlock == -1) {// 磁盘目录块不够,需要新分配一个磁盘块
			temp.head.preLL = preOpBlock;
			temp.head.nextLL = -1;
			temp.head.status[0] = 'F';
			toOpBlock = minAvailable;
			// 找到下一个可用的磁盘块
			while (blockStatus[minAvailable] != 'N' && minAvailable < blockNum) {
				minAvailable++;
			}
			blockStatus[toOpBlock] = 'F';
			// 更新上一个磁盘块的nextLL
			if (preOpBlock != -1) {
				BlockHead tempH;
				diskFile.seekg(preOpBlock * sizeof(Block), ios::beg);
				diskFile.read((char*)&tempH, sizeof(BlockHead));
				tempH.nextLL = toOpBlock;
				diskFile.seekp(preOpBlock * sizeof(Block), ios::beg);
				diskFile.write((char*)&tempH, sizeof(BlockHead));
			}
		}
		else {
			diskFile.seekg(toOpBlock * sizeof(Block), ios::beg);
			diskFile.read((char*)&temp.head, sizeof(BlockHead));
			//如果块状态是目录块，报错
			if (temp.head.status[0] != 'F') {
				cerr << "ERROR: Block status error" << endl;
				//system("pause");
				return -1;
			}
		}

		for (int j = 0; j < 1000; ++j, ++i) {
			if (i >= content.size()) { break; }
			temp.data[j] = content[i];
		}
		// 更新对应的那个磁盘目录块状态
		diskFile.seekp(toOpBlock * sizeof(Block), ios::beg);
		diskFile.write((char*)&temp, sizeof(Block));
		preOpBlock = toOpBlock;
		toOpBlock = temp.head.nextLL;
	}

	// 如果文件已经写完，将最后一块的后续块链的foreLL，BackLL置为 -1 ，status[0]置为N
	if (toOpBlock != -1) {
		BlockHead tempH;
		diskFile.seekg(preOpBlock * sizeof(Block), ios::beg);
		diskFile.read((char*)&tempH, sizeof(BlockHead));
		tempH.nextLL = -1;
		diskFile.seekp(preOpBlock * sizeof(Block), ios::beg);
		diskFile.write((char*)&tempH, sizeof(BlockHead));

		clearBlocks(toOpBlock);
	}
	return 0;
}