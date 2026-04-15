#pragma once
#include <iostream>
#include "typedefs.h"

struct GateBase {
    int id;
    size_t digit;
    GateBase(std::string reg_, size_t digit_) : id(reg_.size()), digit(digit_) {};
    GateBase(int id_, size_t digit_) : id(id_), digit(digit_) {};
    // 虚函数，基类实现
    virtual void display() const {
        std::cout << "Base Gate display function." << std::endl;
    }
};

struct Phase_Int : GateBase {
    double lambda;

    Phase_Int(std::string reg_, size_t digit_, double lambda_) :GateBase(reg_, digit_), lambda(lambda_) {
        // 初始化其他成员变量的逻辑
    }

    Phase_Int(int id_, size_t digit_, double lambda_) : GateBase(id_, digit_), lambda(lambda_) {
        // 初始化其他成员变量的逻辑
    }

    // 重写 display 函数
    void display() const override;
};

struct Zgate_Int : Phase_Int {
    Zgate_Int(std::string reg_, size_t digit_) : Phase_Int(reg_, digit_, qram_simulator::pi) {}

    Zgate_Int(int id_, size_t digit_) : Phase_Int(id_, digit_, qram_simulator::pi) {}

    // 重写 Phase_Int 的 display 函数
    void display() const override;
};