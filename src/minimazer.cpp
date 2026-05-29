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

    std::vector<std::string> miniTable{};

    this->miniTable = &miniTable;

    tree->accept(*this);

    this->miniTable = NULL;

    return tree;
}

void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::CodeBlock& node){

    auto baseLevel = this->miniTable;
    auto miniTable = *this->miniTable;

    this->miniTable = &miniTable;

    for(auto& expr : node.expressions)
    {
        expr->accept(*this);
    }

    this->miniTable = baseLevel;
}
void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::Identifier& node){
    auto currentFuncName = std::find(begin(*this->miniTable), end(*this->miniTable), node.name);
    if (end(*this->miniTable) != currentFuncName) {
        std::size_t currentFuncNamePosition = (currentFuncName - begin(*this->miniTable));

        node.name = minimizate(currentFuncNamePosition);
    }
}
void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::Return& node){
    node.value->accept(*this);
}
void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::Attribute& node){
    
}
void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::VariableInitialization& node){

    if (!ast::haveAttribute(node.attributes, "__minimazer_skip")) {
        (*this->miniTable).push_back(node.name->name);

        node.name->name = minimizate((*this->miniTable).size() - 1);
    }
}
void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::IfElse& node){
    node.ifCondition->accept(*this);
    node.elseBranch->accept(*this);
    node.thenBranch->accept(*this);
}
void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::Operator& node){
    node.left->accept(*this);
    node.right->accept(*this);
}
void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::FuncDeclaration& node){

    if (node.name->name == "main" &&
        node.params.empty() &&
        node.returnType->name == "void") {
        // MAIN FUNC
    }
    else {
        if (!ast::haveAttribute(node.attributes, "__minimazer_skip")) {
            (*this->miniTable).push_back(node.name->name);

            node.name->name = minimizate((*this->miniTable).size() - 1);
        }
    }
}
void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::FuncCall& node){
    auto currentFuncName = std::find(begin(*this->miniTable), end(*this->miniTable), node.name->name);
    if (end(*this->miniTable) != currentFuncName) {
        std::size_t currentFuncNamePosition = (currentFuncName - begin(*this->miniTable));

        node.name->name = minimizate(currentFuncNamePosition);
    }
}
void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::Func& node){
    node.declaration->accept(*this);
    node.code->accept(*this);
}
void shader_precompiler::visitors::MinimazerVisitor::visit(shader_precompiler::ast::nodes::NumberExpr& node){
    
}