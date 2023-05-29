#pragma execution_character_set("utf-8")
#include "GrammerAndSemantic.h"


//输出信息
extern std::string outputString;
extern bool errorFlag;
extern std::string errorString;


bool operator ==(const Symbol&one, const Symbol&other) {
	return one.content == other.content;
}
bool operator <(const Symbol&one, const Symbol&other) {
	return one.content < other.content;
}
bool operator < (const Item&one, const Item& other) {
	return pair<int, int>(one.pro, one.pointPos) < pair<int, int>(other.pro, other.pointPos);
}
bool operator ==(const Item&one, const Item& other) {
	return one.pro == other.pro&&one.pointPos == other.pointPos;
}

//合并两个list
list<int>merge(list<int>& l1, list<int>& l2) {
	list<int>ret;
	ret.assign(l1.begin(), l1.end());
	ret.splice(ret.end(), l2);
	return ret;
}

//构造函数
Symbol::Symbol() {
	this->isVt = 0;
	this->content = "";
}

Symbol::Symbol(const Symbol& sym) {
	this->isVt = sym.isVt;
	this->content = sym.content;
}

Symbol::Symbol(const bool &isVt, const string& content) {
	this->isVt = isVt;
	this->content = content;
}

NewTemper::NewTemper() {
	count = 0;
}

string NewTemper::newTemp() {
	return string("T") + to_string(count++);
}

Id::Id(const Symbol& sym, const string& name) : Symbol(sym) {
	this->name = name;
}

Num::Num(const Symbol& sym, const string& number) : Symbol(sym) {
	this->number = number;
}

FunctionDeclare::FunctionDeclare(const Symbol& sym) : Symbol(sym) {}

Parameter::Parameter(const Symbol& sym) : Symbol(sym) {}

ParameterList::ParameterList(const Symbol& sym) : Symbol(sym) {}

SentenceBlock::SentenceBlock(const Symbol& sym) : Symbol(sym) {}

SentenceList::SentenceList(const Symbol& sym) : Symbol(sym) {}

Sentence::Sentence(const Symbol& sym) : Symbol(sym) {}

WhileSentence::WhileSentence(const Symbol& sym) : Symbol(sym) {}

IfSentence::IfSentence(const Symbol& sym) : Symbol(sym) {}

Expression::Expression(const Symbol& sym) : Symbol(sym) {}

M::M(const Symbol& sym) : Symbol(sym) {}

N::N(const Symbol& sym) : Symbol(sym) {}

AddExpression::AddExpression(const Symbol& sym) : Symbol(sym) {}

Term::Term(const Symbol& sym) : Symbol(sym) {}

Factor::Factor(const Symbol& sym) : Symbol(sym) {}

ArgumentList::ArgumentList(const Symbol& sym) : Symbol(sym) {}

bool isVT(string s) {
	if (s == "int" || s == "void" || s == "if" || s == "while" || s == "else" || s == "return") {
		return true;
	}
	if (s == "+" || s == "-" || s == "*" || s == "/" || s == "=" || s == "==" || s == ">" || s == "<" || s == "!=" || s == ">=" || s == "<=") {
		return true;
	}
	if (s == ";" || s == "," || s == "(" || s == ")" || s == "{" || s == "}" || s == "ID" || s == "NUM") {
		return true;
	}
	return false;
}

//GrammerAndSemantic构造函数
GrammerAndSemantic::GrammerAndSemantic(const char*filein) {
	loadGrammer(filein);
	getFirst();
	getFollow();
	generateDFA();
}

//读入文法
void GrammerAndSemantic::loadGrammer(const char* fileName) {
	ifstream fin;

	//文件打开处理
	fin.open(fileName, ios::in);
	if (!fin.is_open()) {
		errorFlag = 1;
		errorString += "文件" + string(fileName) + "打开失败" + "\n";
		return;
	}

	int index = 0;//产生式序号
	char buf[1024];
	while (fin >> buf) {
		Production p;
		//产生式序号赋值
		p.id = index++;

		//产生式左部赋值
		p.left = Symbol{ false,string(buf) };

		//中间应为::=
		fin >> buf;
		assert(strcmp(buf, "::=") == 0);

		//产生式右部赋值
		fin.getline(buf, 1024);
		stringstream sstream(buf);
		string temp;
		while (sstream >> temp) {
			p.right.push_back(Symbol{ isVT(temp),string(temp) });
		}

		//插入产生式
		productions.push_back(p);
	}
}

//获取first集合
void GrammerAndSemantic::getFirst() {
	bool changeFlag = true;
	while (changeFlag) {
		changeFlag = false;//first集改变标志
		//遍历每一个产生式
		for (vector<Production>::iterator iter = productions.begin(); iter != productions.end(); iter++) {
			vector<Symbol>::iterator symIter;
			//依次遍历产生式右部的所有符号
			for (symIter = iter->right.begin(); symIter != iter->right.end(); symIter++) {
				//这个右部符号是终结符
				if (symIter->isVt) {
					if (first.count(iter->left) == 0) {
						first[iter->left] = set<Symbol>();
					}
					//左部符号的first集不包含该右部符号
					if (first[iter->left].insert(*symIter).second == true) {
						changeFlag = true;
					}
					break;
				}
				//当前右部符号是非终结符
				else {
					bool continueFlag = false;//是否继续读取下一个右部符号的first集
					set<Symbol>::iterator firstIter;
					//遍历该右部符号的first集
					for (firstIter = first[*symIter].begin(); firstIter != first[*symIter].end(); firstIter++) {
						//右部符号的first集中的元素包含EMPTY
						if (firstIter->content == "EMPTY") {
							continueFlag = true;
						}
						//右部符号的first集中的元素不在左部符号first集中
						else if (first[iter->left].find(*firstIter) == first[iter->left].end()) {
							if (first.count(iter->left) == 0) {
								first[iter->left] = set<Symbol>();
							}
							first[iter->left].insert(*firstIter);
							changeFlag = true;
						}
					}
					if (!continueFlag) {
						break;
					}
				}
			}
			//遍历右部符号到了末尾,则EMPTY在其first集中
			if (symIter == iter->right.end()) {
				if (first.count(iter->left) == 0) {
					first[iter->left] = set<Symbol>();
				}
				if (first[iter->left].insert(Symbol{ true,"EMPTY" }).second == true) {
					changeFlag = true;
				}
			}
		}

	}
}

//获取follow集合
void GrammerAndSemantic::getFollow() {
	//将#放入起始符号的FOLLOW集中
	follow[productions[0].left] = set<Symbol>();
	follow[productions[0].left].insert(Symbol{ true,"#" });
	bool changeFlag = true;
	while (changeFlag) {
		changeFlag = false;
		//遍历每一个产生式
		for (vector<Production>::iterator proIter = productions.begin(); proIter != productions.end(); proIter++) {
			//遍历产生式右部的每个符号
			for (vector<Symbol>::iterator symIter = proIter->right.begin(); symIter != proIter->right.end(); symIter++) {
				//遍历产生式右部该符号之后的符号
				vector<Symbol>::iterator nextSymIter;
				for (nextSymIter = symIter + 1; nextSymIter != proIter->right.end(); nextSymIter++) {
					Symbol nextSym = *nextSymIter;
					bool nextFlag = false;
					//如果之后的符号是终结符
					if (nextSym.isVt) {
						if (follow.count(*symIter) == 0) {
							follow[*symIter] = set<Symbol>();
						}
						//如果成功插入新值
						if (follow[*symIter].insert(nextSym).second == true) {
							changeFlag = true;
						}
					}
					else {
						//遍历之后符号的first集
						for (set<Symbol>::iterator fIter = first[nextSym].begin(); fIter != first[nextSym].end(); fIter++) {
							//如果当前符号first集中有 空串
							if (fIter->content == "EMPTY") {
								nextFlag = true;
							}
							else {
								if (follow.count(*symIter) == 0) {
									follow[*symIter] = set<Symbol>();
								}
								//如果成功插入新值
								if (follow[*symIter].insert(*fIter).second == true) {
									changeFlag = true;
								}
							}
						}
					}
					//如果当前符号first集中没有 空串
					if (!nextFlag) {
						break;
					}

				}
				//如果遍历到了结尾,将左部符号的FOLLOW集加入其FOLLOW集中
				if (nextSymIter == proIter->right.end()) {
					//遍历左部符号的FOLLOW集
					for (set<Symbol>::iterator followIter = follow[proIter->left].begin(); followIter != follow[proIter->left].end(); followIter++) {
						if (follow.count(*symIter) == 0) {
							follow[*symIter] = set<Symbol>();
						}
						//如果该FOLLOW集是新值
						if (follow[*symIter].insert(*followIter).second == true) {
							changeFlag = true;
						}
					}
				}
			}
		}
	}
}

//构造项目集闭包
I GrammerAndSemantic::closure(Item item) {
	I i;
	// .在项目产生式的最右边，即是一个规约项目
	if (productions[item.pro].right.size() == item.pointPos) {
		i.items.insert(item);
	}
	// .的右边是终结符
	else if (productions[item.pro].right[item.pointPos].isVt) {
		i.items.insert(item);
	}
	// .的右边是非终结符
	else {
		i.items.insert(item);
		vector<Production>::iterator iter;
		for (iter = productions.begin(); iter < productions.end(); iter++) {
			//产生式的左部 == .右边的非终结符
			if (iter->left == productions[item.pro].right[item.pointPos]) {
				//将产生式的派生加入I中
				I temp = closure(Item{ static_cast<int>(iter - productions.begin()),0 });

				set<Item>::iterator siter;
				for (siter = temp.items.begin(); siter != temp.items.end(); siter++) {
					i.items.insert(*siter);
				}
			}
		}
	}

	return i;
}

//创建DFA
void GrammerAndSemantic::generateDFA() {
	bool newFlag = true;//有新的状态产生标志
	int nowI = 0;//当前状态的编号
	dfa.stas.push_back(closure(Item{ 0,0 }));
	//遍历每一个状态
	for (list<I>::iterator iter = dfa.stas.begin(); iter != dfa.stas.end(); iter++, nowI++) {
		//遍历状态的每一个项目
		for (set<Item>::iterator itIter = iter->items.begin(); itIter != iter->items.end(); itIter++) {
			// .在项目产生式的最右边，即是一个规约项目
			if (productions[itIter->pro].right.size() == itIter->pointPos) {
				set<Symbol>FOLLOW = follow[productions[itIter->pro].left];
				for (set<Symbol>::iterator followIter = FOLLOW.begin(); followIter != FOLLOW.end(); followIter++) {
					if (SLR1_Table.count(GOTO(nowI, *followIter)) == 1) {
						string err = "文法不是SLR1文法，移进规约冲突";
						errorFlag = 1;
						errorString += err + "\n";
						return;
					}
					if (itIter->pro == 0) {
						SLR1_Table[GOTO(nowI, *followIter)] = Action{ success,itIter->pro };
					}
					else {
						SLR1_Table[GOTO(nowI, *followIter)] = Action{ reduce,itIter->pro };
					}

				}
				continue;
			}
			Symbol nextSymbol = productions[itIter->pro].right[itIter->pointPos];//.之后的符号

			//DFA中GOTO(nowI,nextSymbol)已经存在
			if (dfa.goTo.count(GOTO(nowI, nextSymbol)) == 1) {
				continue;
			}

			I newI = closure(Item{ itIter->pro,itIter->pointPos + 1 });//新产生的状态

			//查找当前状态中其他GOTO[nowI,nextSymbol]
			//shiftIter指向当前状态项目的下一个项目
			set<Item>::iterator shiftIter = itIter;
			shiftIter++;
			for (; shiftIter != iter->items.end(); shiftIter++) {
				//如果是规约项目
				if (productions[shiftIter->pro].right.size() == shiftIter->pointPos) {
					continue;
				}
				//如果是移进项目，且移进nextSymbol
				else if (productions[shiftIter->pro].right[shiftIter->pointPos] == nextSymbol) {
					I tempI = closure(Item{ shiftIter->pro,shiftIter->pointPos + 1 });
					newI.items.insert(tempI.items.begin(), tempI.items.end());
				}
			}
			//查找已有状态中是否已经包含该状态
			bool searchFlag = false;
			int index = 0;//当前状态的编号
			for (list<I>::iterator iterHave = dfa.stas.begin(); iterHave != dfa.stas.end(); iterHave++, index++) {
				if (iterHave->items == newI.items) {
					dfa.goTo[GOTO(nowI, nextSymbol)] = index;
					if (SLR1_Table.count(GOTO(nowI, nextSymbol)) == 1) {
						errorFlag = 1;
						errorString += "confict\n";
						return;
					}
					SLR1_Table[GOTO(nowI, nextSymbol)] = Action{ shift,index };
					searchFlag = true;
					break;
				}
			}
			//没有在已有状态中找到该状态
			if (!searchFlag) {
				dfa.stas.push_back(newI);
				dfa.goTo[GOTO(nowI, nextSymbol)] = int(dfa.stas.size() - 1);
				if (SLR1_Table.count(GOTO(nowI, nextSymbol)) == 1) {
					errorFlag = 1;
					errorString += "confict\n";
					return;
				}
				SLR1_Table[GOTO(nowI, nextSymbol)] = Action{ shift,int(dfa.stas.size() - 1) };
			}
			else {
				continue;
			}

		}
	}
}

//输出DFA
void GrammerAndSemantic::outputDFA() {
	int nowI = 0;
	for (list<I>::iterator iter = dfa.stas.begin(); iter != dfa.stas.end(); iter++, nowI++) {
		outputString += "I" + to_string(nowI) + "= [\n";
		for (set<Item>::iterator itIter = iter->items.begin(); itIter != iter->items.end(); itIter++) {
			Production p = productions[itIter->pro];
			outputString += "      ";
			outputString += p.left.content + " -> ";
			for (vector<Symbol>::iterator symIter = p.right.begin(); symIter != p.right.end(); symIter++) {
				if (symIter - p.right.begin() == itIter->pointPos) {
					outputString += ". ";
				}
				outputString += symIter->content + " ";
			}
			if (p.right.size() == itIter->pointPos) {
				outputString += ". ";
			}
			outputString += "\n";
		}
		outputString += "]\n\n";
	}
}

//寻找函数
Func* GrammerAndSemantic::findFunc(string ID) {
	for (vector<Func>::iterator iter = funcTable.begin(); iter != funcTable.end(); iter++) {
		if (iter->name == ID) {
			return &(*iter);
		}
	}
	return NULL;
}

//寻找变量
Var* GrammerAndSemantic::findVar(string ID) {
	for (vector<Var>::reverse_iterator iter = varTable.rbegin(); iter != varTable.rend(); iter++) {
		if (iter->name == ID) {
			return &(*iter);
		}
	}
	return NULL;
}

//确认参数个数是否相同
bool GrammerAndSemantic::compareParaNum(list<string>&argument_list, list<DType>&parameter_list) {
	if (argument_list.size() != parameter_list.size()) {
		return false;
	}
	else {
		return true;
	}
}

//输出符号栈
void GrammerAndSemantic::outputSymbolStack() {
	stack<Symbol*>temp = symStack;
	stack<Symbol*>other;
	while (!temp.empty()) {
		other.push(temp.top());
		temp.pop();
	}
	while (!other.empty()) {
		outputString += other.top()->content + " ";
		other.pop();
	}
	outputString += "\n";
}

//输出状态栈
void GrammerAndSemantic::outputStateStack() {
	stack<int>temp = staStack;
	stack<int>other;
	while (!temp.empty()) {
		other.push(temp.top());
		temp.pop();
	}
	while (!other.empty()) {
		outputString += to_string(other.top()) + " ";
		other.pop();
	}
	outputString += "\n";
}

//输出中间代码
void GrammerAndSemantic::outputIntermediateCode() {
	code.output();
}

//出符号栈
Symbol* GrammerAndSemantic::popSymbol() {
	Symbol* ret = symStack.top();
	symStack.pop();
	staStack.pop();
	return ret;
}

//入符号栈
void GrammerAndSemantic::pushSymbol(int line, int pos, Symbol* sym) {
	symStack.push(sym);
	if (SLR1_Table.count(GOTO(staStack.top(), *sym)) == 0) {
		errorFlag = 1;
		errorString += string("语法分析错误(") + to_string(line) + string(",") + to_string(pos - 1) + ")：不期待的符号 " + sym->content + "\n";
		return;
	}
	Action act = SLR1_Table[GOTO(staStack.top(), *sym)];
	staStack.push(act.nextStat);
}

//语法和语义分析
void GrammerAndSemantic::analyse(list<Token>&words) {
	bool acc = false;
	symStack.push(new Symbol(true, "#"));
	staStack.push(0);
	for (list<Token>::iterator iter = words.begin(); iter != words.end(); ) {
		outputSymbolStack();
		outputStateStack();
		LexerType lt = iter->lt;
		string word = iter->content;
		int line = iter->line;
		int pos = iter->pos;

		//忽略注释和换行
		if (lt == _LCOMMENT || lt == _PCOMMENT || lt == _NEXTLINE) {
			iter++;
			continue;
		}

		Symbol* nextSymbol;
		if (lt == ID) {
			nextSymbol = new Id(Symbol{ true,"ID" }, word);
		}
		else if (lt == NUM) {
			nextSymbol = new Num(Symbol{ true,"NUM" }, word);
		}
		else {
			nextSymbol = new Symbol(true, word);
		}
		if (SLR1_Table.count(GOTO(staStack.top(), *nextSymbol)) == 0) {
			errorFlag = 1;
			errorString += string("语法分析错误(") + to_string(line) + string(",") + to_string(pos - 1) + ")：不期待的符号 " + nextSymbol->content + "\n";
			return;
		}

		Action act = SLR1_Table[GOTO(staStack.top(), *nextSymbol)];
		if (act.pa == shift) {
			symStack.push(nextSymbol);
			staStack.push(act.nextStat);
			iter++;
		}
		else if (act.pa == reduce) {
			Production reductPro = productions[act.nextStat];
			int popSymNum = (int)reductPro.right.size();
			switch (act.nextStat) {
				case 3://declare ::= int ID M A function_declare
				{
					FunctionDeclare *function_declare = (FunctionDeclare*)popSymbol();
					Symbol* A = popSymbol();
					M* m = (M*)popSymbol();
					Id* ID = (Id*)popSymbol();
					Symbol* _int = popSymbol();
					funcTable.push_back(Func{ ID->name,D_INT,function_declare->plist,m->quad });
					pushSymbol(line, pos,  new Symbol(reductPro.left));
					if (errorFlag == 1)
						return;
					break;
				}
				case 4://declare ::= int ID var_declare
				{
					Symbol* var_declare = popSymbol();
					Id* ID = (Id*)popSymbol();
					Symbol* _int = popSymbol();
					varTable.push_back(Var{ ID->name,D_INT,nowLevel });
					pushSymbol(line, pos,  new Symbol(reductPro.left));
					if (errorFlag == 1)
						return;
					break;
				}
				case 5://declare ::= void ID M A function_declare
				{
					FunctionDeclare* function_declare = (FunctionDeclare*)popSymbol();
					Symbol* A = popSymbol();
					M* m = (M*)popSymbol();
					Id* ID = (Id*)popSymbol();
					Symbol* _void = popSymbol();
					funcTable.push_back(Func{ ID->name, D_VOID, function_declare->plist,m->quad });
					pushSymbol(line, pos,  new Symbol(reductPro.left));
					if (errorFlag == 1)
						return;
					break;
				}
				case 6://A ::=
				{
					nowLevel++;
					pushSymbol(line, pos,  new Symbol(reductPro.left));
					if (errorFlag == 1)
						return;
					break;
				}
				case 8://function_declare ::= ( parameter ) sentence_block
				{
					SentenceBlock* sentence_block = (SentenceBlock*)popSymbol();
					Symbol* rparen = popSymbol();
					Parameter* paramter = (Parameter*)popSymbol();
					Symbol* lparen = popSymbol();
					FunctionDeclare* function_declare = new FunctionDeclare(reductPro.left);
					function_declare->plist.assign(paramter->plist.begin(), paramter->plist.end());
					pushSymbol(line, pos,  function_declare);
					if (errorFlag == 1)
						return;
					break;
				}
				case 9://parameter :: = parameter_list
				{
					ParameterList* parameter_list = (ParameterList*)popSymbol();
					Parameter *parameter = new Parameter(reductPro.left);
					parameter->plist.assign(parameter_list->plist.begin(), parameter_list->plist.end());
					pushSymbol(line, pos,  parameter);
					if (errorFlag == 1)
						return;
					break;
				}
				case 10://parameter ::= void
				{
					Symbol* _void = popSymbol();
					Parameter* parameter = new Parameter(reductPro.left);
					pushSymbol(line, pos,  parameter);
					if (errorFlag == 1)
						return;
					break;
				}
				case 11://parameter_list ::= param
				{
					Symbol* param = popSymbol();
					ParameterList* parameter_list = new ParameterList(reductPro.left);
					parameter_list->plist.push_back(D_INT);
					pushSymbol(line, pos,  parameter_list);
					if (errorFlag == 1)
						return;
					break;
				}
				case 12://parameter_list1 ::= param , parameter_list2
				{
					ParameterList* parameter_list2 = (ParameterList*)popSymbol();
					Symbol* comma = popSymbol();
					Symbol* param = popSymbol();
					ParameterList *parameter_list1 = new ParameterList(reductPro.left);
					parameter_list2->plist.push_front(D_INT);
					parameter_list1->plist.assign(parameter_list2->plist.begin(), parameter_list2->plist.end());
					pushSymbol(line, pos,  parameter_list1);
					if (errorFlag == 1)
						return;
					break;
				}
				case 13://param ::= int ID
				{
					Id* ID = (Id*)popSymbol();
					Symbol* _int = popSymbol();
					varTable.push_back(Var{ ID->name,D_INT,nowLevel });
					code.pushCode("get", "_", "_", ID->name);
					pushSymbol(line, pos,  new Symbol(reductPro.left));
					if (errorFlag == 1)
						return;
					break;
				}
				case 14://sentence_block ::= { inner_declare sentence_list }
				{
					Symbol* rbrace = popSymbol();
					SentenceList* sentence_list = (SentenceList*)popSymbol();
					Symbol* inner_declare = popSymbol();
					Symbol* lbrace = popSymbol();
					SentenceBlock* sentence_block = new SentenceBlock(reductPro.left);
					sentence_block->nextList = sentence_list->nextList;
					nowLevel--;
					int popNum = 0;
					for (vector<Var>::reverse_iterator riter = varTable.rbegin(); riter != varTable.rend(); riter++) {
						if (riter->level > nowLevel)
							popNum++;
						else
							break;
					}
					for (int i = 0; i < popNum; i++) {
						varTable.pop_back();
					}
					pushSymbol(line, pos,  sentence_block);
					if (errorFlag == 1)
						return;
					break;
				}
				case 17://inner_var_declare ::= int ID
				{
					Id* ID = (Id*)popSymbol();
					Symbol* _int = popSymbol();
					pushSymbol(line, pos,  new Symbol(reductPro.left));
					if (errorFlag == 1)
						return;
					varTable.push_back(Var{ ID->name,D_INT,nowLevel });
					break;
				}
				case 18://sentence_list ::= sentence M sentence_list
				{
					SentenceList* sentence_list2 = (SentenceList*)popSymbol();
					M* m = (M*)popSymbol();
					Sentence* sentence = (Sentence*)popSymbol();
					SentenceList* sentence_list1 = new SentenceList(reductPro.left);
					sentence_list1->nextList = sentence_list2->nextList;
					code.backFill(sentence->nextList, m->quad);
					pushSymbol(line, pos,  sentence_list1);
					if (errorFlag == 1)
						return;
					break;
				}
				case 19://sentence_list ::= sentence
				{
					Sentence* sentence = (Sentence*)popSymbol();
					SentenceList* sentence_list = new SentenceList(reductPro.left);
					sentence_list->nextList = sentence->nextList;
					pushSymbol(line, pos,  sentence_list);
					if (errorFlag == 1)
						return;
					break;
				}
				case 20://sentence ::= if_sentence
				{
					IfSentence* if_sentence = (IfSentence*)popSymbol();
					Sentence* sentence = new Sentence(reductPro.left);
					sentence->nextList = if_sentence->nextList;
					pushSymbol(line, pos,  sentence);
					if (errorFlag == 1)
						return;
					break;
				}
				case 21://sentence ::= while_sentence
				{
					WhileSentence* while_sentence = (WhileSentence*)popSymbol();
					Sentence* sentence = new Sentence(reductPro.left);
					sentence->nextList = while_sentence->nextList;
					pushSymbol(line, pos,  sentence);
					if (errorFlag == 1)
						return;
					break;
				}
				case 22://sentence ::= return_sentence
				{
					Symbol* return_sentence = popSymbol();
					Sentence* sentence = new Sentence(reductPro.left);
					pushSymbol(line, pos,  sentence);
					if (errorFlag == 1)
						return;
					break;
				}
				case 23://sentence ::= assign_sentence
				{
					Symbol* assign_sentence = popSymbol();
					Sentence* sentence = new Sentence(reductPro.left);
					pushSymbol(line, pos,  sentence);
					if (errorFlag == 1)
						return;
					break;
				}
				case 24://assign_sentence ::= ID = expression ;
				{
					Symbol* comma = popSymbol();
					Expression* expression = (Expression*)popSymbol();
					Symbol* assign = popSymbol();
					Id* ID = (Id*)popSymbol();
					Symbol* assign_sentence = new Symbol(reductPro.left);
					code.pushCode("=", expression->name, "_", ID->name);
					pushSymbol(line, pos,  assign_sentence);
					if (errorFlag == 1)
						return;
					break;
				}
				case 25://return_sentence ::= return ;
				{
					Symbol* comma = popSymbol();
					Symbol* _return = popSymbol();
					code.pushCode("return", "_", "_", "_");
					pushSymbol(line, pos,  new Symbol(reductPro.left));
					if (errorFlag == 1)
						return;
					break;
				}
				case 26://return_sentence ::= return expression ;
				{
					Symbol* comma = popSymbol();
					Expression* expression = (Expression*)popSymbol();
					Symbol* _return = popSymbol();
					code.pushCode("return", expression->name, "_", "_");
					pushSymbol(line, pos,  new Symbol(reductPro.left));
					if (errorFlag == 1)
						return;
					break;
				}
				case 27://while_sentence ::= while M ( expression ) A sentence_block
				{
					SentenceBlock* sentence_block = (SentenceBlock*)popSymbol();
					Symbol* A = popSymbol();
					Symbol* rparen = popSymbol();
					Expression* expression = (Expression*)popSymbol();
					Symbol* lparen = popSymbol();
					M* m = (M*)popSymbol();
					Symbol* _while = popSymbol();
					WhileSentence* while_sentence = new WhileSentence(reductPro.left);
					code.backFill(sentence_block->nextList, m->quad);
					while_sentence->nextList = expression->falseList;
					code.pushCode("j", "_", "_", to_string(m->quad));
					pushSymbol(line, pos,  while_sentence);
					if (errorFlag == 1)
						return;
					break;
				}
				case 28://if_sentence ::= if ( expression ) A sentence_block
				{
					SentenceBlock* sentence_block = (SentenceBlock*)popSymbol();
					Symbol* A = popSymbol();
					Symbol* rparen = popSymbol();
					Expression* expression = (Expression*)popSymbol();
					Symbol* lparen = popSymbol();
					Symbol* _if = popSymbol();
					IfSentence* if_sentence = new IfSentence(reductPro.left);
					expression->falseList.splice(expression->falseList.begin(), sentence_block->nextList);
					if_sentence->nextList = expression->falseList;
					pushSymbol(line, pos,  if_sentence);
					if (errorFlag == 1)
						return;
					break;
				}
				case 29://if_sentence ::= if ( expression ) A1 sentence_block1 N else M A2 sentence_block2
				{
					SentenceBlock* sentence_block2 = (SentenceBlock*)popSymbol();
					Symbol* A2 = popSymbol();
					M* m = (M*)popSymbol();
					Symbol* _else = popSymbol();
					N* n = (N*)popSymbol();
					SentenceBlock* sentence_block1 = (SentenceBlock*)popSymbol();
					Symbol* A1 = popSymbol();
					Symbol* rparen = popSymbol();
					Expression* expression = (Expression*)popSymbol();
					Symbol* lparen = popSymbol();
					Symbol* _if = popSymbol();
					IfSentence* if_sentence = new IfSentence(reductPro.left);
					code.backFill(expression->falseList, m->quad);
					if_sentence->nextList = merge(sentence_block1->nextList, sentence_block2->nextList);
					if_sentence->nextList = merge(if_sentence->nextList, n->nextList);
					pushSymbol(line, pos,  if_sentence);
					if (errorFlag == 1)
						return;
					break;
				}
				case 30://N ::= 
				{
					N* n = new N(reductPro.left);
					n->nextList.push_back(code.quadNum());
					code.pushCode("j", "_", "_", "-1");
					pushSymbol(line, pos,  n);
					if (errorFlag == 1)
						return;
					break;
				}
				case 31://M ::=
				{
					M* m = new M(reductPro.left);
					m->quad = code.quadNum();
					pushSymbol(line, pos,  m);
					if (errorFlag == 1)
						return;
					break;
				}
				case 32://expression ::= add_expression
				{
					AddExpression* add_expression = (AddExpression*)popSymbol();
					Expression* expression = new Expression(reductPro.left);
					expression->name = add_expression->name;
					pushSymbol(line, pos,  expression);
					if (errorFlag == 1)
						return;
					break;
				}
				case 33://expression ::= add_expression1 > add_expression2
				{
					AddExpression* add_expression2 = (AddExpression*)popSymbol();
					Symbol* gt = popSymbol();
					AddExpression* add_expression1 = (AddExpression*)popSymbol();
					Expression* expression = new Expression(reductPro.left);
					expression->falseList.push_back(code.quadNum());
					code.pushCode("j<=", add_expression1->name, add_expression2->name, "-1");
					pushSymbol(line, pos,  expression);
					if (errorFlag == 1)
						return;
					break;
				}
				case 34://expression ::= add_expression1 < add_expression2
				{
					AddExpression* add_expression2 = (AddExpression*)popSymbol();
					Symbol* lt = popSymbol();
					AddExpression* add_expression1 = (AddExpression*)popSymbol();
					Expression* expression = new Expression(reductPro.left);
					expression->falseList.push_back(code.quadNum());
					code.pushCode("j>=", add_expression1->name, add_expression2->name, "-1");
					pushSymbol(line, pos,  expression);
					if (errorFlag == 1)
						return;
					break;
				}
				case 35://expression ::= add_expression1 == add_expression2
				{
					AddExpression* add_expression2 = (AddExpression*)popSymbol();
					Symbol *eq = popSymbol();
					AddExpression *add_expression1 = (AddExpression*)popSymbol();
					Expression *expression = new Expression(reductPro.left);
					expression->falseList.push_back(code.quadNum());
					code.pushCode("j!=", add_expression1->name, add_expression2->name, "-1");
					pushSymbol(line, pos,  expression);
					if (errorFlag == 1)
						return;
					break;
				}
				case 36://expression ::= add_expression1 >= add_expression2
				{
					AddExpression* add_expression2 = (AddExpression*)popSymbol();
					Symbol* get = popSymbol();
					AddExpression* add_expression1 = (AddExpression*)popSymbol();
					Expression* expression = new Expression(reductPro.left);
					expression->falseList.push_back(code.quadNum());
					code.pushCode("j<", add_expression1->name, add_expression2->name, "-1");
					pushSymbol(line, pos,  expression);
					if (errorFlag == 1)
						return;
					break;
				}
				case 37://expression ::= add_expression1 <= add_expression2
				{
					AddExpression* add_expression2 = (AddExpression*)popSymbol();
					Symbol* let = popSymbol();
					AddExpression* add_expression1 = (AddExpression*)popSymbol();
					Expression* expression = new Expression(reductPro.left);
					expression->falseList.push_back(code.quadNum());
					code.pushCode("j>", add_expression1->name, add_expression2->name, "-1");
					pushSymbol(line, pos,  expression);
					if (errorFlag == 1)
						return;
					break;
				}
				case 38://expression ::= add_expression1 != add_expression2
				{
					AddExpression* add_expression2 = (AddExpression*)popSymbol();
					Symbol* neq = popSymbol();
					AddExpression* add_expression1 = (AddExpression*)popSymbol();
					Expression* expression = new Expression(reductPro.left);
					expression->falseList.push_back(code.quadNum());
					code.pushCode("j==", add_expression1->name, add_expression2->name, "-1");
					pushSymbol(line, pos,  expression);
					if (errorFlag == 1)
						return;
					break;
				}
				case 39://add_expression ::= item
				{
					Term* item = (Term*)popSymbol();
					AddExpression* add_expression = new AddExpression(reductPro.left);
					add_expression->name = item->name;
					pushSymbol(line, pos,  add_expression);
					if (errorFlag == 1)
						return;
					break;
				}
				case 40://add_expression1 ::= item + add_expression2
				{
					AddExpression* add_expression2 = (AddExpression*)popSymbol();
					Symbol* add = popSymbol();
					Term* item = (Term*)popSymbol();
					AddExpression* add_expression1 = new AddExpression(reductPro.left);
					add_expression1->name = nt.newTemp();
					code.pushCode("+", item->name, add_expression2->name, add_expression1->name);
					pushSymbol(line, pos,  add_expression1);
					if (errorFlag == 1)
						return;
					break;
				}
				case 41://add_expression ::= item - add_expression
				{
					AddExpression* add_expression2 = (AddExpression*)popSymbol();
					Symbol* sub = popSymbol();
					Term* item = (Term*)popSymbol();
					AddExpression* add_expression1 = new AddExpression(reductPro.left);
					add_expression1->name = nt.newTemp();
					code.pushCode("-", item->name, add_expression2->name, add_expression1->name);
					pushSymbol(line, pos,  add_expression1);
					if (errorFlag == 1)
						return;
					break;
				}
				case 42://item ::= factor
				{
					Factor* factor = (Factor*)popSymbol();
					Term* item = new Term(reductPro.left);
					item->name = factor->name;
					pushSymbol(line, pos,  item);
					if (errorFlag == 1)
						return;
					break;
				}
				case 43://item1 ::= factor * item2
				{
					Term* item2 = (Term*)popSymbol();
					Symbol* mul = popSymbol();
					Factor* factor = (Factor*)popSymbol();
					Term* item1 = new Term(reductPro.left);
					item1->name = nt.newTemp();
					code.pushCode("*", factor->name, item2->name, item1->name);
					pushSymbol(line, pos,  item1);
					if (errorFlag == 1)
						return;
					break;
				}
				case 44://item1 ::= factor / item2
				{
					Term* item2 = (Term*)popSymbol();
					Symbol* div = popSymbol();
					Factor* factor = (Factor*)popSymbol();
					Term* item1 = new Term(reductPro.left);
					item1->name = nt.newTemp();
					code.pushCode("/", factor->name, item2->name, item1->name);
					pushSymbol(line, pos,  item1);
					if (errorFlag == 1)
						return;
					break;
				}
				case 45://factor ::= NUM
				{
					Num* num = (Num*)popSymbol();
					Factor* factor = new Factor(reductPro.left);
					factor->name = num->number;
					pushSymbol(line, pos,  factor);
					if (errorFlag == 1)
						return;
					break;
				}
				case 46://factor ::= ( expression )
				{
					Symbol* rparen = popSymbol();
					Expression* expression = (Expression*)popSymbol();
					Symbol* lparen = popSymbol();
					Factor* factor = new Factor(reductPro.left);
					factor->name = expression->name;
					pushSymbol(line, pos,  factor);
					if (errorFlag == 1)
						return;
					break;
				}
				case 47://factor ::= ID ( argument_list )
				{
					Symbol* rparen = popSymbol();
					ArgumentList* argument_list = (ArgumentList*)popSymbol();
					Symbol* lparen = popSymbol();
					Id* ID = (Id*)popSymbol();
					Factor* factor = new Factor(reductPro.left);
					Func* f = findFunc(ID->name);
					if (!f) {
						errorFlag = 1;
						errorString += string("语法分析错误(") + to_string(line) + string(",") + to_string(pos - 1) + ")：未声明的函数 " + ID->name + "/n";
						return;
					}
					else if (!compareParaNum(argument_list->alist, f->paramTypes)) {
						errorFlag = 1;
						errorString += string("语法分析错误(") + to_string(line) + string(",") + to_string(pos - 1) + ")：函数 " + ID->name + "参数不匹配" + "/n";
						return;
					}
					else {
						for (list<string>::iterator iter = argument_list->alist.begin(); iter != argument_list->alist.end(); iter++) {
							code.pushCode("par", *iter, "_", "_");
						}
						factor->name = nt.newTemp();
						code.pushCode("call", ID->name,"_", "_");
						code.pushCode("=", "@RETURN_PLACE", "_", factor->name);
						
						pushSymbol(line, pos,  factor);
						if (errorFlag == 1)
							return;
					}
					break;
				}
				case 48://factor ::= ID
				{
					Id* ID = (Id*)popSymbol();
					if (findVar(ID->name) == NULL) {
						errorFlag = 1;
						errorString += string("语法分析错误(") + to_string(line) + string(",") + to_string(pos - 1) + ")：变量 " + ID->name + "未声明" + "/n";
						return;
					}
					Factor* factor = new Factor(reductPro.left);
					factor->name = ID->name;
					pushSymbol(line, pos,  factor);
					if (errorFlag == 1)
						return;
					break;
				}
				case 49://argument_list ::= 
				{
					ArgumentList* argument_list = new ArgumentList(reductPro.left);
					pushSymbol(line, pos,  argument_list);
					if (errorFlag == 1)
						return;
					break;
				}
				case 50://argument_list ::= expression
				{
					Expression* expression = (Expression*)popSymbol();
					ArgumentList* argument_list = new ArgumentList(reductPro.left);
					argument_list->alist.push_back(expression->name);
					pushSymbol(line, pos,  argument_list);
					if (errorFlag == 1)
						return;
					break;
				}
				case 51://argument_list1 ::= expression , argument_list2
				{
					ArgumentList* argument_list2 = (ArgumentList*)popSymbol();
					Symbol* comma = popSymbol();
					Expression* expression = (Expression*)popSymbol();
					ArgumentList* argument_list1 = new ArgumentList(reductPro.left);
					argument_list2->alist.push_front(expression->name);
					argument_list1->alist.assign(argument_list2->alist.begin(),argument_list2->alist.end());
					pushSymbol(line, pos,  argument_list1);
					if (errorFlag == 1)
						return;
					break;
				}
				default:
					for (int i = 0; i < popSymNum; i++) {
						popSymbol();
					}
					pushSymbol(line, pos,  new Symbol(reductPro.left));
					if (errorFlag == 1)
						return;
					break;
			}
		}
		else if (act.pa == success) {//P ::= N declare_list
			acc = true;
			Func*f = findFunc("main");
			popSymbol();
			N* n = (N*)popSymbol();
			code.backFill(n->nextList, f->enterPoint);
			break;
		}
	}
	if (!acc) {
		errorFlag = 1;
		errorString += "语法错误：未知的结尾/n";
		return;
	}
}

//获取入口地址
vector<pair<int, string> >GrammerAndSemantic::getFuncEnter() {
	vector<pair<int, string> >ret;
	for (vector<Func>::iterator iter = funcTable.begin(); iter != funcTable.end(); iter++) {
		ret.push_back(pair<int, string>(iter->enterPoint, iter->name));
	}
	sort(ret.begin(), ret.end());
	return ret;
}

//获取中间代码
IntermediateCode* GrammerAndSemantic::getIntermediateCode() {
	return &code;
}