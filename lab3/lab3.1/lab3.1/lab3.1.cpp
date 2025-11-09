#include <iostream>
#include <thread>
#include <mutex>
#include <string>
using namespace std;

std::mutex m;
int coins = 101;

void coin_sharing(string name, int& thief_coins, int& friend_coins) {
    while (true) {
        m.lock();
        if (coins == 1 && thief_coins == friend_coins || coins == 0) {
            m.unlock();
            break;
        }
        else if (coins > 0 && thief_coins <= friend_coins) {
            
            coins--;
            thief_coins++;
            //cout << name << ": " << thief_coins << endl;
            
            m.unlock();
        }
        else {
            m.unlock();
        }
    }
}

int main()
{
    
    int Bob_coins = 0;
    int Tom_coins = 0;
    std::thread thread1(coin_sharing, "BOB", ref(Bob_coins), ref(Tom_coins));
    std::thread thread2(coin_sharing, "TOM", ref(Tom_coins), ref(Bob_coins));
    thread1.join();
    thread2.join();
    cout << "-------------------------" << endl;
    cout << "BOB: " << Bob_coins << endl;
    cout << "TOM: " << Tom_coins << endl;
    cout << "CORPSE: " << coins << endl;
    system("pause");
}