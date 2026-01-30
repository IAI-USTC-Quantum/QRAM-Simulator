#include "displayTest.h"

void Phase_Int::display() const
{
    std::cout << "Phase_Int display function with lambda: " << lambda << std::endl;
}
void Zgate_Int::display() const
{
    //Phase_Int::display(); // 显式调用 Phase_Int 的 display 函数
    std::cout << "Zgate_Int display function." << std::endl;
}

int main() {
    Zgate_Int z_gate("q0", 1);

    // 调用 display 函数
    z_gate.display();  // 输出：Phase_Int display function... Zgate_Int display function...

    // 使用 Phase_Int 指针指向 Zgate_Int 实例
    Phase_Int* phase_ptr = &z_gate;
    phase_ptr->display(); // 输出：Phase_Int display function... Zgate_Int display function...

    // 使用 GateBase 指针指向 Zgate_Int 实例
    GateBase* base_ptr = &z_gate;
    base_ptr->display(); // 输出：Phase_Int display function... Zgate_Int display function...

    return 0;
}

