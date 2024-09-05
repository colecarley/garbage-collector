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

#include "sym_table.h"
#include "bird_exception.h"

class Object
{
public:
    bool marked;
    int value;
    Object *next;

    Object(int value)
    {
        this->value = value;
        this->marked = false;
    }

    int get_value()
    {
        return this->value;
    }

    void set_next(Object *next)
    {
        this->next = next;
    }
};

class Interpreter : public Visitor
{
public:
    std::unique_ptr<SymbolTable<Object *>> environment;
    std::vector<Object *> stack;
    Object *object_list = nullptr;

    Interpreter()
    {
        this->environment = std::make_unique<SymbolTable<Object *>>(
            SymbolTable<Object *>());
    }

    Object *create_object(int value)
    {
        auto object = new Object(value);
        object->set_next(this->object_list);
        this->object_list = object;
        return object;
    }

    void collect_garbage()
    {
        this->mark();
        this->sweep();
    }

    void collect_all()
    {
        this->sweep();
    }

    void mark()
    {
        this->environment->for_each([](Object *value)
                                    { value->marked = true; });
    }

    void sweep()
    {
        auto temp = this->object_list;
        Object *previous = nullptr;
        while (temp != nullptr)
        {
            if (!temp->marked)
            {
                if (previous == nullptr)
                {
                    auto to_delete = temp;
                    previous = temp;
                    temp = temp->next;
                    this->object_list = temp;
                    delete to_delete;
                }
                else
                {
                    previous->next = temp->next;
                    auto to_delete = temp;
                    previous = temp;
                    temp = temp->next;
                    delete to_delete;
                }
            }
            else
            {
                temp->marked = false;
                previous = temp;
                temp = temp->next;
            }
        }
    }

    void evaluate(std::vector<std::unique_ptr<Stmt>> *stmts)
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
        }
        this->stack.clear();
        this->collect_all();
    }

    void visitBlock(Block *block)
    {
        auto new_environment = std::make_unique<SymbolTable<Object *>>(
            SymbolTable<Object *>());
        new_environment->set_enclosing(std::move(this->environment));
        this->environment = std::move(new_environment);

        for (auto &expr : block->stmts)
        {
            expr->accept(this);
        }

        this->environment = std::move(this->environment->get_enclosing());
        this->collect_garbage();
    }

    void visitDeclStmt(DeclStmt *decl_stmt)
    {
        decl_stmt->value->accept(this);

        auto result = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        this->environment->insert(decl_stmt->identifier.lexeme, result);
    }

    void visitExprStmt(ExprStmt *expr_stmt)
    {
        expr_stmt->expr->accept(this);
    }

    void visitPrintStmt(PrintStmt *print_stmt)
    {
        for (auto &arg : print_stmt->args)
        {
            arg->accept(this);
            auto result = this->stack[this->stack.size() - 1];
            this->stack.pop_back();

            std::cout << result->get_value();
        }
        std::cout << std::endl;
    }

    void visitBinary(Binary *binary)
    {
        binary->left->accept(this);
        binary->right->accept(this);

        auto right = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        auto left = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        switch (binary->op.token_type)
        {
        case TokenType::PLUS:
        {
            this->stack.push_back(
                this->create_object(left->get_value() + right->get_value()));
            break;
        }
        case TokenType::MINUS:
        {
            this->stack.push_back(this->create_object(left->get_value() - right->get_value()));
            break;
        }
        case TokenType::SLASH:
        {
            this->stack.push_back(this->create_object(left->get_value() / right->get_value()));
            break;
        }
        case TokenType::STAR:
        {
            this->stack.push_back(this->create_object(left->get_value() * right->get_value()));
            break;
        }
        default:
        {
            throw BirdException("undefined binary operator");
        }
        }
    }

    void visitUnary(Unary *unary)
    {
        unary->expr->accept(this);
        auto expr = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        this->stack.push_back(this->create_object(-expr->get_value()));
    }

    void visitPrimary(Primary *primary)
    {
        switch (primary->value.token_type)
        {
        case TokenType::I32_LITERAL:
        {
            auto value = this->create_object(std::stoi(primary->value.lexeme));
            this->stack.push_back(value);
            break;
        }
        case TokenType::IDENTIFIER:
        {
            auto value = this->environment->get(primary->value.lexeme);
            this->stack.push_back(value);
            break;
        }
        default:
        {
            throw BirdException("undefined primary value");
        }
        }
    }
};
