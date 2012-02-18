#include "ast.h"

// get the line number of the node (or try to derive it from the child node)
ASTNode::ASTNode(Language::Token tok, int line) : root(tok) , _line(line) {}

//! deep copy constructor
ASTNode::ASTNode(ASTNode *other) {
  root = other->root;
  str = other->str;
  setLine(other->getLine());

  // recursively copy children
  for(NodeList::iterator it = other->children.begin(), e = other->children.end(); it < e; ++it) {
    addChild(new ASTNode(*it));
  }
}

//! free the children! (Craig Keilburger reference)
ASTNode::~ASTNode() { 
  while(!children.empty()) {
        delete children.back();
        children.pop_back();
  }
}

int ASTNode::getLine() {
  if ( _line != 0 )
     return _line;
  if( children.size() != 0 )
     return children[0]->getLine();
  return 0;
}
void ASTNode::setLine(int line) {
  _line = line;
}

void ASTNode::addChild(ASTNode *child) {
  children.push_back(child);
}

std::string ASTNode::stringTree(int indent) {
  std::string out;
  out += "("; 
  out += Language::getTokName(root);
  out += "[";
  out += str;
  out += "]";
  
  NodeList::iterator it;
  for(it = children.begin(); it < children.end(); ++it) {
    out += "\n";
    for (int i = 0; i < indent; ++i) {
      out += "\t";
    }
    ASTNode *child = *it;
    if(child == NULL) {
      out += "NULL";
    } else {
      out += child->stringTree(indent+1);
    }
  }
  out += ")";
  return out;
}