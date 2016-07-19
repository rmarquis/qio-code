/*
============================================================================
Copyright (C) 2013 V.

This file is part of Qio source code.

Qio source code is free software; you can redistribute it 
and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

Qio source code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA,
or simply visit <http://www.gnu.org/licenses/>.
============================================================================
*/
// shared/ast.cpp - Abstract Syntax Tree for Doom3 material expressions evaluation
#include "typedefs.h"
#include "array.h"
#include "str.h"
#include "ast.h"
#include "autoCvar.h"
#include <api/coreAPI.h>
#include <math.h>

static aCvar_c ast_printParsedExpressions("ast_printParsedExpressions","0");

#if 1
#define AST_SAVE_SOURCE_STRING
#endif
#define MAX_AST_RECURSION_DEPTH 128

enum parseState_e {
	PS_WHITESPACES,
	PS_NUMBER,
	PS_TOKEN,
};
enum lexemType_e {
	LEX_NUMBER,
	LEX_TOKEN,
	LEX_OPERATOR,
	LEX_ARRAY_BRACKET_LEFT,
	LEX_ARRAY_BRACKET_RIGHT,
	LEX_BRACKET_LEFT,
	LEX_BRACKET_RIGHT,
};
enum operator_e {
	OP_BAD,
	OP_ADD, // '+'
	OP_SUB, // '-'
	OP_MULT, // '*'
	OP_DIV, // '/'
	OP_MOD, // '%'
	OP_EQUAL, // "=="
	OP_NOT_EQUAL, // "!="
	OP_LESS, // "<"
	OP_GREATER, // ">"
	OP_LESS_OR_EQUAL, // "<="
	OP_GREATER_OR_EQUAL, // ">="
};
u32 OP_GetPriority(operator_e op) {
	if(op == OP_NOT_EQUAL ||op == OP_EQUAL || op == OP_LESS || op == OP_GREATER || op == OP_LESS_OR_EQUAL || op == OP_GREATER_OR_EQUAL)
		return 5;
	if(op == OP_MOD)
		return 7;
	if(op == OP_ADD || op == OP_SUB)
		return 10;
	if(op == OP_MULT || op == OP_DIV)
		return 100;
	return 0;
}
struct astLexem_s {
	lexemType_e type;
	str value;
	operator_e opType;

	void setNumberLexem(const char *s) {
		type = LEX_NUMBER;
		value = s;
	}	
	void setTokenLexem(const char *s) {
		type = LEX_TOKEN;
		value = s;
	}
	void setOperatorLexem(operator_e op) {
		type = LEX_OPERATOR;
		opType = op;
	}
	void setType(lexemType_e newType) {
		type = newType;
	}
	u32 getPriority() const {
		if(type == LEX_OPERATOR) {
			return OP_GetPriority(opType);
		}
		return 9999;
	}
	bool isOperator() const {
		if(type == LEX_OPERATOR)
			return true;
		return false;
	}
	bool isBracket() const {
		if(type == LEX_ARRAY_BRACKET_LEFT)
			return true;
		if(type == LEX_ARRAY_BRACKET_RIGHT)
			return true;
		if(type == LEX_BRACKET_LEFT)
			return true;
		if(type == LEX_BRACKET_RIGHT)
			return true;
		return false;
	}
	// array brackets: '[', ']'
	bool isLeftBracketArray() const {
		if(type == LEX_ARRAY_BRACKET_LEFT)
			return true;
		return false;
	}
	bool isLeftBracket() const {
		if(type == LEX_ARRAY_BRACKET_LEFT)
			return true;
		if(type == LEX_BRACKET_LEFT)
			return true;
		return false;
	}
	bool isRightBracket() const {
		if(type == LEX_ARRAY_BRACKET_RIGHT)
			return true;
		if(type == LEX_BRACKET_RIGHT)
			return true;
		return false;
	}
};

operator_e OperatorForString(const char *s, const char **p) {
	if(*s == '+') {
		if(p) {
			*p = s + 1;
		}
		return OP_ADD;
	} else if(*s == '-') {
		if(p) {
			*p = s + 1;
		}
		return OP_SUB;
	} else if(*s == '*') {
		if(p) {
			*p = s + 1;
		}
		return OP_MULT;
	} else if(*s == '/') {
		if(p) {
			*p = s + 1;
		}
		return OP_DIV;
	} else if(*s == '%') {
		if(p) {
			*p = s + 1;
		}
		return OP_MOD;
	} else if(*s == '>') {
		if(s[1] == '=') {
			if(p) {
				*p = s + 2;
			}
			return OP_GREATER_OR_EQUAL;
		}
		if(p) {
			*p = s + 1;
		}
		return OP_GREATER;
	} else if(*s == '<') {
		if(s[1] == '=') {
			if(p) {
				*p = s + 2;
			}
			return OP_LESS_OR_EQUAL;
		}
		if(p) {
			*p = s + 1;
		}
		return OP_LESS;
	} else if(s[0] == '=') {
		if(s[1] == '=') {
			if(p) {
				*p = s + 2;
			}
			return OP_EQUAL;
		}
	} else if(s[0] == '!') {
		if(s[1] == '=') {
			if(p) {
				*p = s + 2;
			}
			return OP_NOT_EQUAL;
		}
	}
	return OP_BAD;
}

// .65 + bfg_flare1_rotate[time*.5] 

// 2*2+2
// here * has higher priority, and the + is in root ast node
// 2*(2+2)
// here + has higher priority, and the * is in root ast node

enum astNodeType_e {
	ANT_BAD,
	ANT_OPERATOR,
	ANT_NUMBER,
	ANT_VARIABLE,
	// for Doom3 tables
	ANT_GETARRAYELEMENT,
	// for math functions
	ANT_FUNCTION,
};

class astNode_c {
	astNodeType_e type;
	astNode_c *children[2];
	operator_e opType; // for ANT_OPERATOR
	float value;
	str varName;
	static u32 c_nullNodeCounter;
public:
	astNode_c() {
		children[0] = children[1] = 0;
	}
	~astNode_c() {
		if(children[0])
			delete children[0];
		if(children[1])
			delete children[1];
	}
	static void resetNullNodeCounter() {
		c_nullNodeCounter = 0;
	}
	static bool hasNullNodes() {
		return c_nullNodeCounter;
	}
	float execute_r(const class astInputAPI_i *in) const {
		if(this == 0) {
			// this should never happen unless AST is invalid
			g_core->RedWarning("astNode_c::execute_r: called on NULL node\n");
			c_nullNodeCounter++;
			return 0.f;
		}
		if(type == ANT_NUMBER) {
			return value;
		}
		if(type == ANT_VARIABLE) {
			if(in == 0) {
				return 0.f;
			}
			return in->getVariableValue(varName);
		}
		if(type == ANT_GETARRAYELEMENT) {
			if(in == 0) {
				return 0.f;
			}
			float idx = children[0]->execute_r(in);
			return in->getTableValue(varName,idx);
		}
		if(type == ANT_FUNCTION) {
			if(in == 0) {
				return 0.f;
			}
			float idx = children[0]->execute_r(in);
#if 0
			return in->getFunctionValue(varName,idx);
#else
			if(!stricmp(varName,"sin")) {
				return sin(idx);
			}
			if(!stricmp(varName,"abs")) {
				return abs(idx);
			}
			return 0.f;
#endif
		}
		
		if(type == ANT_OPERATOR) {
			float a = children[0]->execute_r(in);
			float b = children[1]->execute_r(in);
			float ret;
			if(opType == OP_ADD) {
				ret = a + b;
			} else if(opType == OP_SUB) {
				ret = a - b;
			} else if(opType == OP_MULT) {
				ret = a * b;
			} else if(opType == OP_DIV) {
				ret = a / b;
			} else if(opType == OP_EQUAL) {
				ret = a == b;
			} else if(opType == OP_NOT_EQUAL) {
				ret = a != b;
			} else if(opType == OP_MOD) {
				ret = int(a) % int(b);
			} else if(opType == OP_GREATER) {
				ret = a > b;
			} else if(opType == OP_LESS) {
				ret = a < b;
			} else if(opType == OP_GREATER_OR_EQUAL) {
				ret = a >= b;
			} else if(opType == OP_LESS_OR_EQUAL) {
				ret = a <= b;
			} else {
				g_core->RedWarning("astNode_c::execute_r: unknown operator type %i\n",opType);
				return 0.f;
			}
			return ret;
		}
		return 0.f;
	}
	astNode_c *duplicateNode() const {
		astNode_c *ret = new astNode_c;
		ret->type = this->type;
		ret->value = this->value;
		ret->varName = this->varName;
		ret->opType = this->opType;
		if(children[0]) {
			ret->children[0] = this->children[0]->duplicateNode();
		}
		if(children[1]) {
			ret->children[1] = this->children[1]->duplicateNode();
		}
		return ret;
	}
	void setChild(u32 i, astNode_c *p) {
		children[i] = p;
	}
	void setOperator(operator_e newOp) {
		type = ANT_OPERATOR;
		opType = newOp;
	}
	void setNumber(const char *s) {
		type = ANT_NUMBER;
		value = atof(s);
	}
	void setVariable(const char *s) {
		type = ANT_VARIABLE;
		varName = s;
	}
	void setType(astNodeType_e newType) {
		type = newType;
	}
};
class ast_c : public astAPI_i {
	astNode_c *root;
#ifdef AST_SAVE_SOURCE_STRING
	// char array, and not a string class because it's easier to debug memory leaks this way.
	char sourceString[128];
#endif
public:
	ast_c() {
		root = 0;
	}
	~ast_c() {
		delete root;
	}
#ifdef AST_SAVE_SOURCE_STRING
	void setSourceString(const char *s) {
		u32 len = strlen(s);
		if(len > sizeof(sourceString))
			len = sizeof(sourceString)-1;
		memcpy(sourceString,s,len);
		sourceString[len] = 0;
	}
#endif
	void setRootNode(astNode_c *newRoot) {
		root = newRoot;
	}
	virtual astAPI_i *duplicateAST() const { 
		ast_c *ret = new ast_c;
		ret->setRootNode(root->duplicateNode());
		return ret;
	}
	virtual void destroyAST() {
		delete this;
	}
	virtual float execute(const class astInputAPI_i *in) const {
		return root->execute_r(in);
	}
};
class astParser_c {
	inline static bool isWhiteSpace(char p) {
		if(p == ' ' || p == '\n' || p == '\t' || p == '\r')
			return true;
		return false;
	}
	arraySTD_c<astLexem_s> lexems;
	bool bError;
	void addNumberLexem(const char *s) {
		lexems.pushBack().setNumberLexem(s);
	}
	void addTokenLexem(const char *s) {
		lexems.pushBack().setTokenLexem(s);
	}
	void addOperatorLexem(operator_e op) {
		lexems.pushBack().setOperatorLexem(op);
	}
	void addLexem(lexemType_e type) {
		lexems.pushBack().setType(type);
	}
	u32 findLexemWithLowestPriority(u32 start, u32 stop) {
		u32 bestValue = 9999999;
		u32 best = start;
		int bestLevel = 0;
		int level = 0;
		for(u32 i = start; i < stop; i++) {
			if(lexems[i].isBracket()) {
				if(lexems[i].isLeftBracket()) {
					level++;
				} else {
					level--;
				}
			}
			if(level > bestLevel)
				continue;
			u32 check = lexems[i].getPriority();
			if(check < bestValue) {
				best = i;
				bestValue = check;
				bestLevel = level;
			}
		}
		return best;
	}
	class astNode_c *createLeaf(u32 idx) {
		const astLexem_s &lex = lexems[idx];
		if(lex.type == LEX_NUMBER) {
			astNode_c *ret = new astNode_c;
			ret->setNumber(lex.value);
			return ret;
		} else if(lex.type == LEX_TOKEN) {
			astNode_c *ret = new astNode_c;
			ret->setVariable(lex.value);
			return ret;
		}
		return 0;
	}
	u32 findClosingBracket(u32 left) const {
		int level = 1;
		for(u32 i = left+1; i < lexems.size(); i++) {
			if(lexems[i].isBracket()) {
				if(lexems[i].isLeftBracket()) {
					level++;
				} else {
					level--;
				}
			}
			if(level == 0)
				return i;
		}
		// error, closing bracket not found
		return 0;
	}
	class astNode_c *buildNode_r(u32 start, u32 stop, u32 recursionDepth) {
		// check for stack overflow (it still might happen in some rare cases...)
		if(recursionDepth > MAX_AST_RECURSION_DEPTH) {
			g_core->RedWarning("astParser_c::buildNode_r: recursion depth higher than MAX_AST_RECURSION_DEPTH, interrupting.\n");
			bError = true;
			return 0;
		}
		if(stop < start) {
			bError = true;
			return 0;
		}
		// see if we're creating node or a leaf
		if(start+1 == stop) {
			// create leaf
			return createLeaf(start);
		}
		if(lexems[start].isLeftBracket() && lexems[stop-1].isRightBracket()) {
			start++;
			stop--;
		}
		// create node
		// find lexem with lowest priority
		u32 idx = findLexemWithLowestPriority(start,stop);;
		const astLexem_s &lex = lexems[idx];
		if(lex.isOperator()) {
			astNode_c *left = buildNode_r(start,idx,recursionDepth+1);
			astNode_c *right = buildNode_r(idx+1,stop,recursionDepth+1);
			astNode_c *ret = new astNode_c;
			ret->setOperator(lex.opType);
			ret->setChild(0,left);
			ret->setChild(1,right);
			return ret;
		} else if(lex.type == LEX_TOKEN) {
			u32 left = idx+1;
			if(lexems[left].isLeftBracket() == false) {
				return 0;
			}
			u32 right = findClosingBracket(left);
			astNode_c *arrayIndex = buildNode_r(left+1,right,recursionDepth+1);
			astNode_c *ret = new astNode_c;
			ret->setVariable(lex.value);
			if(lexems[left].isLeftBracketArray()) {
				ret->setType(ANT_GETARRAYELEMENT);
			} else {
				ret->setType(ANT_FUNCTION);
			}
			ret->setChild(0,arrayIndex);
			return ret;
		}
		return 0;
	}
public:
	astParser_c() {
	}
	bool isLastLexemOperator() const {
		if(lexems.size() == 0)
			return false;
		if(lexems[lexems.size()-1].isOperator())
			return true;
		return false;
	}
	bool lexemizeExpression(const char *s) {
		parseState_e parseState = PS_WHITESPACES;
		str tmp;
		bool bHadDot;
		const char *p = s;
		const char *start = p;
		while(*p) {
			if(parseState == PS_WHITESPACES) {
				if(isWhiteSpace(*p)) {
					p++;
					continue;
				}
				start = p;
				// check for numbers
				if(isdigit(*p)) {
					parseState = PS_NUMBER;
					bHadDot = false;
					p++;
				} else if(*p == '.') {
					// in D3 materials numbers usually starts with '.'
					parseState = PS_NUMBER;
					bHadDot = true;
					p++;
				} else if((lexems.size() == 0 || isLastLexemOperator()) && *p == '-') {
					// some floating numbers might start with '-'
					parseState = PS_NUMBER;
					bHadDot = false;
					p++;
				} else {
					// check for array brackets
					if(*p == ']') {
						addLexem(LEX_ARRAY_BRACKET_RIGHT);
						p++; // skip ']'
					} else if(*p == '[') {
						addLexem(LEX_ARRAY_BRACKET_LEFT);
						p++; // skip '['
					} else if(*p == ')') {
						addLexem(LEX_BRACKET_RIGHT);
						p++; // skip ')'
					} else if(*p == '(') {
						addLexem(LEX_BRACKET_LEFT);
						p++; // skip '('
					} else if(*p == '{' || *p == '}') {
						g_core->RedWarning("astParser_c::lexemizeExpression: unwanted curly bracket hit. Input string is invalid.\n");
						bError = true;
						return true; // error
					} else {
						// check for operators
						operator_e op = OperatorForString(p,&p);
						if(op != OP_BAD) {
							addOperatorLexem(op);
						} else {
							// assume it's a token (var/function name)
							parseState = PS_TOKEN;
							p++; // skip character
						}
					}
				}
			} else if(parseState == PS_NUMBER) {
				if(isdigit(*p) == false) {
					if(*p == '.') {
						bHadDot = true;
						p++;
					} else {
						tmp.setFromTo(start,p);
						addNumberLexem(tmp);
						parseState = PS_WHITESPACES;
						// don't skip anything
					}
				} else {
					p++;
				}
			} else if(parseState == PS_TOKEN) {
				if(isWhiteSpace(*p) || *p == '[' || *p == ']' || *p == '(' || *p == ')' || OperatorForString(p,0) != OP_BAD) {
					tmp.setFromTo(start,p);
					addTokenLexem(tmp);
					parseState = PS_WHITESPACES;
					// don't skip anything
				} else {
					p++;
				}
			}
		}
		if(parseState == PS_NUMBER) {
			tmp.setFromTo(start,p);
			addNumberLexem(tmp);
		} else if(parseState == PS_TOKEN) {
			tmp.setFromTo(start,p);
			addTokenLexem(tmp);
		}
		return false; // no error
	}
	ast_c *buildAST() {
		bError = false;
		astNode_c *root = buildNode_r(0,lexems.size(),0);
		if(root == 0) {
			return 0;
		}
		ast_c *ret = new ast_c;
		ret->setRootNode(root);
		return ret;
	}
};
u32 astNode_c::c_nullNodeCounter;

// Doom3 expression usage examples:
// "scroll		time, 0"
// "rgb			sintable[time]"
// "rgb			parm3 * .4"
// "red			(fireballtable[ time * .2]) * parm0"
// "translate 	time * 0.07, 0.1"		
// "rgb			elevenstage [ time * Parm3 + Parm4 + 4.0 / 11 ]"	
// "centerscale	s_lightningtable2[time*4], s_lightningtable2[time*4]"	
// "rotate		( time * -.9 )"	
// "scale		1 / 32 , 1"	
// "if			( parm7 == 0 )"	
// "if			( ( time * 38 ) % 16 == 11 )"	
class astAPI_i *AST_ParseExpression(const char *s) {
	if(ast_printParsedExpressions.getInt()) {
		g_core->Print("AST_ParseExpression: %s\n",s);
	}
	astParser_c ap;
	if(ap.lexemizeExpression(s)) {
		return 0;
	}
	ast_c *ret = ap.buildAST();
#ifdef AST_SAVE_SOURCE_STRING
	if(ret) {
		ret->setSourceString(s);
	}
#endif
	if(ret) {
		astNode_c::resetNullNodeCounter();
		ret->execute(0);
		if(astNode_c::hasNullNodes()) {
			g_core->RedWarning("AST '%s' has null nodes\n",s);
			system("pause");
		}
	}
	return ret;
}