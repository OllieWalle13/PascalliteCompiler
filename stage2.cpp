// Bryson Magee (81369652), Julio Perez (81386385)
// CS 4301
// Stage 1

#include <stage2.h>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <ctype.h>

bool relOp(string token);
bool addLevelOp(string token);
bool multLevelOp(string token);
static int labelCount = 0;
static int notMatchingCount = 0;

// constructor
Compiler::Compiler (char **argv) {
   //open files
   sourceFile.open(argv[1]);
   listingFile.open(argv[2]);
   objectFile.open(argv[3]);
}
// destructor
Compiler::~Compiler() {
   // close all files
   sourceFile.close();
   listingFile.close();
   objectFile.close();
}   
void Compiler::createListingHeader() {
   // get time/date 
   time_t now = time (NULL);
   listingFile << "STAGE2:   Bryson Magee, Julio Perez   " << ctime(&now) << endl;
   listingFile << "LINE NO.              SOURCE STATEMENT" << endl << endl;
}

void Compiler::parser() {
   nextChar();
   if (nextToken() != "program") {
      processError("keyword \"program\" expected");
   }
   prog();
}


void Compiler::createListingTrailer() {
   if (errorCount == 1) {
      listingFile << endl << "COMPILATION TERMINATED      " << errorCount << " ERROR ENCOUNTERED" << endl;
   } else {
      listingFile << endl << "COMPILATION TERMINATED      " << errorCount << " ERRORS ENCOUNTERED" << endl;
   }
   // debug only
   //listingFile << endl << "Symbol table size is: " << symbolTable.size() << endl;
}


// stage 0, production 1
void Compiler::prog() {           
   if (token != "program") 
      processError("keyword \"program\" expected");
   progStmt();
   if (token == "const")
      consts();
   if (token == "var")
      vars();
   if (token != "begin") {
      processError("keyword \"begin\" expected");
   }
   beginEndStmt();
   if (token[0] != END_OF_FILE)
      processError("no text may follow \"end\"");
}

// stage 0, production 2
void Compiler::progStmt(){
   string x;
   if (token != "program") {
      processError("keyword \"program\" expected");
   }
   x = nextToken();
   if (!isNonKeyId(token)) {
      processError("program name expected");
   }
   if (nextToken() != ";") {
      processError("semicolon expected");
   }
   nextToken();
   insert(x, PROG_NAME, CONSTANT, x, NO, 0);
   code("program", x);
}   

// stage 0, production 3   
void Compiler::consts(){
   if (token != "const")
     processError("keyword \"const\" expected");
   if (!isNonKeyId(nextToken()))
     processError("non-keyword identifier must follow \"const\"");
   constStmts();
}

// stage 0, production 4
void Compiler::vars(){
   if (token != "var")
      processError("keyword \"var\" expected");
   if (!isNonKeyId(nextToken()))
      processError("non-keyword identifier must follow \"var\"");
   varStmts();
}

// stage 0, production 5     
void Compiler::beginEndStmt(){
	
   //cout << "in beginEndStmt,before function starts, token is: " << token << endl;
   if (token != "begin") {
      processError("keyword \"begin\" expected");
   }
   notMatchingCount += 1;
   //New code because production is revised, Julio Perez, 11/9/2022, 3:45pm
   ////cout << "in beginEndStmt, before 2nd if, token is: " << token << endl;
    
   if (isNonKeyId(nextToken()) || token == "read" || token == "write" || token == "if" || token == "while" || token == "repeat" || token == ";" || token == "begin") {
      //cout << "in beginEndStmt,in 2nd if, token is: " << token << endl;
      execStmts();
   }
   
   //cout << "in beginEndStmt token is: " << token << endl;
   //nextToken();
   ////cout << "in beginEndStmt token is now: " << token << endl;
   /*if (token == "else") {
	   ////cout << "this is a else" << endl;
	   nextToken();
	   execStmts();
   }*/
   /*while (token == "(") {
	    string x;
		if (token != "(") {
		   processError("'(' expected after \"write\"");
		}
		nextToken();
		//cout << "in while loop tokne is: " << token << endl;
		x = ids();
		if (token != ")") {
		   // error
		   processError("',' or ')' expected after non-keyword identifier");//change later
		}
		code("write", x, "");
		//////////////////////////code is from writeList()////////////
		if (nextToken() != ";") {
		  processError("expected \";\"");
		}
		execStmts();
		
   }*/
   ////cout << "in beginEndStmt, after if token is: " << token << endl;
   if (token != "end") {
      ////cout << "in beginEndStmt token is: " << token << endl;
      processError("one of \";\", \"begin\", \"if\", \"read\", \"repeat\", \"while\", \"write\", \"end\", or \"until\" expected");
   }
   notMatchingCount -= 1;
   nextToken();
   if (token != "." && notMatchingCount == 0) {
      processError("period expected after \"end\"");
   }
   else if (notMatchingCount > 0 && token != ";") {
      processError("in beginEndStmt, else if statement");
   }
   
   if (notMatchingCount == 0 && token == ".") {
      code("end", ".");
      nextToken();
   }
}

// stage 0, production 6
void Compiler::constStmts(){
   //////////cout << "in constStmts" << endl;
   string x,y;
   if (!isNonKeyId(token))
      processError("non-keyword identifier expected");
   x = token;
   if (nextToken() != "=") {
      if (token == ",") {
         processError("No list of consts allowed");
      }
      processError("\"=\" expected");
   }
   y = nextToken();
   if (!(isInteger(y) || isNonKeyId(y) || y == "+" || y == "-" || y == "not" || y == "true" || y == "false"))
      processError("token to right of \"=\" illegal");
   if (y == "+" || y == "-"){
      if (!isInteger(nextToken()))
         processError("integer expected after sign");
      y = y + token;
   }
   if (y == "not"){
      if (!isBoolean(nextToken()))
         processError("boolean expected after \"not\"");
      if (symbolTable.count(token)) {
         token = symbolTable.find(token)->second.getValue();
      }
      if (token == "true")
         y = "false";
      else  
         y = "true";
   }
   if (nextToken() != ";") {
      if (token == ".") {
         processError("Real numbers are not allowed");
      }
      processError("semicolon expected");
   }
   if (!(whichType(y) == INTEGER || whichType(y) == BOOLEAN))
      processError("data type of token on the right-hand side must be INTEGER or BOOLEAN");
   
   insert(x,whichType(y),CONSTANT,whichValue(y),YES,1);
   x = nextToken();
   if (!(isNonKeyId(x) || x == "begin" || x == "var"))
      processError("non-keyword identifier, \"begin\", or \"var\" expected");
   if (isNonKeyId(x))
      constStmts();
}

// stage 0, production 7
void Compiler::varStmts(){
   string x,y;
   if (!isNonKeyId(token))
      processError("non-keyword identifier expected");
   x = ids();
   if (token != ":")
      processError("\":\" expected");
   if (!(nextToken() == "integer" || token == "boolean")) {
      processError("illegal type follows \":\"");
   }
   y = token;
   if (nextToken() != ";") {
      processError("semicolon expected");
   }
   if (y == "integer") {
      insert(x,INTEGER,VARIABLE,"1",YES,1);
   } else {
      insert(x,BOOLEAN,VARIABLE,"1",YES,1);
   }
   if (!(nextToken() == "begin" || isNonKeyId(token))) {
      if (token == "const") {
         processError("const must come before var");
      }
      processError("non-keyword identifier or \"begin\" expected");
   }
   if (isNonKeyId(token))
      varStmts();
}

// stage 0, production 8
string Compiler::ids(){
   string temp, tempString;
   if (!isNonKeyId(token))
      processError("non-keyword identidier expected");
   tempString = token;
   temp = token;
   if (nextToken() == ","){
      if (!isNonKeyId(nextToken())) {
         processError("non-keyword identifier expected");
      }
      tempString = temp + "," + ids();
   }
   return tempString;  
}

//Julio Perez, 11/9/22, 4:01pm
// stage 1, production 2
void Compiler::execStmts() {
   //cout << "in execStmts token is: "<< token << " LineNo is: " << lineNo << endl;
   if (!(isNonKeyId(token) || token == "read" || token == "write" || token == "if" || token == "while" || token == "repeat" || token == ";" || token == "begin")) {
      processError("Error execStmts:expected a non key id, read, write, if, while, repeat, ';', or begin");
   }
   execStmt();
   //cout << "execStmts token is: " << token << " lineNo: " << lineNo << endl;
   if (isNonKeyId(token) || token == "read" || token == "write" || token == "if" || token == "while" || token == "repeat" || token == ";" || token == "begin") {
      execStmts();
   }
}

//Julio Perez, 11/9/22, 4:18pm
// stage 1, production 3
void Compiler::execStmt() {
   //cout << "in execStmt token is: " << token << " and lineNo is: " << lineNo << endl;
   if (isNonKeyId(token)) {
      assignStmt();
   }
   else if (token == "read") {
      readStmt();
   }
   else if (token == "write") {
      writeStmt();
   }
   else if (token == "if") {
      ifStmt();
      ////cout << "in execStmt,after ifStmt(), token is: " << token << endl;
   }
   else if (token == "while") {
      whileStmt();
   }
   else if (token == "repeat") {
      repeatStmt();
   }
   else if (token == ";") {
      nullStmt();
   }
   else if (token == "begin") {
      beginEndStmt();
      nextToken();
   } else {
      processError("expected a non key id, read, write, if, while, repeat, or ;");
   }
}

//Julio Perez, 11/9/22, 5:29pm
// stage 1, production 4
void Compiler::assignStmt() {
   if (!isNonKeyId(token)) {
      processError("expected a non key id");
   }
   pushOperand(token);
   if (nextToken() != ":=") {
      processError("expected \":=\"");
   }
   pushOperator(token);
   nextToken();
   express();
   if (token != ";") {
      processError("expected \";\"");
   }
   string op1 = popOperator();
   string op2 = popOperand();
   string op3 = popOperand();
   code(op1,op2,op3);
   nextToken();
}

//Julio Perez, 11/9/22, 5:36pm
// stage 1, production 5   
void Compiler::readStmt() {
   if (token != "read") {
      processError("expected \"read\"");
   }
   //readList();
   /////////////////////This code is from readList()//////////////
   //////////cout << token << endl;
   if (nextToken() != "(") {
      processError("expected \"(\"");
   }
   //////////cout << token << endl;
   nextToken();
   //////////cout << token << endl;
   string x = ids();
   if (token != ")") {
      processError("expected \")\"");
   }
   code("read",x,"");
   /////////////////////This code is from readList()//////////////
   if (nextToken() != ";") {
      processError("expected \";\"");
   }
   nextToken();
}

// stage 1, production 6
/*void readList() { 
   if (nextToken() != "(") {
      processError("expected \"(\"");
   }
   nextToken();
   string x = ids();
   if (token != ")") {
      processError("expected \")\"");
   }
   code("read",x,"");
}*/

// stage 1, production 7
void Compiler::writeStmt() {
   if (token != "write") {
      processError("expected \"write\"");
   }
   //Function is missing form pdf I think
   //writeList();
   //////////////////////////code is from writeList()////////////
   string x;
   if (nextToken() != "(") {
       processError("'(' expected after \"write\"");
   }
   nextToken();
   x = ids();
   if (token != ")") {
       // error
       processError("',' or ')' expected after non-keyword identifier");//change later
   }
   code("write", x, "");
   //////////////////////////code is from writeList()////////////
   if (nextToken() != ";") {
      processError("expected \";\"");
   } 
   nextToken();
   //cout << "write Statment token: " << token << " lineNo: " << lineNo << endl;
}


/*void Compiler::writeList() {
   string x;
   if (nextToken() != "(") {
       //error
   }
   nextToken();
   x = ids;
   if (token != ")") {
       // error
   }
   code("write", x, "");
}*/

// stage 1, production 9  
void Compiler::express() {
   ////cout << "in express function, token is: " << token << endl;
   term();
   expresses();
}

// stage 1, production 10   
void Compiler::expresses() {
   //////////cout << token << endl;
   if (token == "<>" || token == "=" || token == "<=" || token == ">=" || token == "<" || token == ">") {
      if (!relOp(token)) {
         processError("exprecting a rel op in production 10");
      }
      pushOperator(token);
      nextToken();
      term();
      string op1 = popOperator();
      string op2 = popOperand();
      string op3 = popOperand();
      code(op1,op2,op3);
      expresses();
   }
   else if (token == ")" || token == ";" || token == "do" || token == "then") {
      return;
   }
   else {
      //rewrite this
      ////cout << "in epxresses, token is: " << token << endl;
      processError("one of \"*\", \"and\", \"div\", \"mod\", \")\", \"+\", \"-\", \";\", \"<\", \"<=\", \"<>\", \"=\", \">\", \">=\", or \"or\" expected");
   }
}

// stage 1, production 11
void Compiler::term() {
   ////cout << "term: Token: " << token << endl;
   if (token == "not" || token == "true" || token == "false" || token == "," || token == "(" || token == "+" || token == "-" || isInteger(token) || isNonKeyId(token)) {
      factor();
      terms();
   } else {
      processError("expected non_key_id, integer, \"not\", \"true\", \"false\", '(', '+', or '-'");
   }
}

// stage 1, production 12
void Compiler::terms() {
   if (token == "-" || token == "+" || token == "or") {
      if (!addLevelOp(token)) {
         processError("Expecting a add level operator in production 12");
      }
      pushOperator(token);
      nextToken();
      factor();
      string op1 = popOperator();
      string op2 = popOperand();
      string op3 = popOperand();
      code(op1,op2,op3);
      terms();
   }
   else if (token == "<>" || token == "=" || token == "<=" || token == ">=" || token == "<" || token == ">" || token == ")" || token == ";" || token == "do" || token == "then") {
      return;
   } else {
       processError("terms production 12 violated");//Change later
   }
}   

// stage 1, production 13
void Compiler::factor() { 
   if (token == "not" || token == "true" || token == "false" || token == "(" || token == "+" || token == "-" || isInteger(token) || isNonKeyId(token)) {
      //////////cout << "in factor token is: " << token << endl;
      part();
      factors();
   } else {
      processError("factor production 13 violated");
   }
}

// stage 1, production 14
void Compiler::factors() {
   ////////cout << "in factors, before function starts, Token is: " << token << endl;
   if (token == "*" || token == "div" || token == "mod" || token == "and") {
      if (!multLevelOp(token)) {
         processError("Expecting a mult level operator in production 14");
      }
      pushOperator(token);
      nextToken();
      ////////cout << "in factors, Token is: " << token << endl;
      part();
      string op1 = popOperator();
      string op2 = popOperand();
      string op3 = popOperand();
      code(op1,op2,op3);
      factors();
   }
   else if (token == "<>" || token == "=" || token == "<=" || token == ">=" || token == "<" || token == ">" || token == ")" || token == ";" || token == "-" || token == "+" || token == "or" || token == "do" || token == "then") {
      ////////cout << "in factors, in else if, Token is: " << token << endl;
      return;
   } else {
       //error
       ////cout << "in factors, token is: " << token << endl;
       processError("one of \"*\", \"and\", \"div\", \"mod\", \")\", \"+\", \"-\", \";\", \"<\", \"<=\", \"<>\", \"=\", \">\", \">=\", or \"or\" expected");
   }
}

// stage 1, production 15
void Compiler::part() {
   //////////cout << "in part, Token is: " << token << endl;
   if (token == "not") {
      //////cout << "in part, Token is: " << token << endl;
       if (nextToken() == "(") {
          ////////////cout << "in part, Token is: " << token << endl;
           nextToken();
           express();
           
           if (token != ")") {
               //error
               processError("Expecting \')\'");
           }
           string op1 = popOperand();
           ////////////cout << "part: Token: " << token << endl;
           code("not", op1, "");
           nextToken();
       } else if (isBoolean(token) && isLiteral(token)) {
           if (token == "true") {
              pushOperand("false");
           } else {
              pushOperand("true");
           }
           nextToken();
       } else if (isNonKeyId(token)) {
           //////cout << "isnonkeyid part not - " << token << endl;
           code("not", token);
           nextToken();
       } else {
           // error
           processError("expected '(', boolean, or non-keyword id");
       }
   } else if (token == "+") {
      if (nextToken() == "(") {
         nextToken();
         express();
         if (token != ")") {
            processError("Expecting \')\'");
         }
         nextToken();
      } else if (isInteger(token) || isNonKeyId(token)) {
         pushOperand(token);
         nextToken();
      } else {
         processError("expected '(', integer, or non-keyword id");
      }
      
   } else if (token == "-") {
      if (nextToken() == "(") {
         nextToken();
         express();
         if (token != ")") {
            processError("Expecting \')\'");
         }
         string op1 = popOperand();
         ////////cout << "in part, op1 is: " << op1 << endl;
         code("neg", op1,"");
         nextToken();
      } else if (isInteger(token)) {
         pushOperand('-' + token);
         nextToken();
      } else if (isNonKeyId(token)) {
         ////////cout << "in part, token is: " << token << endl;
         code("neg", token,"");
         nextToken();
      } else {
         processError("expected '(', integer, or non_key_id");
      }
       
   } else if (token == "(") {
      nextToken();
      express();
      if (token != ")") {
         processError("Expecting \')\'");
      }
      nextToken();
   } else if (isInteger(token)) {
      pushOperand(token);
      nextToken();
       
   } else if (isBoolean(token)) {
      pushOperand(token);
      nextToken();
       
   } else if (isNonKeyId(token)) {
      pushOperand(token);
      nextToken();
       
   } else {
       processError("expecting a part");
   }
}

// stage 2, production 3
void Compiler::ifStmt() {
   ////cout << "in if statement production token is: " << token << endl;
   if (token != "if") {
      processError("expected a if");
   }
   nextToken();
   ////cout << "in if statement production token is: " << token << endl;
   express();
   ////cout << "in if statement production token is: " << token << endl;
   if (token != "then") {
      processError("expected a then statement");
   }
   nextToken();
   ////cout << "in if statement production token is: " << token << endl;
   string op1 = popOperand();
   if (whichType(op1) != BOOLEAN) {
      processError("predicate must be boolean valued");
   }
   code("then",op1,"");
   //nextToken();
   ////cout << "in if statement production token is: " << token << endl;
   execStmt();
   //nextToken();
   ////cout << "in if statement production token is: " << token << endl;
   elsePt();
   ////cout << "in if statement production, after elsePt token is: " << token << endl;
}

// stage 2, production 4
void Compiler::elsePt() {
   //cout << "in elsePt token is: " << token << endl;
   if (token == "else") {
      string op1 = popOperand();
      code("else",op1,"");
      nextToken();
      execStmt();
      op1 = popOperand();
      code("post_if",op1,"");
   } else {
      string op1 = popOperand();
      code("post_if",op1,"");
   }
   ////cout << "in elsePt token is: " << token << endl;
}

// stage 2, production 5
void Compiler::whileStmt() {
   ////cout << "in whileStmt token is: "<< token <<endl;
   if (token != "while") {
      processError("expected a while");
   }
   nextToken();
   ////cout << "in whileStmt token is: "<< token <<endl;
   code("while","","");
   express();
   if (token != "do") {
      processError("expected a do");
   }
   nextToken();
   ////cout << "in whileStmt token is: "<< token <<endl;
   string op1 = popOperand();
   if (whichType(op1) != BOOLEAN) {
      processError("predicate of while must be of type boolean");
   }
   
   code("do",op1,"");
   execStmt();
   code("post_while",popOperand(),popOperand());
}

// stage 2, production 6    
void Compiler::repeatStmt() {
   if (token != "repeat") {
      processError("expected a repeat");
   }
   nextToken();
   code("repeat","","");
   execStmts();
   if (token != "until") {
      processError("expected a until");
   }
   nextToken();
   express();
   string op1 = popOperand();
   if (whichType(op1) != BOOLEAN) {
      processError("predicate of until must be of type boolean");
   }
   
   code("until",op1,popOperand());
   if (token != ";") {
      processError("expected a ';'");
   }
   nextToken();
}

// stage 2, production 7
void Compiler::nullStmt() {
   if (token != ";") {
      processError("expected a ';'");
   }
   nextToken();
   //cout << "in null stmt, token is: " << token << endl;
}


// determines if s is a keyword
bool Compiler::isKeyword(string s) const {
   //////cout << "in isKeyword s is: "<< s << endl;
   if (s == "program") {
      return true;
   }
   if (s == "begin") {
      return true;
   }
   if (s == "end") {
      return true;
   }
   if (s == "var") {
      return true;
   }
   if (s == "const") {
      return true;
   }
   if (s == "integer") {
      return true;
   }
   if (s == "boolean") {
      return true;
   }
   if (s == "true") {
      return true;
   }
   if (s == "false") {
      return true;
   }
   if (s == "not") {
      return true;
   }
   if (s == "mod") {
      return true;
   }
   if (s == "div") {
      return true;
   }
   if (s == "and") {
      return true;
   }
   if (s == "or") {
      return true;
   }
   if (s == "read") {
      return true;
   }
   if (s == "write") {
      return true;
   }
   if (s == "if") {
      return true;
   }
   if (s == "then") {
      return true;
   }
   if (s == "else") {
      ////cout << "in isKeyword s is: "<< s << endl;
      return true;
   }
   if (s == "while") {
      //////cout << "in isKeyword,in while if, s is: "<< s << endl;
      return true;
   }
   if (s == "do") {
      return true;
   }
   if (s == "repeat") {
      return true;
   }
   if (s == "until") {
      return true;
   }
   
   
   return false;
}  

// determines if c is a special symbol
bool Compiler::isSpecialSymbol(char c) const {
   if (c == '=') {
      return true;
   }
   if (c == ':') {
      return true;
   }
   if (c == ',') {
      return true;
   }
   if (c == ';') {
      return true;
   }
   if (c == '.') {
      return true;
   }
   if (c == '+') {
      return true;
   }
   if (c == '-') {
      return true;
   }
   if (c == '*') {
      return true;
   }
   if (c == '<') {
      return true;
   }
   if (c == '>') {
      return true;
   }
   if (c == '(') {
      return true;
   }
   if (c == ')') {
      return true;
   }
   
   return false;
} 

// determines if s is a non_key_id
bool Compiler::isNonKeyId(string s) const {
   if (isKeyword(s)) {
      ////cout << "in is NonkeyId function s is: " << s << endl;
      return false;
   }
   for (uint i = 0; i < s.length(); i++) {
      if (i == 0) {
         if (!islower(s[i])) {
            return false;
         }
      }
      if (!(islower(s[i]) || isdigit(s[i]) || s[i] == '_')) {
         return false;
      }
   }
   return true;
}

// determines if s is an integer
bool Compiler::isInteger(string s) const {
   for (uint i = 0; i < s.length(); i++) {
      if (!isdigit(s[i])) {
         return false;
      }
   }
   return true;
}

// determines if s is a boolean
bool Compiler::isBoolean(string s) const {
   if (s == "true" || s == "false") {
      return true;
   } else {
      if (symbolTable.count(s)) {
         if (symbolTable.find(s)->second.getDataType() == BOOLEAN) {
            return true;
         }
         return false;
      }
      return false;
   }
}

// determines if s is a literal
bool Compiler::isLiteral(string s) const {
   string firstThree = s.substr(0,3);
   string firstChar = s.substr(0,1);
   if (s == "true" || s == "false") {
      //////cout << "lit true 1" << endl;
      return true;
   } 
   if (isInteger(s)) {
      //////cout << "lit true 2" << endl;
      return true;
   }
   if (firstThree == "not") {
      string value = s.substr(4);
      if (isBoolean(value)) {
         //////cout << "lit true 3" << endl;
         return true;
      }
   }
   if (firstChar == "+" || firstChar == "-") {
      string value = s.substr(1);
      if (isInteger(value)) {
         //////cout << "lit true 4" << endl;
         return true;
      }
   }
   /*if (s == "while") {
      return true;
   }*/
   //////cout << "in literal s is:" << s <<endl;
   return false;
}

// Action routines
void Compiler::insert(string externalName, storeTypes inType, modes inMode, string inValue, allocation inAlloc, int inUnits) {
   string name, tempname;
   bool oneLeft = false;
   
   size_t comma = externalName.find(",");
   
   
   if (comma != string::npos) {
      name = externalName.substr(0, comma);
      tempname = externalName.substr(comma+1);
      oneLeft = true;
   } else {
      name = externalName;
   }
   
   while (name != "") {
      if (symbolTable.size() > 255) {
         processError("Reached maximum number of variables allowed");
      }
      if (symbolTable.count(name)) {
         string errMsg = "symbol " + name + " is multiply defined";
         processError(errMsg);
      } else if (isKeyword(name)) {
         if (name == "true") {
            symbolTable.insert(pair<string, SymbolTableEntry>(name, SymbolTableEntry("TRUE", inType, inMode, inValue, inAlloc, inUnits)));
         } else if (name == "false") {
            symbolTable.insert(pair<string, SymbolTableEntry>(name, SymbolTableEntry("FALSE", inType, inMode, inValue, inAlloc, inUnits)));
         } else {
            processError("illegal use of keyword");
         }
      } else {
         if (isupper(name[0])) {
            symbolTable.insert(pair<string, SymbolTableEntry>(name, SymbolTableEntry(name, inType, inMode, inValue, inAlloc, inUnits)));
         } else {
            symbolTable.insert(pair<string, SymbolTableEntry>(name, SymbolTableEntry(genInternalName(inType), inType, inMode, inValue, inAlloc, inUnits)));
         }
      }
      comma = tempname.find(",");
      if (comma != string::npos) {
         name = tempname.substr(0, comma);
         tempname = tempname.substr(comma+1);
      } else if (oneLeft) {
         name = tempname;
         oneLeft = false;
      }  else {
         name = "";
      }
   }             
}

// tells which data type a name has
storeTypes Compiler::whichType(string name) {
   storeTypes dataType;
   //////cout << "char:" << ch << "pizzafirst" << endl;
   if (isLiteral(name)) {
      if (isBoolean(name)) {
         dataType = BOOLEAN;
      } else {
         dataType = INTEGER;
      }
   } else {
      if (symbolTable.count(name)) {
         //////////cout << "whichType name found: " << name << endl;
         auto it = symbolTable.find(name);
         dataType = it->second.getDataType();
      } else if (name[0] == 'I') {
         dataType = INTEGER;
      } else if (name[0] == 'B') {
         dataType = BOOLEAN;
      }else {
         //////cout << "in whichType, name is: " << name << endl;
         processError("reference to undefined symbol " + name);
      }
   }
   return dataType;
}

// tells which value a name has
string Compiler::whichValue(string name) {
   string value;
   auto it = symbolTable.find(name);
   if (isLiteral(name))
      value = name;
   else {  //name is an identifier and hopefully a constant
      if(symbolTable.count(name) && (it->second.getValue() != "")) 
         value = it->second.getValue();
      else {
         //////cout << "in whichType, name is: " << name << endl;
         processError("reference to undefined symbol " + name);
      }
   }
   return value;
}

void Compiler::code(string op, string operand1, string operand2) {
   ////cout << "in Code, operand is: " + op << endl;
   if (op == "program") {
      emitPrologue(operand1);
   } else if (op == "end") {
      emitEpilogue();
   } else if (op == "read") {
      emitReadCode(operand1,operand2);
   } else if (op == "write") {
      emitWriteCode(operand1,operand2);
   } else if (op == "+") {
      emitAdditionCode(operand1,operand2);
   } else if (op == "-") {
      emitSubtractionCode(operand1,operand2); 
   } else if (op == "neg") {
      emitNegationCode(operand1,operand2);
   } else if (op == "not") {
      emitNotCode(operand1,operand2);
   } else if (op == "*") {
      emitMultiplicationCode(operand1,operand2);
   } else if (op == "div") {
      emitDivisionCode(operand1,operand2);
   } else if (op == "mod") {
      emitModuloCode(operand1,operand2);
   } else if (op == "=") {
      emitEqualityCode(operand1,operand2);
   } else if (op == "<>") {
      emitInequalityCode(operand1,operand2);
   } else if (op == ":=") {
      emitAssignCode(operand1,operand2);
   } else if (op == "and") {
      emitAndCode(operand1,operand2);
   } else if (op == "or") {
      emitOrCode(operand1,operand2);
   } else if (op == ">") {
      emitGreaterThanCode(operand1,operand2);
   } else if (op == ">=") {
      emitGreaterThanOrEqualToCode(operand1,operand2);
   } else if (op == "<") {
      emitLessThanCode(operand1,operand2);
   } else if (op == "<=") {
      emitLessThanOrEqualToCode(operand1,operand2);
   } else if (op == "then") {
      emitThenCode(operand1,operand2);
   } else if (op == "else") {
      emitElseCode(operand1,operand2);
   } else if (op == "post_if") {
      emitPostIfCode(operand1,operand2);
   } else if (op == "while") {
      ////cout << "in code, for while code operand1 is: " << operand1 << endl;
      emitWhileCode(operand1,operand2);
   } else if (op == "do") {
      emitDoCode(operand1,operand2);
   } else if (op == "post_while") {
      emitPostWhileCode(operand1,operand2);
   } else if (op == "until") {
      emitUntilCode(operand1,operand2);
   } else if (op == "repeat") {
      emitRepeatCode(operand1,operand2);
   } else {
      processError("compiler error since function code should not be called with illegal arguments");
   }
} 

//Julio Perez, 11/9/22, 6:00pm
//push op onto operatorStk
void Compiler::pushOperator(string op) {
   operatorStk.push(op);
}

//Julio Perez, 11/9/22, 6:12pm
//pop op from operatorStk
string Compiler::popOperator() {
   if (operatorStk.empty()) {
      processError("compiler error; operator stack underflow");
   }
   string name = operatorStk.top();
   operatorStk.pop();
   return name;
}

//push name onto operandStk
//if name is a literal, also create a symbol table entry for it
void Compiler::pushOperand(string operand) {
   if (isLiteral(operand)) {
      if (!symbolTable.count(operand)) {
         if (operand == "true") {
            insert("true",BOOLEAN,CONSTANT,"-1",YES,1);
         } else if (operand == "false") {
            insert("false",BOOLEAN,CONSTANT,"0",YES,1);
         } else {
            insert(operand,whichType(operand),CONSTANT,whichValue(operand),YES,1);
         }
      }
   }else if (operand[0] == '.') {//check if the operand is a label
         
   }else {
      if (!symbolTable.count(operand)) {
         ////cout << "in push Operand Operand is:" << operand << endl;
         processError("reference to undefined symbol " + operand);
      }
   }
   
   operandStk.push(operand);
}

//pop name from operandStk
string Compiler::popOperand() {
   if (operandStk.empty()) {
      processError("compiler error; operator stack underflow");
   }
   string name = operandStk.top();
   operandStk.pop();
   return name;
}

// Emit Functions
void Compiler::emit(string label, string instruction, string operands, string comment) {
   objectFile << left;
   objectFile << setw(8) << label << setw(0);
   objectFile << setw(8) << instruction << setw(0);
   objectFile << setw(24) << operands << setw(0);
   objectFile << comment;
   
   // for debuging only 
   // ////////cout << left;
   // ////////cout << "\n" << setw(8) << label << setw(0);
   // ////////cout << setw(8) << instruction << setw(0);
   // ////////cout << setw(24) << operands << setw(0);
   // ////////cout << comment;
   // ////////cout << endl;
}

void Compiler::emitPrologue(string progName, string operand2) {
   time_t now = time (NULL);
   objectFile << "; Bryson Magee, Julio Perez   " << ctime(&now);
   objectFile << "%INCLUDE \"Along32.inc\"" << endl << "%INCLUDE \"Macros_Along.inc\"" << endl << endl;
   // for debugging 
   // ////////cout << "; Bryson Magee, Julio Perez   " << ctime(&now) << endl;
   //////cout << "%INCLUDE \"Along32.inc\"" << endl << "%INCLUDE \"Macros_Along.inc\"" << endl;
   emit("SECTION", ".text");
   objectFile << endl;
   emit("global", "_start", "", "; program " + progName);
   objectFile << endl << endl;
   emit("_start:");
}

void Compiler::emitEpilogue(string operand1, string operand2) {
   objectFile << endl;
   emit("","Exit", "{0}");
   objectFile << endl;
   emitStorage();
}


void Compiler::emitStorage() {
   objectFile << endl;
   emit("SECTION", ".data");
   map<string,SymbolTableEntry>::iterator it;
   for (auto it = symbolTable.begin(); it != symbolTable.end(); it++) {
      if (it->second.getAlloc() == YES && it->second.getMode() == CONSTANT) {
         if (it->second.getValue() == "true") {
            objectFile << endl;
            emit(it->second.getInternalName(), "dd", "-1", "; " + it->first);
         } else if (it->second.getValue() == "false") {
            objectFile << endl;
            emit(it->second.getInternalName(), "dd", "0", "; " + it->first);
         } else {
            objectFile << endl;
            emit(it->second.getInternalName(), "dd", it->second.getValue(), "; " + it->first);
         }
      }
   }
   objectFile << endl;
   objectFile << endl;
   emit("SECTION", ".bss");
   for(auto it = symbolTable.begin(); it != symbolTable.end(); it++) {
      if (it->second.getAlloc() == YES && it->second.getMode() == VARIABLE) {
         objectFile << endl;
         emit(it->second.getInternalName(), "resd", to_string(it->second.getUnits()), "; " + it->first);
      }
   }
   objectFile << endl;
}

void Compiler::emitReadCode(string operand1, string operand2) {
   //This code might need changing
   string name, tempname;
   bool oneLeft = false;
   
   size_t comma = operand1.find(",");
   
   
   if (comma != string::npos) {
      name = operand1.substr(0, comma);
      tempname = operand1.substr(comma+1);
      oneLeft = true;
   } else {
      name = operand1;
   }
   
   //string name = operand1;
   while (name != "") {
      if (!symbolTable.count(name)) {
         processError("reference to undefined variable '" + name + "'");
      }
      if (symbolTable.find(name)->second.getDataType() != INTEGER) {
         processError("can't read variables of this type");
      }
      if (symbolTable.find(name)->second.getMode() != VARIABLE) {
         processError("reading in of read-only location '" + name + "'");
      }
      //emit code to call the Irvine ReadInt function
      //////cout << "pizza?" << endl;
      objectFile << endl;
      emit("","call","ReadInt","; read int; value placed in eax");
      //emit code to store the contents of the A register at name
      //////cout << "pizza?" << endl;
      objectFile << endl;
      emit("", "mov", "[" + symbolTable.find(name)->second.getInternalName() + "],eax", "; store eax at " + name);
      //////cout << "pizza?" << endl;
      //set the contentsOfAReg = name
      contentsOfAReg = name;
      
      comma = tempname.find(",");
      if (comma != string::npos) {
         name = tempname.substr(0, comma);
         tempname = tempname.substr(comma+1);
      } else if (oneLeft) {
         name = tempname;
         oneLeft = false;
      }  else {
         name = "";
      }
   }
   
}

 void Compiler::emitWriteCode(string operand1, string operand2) {
    string name, tempname;
   bool oneLeft = false;
   
   size_t comma = operand1.find(",");
   
   
   if (comma != string::npos) {
      name = operand1.substr(0, comma);
      tempname = operand1.substr(comma+1);
      oneLeft = true;
   } else {
      name = operand1;
   }
   
   //static bool definedStorage = false;
   while (name != "") {
      if (!symbolTable.count(name)) {
         processError("reference to undefined variable '" + name + "'");
      }
      //if name is not in the A register
      if (name != contentsOfAReg) {
         //emit the code to load name in the A register
         objectFile << endl;
         emit("", "mov", "eax,[" + symbolTable.find(name)->second.getInternalName() + "]", "; load " + name + " in eax");
         //set the contentsOfAReg = name
         contentsOfAReg = name;
      }
      if (symbolTable.find(name)->second.getDataType() == INTEGER || symbolTable.find(name)->second.getDataType() == BOOLEAN) {
         //emit code to call the Irvine WriteInt function
         objectFile << endl;
         emit("", "call", "WriteInt", "; write int in eax to standard out");
      }
      //emit code to call the Irvine Crlf function
      objectFile << endl;
      emit("", "call", "Crlf", "; write \\r\\n to standard out");
      comma = tempname.find(",");
      if (comma != string::npos) {
         name = tempname.substr(0, comma);
         tempname = tempname.substr(comma+1);
      } else if (oneLeft) {
         name = tempname;
         oneLeft = false;
      }  else {
         name = "";
      }
   }
}
 
 // op2 = op1
 void Compiler::emitAssignCode(string operand1, string operand2) {
   //////cout << "in emit assign Code, operan1 is "<< operand1 << " operan2 is, " << operand2 << endl;
   if (whichType(operand1) != whichType(operand2)) {
      processError("incompatible types for operator ':='");
   }
   if (symbolTable.find(operand2)->second.getMode() != VARIABLE) {
      processError("symbol on left-hand side of assignment must have a storage mode of VARIABLE");
   }
   if (operand1 == operand2) {
      return;
   }
   if (contentsOfAReg != operand1) {
      // load operand1 in aReg
      objectFile << endl;
      emit("", "mov", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]","; AReg = " + operand1);
   }
   // emit code to store contents of register in memory location of operand2
   objectFile << endl;
   emit("", "mov", "[" + symbolTable.find(operand2)->second.getInternalName() + "],eax", "; " + operand2 + " = AReg");
   contentsOfAReg = operand2;
   //////cout << "operand1: " + operand1 << endl;
   if (isTemporary(operand1)) {
      //////cout << "pizza who?" << endl;
      freeTemp();
   }
}
 
 // op2 + op1
void Compiler::emitAdditionCode(string operand1, string operand2) {
   // Debug
   ////////cout << "operand1: " << operand1 << endl;
   ////////cout << "getInternalName: " << symbolTable.find(operand1)->second.getInternalName() << endl;
   ////////cout << "getDataType: " << symbolTable.find(operand1)->second.getDataType() << endl;
   ////////cout << "getValue: " << symbolTable.find(operand1)->second.getValue() << endl;
   

    if (whichType(operand1) != INTEGER || whichType(operand2) != INTEGER) {
       processError("binary '+' requires integer operands");
    }
    if (isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2) {
       //emit code to store that temp into memory
       objectFile << endl;
       emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
       //change the allocate entry for the temp in the symbol table to yes 
       symbolTable.find(contentsOfAReg)->second.setAlloc(YES);
       //deassign it 
       contentsOfAReg = "";
    }
    //if the A register holds a non-temp not operand1 nor operand2 then deassign it
    if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2) {
       contentsOfAReg = "";
    }
    //if neither operand is in the A register 
    if (contentsOfAReg != operand1 && contentsOfAReg != operand2) {
       //emit code to load operand2 into the A register
       objectFile << endl;
       emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
       contentsOfAReg = operand2;
    }
    //emit code to perform register-memory addition
   if (contentsOfAReg == operand1) {
      objectFile << endl;
      emit("", "add", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + contentsOfAReg + " + " + operand2);
   } else {
      objectFile << endl;
      emit("", "add", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + contentsOfAReg + " + " + operand1);
   }
   if (isTemporary(operand1)) {
      freeTemp();
   }  
   if (isTemporary(operand2)) {
      freeTemp();
   }
   contentsOfAReg = getTemp();
   ////////cout << "emitAdditionCode contentsOfAReg: " << contentsOfAReg << endl;
   symbolTable.find(contentsOfAReg)->second.setDataType(INTEGER);
   ////////cout << "emitAdditionCode contentsOfAReg dataType: " << symbolTable.find(contentsOfAReg)->second.getDataType() << endl;
   pushOperand(contentsOfAReg);
}
 
 // op2 -  op1
 void Compiler::emitSubtractionCode(string operand1, string operand2) {
   if (whichType(operand1) != INTEGER || whichType(operand2) != INTEGER) {
      processError("binary '-' requires integer operands");
   }
	if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2) {
       //emit code to store that temp into memory
      objectFile << endl;
       emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
       //change the allocate entry for the temp in the symbol table to yes 
       symbolTable.find(contentsOfAReg)->second.setAlloc(YES);
       //deassign it 
       contentsOfAReg = "";
    }
	 //if the A register holds a non-temp not operand1 nor operand2 then deassign it
    if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2) {
       contentsOfAReg = "";
    }
	//if neither operand is in the A register 
   if (contentsOfAReg != operand2) {
      //emit code to load operand2 into the A register
      objectFile << endl;
      emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
      contentsOfAReg = operand2;
   }
	//emit code to perform register-memory subtraction
   if (contentsOfAReg == operand1) {
      objectFile << endl;
      emit("", "sub", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + contentsOfAReg + " - " + operand1);
   } else {
      objectFile << endl;
      emit("", "sub", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + contentsOfAReg + " - " + operand1);
   }
   if (isTemporary(operand1)) {
      freeTemp();
   }  
   if (isTemporary(operand2)) {
      freeTemp();
   }
   contentsOfAReg = getTemp();
   symbolTable.find(contentsOfAReg)->second.setDataType(INTEGER);
   pushOperand(contentsOfAReg);
}
 
 // op2 *  op1
 void Compiler::emitMultiplicationCode(string operand1, string operand2) {
    //////cout << "char:" << ch << "pizzafirstfirst" << endl;
    ////////cout << "emitMultiplicationCode: " << operand1 << " " << operand2 << endl;
    if (whichType(operand1) != INTEGER || whichType(operand2) != INTEGER) {
       processError("binary '*' requires integer operands");
    }
    if (isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2) {
       //emit code to store that temp into memory
      objectFile << endl;
       emit("", "mov", "[" + contentsOfAReg + "],eax", "; " + contentsOfAReg + " = AReg");
       //change the allocate entry for the temp in the symbol table to yes 
       symbolTable.find(contentsOfAReg)->second.setAlloc(YES);
       //deassign it 
       contentsOfAReg = "";
    }
    //if the A register holds a non-temp not operand1 nor operand2 then deassign it
    if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2) {
       contentsOfAReg = "";
    }
    //if neither operand is in the A register 
    if (contentsOfAReg != operand1 && contentsOfAReg != operand2) {
       //emit code to load operand2 into the A register
       objectFile << endl;
       emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
       contentsOfAReg = operand2;
    }
    //emit code to perform register-memory multimplication
   if (contentsOfAReg == operand1) {
      objectFile << endl;
      emit("", "imul", "dword [" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + contentsOfAReg + " * " + operand2);
   } else {
      objectFile << endl;
      emit("", "imul", "dword [" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + contentsOfAReg + " * " + operand1);
   }
   if (isTemporary(operand1)) {
      freeTemp();
   }  
   if (isTemporary(operand2)) {
      freeTemp();
   }
   contentsOfAReg = getTemp();
   symbolTable.find(contentsOfAReg)->second.setDataType(INTEGER);
   ////////cout << "in emit multimplication, contents of AReg is: "<< contentsOfAReg << endl;
   pushOperand(contentsOfAReg);
}

// op2 /  op1
void Compiler::emitDivisionCode(string operand1, string operand2) {
   if (whichType(operand1) != INTEGER || whichType(operand2) != INTEGER) {
      processError("binary 'div' requires integer operands");
   }
	if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2) {
       //emit code to store that temp into memory
      objectFile << endl;
       emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
       //change the allocate entry for the temp in the symbol table to yes 
       symbolTable.find(contentsOfAReg)->second.setAlloc(YES);
       //deassign it 
       contentsOfAReg = "";
    }
	 //if the A register holds a non-temp not operand1 nor operand2 then deassign it
    if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2) {
       contentsOfAReg = "";
    }
	//if neither operand is in the A register 
   if (contentsOfAReg != operand2) {
      //emit code to load operand2 into the A register
      objectFile << endl;
      emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
      contentsOfAReg = operand2;
   }
   //Some parts need to be change
   objectFile << endl;
   emit("", "cdq", "", "; sign extend dividend from eax to edx:eax");   
   
	//emit code to perform register-memory division
   if (contentsOfAReg == operand1) {
      objectFile << endl;
      emit("", "idiv", "dword [" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + contentsOfAReg + " div " + operand2);
   } else {
      objectFile << endl;
      emit("", "idiv", "dword [" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + contentsOfAReg + " div " + operand1);
   }
   if (isTemporary(operand1)) {
      freeTemp();
   }  
   if (isTemporary(operand2)) {
      freeTemp();
   }
   contentsOfAReg = getTemp();
   symbolTable.find(contentsOfAReg)->second.setDataType(INTEGER);
   pushOperand(contentsOfAReg);
   
}

// op2 %  op1
void Compiler::emitModuloCode(string operand1, string operand2) {
   if (whichType(operand1) != INTEGER || whichType(operand2) != INTEGER) {
      processError("binary 'mod' requires integer operands");
   }
	if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2) {
       //emit code to store that temp into memory
      objectFile << endl;
       emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
       //change the allocate entry for the temp in the symbol table to yes 
       symbolTable.find(contentsOfAReg)->second.setAlloc(YES);
       //deassign it 
       contentsOfAReg = "";
    }
	 //if the A register holds a non-temp not operand1 nor operand2 then deassign it
    if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2) {
       contentsOfAReg = "";
    }
	//if neither operand is in the A register 
   if (contentsOfAReg != operand2) {
      //emit code to load operand2 into the A register
      objectFile << endl;
      emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
      contentsOfAReg = operand2;
   }
   //Some parts need to be change
   objectFile << endl;
   emit("", "cdq", "", "; sign extend dividend from eax to edx:eax");   
   
	//emit code to perform register-memory modulo
   if (contentsOfAReg == operand1) {
      objectFile << endl;
      emit("", "idiv", "dword [" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + contentsOfAReg + " div " + operand2);
      
      objectFile << endl;
      emit("", "xchg", "eax,edx", "; exchange quotient and remainder");
   } else {
      objectFile << endl;
      emit("", "idiv", "dword [" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + contentsOfAReg + " div " + operand1);
      
      objectFile << endl;
      emit("", "xchg", "eax,edx", "; exchange quotient and remainder");
   }
   if (isTemporary(operand1)) {
      freeTemp();
   }  
   if (isTemporary(operand2)) {
      freeTemp();
   }
   contentsOfAReg = getTemp();
   symbolTable.find(contentsOfAReg)->second.setDataType(INTEGER);
   pushOperand(contentsOfAReg);
   
}

// -op1
void Compiler::emitNegationCode(string operand1, string operand2) {
   ////////cout << "in operand1: " << operand1;
   if (whichType(operand1) != INTEGER) {
      processError("unary '-' requires an integer operand");
   }
	if (isTemporary(contentsOfAReg) && contentsOfAReg != operand1) {
       //emit code to store that temp into memory
      objectFile << endl;
       emit("", "mov", "[" + contentsOfAReg + "],eax", "; " + contentsOfAReg + " = AReg");
       //change the allocate entry for the temp in the symbol table to yes 
       symbolTable.find(contentsOfAReg)->second.setAlloc(YES);
       //deassign it 
       contentsOfAReg = "";
    }
	 //if the A register holds a non-temp not operand1 nor operand2 then deassign it
    if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand1) {
       contentsOfAReg = "";
    }
	//if neither operand is in the A register 
   if (contentsOfAReg != operand1) {
      //emit code to load operand2 into the A register
      objectFile << endl;
      emit("", "mov", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + operand1);
      contentsOfAReg = operand1;
   }
   //Some parts need to be change
   //objectFile << endl;
   //emit("", "cdq", "", "; sign extend dividend from eax to edx:eax");   
   
	//emit code to perform register-memory negate
   /*if (contentsOfAReg == operand1) {
      objectFile << endl;
      emit("", "idiv", "dword [" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + contentsOfAReg + " div " + operand2);
      
      objectFile << endl;
      emit("", "xchg", "eax, edx", "; exchange quotient and remainder");
   } else {
      objectFile << endl;
      emit("", "idiv", "dword [" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + contentsOfAReg + " div " + operand1);
      
      objectFile << endl;
      emit("", "xchg", "eax, edx", "; exchange quotient and remainder");
   }*/
   
   objectFile << endl;
   emit("", "neg", "eax", "; AReg = -AReg");   
   
   if (isTemporary(operand1)) {
      freeTemp();
   }  
   if (isTemporary(operand2)) {
      freeTemp();
   }
   contentsOfAReg = getTemp();
   symbolTable.find(contentsOfAReg)->second.setDataType(INTEGER);
   pushOperand(contentsOfAReg);
}

// !op1
void Compiler::emitNotCode(string operand1, string operand2) {
   ////////cout << "in emitNotCode, operand1 is: " << operand1 << endl;
   if (whichType(operand1) != BOOLEAN) {
      processError("unary 'not' requires a boolean operand");
   }
	if (isTemporary(contentsOfAReg) && contentsOfAReg != operand1) {
       //emit code to store that temp into memory
      objectFile << endl;
       emit("", "mov", "[" + contentsOfAReg + "],eax", "; " + contentsOfAReg + " = AReg");
       //change the allocate entry for the temp in the symbol table to yes 
       symbolTable.find(contentsOfAReg)->second.setAlloc(YES);
       //deassign it 
       contentsOfAReg = "";
    }
	 //if the A register holds a non-temp not operand1 nor operand2 then deassign it
    if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand1) {
       contentsOfAReg = "";
    }
	//if contents of AReg does not equal to operand1
   if (contentsOfAReg != operand1) {
      //emit code to load operand2 into the A register
      objectFile << endl;
      emit("", "mov", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + operand1);
      contentsOfAReg = operand1;
   }
   
   //emit not
   //////cout << "hello" << endl;
   objectFile << endl;
   emit("", "not", "eax" , "; AReg = !AReg");
   
   
   //labelCount += 1;
   
	//emit code to perform register-memory and
   if (contentsOfAReg == operand1) {
      //objectFile << endl;
      //emit("", "and", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + operand2 + " and " + operand1);
      
   } else {
      //objectFile << endl;
      //emit("", "and", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + operand2 + " and " + operand1);
   }
   if (isTemporary(operand1)) {
      freeTemp();
   }  
   if (isTemporary(operand2)) {
      freeTemp();
   }
   contentsOfAReg = getTemp();
   symbolTable.find(contentsOfAReg)->second.setDataType(BOOLEAN);
   pushOperand(contentsOfAReg);
   //labelCount += 1;
}

// op2 && op1
void Compiler::emitAndCode(string operand1, string operand2) {
   //////cout << whichType(operand1) << endl;
   //////cout << operand2 << " " << whichType(operand2) << endl;
   if (whichType(operand1) != BOOLEAN || whichType(operand2) != BOOLEAN) {
      processError("binary 'and' requires boolean operands");
   }
	if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2 && contentsOfAReg != operand1) {
       //emit code to store that temp into memory
      objectFile << endl;
       emit("", "mov", "[" + contentsOfAReg + "],eax", "; " + contentsOfAReg + " = AReg");
       //change the allocate entry for the temp in the symbol table to yes 
       symbolTable.find(contentsOfAReg)->second.setAlloc(YES);
       //deassign it 
       contentsOfAReg = "";
    }
	 //if the A register holds a non-temp not operand1 nor operand2 then deassign it
    if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2 && contentsOfAReg != operand1) {
       contentsOfAReg = "";
    }
	//if neither operand is in the A register 
   if (contentsOfAReg != operand2 && contentsOfAReg != operand1) {
      //emit code to load operand2 into the A register
      objectFile << endl;
      emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
      contentsOfAReg = operand2;
   }
   
	//emit code to perform register-memory and
   if (contentsOfAReg == operand1) {
      objectFile << endl;
      emit("", "and", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand1 + " and " + operand2);
      
   } else {
      objectFile << endl;
      emit("", "and", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + operand2 + " and " + operand1);
   }
   if (isTemporary(operand1)) {
      freeTemp();
   }  
   if (isTemporary(operand2)) {
      freeTemp();
   }
   contentsOfAReg = getTemp();
   symbolTable.find(contentsOfAReg)->second.setDataType(BOOLEAN);
   pushOperand(contentsOfAReg);
   
}

// op2 || op1
void Compiler::emitOrCode(string operand1, string operand2) {
   if (whichType(operand1) != BOOLEAN || whichType(operand2) != BOOLEAN) {
      processError("illegal type in emitOrCode");
   }
	if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2 && contentsOfAReg != operand1) {
       //emit code to store that temp into memory
      objectFile << endl;
       emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
       //change the allocate entry for the temp in the symbol table to yes 
       symbolTable.find(contentsOfAReg)->second.setAlloc(YES);
       //deassign it 
       contentsOfAReg = "";
    }
	 //if the A register holds a non-temp not operand1 nor operand2 then deassign it
    if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2 && contentsOfAReg != operand1) {
       contentsOfAReg = "";
    }
	//if neither operand is in the A register 
   if (contentsOfAReg != operand2 && contentsOfAReg != operand1) {
      //emit code to load operand2 into the A register
      objectFile << endl;
      emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
      contentsOfAReg = operand2;
   }
   
	//emit code to perform register-memory and
   if (contentsOfAReg == operand1) {
      objectFile << endl;
      emit("", "or", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand1 + " or " + operand2);
      
   } else {
      objectFile << endl;
      emit("", "or", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + operand2 + " or " + operand1);
   }
   if (isTemporary(operand1)) {
      freeTemp();
   }  
   if (isTemporary(operand2)) {
      freeTemp();
   }
   contentsOfAReg = getTemp();
   symbolTable.find(contentsOfAReg)->second.setDataType(BOOLEAN);
   
   
   pushOperand(contentsOfAReg);
}

// op2 == op1
void Compiler::emitEqualityCode(string operand1, string operand2) {
   if (whichType(operand1) != whichType(operand2)) {
      processError("binary '=' requires operands of the same type");
   }
   if (isTemporary(contentsOfAReg) && (contentsOfAReg != operand2 && contentsOfAReg != operand1)) {
       //emit code to store that temp into memory
      objectFile << endl;
       emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
       //change the allocate entry for the temp in the symbol table to yes 
       symbolTable.find(contentsOfAReg)->second.setAlloc(YES);
       //deassign it 
       contentsOfAReg = "";
    }
   //if the A register holds a non-temp not operand1 nor operand2 then deassign it
   if (!isTemporary(contentsOfAReg) && (contentsOfAReg != operand2 && contentsOfAReg != operand1)) {
      contentsOfAReg = "";
   }
   if (contentsOfAReg != operand2 && contentsOfAReg != operand1) {
      //////cout << "pizzaWoman " << operand1 << " " << operand2 << " " << contentsOfAReg << endl;
      objectFile << endl;
      emit("", "mov","eax,["+symbolTable.find(operand2)->second.getInternalName()+"]","; AReg = " + operand2);
      contentsOfAReg = operand2;
   }
   if (contentsOfAReg == operand2) {
      objectFile << endl;
      emit("", "cmp", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]" , "; compare " + contentsOfAReg + " and " + operand1);
      objectFile << endl;
      emit("", "je", getLabel() , "; if " + contentsOfAReg + " = " + operand1 + " then jump to set eax to TRUE");
      labelCount += 1;
   } else {
      objectFile << endl;
      emit("", "cmp", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]" , "; compare " + contentsOfAReg + " and " + operand2);
      objectFile << endl;
      emit("", "je", getLabel() , "; if " + operand2 + " = " + contentsOfAReg + " then jump to set eax to TRUE");
      labelCount += 1;
   }
   objectFile << endl;
   emit("", "mov", "eax,[FALSE]" , "; else set eax to FALSE");
   if(!symbolTable.count("false")) {
      insert("false",BOOLEAN,CONSTANT,"0",YES,1);
   }
   objectFile << endl;
   emit("", "jmp", getLabel() , "; unconditionally jump");
   labelCount -= 1;
   objectFile << endl;
   emit(getLabel() + ":");
   objectFile << endl;
   emit("", "mov", "eax,[TRUE]" , "; set eax to TRUE");
   if(!symbolTable.count("true")) {
      insert("true",BOOLEAN,CONSTANT,"-1",YES,1);
   }
   labelCount += 1;
   objectFile << endl;
   emit(getLabel() + ":");
   if (isTemporary(operand1)) {
      freeTemp();
   }  
   if (isTemporary(operand2)) {
      freeTemp();
   }
   contentsOfAReg = getTemp();
   symbolTable.find(contentsOfAReg)->second.setDataType(BOOLEAN);
   pushOperand(contentsOfAReg);
   labelCount += 1;
}

// op2 != op1
void Compiler::emitInequalityCode(string operand1, string operand2) {
   if (whichType(operand1) != whichType(operand2)) {
      processError("binary '<>' requires operands of the same type");
   }
   //cout << "pizza operan1: " << operand1 << " " << operand2 << " " << contentsOfAReg << endl;
   if (isTemporary(contentsOfAReg) && !(contentsOfAReg == operand2 || contentsOfAReg == operand1)) {
       //emit code to store that temp into memory
       //////cout << " pizzaguy is marching" << endl;
      objectFile << endl;
       emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
       //change the allocate entry for the temp in the symbol table to yes 
       symbolTable.find(contentsOfAReg)->second.setAlloc(YES);
       //deassign it 
       contentsOfAReg = "";
    }
   //if the A register holds a non-temp not operand1 nor operand2 then deassign it
   if (!isTemporary(contentsOfAReg) && !(contentsOfAReg == operand2 || contentsOfAReg == operand1)) {
      //////cout << " pizzaguy is marching faster" << endl;
      contentsOfAReg = "";
   }
   if (contentsOfAReg != operand2 && contentsOfAReg != operand1) {
      objectFile << endl;
      //////cout << "pizzaguy: " << operand1 << " " << operand2 << " " << operand2 << endl;
      emit("", "mov","eax,["+symbolTable.find(operand2)->second.getInternalName()+"]","; AReg = " + operand2);
      contentsOfAReg = operand2;
   }
   if (contentsOfAReg == operand2) {
      objectFile << endl;
      emit("", "cmp", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]" , "; compare " + contentsOfAReg + " and " + operand1);
      objectFile << endl;
      emit("", "jne", getLabel() , "; if " + contentsOfAReg + " <> " + operand1 + " then jump to set eax to TRUE");
      labelCount += 1;
   } else {
      objectFile << endl;
      emit("", "cmp", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]" , "; compare " + contentsOfAReg + " and " + operand2);
      objectFile << endl;
      emit("", "jne", getLabel() , "; if " + operand2 + " <> " + contentsOfAReg + " then jump to set eax to TRUE");
      labelCount += 1;
   }
   objectFile << endl;
   emit("", "mov", "eax,[FALSE]" , "; else set eax to FALSE");
   if(!symbolTable.count("false")) {
      insert("false",BOOLEAN,CONSTANT,"0",YES,1);
   }
   objectFile << endl;
   emit("", "jmp", getLabel() , "; unconditionally jump");
   labelCount -= 1;
   objectFile << endl;
   emit(getLabel() + ":");
   objectFile << endl;
   emit("", "mov", "eax,[TRUE]" , "; set eax to TRUE");
   if(!symbolTable.count("true")) {
      insert("true",BOOLEAN,CONSTANT,"-1",YES,1);
   }
   labelCount += 1;
   objectFile << endl;
   emit(getLabel() + ":");
   if (isTemporary(operand1)) {
      freeTemp();
   }  
   if (isTemporary(operand2)) {
      freeTemp();
   }
   contentsOfAReg = getTemp();
   symbolTable.find(contentsOfAReg)->second.setDataType(BOOLEAN);
   pushOperand(contentsOfAReg);
   labelCount += 1;
}

// op2 <  op1
void Compiler::emitLessThanCode(string operand1, string operand2) {
   //cout << "operand1 is " << operand1 << endl;
   //cout << "operand2 is " << operand2 << endl;
   //cout << "Areg is " << contentsOfAReg << endl;
   if (whichType(operand1) != INTEGER || whichType(operand2) != INTEGER) {
      processError("binary '<' requires integer operands");
   }
	if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2 && contentsOfAReg != operand1) {
       //emit code to store that temp into memory
      objectFile << endl;
       emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
       //change the allocate entry for the temp in the symbol table to yes 
       symbolTable.find(contentsOfAReg)->second.setAlloc(YES);
       //deassign it 
       contentsOfAReg = "";
    }
	 //if the A register holds a non-temp not operand1 nor operand2 then deassign it
    if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2 && contentsOfAReg != operand1) {
       contentsOfAReg = "";
    }
	//if neither operand is in the A register 
   if (contentsOfAReg != operand2) {
      //emit code to load operand2 into the A register
      objectFile << endl;
      emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
      contentsOfAReg = operand2;
   }
   //////cout << "we are here" << endl;
   objectFile << endl;
   emit("", "cmp", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]" , "; compare " + contentsOfAReg + " and " + operand1);
   objectFile << endl;
   emit("", "jl", getLabel() , "; if " + contentsOfAReg + " < " + operand1 + " then jump to set eax to TRUE");
   labelCount += 1;
   objectFile << endl;
   emit("", "mov", "eax,[FALSE]" , "; else set eax to FALSE");
   //////cout << "we are here" << endl;
   
   //if symboltable does not have false
   if(!symbolTable.count("false")) {
      insert("false",BOOLEAN,CONSTANT,"0",YES,1);
   }
   objectFile << endl;
   emit("", "jmp", getLabel() , "; unconditionally jump");
   //////cout << "now we are here" << endl;
   labelCount -= 1;
   objectFile << endl;
   emit(getLabel() + ":");
   objectFile << endl;
   emit("", "mov", "eax,[TRUE]" , "; set eax to TRUE");
   
   //if symbol im lazy figure it out
   if(!symbolTable.count("true")) {
      insert("true",BOOLEAN,CONSTANT,"-1",YES,1);
   }
   labelCount += 1;
   objectFile << endl;
   emit(getLabel() + ":");
   if (isTemporary(operand1)) {
      freeTemp();
   }  
   if (isTemporary(operand2)) {
      freeTemp();
   }
   contentsOfAReg = getTemp();
   symbolTable.find(contentsOfAReg)->second.setDataType(BOOLEAN);
   pushOperand(contentsOfAReg);
   labelCount += 1;
}

// op2 <= op1
void Compiler::emitLessThanOrEqualToCode(string operand1, string operand2) {
   ////////cout << "operand1 is " << operand1 << endl;
   ////////cout << "operand2 is " << operand2 << endl;
   ////////cout << "Areg is " << contentsOfAReg << endl;
   if (whichType(operand1) != INTEGER || whichType(operand2) != INTEGER) {
      processError("{binary '<=' requires integer operands");
   }
	if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2 && contentsOfAReg != operand1) {
       //emit code to store that temp into memory
      objectFile << endl;
       emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
       //change the allocate entry for the temp in the symbol table to yes 
       symbolTable.find(contentsOfAReg)->second.setAlloc(YES);
       //deassign it 
       contentsOfAReg = "";
    }
	 //if the A register holds a non-temp not operand1 nor operand2 then deassign it
    if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2 && contentsOfAReg != operand1) {
       contentsOfAReg = "";
    }
	//if neither operand is in the A register 
   if (contentsOfAReg != operand2 && contentsOfAReg != operand1) {
      //emit code to load operand2 into the A register
      objectFile << endl;
      emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
      contentsOfAReg = operand2;
   }
   //////cout << "we are here" << endl;
   objectFile << endl;
   emit("", "cmp", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]" , "; compare " + contentsOfAReg + " and " + operand1);
   objectFile << endl;
   emit("", "jle", getLabel() , "; if " + contentsOfAReg + " <= " + operand1 + " then jump to set eax to TRUE");
   labelCount += 1;
   objectFile << endl;
   emit("", "mov", "eax,[FALSE]" , "; else set eax to FALSE");
   //////cout << "we are here" << endl;
   
   //if symboltable does not have false
   if(!symbolTable.count("false")) {
      insert("false",BOOLEAN,CONSTANT,"0",YES,1);
   }
   objectFile << endl;
   emit("", "jmp", getLabel() , "; unconditionally jump");
   //////cout << "now we are here" << endl;
   labelCount -= 1;
   objectFile << endl;
   emit(getLabel() + ":");
   objectFile << endl;
   emit("", "mov", "eax,[TRUE]" , "; set eax to TRUE");
   
   //if symbol im lazy figure it out
   if(!symbolTable.count("true")) {
      insert("true",BOOLEAN,CONSTANT,"-1",YES,1);
   }
   labelCount += 1;
   objectFile << endl;
   emit(getLabel() + ":");
   if (isTemporary(operand1)) {
      freeTemp();
   }  
   if (isTemporary(operand2)) {
      freeTemp();
   }
   contentsOfAReg = getTemp();
   symbolTable.find(contentsOfAReg)->second.setDataType(BOOLEAN);
   pushOperand(contentsOfAReg);
   labelCount += 1;
}

// op2 >  op1
void Compiler::emitGreaterThanCode(string operand1, string operand2) {
   ////////cout << "operand1 is " << operand1 << endl;
   ////////cout << "operand2 is " << operand2 << endl;
   ////////cout << "Areg is " << contentsOfAReg << endl;
   if (whichType(operand1) != INTEGER || whichType(operand2) != INTEGER) {
      processError("binary '>' requires integer operands");
   }
	if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2 && contentsOfAReg != operand1) {
       //emit code to store that temp into memory
      objectFile << endl;
       emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
       //change the allocate entry for the temp in the symbol table to yes 
       symbolTable.find(contentsOfAReg)->second.setAlloc(YES);
       //deassign it 
       contentsOfAReg = "";
    }
	 //if the A register holds a non-temp not operand1 nor operand2 then deassign it
    if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2 && contentsOfAReg != operand1) {
       contentsOfAReg = "";
    }
	//if neither operand is in the A register 
   if (contentsOfAReg != operand2 && contentsOfAReg != operand1) {
      //emit code to load operand2 into the A register
      objectFile << endl;
      emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
      contentsOfAReg = operand2;
   }
   //////cout << "we are here" << endl;
   objectFile << endl;
   emit("", "cmp", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]" , "; compare " + contentsOfAReg + " and " + operand1);
   objectFile << endl;
   emit("", "jg", getLabel() , "; if " + contentsOfAReg + " > " + operand1 + " then jump to set eax to TRUE");
   labelCount += 1;
   objectFile << endl;
   emit("", "mov", "eax,[FALSE]" , "; else set eax to FALSE");
   //////cout << "we are here" << endl;
   
   //if symboltable does not have false
   if(!symbolTable.count("false")) {
      insert("false",BOOLEAN,CONSTANT,"0",YES,1);
   }
   objectFile << endl;
   emit("", "jmp", getLabel() , "; unconditionally jump");
   //////cout << "now we are here" << endl;
   labelCount -= 1;
   objectFile << endl;
   emit(getLabel() + ":");
   objectFile << endl;
   emit("", "mov", "eax,[TRUE]" , "; set eax to TRUE");
   
   //if symbol im lazy figure it out
   if(!symbolTable.count("true")) {
      insert("true",BOOLEAN,CONSTANT,"-1",YES,1);
   }
   labelCount += 1;
   objectFile << endl;
   emit(getLabel() + ":");
   if (isTemporary(operand1)) {
      freeTemp();
   }  
   if (isTemporary(operand2)) {
      freeTemp();
   }
   contentsOfAReg = getTemp();
   symbolTable.find(contentsOfAReg)->second.setDataType(BOOLEAN);
   pushOperand(contentsOfAReg);
   labelCount += 1;
}

// op2 >= op1
void Compiler::emitGreaterThanOrEqualToCode(string operand1, string operand2) {
   ////////cout << "operand1 is " << operand1 << endl;
   ////////cout << "operand2 is " << operand2 << endl;
   ////////cout << "Areg is " << contentsOfAReg << endl;
   if (whichType(operand1) != INTEGER || whichType(operand2) != INTEGER) {
      processError("illegal type in emitLessThan");
   }
	if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2 && contentsOfAReg != operand1) {
       //emit code to store that temp into memory
      objectFile << endl;
       emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
       //change the allocate entry for the temp in the symbol table to yes 
       symbolTable.find(contentsOfAReg)->second.setAlloc(YES);
       //deassign it 
       contentsOfAReg = "";
    }
	 //if the A register holds a non-temp not operand1 nor operand2 then deassign it
    if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2 && contentsOfAReg != operand1) {
       contentsOfAReg = "";
    }
	//if neither operand is in the A register 
   if (contentsOfAReg != operand2 && contentsOfAReg != operand1) {
      //emit code to load operand2 into the A register
      objectFile << endl;
      emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
      contentsOfAReg = operand2;
   }
   //////cout << "we are here" << endl;
   objectFile << endl;
   emit("", "cmp", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]" , "; compare " + contentsOfAReg + " and " + operand1);
   objectFile << endl;
   emit("", "jge", getLabel() , "; if " + contentsOfAReg + " >= " + operand1 + " then jump to set eax to TRUE");
   labelCount += 1;
   objectFile << endl;
   emit("", "mov", "eax,[FALSE]" , "; else set eax to FALSE");
   //////cout << "we are here" << endl;
   
   //if symboltable does not have false
   if(!symbolTable.count("false")) {
      insert("false",BOOLEAN,CONSTANT,"0",YES,1);
   }
   objectFile << endl;
   emit("", "jmp", getLabel() , "; unconditionally jump");
   //////cout << "now we are here" << endl;
   labelCount -= 1;
   objectFile << endl;
   emit(getLabel() + ":");
   objectFile << endl;
   emit("", "mov", "eax,[TRUE]" , "; set eax to TRUE");
   
   //if symbol im lazy figure it out
   if(!symbolTable.count("true")) {
      insert("true",BOOLEAN,CONSTANT,"-1",YES,1);
   }
   labelCount += 1;
   objectFile << endl;
   emit(getLabel() + ":");
   if (isTemporary(operand1)) {
      freeTemp();
   }  
   if (isTemporary(operand2)) {
      freeTemp();
   }
   contentsOfAReg = getTemp();
   symbolTable.find(contentsOfAReg)->second.setDataType(BOOLEAN);
   pushOperand(contentsOfAReg);
   labelCount += 1;
}

// Emit functions for Stage 2
// emit code which follows 'then' and statement predicate
void Compiler::emitThenCode(string operand1, string operand2) {
   //cout << "operand1: " << operand1 << " Operand2: " << operand2 << " Areg: " << contentsOfAReg << endl;
   string tempLabel;
   if (whichType(operand1) != BOOLEAN) {
      processError("if predicate must be of type boolean");
   }
   tempLabel = getLabel();
   if (operand1 != contentsOfAReg) {
	  objectFile << endl;
      emit("", "mov", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + operand1);
   }
   
   objectFile << endl;
   //emit instruction to compare the A register to zero (false)
   emit("", "cmp", "eax,0" , "; compare eax to 0");
   
   
   objectFile << endl;
   //emit code to branch to tempLabel if the compare indicates equality
   emit("", "je", tempLabel , "; if " + operand1 +" is false then jump to end of if");
   
   pushOperand(tempLabel);
   
   if(isTemporary(operand1)) {
      freeTemp();
   }
   
   contentsOfAReg = "";
   labelCount += 1;
}

// emit code which follows 'else' clause of 'if' statement
void Compiler::emitElseCode(string operand1, string operand2) {
   string tempLabel;
   tempLabel = getLabel();
   
   objectFile << endl;
   //emit instruction to branch unconditionally to tempLabel
   emit("", "jmp", tempLabel , "; jump to end if");
   
   objectFile << endl;
   //emit instruction to label this point of object code with the argument operand1
   emit(operand1 + ":", "", "" , "; else");
   
   pushOperand(tempLabel);
   
   if(isTemporary(operand1)) {
      freeTemp();
   }
   if (isTemporary(operand2)) {
      freeTemp();
   }
   
   contentsOfAReg = "";
   labelCount += 1;
}

// emit code which follows end of 'if' statement
void Compiler::emitPostIfCode(string operand1, string operand2) {
   
   objectFile << endl;
   //emit instruction to label this point of object code with the argument operand1
   emit(operand1 + ":", "", "" , "; end if");
   
   if(isTemporary(operand1)) {
      freeTemp();
   }
   if (isTemporary(operand2)) {
      freeTemp();
   }
   
   contentsOfAReg = "";
   
}


// emit code following 'while'
void Compiler::emitWhileCode(string operand1, string operand2) {
   string tempLabel;
   tempLabel = getLabel();
   //emit instruction to label this point of object code as tempLabel
   objectFile << endl;
   emit(tempLabel + ":", "", "" , "; while");
   
   ////cout << "in emitWhileCode, tempLabel is: " << tempLabel << endl;
   
   //push tempLabel onto operandStk
   pushOperand(tempLabel);
   contentsOfAReg = "";
   labelCount += 1;
}

// emit code following 'do'
void Compiler::emitDoCode(string operand1, string operand2) {
   string tempLabel;
   if (whichType(operand1) != BOOLEAN) {
      processError("while predicate must be of type boolean");
   }
   tempLabel = getLabel();
   if (operand1 != contentsOfAReg) {
      objectFile << endl;
      emit("", "mov", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + operand1);
   }
   
   objectFile << endl;
   //emit instruction to compare the A register to zero (false)
   emit("", "cmp", "eax,0" , "; compare eax to 0");
   
   //emit code to branch to tempLabel if the compare indicates equality
   objectFile << endl;
   emit("", "je", tempLabel , "; if " + operand1 + " is false then jump to end while");
   
   //push tempLabel onto operandStk
   pushOperand(tempLabel);
   
   if(isTemporary(operand1)) {
      freeTemp();
   }
   
   contentsOfAReg = "";
   labelCount += 1;
}

// emit code at end of 'while' loop;
// operand2 is the label of the beginning of the loop
// operand1 is the label which should follow the end of the loop
void Compiler::emitPostWhileCode(string operand1, string operand2) {
   objectFile << endl;
   //emit instruction which branches unconditionally to the beginning of the loop, i.e., to the value of operand2
   emit("", "jmp", operand1 , "; end while");
   
   objectFile << endl;
   //emit instruction which labels this point of the object code with the argument operand1
   emit(operand2 + ":", "", "" , "");
   contentsOfAReg = "";
}

// emit code which follows 'repeat'
void Compiler::emitRepeatCode(string operand1, string operand2) {
   string tempLabel;
   tempLabel = getLabel();
   objectFile << endl;
   //emit instruction to label this point in the object code with the value of tempLabel
   emit(tempLabel + ":", "", "" , "; repeat");
   //push tempLabel onto operandStk
   pushOperand(tempLabel);
   contentsOfAReg = "";
   labelCount += 1;
}

// emit code which follows 'until' and the predicate of loop
// operand1 is the value of the predicate
// operand2 is the label which points to the beginning of the loop
void Compiler::emitUntilCode(string operand1, string operand2) {
   if (whichType(operand1) != BOOLEAN) {
      processError("while predicate must be of type boolean");
   }
   
   if (operand1 != contentsOfAReg) {
      objectFile << endl;
      emit("", "mov", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + operand1);
   }
   
   objectFile << endl;
   //emit instruction to compare the A register to zero (false)
   emit("", "cmp", "eax,0" , "; compare eax to 0");
   
   //emit code to branch to operand2 if the compare indicates equality
   objectFile << endl;
   emit("", "je", operand2 , "; until " + operand1 + " is true");
   
   if(isTemporary(operand1)) {
      freeTemp();
   }
   
   contentsOfAReg = "";
}



// Lexical routines
// returns the next character or END_OF_FILE marker
char Compiler::nextChar() {
   char nextChar;
   static char lastChar;
   sourceFile.get(nextChar);
   
   if (lineNo == 0) {
      lastChar = '\n';
   }
   
   if (sourceFile.eof()) {
      ch = END_OF_FILE;
   } else {
      ch = nextChar;
      if (lastChar == '\n') {
         lineNo += 1;
         listingFile << right << setw(5) << lineNo << "|" << left << setw(0);
         listingFile << ch;
      } else {
         listingFile << ch;
      }
   }
   lastChar = ch;
   return ch;
}    

// returns the next token or END_OF_FILE marker
string Compiler::nextToken() {
   token = "";
   while (token == "") {
         if (ch == '{') {
            while (!(nextChar() == END_OF_FILE || ch == '}')) {
            }
            if (ch == END_OF_FILE) {
               processError("unexpected end of file");
            } else {
               nextChar();
            }
         } else if (ch == '}') {
            processError("'}' cannot begin token");
         } else if (isspace(ch)) {
            nextChar();
         } else if (isSpecialSymbol(ch)) {
            token = ch;
            //////cout << "char:" << ch << "pizzaChar" << endl;
            //nextChar();
            if (token == ":") {
               if (nextChar() == '=') {
                  token += ch;
                  nextChar();
               }
            } else if (token == "<") {
               if (nextChar() == '=' || ch == '>') {
                  token += ch;
                  nextChar();
               }
            } else if (token == ">") {
               if (nextChar() == '=') {
                  token += ch;
                  nextChar();
               }
            } else {
               nextChar();
            }
            
         } else if (islower(ch)) {
            token = ch;
            int count = 0;
            static char lastChar = ch;
            while ((isalpha(nextChar()) || isdigit(ch) || ch == '_') && ch != END_OF_FILE) {
               count += 1;
               if (! (count >= 15)) {
                  if (ch == '_' && lastChar == '_') {
                     processError("Expect number or letter after underscore");
                  }
                  lastChar = ch;
                  token += ch;
               }
            }
            if (ch == END_OF_FILE) {
               processError("unexpected end of file");
            }
            if (token[token.length() - 1] == '_') {
               processError("names cannot end with '_'");
            }
         } else if (isdigit(ch)) {
            token = ch;
            while (isdigit(nextChar()) && ch != END_OF_FILE) {
               token += ch;
            }
         } else if (ch == END_OF_FILE) {
            token = ch;
         } else {
            
            if (ch == '_') {
               processError("tokens can not begin with '_'");
            }
            
            if (isupper(ch)) {
               processError("capital letters not allowed");
            }
            processError("illegal symbol");
         }
      }
   // debug only
   //////////cout << endl << token << endl;
   return token;
}

//Other routines
string Compiler::genInternalName(storeTypes stype) const {
   static int intCount = 0;
   static int boolCount = 0;
   string name;
   if (stype == INTEGER) {
      name = "I" + to_string(intCount);
      intCount += 1;
   }
   if (stype == BOOLEAN) {
      name = "B" + to_string(boolCount);
      boolCount += 1;
   }
   //New code
   if (stype == PROG_NAME) {
      name = "P" + to_string(0);
   }
   
   return name;
}

void Compiler::processError(string err) {
   objectFile << endl;
   listingFile << endl << "Error: Line " << lineNo << ": " << err << endl;
   errorCount += 1;
   createListingTrailer();
   exit(0);
}

void Compiler::freeTemp() {
   currentTempNo -= 1;
   if (currentTempNo < -1) {
      processError("Compiler error, currentTempNo should be >= -1");
   }
}

string Compiler::getTemp() {
   string temp;
   currentTempNo += 1;
   temp = "T" + to_string(currentTempNo);
   if (currentTempNo > maxTempNo) {
      insert(temp, UNKNOWN, VARIABLE, "", NO, 1);
      maxTempNo += 1;
   }
   return temp;
}

string Compiler::getLabel() {
   return ".L" + to_string(labelCount);
}

// determines if s represents a temporary
bool Compiler::isTemporary(string s) const {
   if (symbolTable.count(s)) {
         if (symbolTable.find(s)->second.getInternalName()[0] == 'T' && symbolTable.find(s)->second.getInternalName()[1] != 'R') {
            return true;
         }
         return false;
    }
    
	return false;
}

//stage1, production 16
bool relOp(string token) {
   if (token == "=" || token == "<>" || token == "<=" || token == ">=" || token == "<" || token == ">") {
      return true;
   } else {
       return false;
   }
}

//stage1, production 17
bool addLevelOp(string token) {
   if (token == "+" || token == "-" || token == "or") {
      return true;
   } else {
      return false;
   }
}

//stage1, production 18
bool multLevelOp(string token) {
   if (token == "*" || token == "div" || token == "mod" || token == "and") {
      return true;
   } else {
       return false;
   }
}