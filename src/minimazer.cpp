#include "minimazer.hpp"

#include <string>

std::string minimizate(std::size_t num) {
	const std::string letters = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNNM";
    const std::size_t base = letters.size();

    if (num == 0) return std::string(1, letters[0]);

    std::string result;
    while (num > 0) {
        result = letters[num % base] + result;
        num /= base;
    }
    return result;
}

std::shared_ptr<shader_precompiler::ast::nodes::CodeBlock> shader_precompiler::visitors::MinimazerVisitor::processTree() {
    auto tree = from.processTree();

    std::size_t miniLevel{};

    this->currentMiniLevel = &miniLevel;

    tree->accept(*this);

    this->currentMiniLevel = NULL;

    return tree;
}

void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::CodeBlock& node){

    auto baseLevel = this->currentMiniLevel;
    std::size_t miniLevel = *this->currentMiniLevel;

    this->currentMiniLevel = &miniLevel;

    for(auto& expr : node.expressions)
    {
        expr->accept(*this);
    }

}
void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::Identifier& node){
    
}
void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::Return& node){
    
}
void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::Attribute& node){
    
}
void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::VariableInitialization& node){
    
    node.name->name = minimizate((*this->currentMiniLevel)++);
}
void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::IfElse& node){
    
}
void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::Operator& node){
    node.left->accept(*this);
    node.right->accept(*this);
}
void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::FuncDeclaration& node){

    node.name->name = minimizate((*this->currentMiniLevel)++);
}
void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::FuncCall& node){
    
}
void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::Func& node){
    node.code->accept(*this);
    node.declaration->accept(*this);
}
void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::NumberExpr& node){
    
}