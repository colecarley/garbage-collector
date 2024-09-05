#pragma once

#include <memory>
#include <vector>
#include "stmt.h"
#include "expr.h"

#include "binary.h"
#include "unary.h"
#include "primary.h"

#include "decl_stmt.h"
#include "print_stmt.h"
#include "expr_stmt.h"
#include "block.h"

class AstPrinter : public Visitor
{
public:
    void print_ast(std::vector<std::unique_ptr<Stmt>> *stmts)
    {

        for (auto &stmt : *stmts)
        {
            if (auto decl_stmt = dynamic_cast<DeclStmt *>(stmt.get()))
            {
                decl_stmt->accept(this);
            }

            if (auto print_stmt = dynamic_cast<PrintStmt *>(stmt.get()))
            {
                print_stmt->accept(this);
            }

            if (auto block = dynamic_cast<Block *>(stmt.get()))
            {
                block->accept(this);
            }

            if (auto expr_stmt = dynamic_cast<ExprStmt *>(stmt.get()))
            {
                expr_stmt->accept(this);
            }
            std::cout << std::endl;
        }
    }

    void visitBlock(Block *block)
    {
        std::cout << "{";
        for (auto &expr : block->stmts)
        {
            expr->accept(this);
        }
        std::cout << "}";
    }

    void
    visitDeclStmt(DeclStmt *decl_stmt)
    {
        std::cout << "(let ";
        std::cout << decl_stmt->identifier.lexeme << " ";
        decl_stmt->value->accept(this);
        std::cout << ")";
    }

    void visitPrintStmt(PrintStmt *print_stmt)
    {
        std::cout << "(";
        std::cout << "puts ";
        for (auto &arg : print_stmt->args)
        {
            arg->accept(this);
        }
        std::cout << ")";
    }

    void visitExprStmt(ExprStmt *expr_stmt)
    {
        expr_stmt->expr->accept(this);
        std::cout << ";";
    }

    void visitBinary(Binary *binary)
    {
        std::cout << "(";
        std::cout << binary->op.lexeme << " ";
        binary->left->accept(this);
        std::cout << " ";
        binary->right->accept(this);
        std::cout << ")";
    }

    void visitUnary(Unary *unary)
    {
        std::cout << unary->op.lexeme;
        unary->expr->accept(this);
    }

    void visitPrimary(Primary *primary)
    {
        std::cout << primary->value.lexeme;
    }
};
