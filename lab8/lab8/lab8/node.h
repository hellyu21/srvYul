#pragma once

namespace lf {
    template<typename T>
    struct Node {
        typedef Node<T>* NodePtr;

        T data;
        NodePtr next;
        uint64_t version;
    };
}