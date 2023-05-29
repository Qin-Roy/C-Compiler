#pragma execution_character_set("utf-8")
#include "IntermediateCode.h"

//输出信息
extern std::string outputString;
extern bool errorFlag;
extern std::string errorString;

//构造函数
NewLabel::NewLabel() {
	index = 1;
}

//将标签转换为string格式
string NewLabel::label2string () {
	return string("Label") + to_string(index++);
}

//划分基本块
void IntermediateCode::divideBlocks(vector<pair<int, string> > funcEnter) {
	for (vector<pair<int, string> >::iterator iter = funcEnter.begin(); iter != funcEnter.end(); iter++) {//对每一个函数块
		vector<Block>blocks;
		priority_queue<int, vector<int>, greater<int> >block_enter;//记录所有基本块的入口点
		block_enter.push(iter->first);

		int endIndex = int(iter + 1 == funcEnter.end()? code.size(): (iter + 1)->first);
		for (int i = iter->first; i != endIndex; i++) {
			if (code[i].op[0] == 'j') {
				if (code[i].op == "j") {//若操作符是j
					block_enter.push(atoi(code[i].des.c_str()));
				}
				else {//若果操作符是j=-,,j!=.j>=，j>，j<=，j<
					if (i + 1 < endIndex) {
						block_enter.push(i + 1);
					}
					block_enter.push(atoi(code[i].des.c_str()));
				}
			}
			else if (code[i].op == "return" || code[i].op == "call") {
				if (i + 1 < endIndex) {
					block_enter.push(i + 1);
				}
			}
		}

		//划分基本块	
		Block block;
		map<int, string>labelEnter;//入口点和标签的对应关系
		map<int, int>enter_block;//建立入口点和block的对应关系
		int firstFlag = true;//函数块第一块标记，该块命名为函数名
		int enter;
		int lastEnter = block_enter.top();
		block_enter.pop();
		while (!block_enter.empty()) {
			//插入四元式到block中
			enter = block_enter.top();
			block_enter.pop();

			if (enter == lastEnter) {
				continue;
			}

			for (int i = lastEnter; i != enter; i++) {
				block.codes.push_back(code[i]);
			}

			if (!firstFlag) {//该基本块不是函数块的第一块基本块
				block.name = nl.label2string();
				labelEnter[lastEnter] = block.name;
			}
			else {//该基本块是函数块的第一块基本块
				block.name = iter->second;
				firstFlag = false;
			}
			enter_block[lastEnter] = (int)blocks.size();
			blocks.push_back(block);
			lastEnter = enter;
			block.codes.clear();
		}
		if (!firstFlag) {//该基本块不是函数块的第一块基本块
			block.name = nl.label2string();
			labelEnter[lastEnter] = block.name;
		}
		else {//该基本块是函数块的第一块基本块
			block.name = iter->second;
			firstFlag = false;
		}
		if (iter + 1 != funcEnter.end()) {//在两个函数的起点之间
			for (int i = lastEnter; i != (iter+1)->first; i++) {
				block.codes.push_back(code[i]);
			}
		}
		else {//在最后一个函数至中间代码末尾
			for (int i = lastEnter; i != code.size(); i++) {
				block.codes.push_back(code[i]);
			}
		}
		enter_block[lastEnter] = (int)blocks.size();
		blocks.push_back(block);

		int blockIndex = 0;
		for (vector<Block>::iterator bIter = blocks.begin(); bIter != blocks.end(); bIter++, blockIndex++) {
			vector<Quaternion>::reverse_iterator lastCode = bIter->codes.rbegin();
			if (lastCode->op[0] == 'j') {
				if (lastCode->op == "j") {//若操作符是j
					bIter->next1 = enter_block[atoi(lastCode->des.c_str())];
					bIter->next2 = -1;
				}
				else {//若果操作符是j=-,,j!=.j>=，j>，j<=，j<
					bIter->next1 = blockIndex + 1;
					bIter->next2 = enter_block[atoi(lastCode->des.c_str())];
					bIter->next2 = bIter->next1 == bIter->next2 ? -1 : bIter->next2;
				}
				lastCode->des = labelEnter[atoi(lastCode->des.c_str())];
			}
			else if (lastCode->op == "return") {
				bIter->next1 = bIter->next2 = -1;
			}
			else {
				bIter->next1 = blockIndex + 1;
				bIter->next2 = -1;
			}
			
			
		}

		funcBlocks[iter->second] = blocks;
	}
}

//输出中间代码
void IntermediateCode::output() {
	int i = 0;
	for (vector<Quaternion>::iterator iter = code.begin(); iter != code.end(); iter++, i++) {
		int len = 4 - to_string(i).length();
		while (len) {
			len--;
			outputString += " ";
		}
		outputString += to_string(i);
		outputString += "  ";
		outputString += "( " + iter->op + " , ";
		outputString += iter->src1 + " , ";
		outputString += iter->src2 + " , ";
		outputString += iter->des + " )";
		outputString += "\n";
	}
}

//输出基本块
void IntermediateCode::outputBlocks() {
	for (map<string, vector<Block> >::iterator iter = funcBlocks.begin(); iter != funcBlocks.end(); iter++) {
		outputString += "[" + iter->first + "]\n";
		for (vector<Block>::iterator bIter = iter->second.begin(); bIter != iter->second.end(); bIter++) {
			outputString += bIter->name + ":\n";
			for (vector<Quaternion>::iterator cIter = bIter->codes.begin(); cIter != bIter->codes.end(); cIter++) {
				outputString += "    (" + cIter->op + "," + cIter->src1 + "," + cIter->src2 + "," + cIter->des + ")\n";
			}
			outputString += "    next1 = " + to_string(bIter->next1) + "\n";
			outputString += "    next2 = " + to_string(bIter->next2) + "\n";
		}
		outputString += "\n";
	}
}

//插入中间代码
void IntermediateCode::pushCode(Quaternion q) {
	code.push_back(q);
}

//插入中间代码
void IntermediateCode::pushCode(string op, string src1, string src2, string des) {
	pushCode(Quaternion{ op,src1,src2,des });
}

//回填
void IntermediateCode::backFill(list<int>nextList, int quad) {
	for (list<int>::iterator iter = nextList.begin(); iter != nextList.end(); iter++) {
		code[*iter].des = to_string(quad);
	}
}

//获取中间代码个数
int IntermediateCode::quadNum() {
	return (int)code.size();
}

//获取基本块
map<string, vector<Block> >* IntermediateCode::getBlock() {
	return &funcBlocks;
}