#pragma once
#include "Global.h"

//新标签
class NewLabel {
private:
	int index;
public:
	NewLabel();
	string label2string(); //将标签转换为string格式
};

//中间代码
class IntermediateCode {
private:
	vector<Quaternion> code; //中间代码
	map<string, vector<Block> >funcBlocks;//代码块
	NewLabel nl;//新标签

public:
	void pushCode(Quaternion q);//插入中间代码
	void pushCode(string op, string src1, string src2, string des);//插入中间代码
	void backFill(list<int>nextList,int quad);//回填
	void output();//输出中间代码
	void divideBlocks(vector<pair<int, string> > funcEnter);//划分基本块
	void outputBlocks();//输出基本块
	int quadNum();//获取中间代码个数
	map<string, vector<Block> >* getBlock();//获取基本块
};