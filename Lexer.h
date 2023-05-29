#pragma once
#include"Global.h"

class Lexer {
private:
	ifstream filein;//�����ļ�
	list<Token> tokenList;//token�б�
	int lineCount;  //��¼����
	int posCount;   //��¼����
private:
	void openFile(const char* path);//���ļ�
	char getNextChar();//��ȡ��һ���ַ�
	Token getNextToken();//��ȡ��һ��token
public:
	Lexer(const char* path);
	~Lexer();
	void analyse();//�ʷ�����
	void output();//����ִʽ��
	list<Token>&getResult();//��ȡtoken list
};
