#include "semantic.hpp"


std::vector<std::unique_ptr<shader_precompiler::ast::nodes::Node>> shader_precompiler::SemanticVisitor::processTree() {
    auto tree = from.processTree();

    std::vector<shader_precompiler::SemanticVisitor::Variable> currentVariables{};

    this->currentVariables = &currentVariables;

    for (auto& t : tree)
    {
        t->accept(*this);
    }

    this->currentVariables = NULL;

    return tree;
}

void shader_precompiler::SemanticVisitor::visit(shader_precompiler::ast::nodes::CodeBlock& node) {

    auto base = this->currentVariables;
    auto currentVariables = *this->currentVariables;

    this->currentVariables = &currentVariables;

    for (auto& expr : node.expressions)
    {
        if (expr == NULL) {
            printError(Error::Level::FATAL, Error::ErrorCodes::TOKEN_IS_NULL, Error::makeStore(), node.location);
            continue;
        }
        expr->accept(*this);
    }

    this->currentVariables = base;
}
void shader_precompiler::SemanticVisitor::visit(shader_precompiler::ast::nodes::Identifier& node) {

    if (!isVariableName(node.name)) {
        printError(Error::Level::FATAL, Error::ErrorCodes::UNDEFINDED_VARIABLE, Error::makeStore(node.name), node.location);
    }
}
void shader_precompiler::SemanticVisitor::visit(shader_precompiler::ast::nodes::Return& node) {
    if (node.value == NULL) {
        printError(Error::Level::FATAL, Error::ErrorCodes::TOKEN_IS_NULL, Error::makeStore(), node.location);
        return;
    }
    node.value->accept(*this);
}
void shader_precompiler::SemanticVisitor::visit(shader_precompiler::ast::nodes::Attribute& node) {

}
void shader_precompiler::SemanticVisitor::visit(shader_precompiler::ast::nodes::VariableInitialization& node) {
    if (node.type == NULL) {
        printError(Error::Level::FATAL, Error::ErrorCodes::TOKEN_IS_NULL, Error::makeStore(), node.location);
        return;
    }
    if (!isType(node.type->name)) {
        printError(Error::Level::FATAL, Error::ErrorCodes::UNDEFINDED_TYPE, Error::makeStore(node.type->name), node.type->location);
    }
    if (node.name == NULL) {
        printError(Error::Level::FATAL, Error::ErrorCodes::TOKEN_IS_NULL, Error::makeStore(), node.location);
        return;
    }

    if (isVariableName(node.name->name)) {
        printError(Error::Level::FATAL, Error::ErrorCodes::REDEFINITION_VARIABLE, Error::makeStore(node.type->name), node.type->location);
    }
    else {
        addVariable(Variable{ node.type->name, node.name->name });
    }

}
void shader_precompiler::SemanticVisitor::visit(shader_precompiler::ast::nodes::IfElse& node) {
    if (node.ifCondition == NULL) {
        printError(Error::Level::FATAL, Error::ErrorCodes::TOKEN_IS_NULL, Error::makeStore(), node.location);
        return;
    }
    else {
        node.ifCondition->accept(*this);
    }

    if (node.elseBranch == NULL) {
        printError(Error::Level::FATAL, Error::ErrorCodes::TOKEN_IS_NULL, Error::makeStore(), node.location);
        return;
    }
    else {
        node.elseBranch->accept(*this);
    }

    if (node.thenBranch == NULL) {
        printError(Error::Level::FATAL, Error::ErrorCodes::TOKEN_IS_NULL, Error::makeStore(), node.location);
        return;
    }
    else {
        node.thenBranch->accept(*this);
    }
}
void shader_precompiler::SemanticVisitor::visit(shader_precompiler::ast::nodes::Operator& node) {

    if (node.left == NULL) {
        printError(Error::Level::FATAL, Error::ErrorCodes::TOKEN_IS_NULL, Error::makeStore(), node.location);
        return;
    }
    else {
        node.left->accept(*this);
    }

    if (node.right == NULL) {
        printError(Error::Level::FATAL, Error::ErrorCodes::TOKEN_IS_NULL, Error::makeStore(), node.location);
        return;
    }
    else {
        node.right->accept(*this);
    }
}
void shader_precompiler::SemanticVisitor::visit(shader_precompiler::ast::nodes::FuncDeclaration& node) {
    if (node.returnType == NULL) {
        printError(Error::Level::FATAL, Error::ErrorCodes::TOKEN_IS_NULL, Error::makeStore(), node.location);
        return;
    }

    if (!isType(node.returnType->name)) {
        printError(Error::Level::FATAL, Error::ErrorCodes::UNDEFINDED_TYPE, Error::makeStore(node.returnType->name), node.location);
    }

    if (node.name == NULL) {
        printError(Error::Level::FATAL, Error::ErrorCodes::TOKEN_IS_NULL, Error::makeStore(), node.location);
        return;
    }

    std::vector<Variable> params{};

    for (auto& p : node.params)
    {
        if (p == NULL) {
            printError(Error::Level::FATAL, Error::ErrorCodes::TOKEN_IS_NULL, Error::makeStore(), node.location);
            continue;
        }

        Variable var;

        if (p->name == NULL) {
            printError(Error::Level::FATAL, Error::ErrorCodes::TOKEN_IS_NULL, Error::makeStore(), p->location);
            continue;
        }
        else {
            var.name = p->name->name;
        }

        if (p->type == NULL) {
            printError(Error::Level::FATAL, Error::ErrorCodes::TOKEN_IS_NULL, Error::makeStore(), p->location);
            continue;
        }
        else {
            var.type = p->type->name;
        }

        params.push_back(var);
    }

    Func func{ node.returnType->name, node.name->name, params };

    if (isFunctionName(node.name->name)) {
        printError(Error::Level::FATAL, Error::ErrorCodes::REDEFINITION_FUNCTION, Error::makeStore(node.name->name), node.location);
    }
    else {
        addFunctions(func);
    }
}
void shader_precompiler::SemanticVisitor::visit(shader_precompiler::ast::nodes::FuncCall& node) {
    if (node.name == NULL) {
        printError(Error::Level::FATAL, Error::ErrorCodes::TOKEN_IS_NULL, Error::makeStore(), node.location);
        return;
    }

    if (!isFunctionName(node.name->name)) {
        printError(Error::Level::FATAL, Error::ErrorCodes::UNDEFINDED_FUNCTION, Error::makeStore(node.name->name), node.location);
    }
}
void shader_precompiler::SemanticVisitor::visit(shader_precompiler::ast::nodes::Func& node) {

    if (node.code == NULL) {
        printError(Error::Level::FATAL, Error::ErrorCodes::TOKEN_IS_NULL, Error::makeStore(), node.location);
        return;
    }
    else {
        node.code->accept(*this);
    }

    if (node.declaration == NULL) {
        printError(Error::Level::FATAL, Error::ErrorCodes::TOKEN_IS_NULL, Error::makeStore(), node.location);
        return;
    }
    else {
        node.declaration->accept(*this);
    }
}
void shader_precompiler::SemanticVisitor::visit(shader_precompiler::ast::nodes::NumberExpr& node) {

}