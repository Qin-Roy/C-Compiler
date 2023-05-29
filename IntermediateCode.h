#pragma once
#include "Global.h"

//�±�ǩ
class NewLabel {
private:
	int index;
public:
	NewLabel();
	string label2string(); //����ǩת��Ϊstring��ʽ
};

//�м����
class IntermediateCode {
private:
	vector<Quaternion> code; //�м����
	map<string, vector<Block> >funcBlocks;//�����
	NewLabel nl;//�±�ǩ

public:
	void pushCode(Quaternion q);//�����м����
	void pushCode(string op, string src1, string src2, string des);//�����м����
	void backFill(list<int>nextList,int quad);//����
	void output();//����м����
	void divideBlocks(vector<pair<int, string> > funcEnter);//���ֻ�����
	void outputBlocks();//���������
	int quadNum();//��ȡ�м�������
	map<string, vector<Block> >* getBlock();//��ȡ������
};