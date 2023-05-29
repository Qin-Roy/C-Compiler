#pragma once
#include "Global.h"

//����/��Ծ��Ϣ
class VarInfo {
public:
	int next;//������Ϣ
	bool active;//��Ծ��Ϣ

	VarInfo(int next, bool active);
	VarInfo(const VarInfo&other);
	VarInfo();
	void output(ostream& out);
	void output();
};

//��Ԫʽ����/��Ծ��Ϣ��
class QuaternionInfo {
public:
	Quaternion q;//��Ԫʽ
	VarInfo info1;//��ֵ
	VarInfo info2;//�������
	VarInfo info3;//�Ҳ�����

	QuaternionInfo(Quaternion q, VarInfo info1, VarInfo info2, VarInfo info3);
	void output(ostream& out);
	void output();
};

//������Ϣ���
struct InfoBlock {
	string name;//����
	vector<QuaternionInfo> codes;//��Ԫʽ
	int next1;//��һ���ӿ�
	int next2;//��һ���ӿ�
};

//������ʱ���� t0 t1�Ĵ���
//���溯���ķ���ֵ v0�Ĵ���
class ObjectCode {
private:
	map<string,vector<InfoBlock> >funcIBlocks;
	map<string, set<string> >Avalue;//��̬������������
	map<string, set<string> >Rvalue;//�Ĵ�����������
	map<string, int>varOffset;//�������Ĵ洢λ��
	int top;//��ǰջ��
	list<string>freeReg;//���еļĴ������
	map<string, vector<set<string> > >funcOUTL;//���������л�����ĳ��ڻ�Ծ������
	map<string, vector<set<string> > >funcINL;//���������л��������ڻ�Ծ������
	string nowFunc;//��ǰ�����ĺ���
	vector<InfoBlock>::iterator nowIBlock;//��ǰ�����Ļ�����
	vector<QuaternionInfo>::iterator nowQuaternion;//��ǰ��������Ԫʽ
	vector<string>objectCodes;//Ŀ�����

	void outputObjectCode(ostream& out);
	void storeVar(string reg, string var);
	void storeOutLiveVar(set<string>&outl);
	void releaseVar(string var);
	string getReg();
	string allocateReg();
	string allocateReg(string var);

	void generateCodeForFuncBlocks(map<string, vector<InfoBlock> >::iterator &fiter);
	void generateCodeForBaseBlocks(int nowBaseBlockIndex);
	void generateCodeForQuaternion(int nowBaseBlockIndex, int &arg_num, int &par_num, list<pair<string, bool> > &par_list);
public:
	ObjectCode();
	void generateCode();
	void analyseBlock(map<string, vector<Block> >*funcBlocks);
	void outputIBlocks();
	void outputObjectCode();
	void outputObjectCode(const char* fileName);
};