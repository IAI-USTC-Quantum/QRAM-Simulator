#include "sparse_state_simulator.h"

using namespace qram_simulator;

int example_1() {
    /***********    
    * Example 1
     * 包括量子态的申请，量子寄存器的申请，以及对量子态施加量子门操作。
     * 
     * 1. 首先，需要创建一个SparseState对象，这个对象是用来表示量子态的。
     * 2. 然后，需要申请你所需要的量子寄存器，并对它们进行初始化。
     * 3. 在调用量子门操作时，需要构造一个可调用对象（Callable），这个对象是一个对SparseState对象进行操作的函数。
     * 
     * 具体的操作可以参考README中的定义。
     **********/

    // 创建空的量子态
    SparseState s;

    // 1. 开始申请你所需要的量子寄存器
    // 1) 申请一个2个比特，类型为UnsignedInteger的寄存器
    auto reg0 = AddRegister("reg0", UnsignedInteger, 2)(s);
    // 2) 申请一个1个比特，类型为Boolean的寄存器
    auto reg1 = AddRegister("reg1", Boolean, 1)(s);
    // 3) 打印它的目前的状态
    (StatePrint(Detail))(s);

    // 输出如下：
    // StatePrint (mode=Detail)
    // |(0)reg0 : UInt2 | |(1)reg1 : Bool1 |   （这一行表示寄存器信息）
    // 1.000000+0.000000i  reg0=|0> reg1=|false> （这一行开始枚举了每个量子态分量）

    // 2. 开始对寄存器进行操作
    // 1) 使用Hadamard门对reg0作用: 首先通过参数列表创建一个Hadamard_Int_Full的对象，这个对象是一个可调用对象（Callable），可以对SparseState对象进行操作。
    (Hadamard_Int_Full(reg0))(s);

    // 2) 使用Pauli-X门对reg1作用: 同样，创建一个Xgate_Bool对象，并对reg2作用。
    (Xgate_Bool(reg1))(s);

    // 3. 打印它目前的状态
    (StatePrint(Detail))(s);

    // 输出如下：
    // StatePrint (mode=Detail)
    // | (0)reg0 : UInt2 | |(1)reg1 : Bool1 |
    // 0.500000 + 0.000000i  reg0 = | 0 > reg1 = | true >
    // 0.500000 + 0.000000i  reg0 = | 1 > reg1 = | true >
    // 0.500000 + 0.000000i  reg0 = | 2 > reg1 = | true >
    // 0.500000 + 0.000000i  reg0 = | 3 > reg1 = | true >
    
    // reg0的量子态已经被Hadamard门作用，reg1的量子态已经被Pauli-X门作用。

    return 0;
}

int example_2() {
    /***********
     * Example 2
     *
     * 展示如何对量子操作增加控制操作。
     * 总的来说，一共有4种控制操作
     * conditioned_by_all_ones : 寄存器所有比特都为1的控制
     * conditioned_by_bit : 寄存器指定比特为1的控制
     * conditioned_by_nonzeros : 寄存器只要有一个比特不为0的控制
     * conditioned_by_value : 寄存器指定比特的值的控制，例如对3个比特寄存器，可以控制它为0~7中的任意值
     *
    **********/

    {
        /* Example 2.1 */
        // 创建空的量子态
        SparseState s;

        /* 申请两个 General 类型的寄存器，分别有2个和1个比特。 */
        auto reg0 = AddRegister("reg0", General, 2)(s);
        auto reg1 = AddRegister("reg1", General, 1)(s);

        /*
        reg0 (0) -- X -- C --
        reg0 (1) ------- C --
        reg1 (0) ------- X --
        */

        // 对 reg0的第0个比特施加X门，并对reg1的第0个比特施加CCX (Toffoli) 门
        (Xgate_Bool(reg0, 0))(s);
        (Xgate_Bool(reg1).conditioned_by_all_ones(reg0))(s);

        // 打印量子态
        (StatePrint(Detail))(s);

        // 输出如下：
        // StatePrint (mode=Detail)
        // | (0)reg0 : Reg2 | |(1)reg1 : Reg1 |
        // 1.000000 + 0.000000i  reg0 = | 01 > reg1 = | 0 >
        // 这代表reg1上的X门收到reg0上的所有比特为1的控制，但是reg0为|01>，所以reg1上的X门没有作用。
    }

    /* 通过System::clear()函数，可以清除系统中所有量子寄存器信息。*/
    System::clear();

    {
        /* Example 2.2 */
        // 创建空的量子态
        SparseState s;

        /* 申请两个 General 类型的寄存器，分别有3个和1个比特。 */
        auto reg0 = AddRegister("reg0", General, 3)(s);
        auto reg1 = AddRegister("reg1", General, 1)(s);

        /*
        reg0 (0) -- X -- C --
        reg0 (1) ------------
        reg0 (2) ---X----C --
        reg1 (0) ------- X --
        */

        // 对 reg0的第0个比特施加X门，并对reg1的第0个比特施加CCX (Toffoli) 门
        (Xgate_Bool(reg0, 0))(s);
        (Xgate_Bool(reg0, 2))(s);
        (Xgate_Bool(reg1).conditioned_by_bit({ {reg0, 0}, {reg0, 2} }))(s);

        // 打印量子态
        (StatePrint(Detail))(s);

        // 输出如下：
        // StatePrint (mode=Detail)
        // | (0)reg0 : Reg3 | |(1)reg1 : Reg1 |
        // 1.000000 + 0.000000i  reg0 = | 101 > reg1 = | 1 >
        // 这表示reg1上的X门收到reg0的第0和2个比特为1的控制，因为reg0为|101>，所以reg1上的X门作用了。
    }
    return 0;
}


int main() {
    //example_1();
    example_2();
    return 0;
}