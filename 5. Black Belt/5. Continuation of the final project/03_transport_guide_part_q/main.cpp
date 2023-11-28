#include <iostream>
#include <string_view>
#include "transport_guide_manager.h"

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: transport_catalog_part_o [make_base|process_requests]\n";
        return 5;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base") {
        try {
            TransportGuideManager::PerformMakeBaseRequests();
        } catch (const std::exception &e) {
            std::cerr << "make_base: " << e.what() << "\n";
            throw std::runtime_error("make_base");
        }
    } else if (mode == "process_requests") {
        TransportGuideManager manager;
        try {
            manager.PerformWriteQueries();
        } catch (const std::exception &e) {
            std::cerr << "process_requests: " << e.what() << "\n";
            throw std::runtime_error("process_requests");
        }
    }

    return 0;
}