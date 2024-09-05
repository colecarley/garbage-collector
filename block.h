#pragma once

#include <string>
#include <memory>
#include <vector>

#include "stmt.h"
#include "lexer.h"
#include "visitor.h"

class Block : public Stmt
{
public:
    std::vector<std::unique_ptr<Stmt>> stmts;

    Block(std::vector<std::unique_ptr<Stmt>> stmts)
    {
        this->stmts = std::move(stmts);
    }

    void accept(Visitor *visitor)
    {
        visitor->visitBlock(this);
    }
};
