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

typedef enum //token类型
{
	/* 结束符和错误 */
	_ENDFILE, _ERROR,
	/* 关键字 */
	_IF, _ELSE, _INT, _RETURN, _VOID, _WHILE,
	/* 标识符和数字 */
	ID, NUM,
	/* 算符 */ 
	/*  {        }     >=   <=   !=   ==      =     <     >     +      -      *      /      (         )       ;      ,   */
	_LBRACE, _RBRACE, _GE, _LE, _NE, _EQ, _ASSIGN, _LT, _GT, _PLUS, _MINUS, _MULT, _DIV, _LPAREN, _RPAREN, _SEMI, _COMMA,
	/* 行注释  段注释 */
	_LCOMMENT, _PCOMMENT,
	/* 换行符 */
	_NEXTLINE
} LexerType;

struct Token { //token
	LexerType lt;//token类型
	string content;//内容
	int line;//所在行
	int pos;//所在列
};

struct Quaternion {  //四元式
	string op;//操作符
	string src1;//源操作数1
	string src2;//源操作数2
	string des;//目的操作数
};

struct Block {  //基本块
	string name;//基本块的名称
	vector<Quaternion> codes;//基本块中的四元式
	int next1;//基本块的下一连接块
	int next2;//基本块的下一连接块
};