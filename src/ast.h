#ifndef _AST_H_
#define _AST_H_

#include "language.h"

#include <vector>
#include <string>

class ASTNode;

typedef std::vector<ASTNode*> NodeList;

class ASTNode {
public:
    ASTNode(Language::Token tok, int line = 0);
    ASTNode(ASTNode *other);
    ~ASTNode();
    int getLine();
    void setLine(int line);
    
    std::string stringTree(int indent = 0);
    
    void addChild(ASTNode *child);
    
    Language::Token root; // the operation of this AST node
    std::string str; // string data
    
    NodeList children; // sub nodes
    
private:
    int Line;
    std::string FileName;
};

#endif