#pragma once

#include <map>
#include <string>
#include <memory>

#include "bird_exception.h"

template <typename T>
class SymbolTable
{
    std::map<std::string, T> vars;
    std::unique_ptr<SymbolTable> enclosing;

public:
    void insert(std::string identifier, T value)
    {
        this->vars[identifier] = value;
    }

    T get(std::string identifier)
    {
        if (this->vars.count(identifier))
        {
            return this->vars[identifier];
        }

        if (this->enclosing.get() == nullptr)
        {
            throw BirdException("undefined variable");
        }

        return this->enclosing.get()->get(identifier);
    }

    void set_enclosing(std::unique_ptr<SymbolTable<T>> enclosing)
    {
        this->enclosing = std::move(enclosing);
    }

    std::unique_ptr<SymbolTable<T>> get_enclosing()
    {
        return std::move(this->enclosing);
    }

    void for_each(std::function<void(T)> lambda)
    {
        for (const auto &[key, value] : this->vars)
        {
            lambda(value);
        }

        if (this->enclosing.get() != nullptr)
        {
            this->enclosing->for_each(lambda);
        }
    }
};
