#pragma once
#include "Global.h"
#include "IntermediateCode.h"

//符号
class Symbol {
public:
	bool isVt;//是否为终结符
	string content;//内容
	friend bool operator ==(const Symbol&one, const Symbol&other);
	friend bool operator < (const Symbol&one, const Symbol&other);
	Symbol();
	Symbol(const Symbol& sym);
	Symbol(const bool &isVt, const string& content);
};

//声明类型（变量声明/函数声明）
enum DecType {
	DEC_VAR,DEC_FUN
};

//数据类型（int/void）
enum DType { D_VOID, D_INT };

//变量
struct Var {
	string name;
	DType type;
	int level;
};

//函数
struct Func {
	string name;
	DType returnType;
	list<DType> paramTypes;
	int enterPoint;
};

//标识符
class Id :public Symbol {
public:
	string name;
	Id(const Symbol& sym, const string& name);
};

//数字
class Num :public Symbol {
public:
	string number;
	Num(const Symbol& sym,const string& number);
};

//函数声明
class FunctionDeclare :public Symbol {
public:
	list<DType>plist;
	FunctionDeclare(const Symbol& sym);
};

//参数
class Parameter :public Symbol {
public:
	list<DType>plist;
	Parameter(const Symbol& sym);
};

//参数列表
class ParameterList :public Symbol {
public:
	list<DType>plist;
	ParameterList(const Symbol& sym);
};

//语句块
class SentenceBlock :public Symbol {
public:
	list<int>nextList;
	SentenceBlock(const Symbol& sym);
};

//语句串
class SentenceList :public Symbol {
public:
	list<int>nextList;
	SentenceList(const Symbol& sym);
};

//语句
class Sentence :public Symbol {
public:
	list<int>nextList;
	Sentence(const Symbol& sym);
};

//While
class WhileSentence :public Symbol {
public:
	list<int>nextList;
	WhileSentence(const Symbol& sym);
};

//If
class IfSentence :public Symbol {
public:
	list<int>nextList;
	IfSentence(const Symbol& sym);
};

//表达式
class Expression :public Symbol {
public:
	string name;
	list<int>falseList;
	Expression(const Symbol& sym);
};

//M
class M :public Symbol {
public:
	int quad;
	M(const Symbol& sym);
};

//N
class N :public Symbol {
public:
	list<int> nextList;
	N(const Symbol& sym);
};

//加法表达式
class AddExpression :public Symbol {
public:
	string name;
	AddExpression(const Symbol& sym);
};

//项
class Term :public Symbol {
public:
	string name;
	Term(const Symbol& sym);
};

//因子
class Factor :public Symbol {
public:
	string name;
	Factor(const Symbol& sym);
};

//实参列表
class ArgumentList :public Symbol {
public:
	list<string> alist;
	ArgumentList(const Symbol& sym);
};

//产生式
struct Production {
	int id;//产生式的标识id，便于比较
	Symbol left;
	vector<Symbol>right;
};

//项目
struct Item {
	int pro;//产生式id
	int pointPos;//.的位置
	friend bool operator ==(const Item&one, const Item& other);
	friend bool operator <(const Item&one, const Item& other);
};

//DFA状态
struct I {
	set<Item> items;
};

//GOTO
typedef pair<int, Symbol> GOTO;

//DFA
struct DFA {
	list<I> stas;
	map<GOTO, int> goTo;
};

//Action规定的四种过程
//                   移进   规约    接受    错误
enum ParserAction { shift, reduce, success, error};
struct Action {
	ParserAction pa;
	int nextStat;
};

//临时变量
class NewTemper {
private:
	int count;
public:
	NewTemper();
	string newTemp();
};

//语法分析和语义分析
class GrammerAndSemantic {
private:
	int nowLevel;//当前分析行所在的语句块级次
	vector<Production>productions;//产生式
	DFA dfa;//DFA
	map<GOTO,Action> SLR1_Table;//SLR1表
	map<Symbol,set<Symbol> >first;//first集
	map<Symbol,set<Symbol> >follow;//follow集
	stack<Symbol*> symStack;//符号栈
	stack<int> staStack;//状态栈
	vector<Var> varTable;//变量表
	vector<Func> funcTable;//函数表
	IntermediateCode code;//生成的四元式
	NewTemper nt;//临时变量

	void loadGrammer(const char*filein);//读入文法
	void getFirst();//获取first集合
	void getFollow();//获取follow集合
	I closure(Item item);//构造项目集闭包
	void generateDFA(); //创建DFA
	void outputSymbolStack();//输出符号栈
	void outputStateStack();//输出状态栈
	Func* findFunc(string ID);//寻找函数
	Var* findVar(string ID);//寻找变量
	bool compareParaNum(list<string>&argument_list,list<DType>&parameter_list);//确认参数个数是否相同
	Symbol* popSymbol();//出符号栈
	void pushSymbol(int line, int pos, Symbol* sym);//入符号栈
public:
	GrammerAndSemantic(const char*fileName);//构造函数
	void outputDFA();//输出DFA
	void outputIntermediateCode();//输出中间代码
	void analyse(list<Token>&words);//语法和语义分析
	vector<pair<int, string> > getFuncEnter();//获取入口地址
	IntermediateCode* getIntermediateCode();//获取中间代码
};
