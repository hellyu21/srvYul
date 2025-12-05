#include <atomic>
#include <vector>
#include <memory>
#include <thread>
#include <cmath>
#include <random>
#include <iostream>
#include <chrono>

#include "lockfree_stack.h"
#include "node.h"

namespace {
    struct Position {
        double x;
        double y;
    };

    const double START_X = 0.0;
    const double END_X = 4.0;
    const double STEP = 0.001;
    const double EPSILON = 1e-10;

    size_t readers_num = 4;
    lf::LockFreeVersionedStack<Position> stack(readers_num);


    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist_push(50, 200);
    std::uniform_int_distribution<> dist_pop(50, 200);

    void writer() {
        double current_x = START_X;
        size_t total_operations = 0;

        while (current_x <= END_X && total_operations < 100000) {
            int push_count = dist_push(gen);
            for (int i = 0; i < push_count && current_x <= END_X; i++) {
                double y = -(current_x * current_x) + 4 * current_x;
                stack.push({ current_x, y });
                current_x += STEP;
                total_operations++;
            }

            int pop_count = dist_pop(gen);
            for (int i = 0; i < pop_count && !stack.is_stopped(); i++) {
                if (!stack.pop())
                    break;
                current_x -= STEP;
                total_operations++;
            }

            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }

        stack.stop();
        std::cout << "Writer finished. Total operations: " << total_operations << std::endl;
        std::cout << "All versions: " << stack.last_version() << std::endl;
    }

    class Reader {
    public:
        Reader(unsigned int id, lf::LockFreeVersionedStack<Position>* stack)
            : id_(id), stack_(stack), versions_cnt(0), consistent_versions(0) {}

        void life() {
            while (!stack_->is_stopped()) {
                auto data = read();
                if (!data.empty()) {
                    if (check(data)) {
                        consistent_versions++;
                    }
                }
            }
        }

        std::vector<Position> read() {
            lf::LockFreeVersionedStack<Position>::NodePtr data_ptr;
            if (!stack_->subscribe(id_, data_ptr)) {
                return {};
            }

            std::vector<Position> result;
            while (data_ptr != nullptr) {
                result.push_back(data_ptr->data);
                data_ptr = data_ptr->next;
            }
            versions_cnt++;
            return result;
        }

        bool check(const std::vector<Position>& data) {
            if (data.empty()) return false;

            
            for (size_t i = 0; i < data.size(); i++) {
                double expected_y = -(data[i].x * data[i].x) + 4 * data[i].x;
                if (std::abs(data[i].y - expected_y) > EPSILON) {
                    std::cerr << "Reader " << id_ << ": Point doesn't belong to parabola! "
                        << "x=" << data[i].x << ", y=" << data[i].y
                        << ", expected_y=" << expected_y << std::endl;
                    return false;
                }
            }

            
            for (size_t i = 1; i < data.size(); i++) {
                double actual_step = data[i - 1].x - data[i].x;
                if (std::abs(actual_step - STEP) > EPSILON) {
                    std::cerr << "Reader " << id_ << ": Invalid sequence step! "
                        << "x[" << i - 1 << "]=" << data[i - 1].x
                        << ", x[" << i << "]=" << data[i].x
                        << ", step=" << actual_step << ", expected=" << STEP << std::endl;
                    return false;
                }
            }

            return true;
        }

        unsigned int versions_cnt;
        unsigned int consistent_versions;

    private:
        int id_;
        lf::LockFreeVersionedStack<Position>* stack_;
    };
}

int main() {
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<Reader> readers;
    std::vector<std::thread> threads;

    for (unsigned int i = 0; i < readers_num; i++) {
        readers.emplace_back(i, &stack);
    }

    for (unsigned int i = 0; i < readers_num; i++) {
        threads.emplace_back(&Reader::life, &readers[i]);
    }

    writer();

    for (size_t i = 0; i < readers_num; i++) {
        threads[i].join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "\n=== Results ===" << std::endl;
    std::cout << "Time: " << duration.count() << " ms" << std::endl;
    for (size_t i = 0; i < readers_num; i++) {
        std::cout << "Reader " << i << ": "
            << readers[i].versions_cnt << " versions read, "
            << readers[i].consistent_versions << " consistent" << std::endl;
    }

    return 0;
}