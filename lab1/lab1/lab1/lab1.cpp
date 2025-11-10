#include <iostream>
#include <time.h>
#include <thread>
#include <mutex>
#include <string>
using namespace std;

void Func(string name)
{
    for (int i = 0; i <= 10000000; i++) {
        int factorial = 1;
        for (int j = 2; j <= 10; j++) {
            factorial *= j;
        }
    }
}


int main()
{
    clock_t start = clock();

    std::thread thread1(Func, "t1");
    /*std::thread thread2(Func, "t2");*/
    thread1.join();
    /*thread2.join();*/

    clock_t end = clock();
    double seconds = (double)(end - start) / CLOCKS_PER_SEC;
    cout << seconds<< endl;

    system("pause");
}