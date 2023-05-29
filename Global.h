#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <utility>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <queue>
#include <cstdlib>
#include <cassert>
#include <functional>
using namespace std;

typedef enum //token����
{
	/* �������ʹ��� */
	_ENDFILE, _ERROR,
	/* �ؼ��� */
	_IF, _ELSE, _INT, _RETURN, _VOID, _WHILE,
	/* ��ʶ�������� */
	ID, NUM,
	/* ��� */ 
	/*  {        }     >=   <=   !=   ==      =     <     >     +      -      *      /      (         )       ;      ,   */
	_LBRACE, _RBRACE, _GE, _LE, _NE, _EQ, _ASSIGN, _LT, _GT, _PLUS, _MINUS, _MULT, _DIV, _LPAREN, _RPAREN, _SEMI, _COMMA,
	/* ��ע��  ��ע�� */
	_LCOMMENT, _PCOMMENT,
	/* ���з� */
	_NEXTLINE
} LexerType;

struct Token { //token
	LexerType lt;//token����
	string content;//����
	int line;//������
	int pos;//������
};

struct Quaternion {  //��Ԫʽ
	string op;//������
	string src1;//Դ������1
	string src2;//Դ������2
	string des;//Ŀ�Ĳ�����
};

struct Block {  //������
	string name;//�����������
	vector<Quaternion> codes;//�������е���Ԫʽ
	int next1;//���������һ���ӿ�
	int next2;//���������һ���ӿ�
};