#include "dnsname.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <boost/range/iterator.hpp>
#include <chrono>
#include <locale>
#include <fstream>
#include <iostream>

struct nth_label_cmp {
    size_t const N = 0;

    bool
    operator() (DNSLabel const& lhs, DNSName const& name) const noexcept {
        if (name.label_count() <= N) {
            return false;
        }
        auto rhs = std::next (name.rbegin(), N);
        return (lhs < *rhs);
    }

    bool
    operator() (DNSName const& name, DNSLabel const& rhs) const noexcept {
        if (name.label_count() <= N) {
            return true;
        }
        auto lhs = std::next (name.rbegin(), N);
        return (*lhs < rhs);
    }
};

template <typename SortedDNSNames>
typename boost::range_iterator<SortedDNSNames>::type
find_parent (
    DNSName const& query,
    SortedDNSNames& sorted_names
){
    using std::begin;
    using std::end;
    auto notless = begin (sorted_names);
    auto greater = end (sorted_names);
    auto label = query.rbegin();
    for (std::size_t i = 0; i < query.label_count(); ++i) {
        std::tie (notless, greater)
            = std::equal_range (notless, greater, *label++, nth_label_cmp{i});
        if (notless == greater) {
            break;
        }
        if (std::next(notless) == greater) {
            return notless;
        }
    }
    return end (sorted_names);
}

int main() {
    std::vector<DNSName> block_list;
    block_list.reserve (1000000);

    /* Load the block list
     */
    std::string line;
    std::ifstream top1m ("testdata/top-1m-201506201230UTC.csv",
                       std::ios::in | std::ios::binary);
    top1m.imbue (std::locale::classic());

    auto start = std::chrono::steady_clock::now();
    for (std::size_t n = 1; n <= 1000000; ++n) {
        std::size_t l;
        top1m >> l;
        assert (l == n);
        top1m.ignore (1, ',');
        std::getline (top1m, line, '\n');
        block_list.emplace_back (std::move (line));
    }
    auto stop = std::chrono::steady_clock::now();
    std::cout << "Load took "
              << std::chrono::duration_cast<std::chrono::milliseconds>
                 (stop - start).count()
              << " ms" << std::endl;

    /* Sort the block list
     */
    start = std::chrono::steady_clock::now();
    std::sort (begin (block_list), end (block_list));
    stop = std::chrono::steady_clock::now();
    std::cout << "Sort took "
              << std::chrono::duration_cast<std::chrono::milliseconds>
                 (stop - start).count()
              << " ms" << std::endl;

    /* Lookup something
     */
    std::cout << "Enter a domain to lookup" << std::endl;
    while (true) {
        std::cout << "> ";
        std::getline (std::cin, line);
        DNSName name (std::move(line));
        auto start = std::chrono::high_resolution_clock::now();
        auto blocker = find_parent (name, block_list);
        auto stop = std::chrono::high_resolution_clock::now();
        std::cout << "  Lookup took "
                  << std::chrono::duration_cast<std::chrono::microseconds>
                     (stop - start).count()
                  << " us" << std::endl;
        if (blocker == end (block_list)) {
            std::cout << "  OK: '" << name << "' is not blocked! :D"
                      << std::endl;
            continue;
        }
        std::cout << "  BLOCKED: '" << name << "' is blocked by '"
                  << *blocker << "' :(" << std::endl;
    };
}
