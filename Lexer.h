#pragma once
#include"Global.h"

class Lexer {
private:
	ifstream filein;//输入文件
	list<Token> tokenList;//token列表
	int lineCount;  //记录行数
	int posCount;   //记录列数
private:
	void openFile(const char* path);//打开文件
	char getNextChar();//获取下一个字符
	Token getNextToken();//获取下一个token
public:
	Lexer(const char* path);
	~Lexer();
	void analyse();//词法分析
	void output();//输出分词结果
	list<Token>&getResult();//获取token list
};
