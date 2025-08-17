#pragma once

namespace fast {
    template<typename T>
    struct vector {
        T data[256];
        int curSize = 0;

        T &operator[](uint16_t index) {
			return data[index];
		}

        void push_back(const T& value) {
            data[curSize++] = value;
        }

        int size() const {
            return curSize;
        }

        void clear() {
            curSize = 0;
        }

		T * __restrict__ begin() {
			return data;
		}

		T * __restrict__ end() {
			return data + curSize;
		}

		const T * __restrict__ begin() const {
			return data;
		}

		const T * __restrict__ end() const {
			return data + curSize;
		}
    };
}