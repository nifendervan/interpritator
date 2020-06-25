#include "lexer.h"

#include <algorithm>
#include <iostream>
#include <charconv>
#include <unordered_map>

using namespace std;

namespace Parse {

bool operator == (const Token& lhs, const Token& rhs) {
  using namespace TokenType;

  if (lhs.index() != rhs.index()) {
    return false;
  }
  if (lhs.Is<Char>()) {
    return lhs.As<Char>().value == rhs.As<Char>().value;
  } else if (lhs.Is<Number>()) {
    return lhs.As<Number>().value == rhs.As<Number>().value;
  } else if (lhs.Is<String>()) {
    return lhs.As<String>().value == rhs.As<String>().value;
  } else if (lhs.Is<Id>()) {
    return lhs.As<Id>().value == rhs.As<Id>().value;
  } else {
    return true;
  }
}

std::ostream& operator << (std::ostream& os, const Token& rhs) {
  using namespace TokenType;

#define VALUED_OUTPUT(type) \
  if (auto p = rhs.TryAs<type>()) return os << #type << '{' << p->value << '}';

  VALUED_OUTPUT(Number);
  VALUED_OUTPUT(Id);
  VALUED_OUTPUT(String);
  VALUED_OUTPUT(Char);

#undef VALUED_OUTPUT

#define UNVALUED_OUTPUT(type) \
    if (rhs.Is<type>()) return os << #type;

  UNVALUED_OUTPUT(Class);
  UNVALUED_OUTPUT(Return);
  UNVALUED_OUTPUT(If);
  UNVALUED_OUTPUT(Else);
  UNVALUED_OUTPUT(Def);
  UNVALUED_OUTPUT(Newline);
  UNVALUED_OUTPUT(Print);
  UNVALUED_OUTPUT(Indent);
  UNVALUED_OUTPUT(Dedent);
  UNVALUED_OUTPUT(And);
  UNVALUED_OUTPUT(Or);
  UNVALUED_OUTPUT(Not);
  UNVALUED_OUTPUT(Eq);
  UNVALUED_OUTPUT(NotEq);
  UNVALUED_OUTPUT(LessOrEq);
  UNVALUED_OUTPUT(GreaterOrEq);
  UNVALUED_OUTPUT(None);
  UNVALUED_OUTPUT(True);
  UNVALUED_OUTPUT(False);
  UNVALUED_OUTPUT(Eof);

#undef UNVALUED_OUTPUT

  return os << "Unknown token :(";
}

Lexer::Lexer(std::istream& input) : input_(input) {
	NextToken();
}

const Token& Lexer::CurrentToken() const {
	return tokens_.back();
}

Token Lexer::NextToken() {
	if (new_line_) {
		new_line_ = false;

  	int count = 0;
  	while (input_.peek() == ' ' || input_.peek() == '\n') {
  		if (input_.get() == '\n') {
  			count = 0;
  		} else {
    		++count;
  		}
  	}

  	if (input_.peek() == EOF) {
  		if (count < indent_count_) {
				tokens_.push_back(TokenType::Dedent{});
				indent_count_ -= 2;
				new_line_ = (indent_count_ != count);
  		} else {
    		tokens_.push_back(TokenType::Eof{});
  		}
  		return tokens_.back();
  	} else if (indent_count_ != count) {
  		if (count > indent_count_) {
  			tokens_.push_back(TokenType::Indent{});
      	indent_count_ += 2;
    	} else if (count < indent_count_) {
				tokens_.push_back(TokenType::Dedent{});
				indent_count_ -= 2;
				new_line_ = (indent_count_ != count);
    	}
  		return tokens_.back();
  	}
	}


  if (input_.peek() == '\n') {

  	new_line_ = true;
  	input_.get();
  	tokens_.push_back(TokenType::Newline{});

  } else {

  	char c;

    if (input_ >> c) {

      if (c == '\'' || c == '"') {

        string line;
        getline(input_, line, c);
        tokens_.push_back(TokenType::String{move(line)});

      } else if (isdigit(c)) {

      	input_.putback(c);
      	int number;
      	input_ >> number;
        tokens_.push_back(TokenType::Number{number});

      } else if (isalpha(c) || c == '_') {

      	string word;
      	word.push_back(c);
      	while (isalnum(input_.peek()) || input_.peek() == '_') {
      		word.push_back(input_.get());
      	}

//      	while (input_.peek() == ' ') {
//      		input_.get();
//      	}

    		if (auto it = mapping_.find(word); it != mapping_.end()) {
    			tokens_.push_back(it->second);
    		} else {
    			tokens_.push_back(TokenType::Id{move(word)});
    		}

      } else {

      	if (input_.peek() == '=') {
        	string oper;
        	oper += c;
        	oper += '=';
      		if (auto it = mapping_.find(oper); it != mapping_.end()) {
      			tokens_.push_back(it->second);
      			input_.get();
      		} else {
      			tokens_.push_back(TokenType::Char{c});
      		}
      	} else {
        	tokens_.push_back(TokenType::Char{c});
      	}

      }

    } else {

    	if (tokens_.back().Is<TokenType::Eof>()
    			|| tokens_.back().Is<TokenType::Newline>()
					|| tokens_.back().Is<TokenType::Dedent>()) {
      	tokens_.push_back(TokenType::Eof{});
    	} else {
      	tokens_.push_back(TokenType::Newline{});
    	}

    }
  }

	return tokens_.back();
}

const std::unordered_map<std::string, Token> Lexer::mapping_ = {
		{"class", TokenType::Class{}},
		{"return", TokenType::Return{}},
		{"if", TokenType::If{}},
		{"else", TokenType::Else{}},
		{"def", TokenType::Def{}},
		{"print", TokenType::Print{}},
		{"eof", TokenType::Eof{}},
		{"and", TokenType::And{}},
		{"or", TokenType::Or{}},
		{"not", TokenType::Not{}},
		{"==", TokenType::Eq{}},
		{"!=", TokenType::NotEq{}},
		{"<=", TokenType::LessOrEq{}},
		{">=", TokenType::GreaterOrEq{}},
		{"None", TokenType::None{}},
		{"True", TokenType::True{}},
		{"False", TokenType::False{}}
};

} /* namespace Parse */
