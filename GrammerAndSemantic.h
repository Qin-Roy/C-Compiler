#pragma once
#include "Global.h"
#include "IntermediateCode.h"

//����
class Symbol {
public:
	bool isVt;//�Ƿ�Ϊ�ս��
	string content;//����
	friend bool operator ==(const Symbol&one, const Symbol&other);
	friend bool operator < (const Symbol&one, const Symbol&other);
	Symbol();
	Symbol(const Symbol& sym);
	Symbol(const bool &isVt, const string& content);
};

//�������ͣ���������/����������
enum DecType {
	DEC_VAR,DEC_FUN
};

//�������ͣ�int/void��
enum DType { D_VOID, D_INT };

//����
struct Var {
	string name;
	DType type;
	int level;
};

//����
struct Func {
	string name;
	DType returnType;
	list<DType> paramTypes;
	int enterPoint;
};

//��ʶ��
class Id :public Symbol {
public:
	string name;
	Id(const Symbol& sym, const string& name);
};

//����
class Num :public Symbol {
public:
	string number;
	Num(const Symbol& sym,const string& number);
};

//��������
class FunctionDeclare :public Symbol {
public:
	list<DType>plist;
	FunctionDeclare(const Symbol& sym);
};

//����
class Parameter :public Symbol {
public:
	list<DType>plist;
	Parameter(const Symbol& sym);
};

//�����б�
class ParameterList :public Symbol {
public:
	list<DType>plist;
	ParameterList(const Symbol& sym);
};

//����
class SentenceBlock :public Symbol {
public:
	list<int>nextList;
	SentenceBlock(const Symbol& sym);
};

//��䴮
class SentenceList :public Symbol {
public:
	list<int>nextList;
	SentenceList(const Symbol& sym);
};

//���
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

//���ʽ
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

//�ӷ����ʽ
class AddExpression :public Symbol {
public:
	string name;
	AddExpression(const Symbol& sym);
};

//��
class Term :public Symbol {
public:
	string name;
	Term(const Symbol& sym);
};

//����
class Factor :public Symbol {
public:
	string name;
	Factor(const Symbol& sym);
};

//ʵ���б�
class ArgumentList :public Symbol {
public:
	list<string> alist;
	ArgumentList(const Symbol& sym);
};

//����ʽ
struct Production {
	int id;//����ʽ�ı�ʶid�����ڱȽ�
	Symbol left;
	vector<Symbol>right;
};

//��Ŀ
struct Item {
	int pro;//����ʽid
	int pointPos;//.��λ��
	friend bool operator ==(const Item&one, const Item& other);
	friend bool operator <(const Item&one, const Item& other);
};

//DFA״̬
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

//Action�涨�����ֹ���
//                   �ƽ�   ��Լ    ����    ����
enum ParserAction { shift, reduce, success, error};
struct Action {
	ParserAction pa;
	int nextStat;
};

//��ʱ����
class NewTemper {
private:
	int count;
public:
	NewTemper();
	string newTemp();
};

//�﷨�������������
class GrammerAndSemantic {
private:
	int nowLevel;//��ǰ���������ڵ����鼶��
	vector<Production>productions;//����ʽ
	DFA dfa;//DFA
	map<GOTO,Action> SLR1_Table;//SLR1��
	map<Symbol,set<Symbol> >first;//first��
	map<Symbol,set<Symbol> >follow;//follow��
	stack<Symbol*> symStack;//����ջ
	stack<int> staStack;//״̬ջ
	vector<Var> varTable;//������
	vector<Func> funcTable;//������
	IntermediateCode code;//���ɵ���Ԫʽ
	NewTemper nt;//��ʱ����

	void loadGrammer(const char*filein);//�����ķ�
	void getFirst();//��ȡfirst����
	void getFollow();//��ȡfollow����
	I closure(Item item);//������Ŀ���հ�
	void generateDFA(); //����DFA
	void outputSymbolStack();//�������ջ
	void outputStateStack();//���״̬ջ
	Func* findFunc(string ID);//Ѱ�Һ���
	Var* findVar(string ID);//Ѱ�ұ���
	bool compareParaNum(list<string>&argument_list,list<DType>&parameter_list);//ȷ�ϲ��������Ƿ���ͬ
	Symbol* popSymbol();//������ջ
	void pushSymbol(int line, int pos, Symbol* sym);//�����ջ
public:
	GrammerAndSemantic(const char*fileName);//���캯��
	void outputDFA();//���DFA
	void outputIntermediateCode();//����м����
	void analyse(list<Token>&words);//�﷨���������
	vector<pair<int, string> > getFuncEnter();//��ȡ��ڵ�ַ
	IntermediateCode* getIntermediateCode();//��ȡ�м����
};
