#pragma execution_character_set("utf-8")
#include "Lexer.h"

//输出信息
extern std::string outputString;
extern bool errorFlag;
extern std::string errorString;

// 对应token类型至string格式
const char* LexerToStr[] = {
	"ENDFILE", "ERROR",
	"IF", "ELSE", "INT", "RETURN", "VOID", "WHILE",
	"ID", "NUM",
	"LBRACE", "RBRACE", "GE", "LE", "NQ", "EQ", "ASSIGN", "LT", "GT", "PLUS", "MINUS", "MULT", "DIV", "LPAREN", "RPAREN", "SEMI", "COMMA",
};

// 将token转化为string格式
string token_to_string(Token t) {
	string res;
	res += "(" + to_string(t.line) + "," + to_string(t.pos) + ")   ";
	int len = 12 - res.length();
	while (len) {
		len--;
		res += " ";
	}
	res += LexerToStr[t.lt];
	len = 25 - res.length();
	while (len) {
		len--;
		res += " ";
	}
	res += t.content;
	return res;
}

//构造
Lexer::Lexer(const char* path) {
	lineCount = 1;
	posCount = 0;
	openFile(path);
}

//析构
Lexer::~Lexer() {
	if (filein.is_open()) {
		filein.close();
	}
}

//打开文件
void Lexer::openFile(const char* path) {
	filein.open(path, ios::in);
	if (!filein.is_open()) {
		errorFlag = 1;
		errorString += "文件" + string(path) + "打开失败";
	}
}

//获取下一个字符
char Lexer::getNextChar() {
	char c;
	while (filein.get(c)) {//获取一个字符
		if (c == ' '|| c=='\t') {//若为空白则跳过
			posCount++;
			continue;
		}
		else if (c == '\n') {//若为换行
			lineCount++;
			posCount = 0;
			return '\n';
		}
		posCount++;
		break;
	}
	if (filein.eof())//读到文件末尾
		return 0;
	else
		return c;
}

//获取下一个token
Token Lexer::getNextToken() {
	char c = getNextChar();
	Token t;
	switch (c) {
		case '\n':
			t.lt = _NEXTLINE;
			t.content = "\n";
			t.line = lineCount;
			t.pos = posCount;
			return t;
			break;
		case '(':
			t.lt = _LPAREN;
			t.content = "(";
			t.line = lineCount;
			t.pos = posCount;
			return t;
			break;
		case ')':
			t.lt = _RPAREN;
			t.content = ")";
			t.line = lineCount;
			t.pos = posCount;
			return t;
			break;
		case '{':
			t.lt = _LBRACE;
			t.content = "{";
			t.line = lineCount;
			t.pos = posCount;
			return t;
			break;
		case '}':
			t.lt = _RBRACE;
			t.content = "}";
			t.line = lineCount;
			t.pos = posCount;
			return t;
			break;
		case '#':
			t.lt = _ENDFILE;
			t.content = "#";
			t.line = lineCount;
			t.pos = posCount;
			return t;
			break;
		case 0:
			t.lt = _ENDFILE;
			t.content = "#";
			t.line = lineCount;
			t.pos = posCount;
			return t;
			break;
		case '+':
			t.lt = _PLUS;
			t.content = "+";
			t.line = lineCount;
			t.pos = posCount;
			return t;
			break;
		case '-':
			t.lt = _MINUS;
			t.content = "-";
			t.line = lineCount;
			t.pos = posCount;
			return t;
			break;
		case '*':
			t.lt = _MULT;
			t.content = "*";
			t.line = lineCount;
			t.pos = posCount;
			return t;
			break;
		case ',':
			t.lt = _COMMA;
			t.content = ",";
			t.line = lineCount;
			t.pos = posCount;
			return t;
			break;
		case ';':
			t.lt = _SEMI;
			t.content = ";";
			t.line = lineCount;
			t.pos = posCount;
			return t;
			break;
		case '=':
			if (filein.peek() == '=') {
				filein.get();
				t.lt = _EQ;
				t.content = "==";
				t.line = lineCount;
				t.pos = posCount;
				posCount++;
				return t;
			}
			else {
				t.lt = _ASSIGN;
				t.content = "=";
				t.line = lineCount;
				t.pos = posCount;
				return t;
			}
			break;
		case '>':
			if (filein.peek() == '=') {
				filein.get();
				t.lt = _GE;
				t.content = ">=";
				t.line = lineCount;
				t.pos = posCount;
				posCount++;
				return t;
			}
			else {
				t.lt = _GT;
				t.content = ">";
				t.line = lineCount;
				t.pos = posCount;
				return t;
			}
			break;
		case '<':
			if (filein.peek() == '=') {
				filein.get();
				t.lt = _LE;
				t.content = "<=";
				t.line = lineCount;
				t.pos = posCount;
				posCount++;
				return t;
			}
			else {
				t.lt = _LT;
				t.content = "<";
				t.line = lineCount;
				t.pos = posCount;
				return t;
			}
			break;
		case '!':
			if (filein.peek() == '=') {
				filein.get();
				t.lt = _NE;
				t.content = "!=";
				t.line = lineCount;
				t.pos = posCount;
				posCount++;
				return t;
			}
			else {
				t.lt = _ERROR;
				t.content = string("词法分析错误(") + to_string(lineCount) + string(",") + to_string(posCount) + string(")：未识别的符号!");
				t.line = lineCount;
				t.pos = posCount;
				return t;
			}
			break;
		case '/':
			//行注释
			if (filein.peek() == '/') {
				char buf[1024];
				filein.getline(buf, 1024);
				t.lt = _LCOMMENT;
				t.content = string("/") + buf;
				t.line = lineCount;
				t.pos = posCount;
				lineCount++;
				posCount = 0;
				return t;
			}
			//段注释
			else if (filein.peek() == '*') {
				int line1 = lineCount;
				int pos1 = posCount;
				filein.get();
				posCount++;
				string buf = "/*";
				while (filein.get(c)) {
					if (c == '\n') {
						lineCount++;
						posCount = 0;
					}
					else
						posCount++;
					buf += c;
					if (c == '*') {
						filein.get(c);
						buf += c;
						if (c == '\n') {
							lineCount++;
							posCount = 0;
						}
						else
							posCount++;
						if (c == '/') {
							t.lt = _PCOMMENT;
							t.content = buf;
							t.line = line1;
							t.pos = pos1;
							return t;
							break;
						}
					}
				}
				//读到最后都没找到*/，因不满足while循环条件退出
				if (filein.eof()) {
					t.lt = _ERROR;
					t.content = string("词法分析错误(") + to_string(lineCount) + string(",") + to_string(posCount) + string(")：段注释没有匹配的*/");
					t.line = line1;
					t.pos = pos1;
					return t;
				}
			}
			//除法
			else {
				t.lt = _DIV;
				t.content = "/";
				t.line = lineCount;
				t.pos = posCount;
				return t;
			}
			break;
		default:
			if (isdigit(c)) {
				int line1 = lineCount;
				int pos1 = posCount;
				string buf;
				buf.push_back(c);
				while (c = filein.peek()) {
					if (isdigit(c)) {
						filein.get(c);
						posCount++;
						buf += c;
					}
					else {
						break;
					}
				}
				t.lt = NUM;
				t.content = buf;
				t.line = line1;
				t.pos = pos1;
				return t;
			}
			else if (isalpha(c)) {
				int line1 = lineCount;
				int pos1 = posCount;
				string buf;
				buf.push_back(c);
				while (c = filein.peek()) {
					if (isdigit(c)||isalpha(c)) {
						filein.get(c);
						posCount++;
						buf += c;
					}
					else {
						break;
					}
				}
				if (buf == "int") {
					t.lt = _INT;
					t.content = "int";
					t.line = line1;
					t.pos = pos1;
					return t;
				}
				else if (buf == "void") {
					t.lt = _VOID;
					t.content = "void";
					t.line = line1;
					t.pos = pos1;
					return t;
				}
				else if (buf == "if") {
					t.lt = _IF;
					t.content = "if";
					t.line = line1;
					t.pos = pos1;
					return t;
				}
				else if (buf == "else") {
					t.lt = _ELSE;
					t.content = "else";
					t.line = line1;
					t.pos = pos1;
					return t;
				}
				else if (buf == "while") {
					t.lt = _WHILE;
					t.content = "while";
					t.line = line1;
					t.pos = pos1;
					return t;
				}
				else if (buf == "return") {
					t.lt = _RETURN;
					t.content = "return";
					t.line = line1;
					t.pos = pos1;
					return t;
				}
				else {
					t.lt = ID;
					t.content = buf;
					t.line = line1;
					t.pos = pos1;
					return t;
				}
			}
			else {
				t.lt = _ERROR;
				t.content = string("词法分析(") + to_string(lineCount) + string(",") + to_string(posCount) + string(")：未识别的符号") + c;
				t.line = lineCount;
				t.pos = posCount;
				return t;
			}
	}
	t.lt = _ERROR;
	t.content = "UNKOWN ERROR";
	t.line = lineCount;
	t.pos = posCount;
	return t;
}

//词法分析
void Lexer::analyse() {
	while (1) {
		Token t = getNextToken();//获取下一个token
		tokenList.push_back(t);//加入tokenlist
		if (t.lt == _ERROR) {//错误
			errorFlag = 1;
			errorString += t.content;
		}
		else if (t.lt == _ENDFILE) {//结尾
			break;
		}
	}
}

//输出分词结果
void Lexer::output() {
	if (tokenList.back().lt == _ERROR) {
		outputString += token_to_string(tokenList.back()) + "\n";
	}
	else {
		list<Token>::iterator iter;
		for (iter = tokenList.begin(); iter != tokenList.end(); iter++) {
			if (iter->lt != _NEXTLINE && iter->lt != _LCOMMENT && iter->lt != _PCOMMENT) {
				outputString += token_to_string(*iter) + "\n";
			}
		}
	}
}

//获取token list
list<Token>&Lexer::getResult() {
	return tokenList;
}