#include "nth_label_iterator.hpp"

std::size_t cmpcnt = 0;
std::size_t ncharcmp = 0;

static inline __attribute__((noinline)) auto
ccmp (char c1, char c2) noexcept -> int {
    if ((c1 >= 'A') && (c1 <= 'Z')) { c1 |= 0x20; }
    if ((c2 >= 'A') && (c2 <= 'Z')) { c2 |= 0x20; }
    ++ncharcmp;
    return (static_cast<unsigned char>(c1) - static_cast<unsigned char>(c2));
};


template <typename SortedDNSNames>
auto
__attribute__((noinline))
find_root (
    DNSName const& query,
    SortedDNSNames& sorted_names
){
    using std::begin;
    using std::end;
    auto notless = make_nth_label_iterator (begin (sorted_names));
    auto greater = make_nth_label_iterator (end (sorted_names));
    auto label = query.rbegin();

    for (std::size_t i = 0; i < query.label_count(); ++i) {
        std::tie (notless, greater)
            = lexicographical_equal_range_3way
                (notless, greater, begin(*label), end(*label), ccmp);
        if (notless == greater) {
            break;
        }
        if (std::next(notless) == greater) {
            return notless.name();
        }
        ++label;
        notless.bump();
        greater.bump();
    }
    return end (sorted_names);
}

#include <iostream>
#include <chrono>
#include <fstream>

int
main() {
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
    {
        std::cout << "> ";
        std::getline (std::cin, line);
        DNSName name (std::move (line));

        decltype(find_root(name, block_list)) blocker;
        auto const iterations = 1ul;
        cmpcnt = 0;
        ncharcmp = 0;

        auto start = std::chrono::high_resolution_clock::now();
        for (auto i = iterations; i != 0; --i) {
            blocker = find_root (name, block_list);
            __asm__ __volatile__ ("": : "g"(&blocker): "memory");
        }
        auto stop = std::chrono::high_resolution_clock::now();

        std::cout << "  Lookup took "
                  << std::chrono::duration_cast<std::chrono::nanoseconds>
                     (stop - start).count() / iterations
                  << " ns, cmpcnt = " << cmpcnt
                  << ", ncharcmp = " << ncharcmp
                  << ", ncharcmp/cmpcnt = " << (double (ncharcmp) / cmpcnt)
                  << std::endl;

        if (blocker == end (block_list)) {
            std::cout << "  OK: '" << name << "' is not blocked! :D"
                      << std::endl;
            goto fail;
        }

        std::cout << "  BLOCKED: '" << name << "' is blocked by '"
                  << *blocker << "' :(" << std::endl;
    }

    fail: {}
}
