#include "dnsname.hpp"
#include <vector>
#include <string>
#include <locale>
#include <chrono>
#include <fstream>
#include <iostream>

int main() {
    std::ifstream top1m ("testdata/top-1m-201506201230UTC.csv",
                       std::ios::in | std::ios::binary);
    top1m.imbue (std::locale::classic());
    std::vector<DNSName> block_list;
    block_list.reserve (1000000);
    
    std::string line;
    auto start = std::chrono::steady_clock::now();
    for (std::size_t n = 1; n <= block_list.capacity(); ++n) {
        std::size_t l;
        top1m >> l;
        assert (l == n);
        top1m.ignore (1, ',');
        std::getline (top1m, line, '\n');
        block_list.emplace_back (std::move(line));
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>
                    (end - start).count() << " ms" << std::endl;
}