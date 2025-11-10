#include <iostream>
#include <thread>
#include <mutex>
#include <string>
using namespace std;

//std::mutex m;

void Func(string name) {
    long double i = 0;

    auto start = chrono::steady_clock::now();
    auto end = start + chrono::seconds(1);

    while (chrono::steady_clock::now() < end) {
        i += 1e-9;
    }

    //m.lock();
    cout << name << ": " << i << endl;
    //m.unlock();
}

int main()
{
    std::thread thread1(Func, "t1");
    std::thread thread2(Func, "t2");
    std::thread thread3(Func, "t3");
    thread1.join();
    thread2.join();
    thread3.join();

    system("pause");
}