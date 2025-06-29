#include "mainwindow.h"

#include <QApplication>
#include "simulator.h"
#include <iostream>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    //Simulator sim;
       //sim.loadBinary(":/test_program.bin");
    //sim.memory1().store_word(0x1000, 0x00A00093); // addi x1, x0, 10
      //  sim.memory1().store_word(0x1004, 0x01400113); // addi x2, x0, 20
        //sim.memory1().store_word(0x1008, 0x002081B3); // add  x3, x1, x2

       // for (int i = 0; i < 3; ++i) sim.step();

        //std::cout << "x1 = " << sim.getRegisterValue(1) << std::endl;
       // std::cout << "x2 = " << sim.getRegisterValue(2) << std::endl;
       // std::cout << "x3 = " << sim.getRegisterValue(3) << std::endl;
        //return 0;
    return a.exec();
}
