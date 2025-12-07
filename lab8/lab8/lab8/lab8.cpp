#include <atomic>
#include <vector>
#include <memory>
#include <thread>
#include <cmath>
#include <random>

#include "lockfree_stack.h"
#include "node.h"

#define min(a,b)            (((a) < (b)) ? (a) : (b))


namespace {
	struct Position {
		double x;
		double y;
	};

	const double START_X = 0.0;
	const double END_X = 4.0;
	const double STEP = 0.001;
	size_t readers_num = 4;
	lf::LockFreeVersionedStack<Position> stack(readers_num);

	void writer() {
		double current_x = START_X;
		for (size_t i = 0; i < 10000; i++)
		{
			for (int i = 0; i < 400; i++)
			{
				double y = -(current_x * current_x) + 4 * current_x;
				stack.push({ current_x, y });
				current_x += STEP;
			}
			for (int i = 0; i < 400; i++)
			{
				if (!stack.pop())
					throw std::runtime_error("can't delete element");
				current_x -= STEP;
			}
			for (int i = 0; i < 100; i++)
			{
				double y = -(current_x * current_x) + 4 * current_x;
				stack.push({ current_x, y });
				current_x += STEP;
			}
			for (int i = 0; i < 100; i++)
			{
				if (!stack.pop())
				{
					throw std::runtime_error("can't delete element");
				}
				current_x -= STEP;
			}
		}

		stack.stop();
		std::cout << "All versions: " << stack.last_version() << std::endl;
	}

	class Reader {
	public:
		Reader(unsigned int id, lf::LockFreeVersionedStack<Position>* stack) : id_(id), stack_(stack) {}

		int consistent = 0;

		void life() {
			while (!stack_->is_stopped()) {
				auto data = read();
				if (check(data))
				{
					consistent++;
				};
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
		}

		bool check(std::vector<Position>& data) {
			double prev_x = START_X;
			double step = 0.001;
			for (const auto& pos : data) {
				double expected_y = -(pos.x * pos.x) + 4 * pos.x;
				if (std::abs(pos.y - expected_y) != 0) {
					throw std::logic_error("Inconsistent data: Point does not belong to the parabola");
					return false;
				}

				if (std::abs(pos.x - prev_x) == step) {
					throw std::logic_error("Inconsistent data: Non-linear sequence");
					return false;
				}

				prev_x = pos.x;
			}
			return true;
		}

		unsigned int versions_cnt = 0;

	private:
		int id_;
		lf::LockFreeVersionedStack<Position>* stack_;
	};
}

int main() {
	std::vector<Reader> readers;
	std::vector<std::thread> threads;
	for (int i = 0; i < readers_num; i++) {
		readers.emplace_back(i, &stack);
	}
	for (int i = 0; i < readers_num; i++) {
		threads.emplace_back(&Reader::life, &readers[i]);
	}

	writer();

	for (size_t i = 0; i < readers_num; i++) {
		threads[i].join();
	}
	for (size_t i = 0; i < readers_num; i++) {
		std::cout << "Reader " << i << " : " << readers[i].versions_cnt << "; " << readers[i].consistent << std::endl;
	}
}