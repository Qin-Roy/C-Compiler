#pragma once
#include "Global.h"

//待用/活跃信息
class VarInfo {
public:
	int next;//待用信息
	bool active;//活跃信息

	VarInfo(int next, bool active);
	VarInfo(const VarInfo&other);
	VarInfo();
	void output(ostream& out);
	void output();
};

//四元式待用/活跃信息表
class QuaternionInfo {
public:
	Quaternion q;//四元式
	VarInfo info1;//左值
	VarInfo info2;//左操作数
	VarInfo info3;//右操作数

	QuaternionInfo(Quaternion q, VarInfo info1, VarInfo info2, VarInfo info3);
	void output(ostream& out);
	void output();
};

//基本信息表块
struct InfoBlock {
	string name;//名称
	vector<QuaternionInfo> codes;//四元式
	int next1;//下一连接块
	int next2;//下一连接块
};

//保存临时常数 t0 t1寄存器
//保存函数的返回值 v0寄存器
class ObjectCode {
private:
	map<string,vector<InfoBlock> >funcIBlocks;
	map<string, set<string> >Avalue;//动态变量描述数组
	map<string, set<string> >Rvalue;//寄存器描述数组
	map<string, int>varOffset;//各变量的存储位置
	int top;//当前栈顶
	list<string>freeReg;//空闲的寄存器编号
	map<string, vector<set<string> > >funcOUTL;//各函数块中基本块的出口活跃变量集
	map<string, vector<set<string> > >funcINL;//各函数块中基本块的入口活跃变量集
	string nowFunc;//当前分析的函数
	vector<InfoBlock>::iterator nowIBlock;//当前分析的基本块
	vector<QuaternionInfo>::iterator nowQuaternion;//当前分析的四元式
	vector<string>objectCodes;//目标代码

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