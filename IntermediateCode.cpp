#pragma execution_character_set("utf-8")
#include "IntermediateCode.h"

//�����Ϣ
extern std::string outputString;
extern bool errorFlag;
extern std::string errorString;

//���캯��
NewLabel::NewLabel() {
	index = 1;
}

//����ǩת��Ϊstring��ʽ
string NewLabel::label2string () {
	return string("Label") + to_string(index++);
}

//���ֻ�����
void IntermediateCode::divideBlocks(vector<pair<int, string> > funcEnter) {
	for (vector<pair<int, string> >::iterator iter = funcEnter.begin(); iter != funcEnter.end(); iter++) {//��ÿһ��������
		vector<Block>blocks;
		priority_queue<int, vector<int>, greater<int> >block_enter;//��¼���л��������ڵ�
		block_enter.push(iter->first);

		int endIndex = int(iter + 1 == funcEnter.end()? code.size(): (iter + 1)->first);
		for (int i = iter->first; i != endIndex; i++) {
			if (code[i].op[0] == 'j') {
				if (code[i].op == "j") {//����������j
					block_enter.push(atoi(code[i].des.c_str()));
				}
				else {//������������j=-,,j!=.j>=��j>��j<=��j<
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

		//���ֻ�����	
		Block block;
		map<int, string>labelEnter;//��ڵ�ͱ�ǩ�Ķ�Ӧ��ϵ
		map<int, int>enter_block;//������ڵ��block�Ķ�Ӧ��ϵ
		int firstFlag = true;//�������һ���ǣ��ÿ�����Ϊ������
		int enter;
		int lastEnter = block_enter.top();
		block_enter.pop();
		while (!block_enter.empty()) {
			//������Ԫʽ��block��
			enter = block_enter.top();
			block_enter.pop();

			if (enter == lastEnter) {
				continue;
			}

			for (int i = lastEnter; i != enter; i++) {
				block.codes.push_back(code[i]);
			}

			if (!firstFlag) {//�û����鲻�Ǻ�����ĵ�һ�������
				block.name = nl.label2string();
				labelEnter[lastEnter] = block.name;
			}
			else {//�û������Ǻ�����ĵ�һ�������
				block.name = iter->second;
				firstFlag = false;
			}
			enter_block[lastEnter] = (int)blocks.size();
			blocks.push_back(block);
			lastEnter = enter;
			block.codes.clear();
		}
		if (!firstFlag) {//�û����鲻�Ǻ�����ĵ�һ�������
			block.name = nl.label2string();
			labelEnter[lastEnter] = block.name;
		}
		else {//�û������Ǻ�����ĵ�һ�������
			block.name = iter->second;
			firstFlag = false;
		}
		if (iter + 1 != funcEnter.end()) {//���������������֮��
			for (int i = lastEnter; i != (iter+1)->first; i++) {
				block.codes.push_back(code[i]);
			}
		}
		else {//�����һ���������м����ĩβ
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
				if (lastCode->op == "j") {//����������j
					bIter->next1 = enter_block[atoi(lastCode->des.c_str())];
					bIter->next2 = -1;
				}
				else {//������������j=-,,j!=.j>=��j>��j<=��j<
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

//����м����
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

//���������
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

//�����м����
void IntermediateCode::pushCode(Quaternion q) {
	code.push_back(q);
}

//�����м����
void IntermediateCode::pushCode(string op, string src1, string src2, string des) {
	pushCode(Quaternion{ op,src1,src2,des });
}

//����
void IntermediateCode::backFill(list<int>nextList, int quad) {
	for (list<int>::iterator iter = nextList.begin(); iter != nextList.end(); iter++) {
		code[*iter].des = to_string(quad);
	}
}

//��ȡ�м�������
int IntermediateCode::quadNum() {
	return (int)code.size();
}

//��ȡ������
map<string, vector<Block> >* IntermediateCode::getBlock() {
	return &funcBlocks;
}