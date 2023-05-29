#pragma execution_character_set("utf-8")
#include "ObjectCode.h"

//�����Ϣ
extern std::string outputString;
extern bool errorFlag;
extern std::string errorString;

//�Ƿ�Ϊ����
bool isVar(string name) {
	return isalpha(name[0]);
}

//�Ƿ�Ϊ����
bool isNum(string name) {
	return isdigit(name[0]);
}

//�Ƿ�Ϊ����ָ��
bool isControl(string op) {
	if (op[0] == 'j' || op == "call" || op == "return" || op == "get") {
		return true;
	}
	return false;
}

//���캯��
VarInfo::VarInfo(int next, bool active) {
	this->next = next;
	this->active = active;
}

VarInfo::VarInfo(const VarInfo&other) {
	this->active = other.active;
	this->next = other.next;
}

VarInfo::VarInfo() {
	this->active = 0;
	this->next = 0;
}

//���������Ϣ
void VarInfo::output(ostream& out) {
	out << "(";
	if (next == -1)
		out << "^";
	else
		out << next;
	out << ",";
	if (active)
		out << "y";
	else
		out << "^";

	out << ")";
}

//���������Ϣ
void VarInfo::output() {
	outputString += "(";
	if (next == -1)
		outputString += "^";
	else
		outputString += next;
	outputString += ",";
	if (active)
		outputString += "y";
	else
		outputString += "^";

	outputString += ")";
}

//���캯��
QuaternionInfo::QuaternionInfo(Quaternion q, VarInfo info1, VarInfo info2, VarInfo info3) {
	this->q = q;
	this->info1 = info1;
	this->info2 = info2;
	this->info3 = info3;
}

//�����Ԫʽ��Ϣ
void QuaternionInfo::output(ostream& out) {
	out << "(" << q.op << "," << q.src1 << "," << q.src2 << "," << q.des << ")";
	info1.output(out);
	info2.output(out);
	info3.output(out);
}

//�����Ԫʽ��Ϣ
void QuaternionInfo::output() {
	outputString += "(" + q.op + "," + q.src1 + "," + q.src2 + "," + q.des + ")";
	info1.output();
	info2.output();
	info3.output();
}

//���캯��
ObjectCode::ObjectCode() {

}

//�������
void ObjectCode::storeVar(string reg, string var) {
	if (varOffset.find(var) != varOffset.end()) {//����Ѿ�Ϊ*iter������˴洢�ռ�
		objectCodes.push_back(string("sw ") + reg + " " + to_string(varOffset[var]) + "($sp)");
	}
	else {
		varOffset[var] = top;
		top += 4;
		objectCodes.push_back(string("sw ") + reg + " " + to_string(varOffset[var]) + "($sp)");
	}
	Avalue[var].insert(var);
}

//�ͷű���
void ObjectCode::releaseVar(string var) {
	for (set<string>::iterator iter = Avalue[var].begin(); iter != Avalue[var].end(); iter++) {
		if ((*iter)[0] == '$') {
			Rvalue[*iter].erase(var);
			if (Rvalue[*iter].size() == 0 && (*iter)[1] == 's') {
				freeReg.push_back(*iter);
			}
		}
	}
	Avalue[var].clear();
}

//Ϊ���ñ�������Ĵ���
string ObjectCode::allocateReg() {
	//�������δ����ļĴ����������ѡȡһ��RiΪ����Ҫ�ļĴ���R
	string ret;
	if (freeReg.size()) {
		ret = freeReg.back();
		freeReg.pop_back();
		return ret;
	}

	/*
	���ѷ���ļĴ�����ѡȡһ��RiΪ����Ҫ�ļĴ���R�����ʹ��Ri��������������
	ռ��Ri�ı�����ֵҲͬʱ����ڸñ��������浥Ԫ��
	�����ڻ�������Ҫ����Զ�Ľ����Ż����õ��򲻻����õ���
	*/

	const int inf = 1000000;
	int maxNextPos = 0;
	for (map<string, set<string> >::iterator iter = Rvalue.begin(); iter != Rvalue.end(); iter++) {//�������еļĴ���
		int nextpos = inf;
		for (set<string>::iterator viter = iter->second.begin(); viter != iter->second.end(); viter++) {//�����Ĵ����д���ı���
			bool inFlag = false;//�������������ط��洢�ı�־
			for (set<string>::iterator aiter = Avalue[*viter].begin(); aiter != Avalue[*viter].end(); aiter++) {//���������Ĵ洢λ��
				if (*aiter != iter->first) {//��������洢�������ط�
					inFlag = true;
					break;
				}
			}
			if (!inFlag) {//����������洢�ڼĴ����У��Ϳ�δ���ںδ������øñ���
				for (vector<QuaternionInfo>::iterator cIter = nowQuaternion; cIter != nowIBlock->codes.end(); cIter++) {
					if (*viter == cIter->q.src1 || *viter == cIter->q.src2) {
						nextpos = int(cIter - nowQuaternion);
					}
					else if (*viter == cIter->q.des) {
						break;
					}
				}
			}
		}
		if (nextpos == inf) {
			ret = iter->first;
			break;
		}
		else if (nextpos > maxNextPos) {
			maxNextPos = nextpos;
			ret = iter->first;
		}
	}

	for (set<string>::iterator iter = Rvalue[ret].begin(); iter != Rvalue[ret].end(); iter++) {
		//��ret�ļĴ����б���ı���*iter�����Ƕ������ٴ洢��ret��
		Avalue[*iter].erase(ret);
		//���V�ĵ�ַ��������AVALUE[V]˵V��������R֮��������ط�������Ҫ���ɴ���ָ��
		if (Avalue[*iter].size() > 0) {
			//pass
		}
		//���V�����ڴ�֮��ʹ�ã�����Ҫ���ɴ���ָ��
		else {
			bool storeFlag = true;
			vector<QuaternionInfo>::iterator cIter;
			for (cIter = nowQuaternion; cIter != nowIBlock->codes.end(); cIter++) {
				if (cIter->q.src1 == *iter || cIter->q.src2 == *iter) {//���V�ڱ��������б�����
					storeFlag = true;
					break;
				}
				if (cIter->q.des == *iter) {//���V�ڱ��������б���ֵ
					storeFlag = false;
					break;
				}
			}
			if (cIter == nowIBlock->codes.end()) {//���V�ڱ���������δ�����ã���Ҳû�б���ֵ
				int index = int(nowIBlock - funcIBlocks[nowFunc].begin());
				if (funcOUTL[nowFunc][index].count(*iter) == 1) {//����˱����ǳ���֮��Ļ�Ծ����
					storeFlag = true;
				}
				else {
					storeFlag = false;
				}
			}
			if (storeFlag) {//���ɴ���ָ��
				storeVar(ret, *iter);
			}
		}
	}
	Rvalue[ret].clear();//���ret�Ĵ����б���ı���

	return ret;
}

//Ϊ���ñ�������Ĵ���
string ObjectCode::allocateReg(string var) {
	if (isNum(var)) {
		string ret = allocateReg();
		objectCodes.push_back(string("addi ") + ret + " $zero " + var);
		return ret;
	}

	for (set<string>::iterator iter = Avalue[var].begin(); iter != Avalue[var].end(); iter++) {
		if ((*iter)[0] == '$') {//��������Ѿ�������ĳ���Ĵ�����
			return *iter;//ֱ�ӷ��ظüĴ���
		}
	}

	//����ñ���û����ĳ���Ĵ�����
	string ret = allocateReg();
	objectCodes.push_back(string("lw ") + ret + " " + to_string(varOffset[var]) + "($sp)");
	Avalue[var].insert(ret);
	Rvalue[ret].insert(var);
	return ret;
}

//ΪĿ���������Ĵ���
string ObjectCode::getReg() {
	//i: A:=B op C
	//���B������ֵ��ĳ���Ĵ���Ri�У�RVALUE[Ri]��ֻ����B
	//���⣬����B��A��ͬһ����ʶ������B������ֵ��ִ����ԪʽA:=B op C֮�󲻻�������
	//��ѡȡRiΪ����Ҫ�ļĴ���R

	//���src1��������
	if (!isNum(nowQuaternion->q.src1)) {
		//����src1���ڵļĴ���
		set<string>&src1pos = Avalue[nowQuaternion->q.src1];
		for (set<string>::iterator iter = src1pos.begin(); iter != src1pos.end(); iter++) {
			if ((*iter)[0] == '$') {
				if (Rvalue[*iter].size() == 1) {//����üĴ�����ֵ��������src1
					if (nowQuaternion->q.des == nowQuaternion->q.src1 || !nowQuaternion->info1.active) {//���A,B��ͬһ��ʶ����B�Ժ󲻻�Ծ
						Avalue[nowQuaternion->q.des].insert(*iter);
						Rvalue[*iter].insert(nowQuaternion->q.des);
						return *iter;
					}
				}
			}
		}
	}

	//ΪĿ�����������ܲ���ȷ
	//return allocateReg(nowQuaternion->q.des);
	string ret = allocateReg();
	Avalue[nowQuaternion->q.des].insert(ret);
	Rvalue[ret].insert(nowQuaternion->q.des);
	return ret;
}

//��������
void ObjectCode::analyseBlock(map<string, vector<Block> >*funcBlocks) {
	for (map<string, vector<Block> >::iterator fbiter = funcBlocks->begin(); fbiter != funcBlocks->end(); fbiter++) {
		vector<InfoBlock> iBlocks;
		vector<Block>& blocks = fbiter->second;
		vector<set<string> >INL, OUTL, DEF, USE;

		//��Ծ����������������
		//ȷ��DEF��USE
		for (vector<Block>::iterator biter = blocks.begin(); biter != blocks.end(); biter++) {
			set<string>def, use;
			for (vector<Quaternion>::iterator citer = biter->codes.begin(); citer != biter->codes.end(); citer++) {
				if (citer->op == "j" || citer->op == "call") {
					//pass
				}
				else if (citer->op[0] == 'j') {//j>= j<=,j==,j!=,j>,j<
					if (isVar(citer->src1) && def.count(citer->src1) == 0) {//���Դ������1��û�б���ֵ
						use.insert(citer->src1);
					}
					if (isVar(citer->src2) && def.count(citer->src2) == 0) {//���Դ������2��û�б���ֵ
						use.insert(citer->src2);
					}
				}
				else {
					if (isVar(citer->src1) && def.count(citer->src1) == 0) {//���Դ������1��û�б���ֵ
						use.insert(citer->src1);
					}
					if (isVar(citer->src2) && def.count(citer->src2) == 0) {//���Դ������2��û�б���ֵ
						use.insert(citer->src2);
					}
					if (isVar(citer->des) && use.count(citer->des) == 0) {//���Ŀ�Ĳ�������û�б�����
						def.insert(citer->des);
					}
				}
			}
			INL.push_back(use);
			DEF.push_back(def);
			USE.push_back(use);
			OUTL.push_back(set<string>());
		}

		//ȷ��INL��OUTL
		bool change = true;
		while (change) {
			change = false;
			int blockIndex = 0;
			for (vector<Block>::iterator biter = blocks.begin(); biter != blocks.end(); biter++, blockIndex++) {
				int next1 = biter->next1;
				int next2 = biter->next2;
				if (next1 != -1) {
					for (set<string>::iterator inlIter = INL[next1].begin(); inlIter != INL[next1].end(); inlIter++) {
						if (OUTL[blockIndex].insert(*inlIter).second == true) {
							if (DEF[blockIndex].count(*inlIter) == 0) {
								INL[blockIndex].insert(*inlIter);
							}
							change = true;
						}
					}
				}
				if (next2 != -1) {
					for (set<string>::iterator inlIter = INL[next2].begin(); inlIter != INL[next2].end(); inlIter++) {
						if (OUTL[blockIndex].insert(*inlIter).second == true) {
							if (DEF[blockIndex].count(*inlIter) == 0) {
								INL[blockIndex].insert(*inlIter);
							}
							change = true;
						}
					}
				}
			}
		}
		funcOUTL[fbiter->first] = OUTL;
		funcINL[fbiter->first] = INL;

		for (vector<Block>::iterator iter = blocks.begin(); iter != blocks.end(); iter++) {
			InfoBlock iBlock;
			iBlock.next1 = iter->next1;
			iBlock.next2 = iter->next2;
			iBlock.name = iter->name;
			for (vector<Quaternion>::iterator qIter = iter->codes.begin(); qIter != iter->codes.end(); qIter++) {
				iBlock.codes.push_back(QuaternionInfo(*qIter, VarInfo(-1, false), VarInfo(-1, false), VarInfo(-1, false)));
			}
			iBlocks.push_back(iBlock);
		}

		vector<map<string, VarInfo> > symTables;//ÿ���������Ӧһ�ŷ��ű�
		//��ʼ�����ű�
		for (vector<Block>::iterator biter = blocks.begin(); biter != blocks.end(); biter++) {//����ÿһ��������
			map<string, VarInfo>symTable;
			for (vector<Quaternion>::iterator citer = biter->codes.begin(); citer != biter->codes.end(); citer++) {//�����������е�ÿ����Ԫʽ
				if (citer->op == "j" || citer->op == "call") {
					//pass
				}
				else if (citer->op[0] == 'j') {//j>= j<=,j==,j!=,j>,j<
					if (isVar(citer->src1)) {
						symTable[citer->src1] = VarInfo{ -1,false };
					}
					if (isVar(citer->src2)) {
						symTable[citer->src2] = VarInfo{ -1,false };
					}
				}
				else {
					if (isVar(citer->src1)) {
						symTable[citer->src1] = VarInfo{ -1,false };
					}
					if (isVar(citer->src2)) {
						symTable[citer->src2] = VarInfo{ -1,false };
					}
					if (isVar(citer->des)) {
						symTable[citer->des] = VarInfo{ -1,false };
					}
				}
			}
			symTables.push_back(symTable);
		}
		int blockIndex = 0;
		for (vector<set<string> >::iterator iter = OUTL.begin(); iter != OUTL.end(); iter++, blockIndex++) {//����ÿ��������Ļ�Ծ������
			for (set<string>::iterator viter = iter->begin(); viter != iter->end(); viter++) {//������Ծ�������еı���
				symTables[blockIndex][*viter] = VarInfo{ -1,true };
			}

		}

		blockIndex = 0;
		//����ÿ����Ԫʽ�Ĵ�����Ϣ�ͻ�Ծ��Ϣ
		for (vector<InfoBlock>::iterator ibiter = iBlocks.begin(); ibiter != iBlocks.end(); ibiter++, blockIndex++) {//����ÿһ��������
			int codeIndex = int(ibiter->codes.size() - 1);
			for (vector<QuaternionInfo>::reverse_iterator citer = ibiter->codes.rbegin(); citer != ibiter->codes.rend(); citer++, codeIndex--) {//��������������еĴ���
				if (citer->q.op == "j" || citer->q.op == "call") {
					//pass
				}
				else if (citer->q.op[0] == 'j') {//j>= j<=,j==,j!=,j>,j<
					if (isVar(citer->q.src1)) {
						citer->info1 = symTables[blockIndex][citer->q.src1];
						symTables[blockIndex][citer->q.src1] = VarInfo{ codeIndex,true };
					}
					if (isVar(citer->q.src2)) {
						citer->info2 = symTables[blockIndex][citer->q.src2];
						symTables[blockIndex][citer->q.src2] = VarInfo{ codeIndex,true };
					}
				}
				else {
					if (isVar(citer->q.src1)) {
						citer->info1 = symTables[blockIndex][citer->q.src1];
						symTables[blockIndex][citer->q.src1] = VarInfo{ codeIndex,true };
					}
					if (isVar(citer->q.src2)) {
						citer->info2 = symTables[blockIndex][citer->q.src2];
						symTables[blockIndex][citer->q.src2] = VarInfo{ codeIndex,true };
					}
					if (isVar(citer->q.des)) {
						citer->info3 = symTables[blockIndex][citer->q.des];
						symTables[blockIndex][citer->q.des] = VarInfo{ -1,false };
					}
				}
			}
		}

		funcIBlocks[fbiter->first] = iBlocks;
	}
}

//�����Ϣ��
void ObjectCode::outputIBlocks() {
	for (map<string, vector<InfoBlock> >::iterator iter = funcIBlocks.begin(); iter != funcIBlocks.end(); iter++) {
		outputString += "[" + iter->first + "]\n";
		for (vector<InfoBlock>::iterator bIter = iter->second.begin(); bIter != iter->second.end(); bIter++) {
			outputString += bIter->name + ":\n";
			for (vector<QuaternionInfo>::iterator cIter = bIter->codes.begin(); cIter != bIter->codes.end(); cIter++) {
				outputString += "    ";
				cIter->output();
				outputString += "\n";
			}
			outputString += "    next1 = " + to_string(bIter->next1) + "\n";
			outputString += "    next2 = " + to_string(bIter->next2) + "\n";
		}
		outputString += "\n";
	}
}

//���Ŀ�����
void ObjectCode::outputObjectCode(ostream& out) {
	for (vector<string>::iterator iter = objectCodes.begin(); iter != objectCodes.end(); iter++) {
		out << *iter << endl;
	}
}

//���Ŀ�����
void ObjectCode::outputObjectCode() {
	for (vector<string>::iterator iter = objectCodes.begin(); iter != objectCodes.end(); iter++) {
		outputString += *iter + "\n";
	}
}

//���Ŀ�����
void ObjectCode::outputObjectCode(const char* fileName) {
	ofstream fout;
	fout.open(fileName);
	if (!fout.is_open()) {
		cerr << "file " << fileName << " open error" << endl;
		return;
	}
	outputObjectCode(fout);

	fout.close();
}

//��������ڣ������ڻ�Ծ�����������ڴ�
void ObjectCode::storeOutLiveVar(set<string>&outl) {
	for (set<string>::iterator oiter = outl.begin(); oiter != outl.end(); oiter++) {
		string reg;//��Ծ�������ڵļĴ�������
		bool inFlag = false;//��Ծ�������ڴ��еı�־
		for (set<string>::iterator aiter = Avalue[*oiter].begin(); aiter != Avalue[*oiter].end(); aiter++) {
			if ((*aiter)[0] != '$') {//�û�Ծ�����Ѿ��洢���ڴ���
				inFlag = true;
				break;
			}
			else {
				reg = *aiter;
			}
		}
		if (!inFlag) {//����û�Ծ���������ڴ��У���reg�е�var���������ڴ�
			storeVar(reg, *oiter);
		}
	}
}

//Ϊ��Ԫʽ����Ŀ�����
void ObjectCode::generateCodeForQuaternion(int nowBaseBlockIndex, int &arg_num, int &par_num, list<pair<string, bool> > &par_list) {
	if (nowQuaternion->q.op[0] != 'j'&&nowQuaternion->q.op != "call") {
		if (isVar(nowQuaternion->q.src1) && Avalue[nowQuaternion->q.src1].empty()) {
			errorFlag = 1;
			errorString += string("����") + nowQuaternion->q.src1 + "������ǰδ��ֵ";
		}
		if (isVar(nowQuaternion->q.src2) && Avalue[nowQuaternion->q.src2].empty()) {
			errorFlag = 1;
			errorString += string("����") + nowQuaternion->q.src2 + "������ǰδ��ֵ";
		}
	}

	if (nowQuaternion->q.op == "j") {
		objectCodes.push_back(nowQuaternion->q.op + " " + nowQuaternion->q.des);
	}
	else if (nowQuaternion->q.op[0] == 'j') {//j>= j<=,j==,j!=,j>,j<
		string op;
		if (nowQuaternion->q.op == "j>=")
			op = "bge";
		else if (nowQuaternion->q.op == "j>")
			op = "bgt";
		else if (nowQuaternion->q.op == "j==")
			op = "beq";
		else if (nowQuaternion->q.op == "j!=")
			op = "bne";
		else if (nowQuaternion->q.op == "j<")
			op = "blt";
		else if (nowQuaternion->q.op == "j<=")
			op = "ble";
		string pos1 = allocateReg(nowQuaternion->q.src1);
		string pos2 = allocateReg(nowQuaternion->q.src2);
		objectCodes.push_back(op + " " + pos1 + " " + pos2 + " " + nowQuaternion->q.des);
		if (!nowQuaternion->info1.active) {
			releaseVar(nowQuaternion->q.src1);
		}
		if (!nowQuaternion->info2.active) {
			releaseVar(nowQuaternion->q.src2);
		}
	}
	else if (nowQuaternion->q.op == "par") {
		par_list.push_back(pair<string, bool>(nowQuaternion->q.src1, nowQuaternion->info1.active));
	}
	else if (nowQuaternion->q.op == "call") {
		/*������ѹջ*/
		for (list<pair<string, bool> >::iterator aiter = par_list.begin(); aiter != par_list.end(); aiter++) {
			string pos = allocateReg(aiter->first);
			objectCodes.push_back(string("sw ") + pos + " " + to_string(top + 4 * (++arg_num + 1)) + "($sp)");
			if (!aiter->second) {
				releaseVar(aiter->first);
			}
		}
		/*����$sp*/
		objectCodes.push_back(string("sw $sp ") + to_string(top) + "($sp)");
		objectCodes.push_back(string("addi $sp $sp ") + to_string(top));

		/*��ת����Ӧ����*/
		objectCodes.push_back(string("jal ") + nowQuaternion->q.src1);

		/*�ָ��ֳ�*/
		objectCodes.push_back(string("lw $sp 0($sp)"));
	}
	else if (nowQuaternion->q.op == "return") {
		if (isNum(nowQuaternion->q.src1)) {//����ֵΪ����
			objectCodes.push_back("addi $v0 $zero " + nowQuaternion->q.src1);
		}
		else if (isVar(nowQuaternion->q.src1)) {//����ֵΪ����
			set<string>::iterator piter = Avalue[nowQuaternion->q.src1].begin();
			if ((*piter)[0] == '$') {
				objectCodes.push_back(string("add $v0 $zero ") + *piter);
			}
			else {
				objectCodes.push_back(string("lw $v0 ") + to_string(varOffset[*piter]) + "($sp)");
			}
		}
		if (nowFunc == "main") {
			objectCodes.push_back("j end");
		}
		else {
			objectCodes.push_back("lw $ra 4($sp)");
			objectCodes.push_back("jr $ra");
		}
	}
	else if (nowQuaternion->q.op == "get") {
		//varOffset[nowQuaternion->q.src1] = 4 * (par_num++ + 2);
		varOffset[nowQuaternion->q.des] = top;
		top += 4;
		Avalue[nowQuaternion->q.des].insert(nowQuaternion->q.des);
	}
	else if (nowQuaternion->q.op == "=") {
		//Avalue[nowQuaternion->q.des] = set<string>();
		string src1Pos;
		if (nowQuaternion->q.src1 == "@RETURN_PLACE") {
			src1Pos = "$v0";
		}
		else {
			src1Pos = allocateReg(nowQuaternion->q.src1);
		}
		Rvalue[src1Pos].insert(nowQuaternion->q.des);
		Avalue[nowQuaternion->q.des].insert(src1Pos);
	}
	else {// + - * /
		string src1Pos = allocateReg(nowQuaternion->q.src1);
		string src2Pos = allocateReg(nowQuaternion->q.src2);
		string desPos = getReg();
		if (nowQuaternion->q.op == "+") {
			objectCodes.push_back(string("add ") + desPos + " " + src1Pos + " " + src2Pos);
		}
		else if (nowQuaternion->q.op == "-") {
			objectCodes.push_back(string("sub ") + desPos + " " + src1Pos + " " + src2Pos);
		}
		else if (nowQuaternion->q.op == "*") {
			objectCodes.push_back(string("mul ") + desPos + " " + src1Pos + " " + src2Pos);
		}
		else if (nowQuaternion->q.op == "/") {
			objectCodes.push_back(string("div ") + src1Pos + " " + src2Pos);
			objectCodes.push_back(string("mflo ") + desPos);
		}
		if (!nowQuaternion->info1.active) {
			releaseVar(nowQuaternion->q.src1);
		}
		if (!nowQuaternion->info2.active) {
			releaseVar(nowQuaternion->q.src2);
		}
	}
}

void ObjectCode::generateCodeForBaseBlocks(int nowBaseBlockIndex) {
	int arg_num = 0;//par��ʵ�θ���
	int par_num = 0;//get���βθ���
	list<pair<string, bool> > par_list;//���������õ���ʵ�μ�list<ʵ����,�Ƿ��Ծ>

	if (nowFunc == "program") {
		int a = 1;
	}

	Avalue.clear();
	Rvalue.clear();
	set<string>& inl = funcINL[nowFunc][nowBaseBlockIndex];
	for (set<string>::iterator iter = inl.begin(); iter != inl.end(); iter++) {
		Avalue[*iter].insert(*iter);
	}

	//��ʼ�����мĴ���
	freeReg.clear();
	for (int i = 0; i <= 7; i++) {
		freeReg.push_back(string("$s") + to_string(i));
	}

	objectCodes.push_back(nowIBlock->name + ":");
	if (nowBaseBlockIndex == 0) {
		if (nowFunc == "main") {
			top = 8;
		}
		else {
			objectCodes.push_back("sw $ra 4($sp)");//�ѷ��ص�ַѹջ
			top = 8;
		}
	}

	for (vector<QuaternionInfo>::iterator cIter = nowIBlock->codes.begin(); cIter != nowIBlock->codes.end(); cIter++) {//�Ի������ڵ�ÿһ�����
		nowQuaternion = cIter;
		//����ǻ���������һ�����
		if (cIter + 1 == nowIBlock->codes.end()) {
			//������һ������ǿ�����䣬���Ƚ����ڻ�Ծ�������棬�ٽ�����ת(j,call,return)
			if (isControl(cIter->q.op)) {
				storeOutLiveVar(funcOUTL[nowFunc][nowBaseBlockIndex]);
				generateCodeForQuaternion(nowBaseBlockIndex, arg_num, par_num, par_list);
			}
			//������һ����䲻�ǿ�����䣨�Ǹ�ֵ��䣩�����ȼ��㣬�ٽ����ڻ�Ծ��������
			else {
				generateCodeForQuaternion(nowBaseBlockIndex, arg_num, par_num, par_list);
				storeOutLiveVar(funcOUTL[nowFunc][nowBaseBlockIndex]);
			}
		}
		else {
			generateCodeForQuaternion(nowBaseBlockIndex, arg_num, par_num, par_list);
		}

	}
}

void ObjectCode::generateCodeForFuncBlocks(map<string, vector<InfoBlock> >::iterator &fiter) {
	varOffset.clear();
	nowFunc = fiter->first;
	vector<InfoBlock>&iBlocks = fiter->second;
	for (vector<InfoBlock>::iterator iter = iBlocks.begin(); iter != iBlocks.end(); iter++) {//��ÿһ��������
		nowIBlock = iter;
		generateCodeForBaseBlocks(int(nowIBlock - iBlocks.begin()));
	}
}

//����Ŀ�����
void ObjectCode::generateCode() {
	objectCodes.push_back("lui $sp,0x1001");
	objectCodes.push_back("j main");
	for (map<string, vector<InfoBlock> >::iterator fiter = funcIBlocks.begin(); fiter != funcIBlocks.end(); fiter++) {//��ÿһ��������
		generateCodeForFuncBlocks(fiter);
	}
	objectCodes.push_back("end:");
}