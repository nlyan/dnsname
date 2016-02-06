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
#include <memory>

size_t cmpcnt = 0;
size_t ncharcmp = 0;

namespace my {

template <typename Iterator, typename T, typename Compare>
__attribute__((noinline))
Iterator
lower_bound (Iterator begin, Iterator const& end, T const& needle,
             Compare& cmp) {
    auto count = std::distance (begin, end);
    assert (count >= 0);
    if (!count) {
        return end;
    }
    decltype(begin) notless;
    do {
        auto step = count >> 1;
        notless = std::next (begin, step);
        if (cmp (*notless, needle).first < 0) {
            begin = ++notless;
            count -= step + 1;
        } else {
            count = step;
        }
    } while (count);
    return notless;
}

template <typename Iterator, typename T, typename Compare>
__attribute__((noinline))
Iterator
upper_bound (Iterator begin, Iterator const& end, T const& needle,
             Compare& cmp) {
    auto count = std::distance (begin, end);
    assert (count >= 0);
    if (!count) {
        return end;
    }
    decltype(begin) greater;
    do {
        auto step = count >> 1;
        greater = std::next (begin, step);
        if (cmp (*greater, needle).first <= 0) {
            begin = ++greater;
            count -= step + 1;
        } else {
            count = step;
        }
    } while (count);
    return greater;
}


template <typename Iterator, typename T, typename Compare>
__attribute__((noinline))
std::pair<Iterator, Iterator>
equal_range (Iterator begin, Iterator end, T const& needle, Compare cmp) {
    auto count = std::distance (begin, end);
    assert (count >= 0);
    while (count) {
        auto step = count >> 1;
        auto middle = std::next (begin, step);
        auto const result = cmp (*middle, needle);
        if (result.first < 0) {
            begin = ++middle;
            count -= step + 1;
        } else if (result.first > 0) {
            count = step;
        } else {
            assert (result.first == 0);
            auto notless = my::lower_bound (begin, middle, needle, cmp);
            end = std::next (begin, count);
            auto greater = my::upper_bound (++middle, end, needle, cmp);
            return {notless, greater};
        }
    }
    return {begin, begin};
}

std::pair<int,size_t> __attribute__((noinline))
strncasecmp (char const* s1, char const* s2, std::size_t const n) {
  int i1, i2, x;
  auto i = n;
  while (i) {
      ++ncharcmp;
      i1 = static_cast<unsigned char>(*s1);
      i2 = static_cast<unsigned char>(*s2);
      x = i1 ^ i2;
      if (__builtin_expect(x, false)) {
        x &= ~0x20;
        if (!x) {
            int lower = i1 | 0x20;
            if (__builtin_expect(((lower >= 'a') && (lower <= 'z')), true)) {
                goto next;
            }
        }
        return {i1 - i2, n - i};
      }
  next:
      --i;
   	  ++s1;
      ++s2;
  }
  return {0, n - i};
}

} // namespace my


/* Compares the Nth label of a name, against a prebound label.
 */
struct nth_label_cmp {
    size_t const N = 0;
    size_t mutable prefix_lb = 0;
    size_t mutable prefix_ub = 0;

    std::pair<int,size_t>
    operator() (DNSName const& name, DNSLabel const& rhs) const noexcept {
        /* If the the name doesn't have an Nth label then it's the lesser, e.g.
         * "bar.com" < "foo.bar.com".
         */
        if (name.label_count() <= N) {
            return {-1, 0};
        }
        auto lhs = std::next (name.rbegin(), N);
        auto result = (*this)(*lhs, rhs);
#ifndef NDEBUG
        std::cout << N << ": " << name;
        std::cout << ((result.first < 0) ? " < "
                        : ((result.first == 0) ? " == " : " > "));
        std::cout << rhs << std::endl;
#endif
        return result;
    }

    std::pair<int, size_t>
    operator() (DNSLabel const& lhs, DNSLabel const& rhs) const noexcept {
        ++cmpcnt;
        auto const minlen = std::min (lhs.size(), rhs.size());
        auto const prefix = std::min (prefix_lb, prefix_ub);
        assert (lhs.size() >= prefix);
        //if (lhs.size() < prefix) {
        //    std::cout << "FAIL:" << prefix << ", " << lhs << ", " << rhs << std::endl;
        //}
        assert (rhs.size() >= prefix);
        assert (lhs.substr(0, prefix) == rhs.substr(0, prefix));
        auto result = my::strncasecmp (lhs.data(), rhs.data(), minlen);

        /* Check to see if one label is a prefix of the other, e.g.
         * a.example.com < aaaa.example.com. It's not clear what to return for
         * result.first here. The difference between a character and a
         * non-existant character is a question of philosophy. I return -1 or 1,
         * which means "a" < "ab" returns -1 the same as "aa" < "ab".
         */
        if (!result.first) {
            assert (result.second == minlen);
            if (minlen < rhs.size()) {
                result.first = -1;
            } else if (minlen < lhs.size()) {
                result.first = 1;
            }
        }
        
        if (result.first < 0) {
            /* The name (lhs) is less than the needle (rhs) e.g. 
               "ww2" < "www", so we know all future values that compare less
               than "www"  will be >= "ww2", or in other words will share the 
               common prefix "ww". Future values that compare greater than www
               may not share this common prefix though, which is why we must
               keep track of the prefixes of *both* the last lower and upper 
               bound
             */
#ifndef NDEBUG
             std::cout << lhs << " < " << rhs << ": " << result.second 
                       << std::endl;
#endif
             assert (prefix_lb <= result.second);
             prefix_lb = result.second;
        } else if (result.first > 0) {
#ifndef NDEBUG
             std::cout << lhs << " > " << rhs << ": " << result.second 
                       << std::endl;
#endif
             assert (prefix_ub <= result.second);
             prefix_ub = result.second;
        } else {
#ifndef NDEBUG
             std::cout << lhs << " == " << rhs << ": " << result.second 
                       << std::endl;
             std::cout << prefix_lb << ", " << prefix_ub << std::endl;
#endif
             assert (prefix_lb <= result.second);
             assert (prefix_ub <= result.second);
             //prefix_lb = result.second;
             //prefix_ub = result.second;
        }
        
        return result;
    }
};

template <typename SortedDNSNames>
typename boost::range_iterator<SortedDNSNames>::type
__attribute__((noinline))
find_root (
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
            = my::equal_range (notless, greater, *label++, nth_label_cmp{i});
        if (notless == greater) {
            break;
        }
        if (std::next(notless) == greater) {
            return notless;
        }
    }
    return end (sorted_names);
}
/*
#include <algorithm>
#include <iterator>
#include <queue>
#include <iomanip>

template <typename Iterator>
struct sorted_range {
    Iterator begin;
    Iterator end;
    std::size_t size;
};

template <typename Iterator> inline
auto
make_sorted_range (Iterator begin, Iterator end, std::size_t size) {
    return sorted_range<Iterator>{begin, end, size};
}

template <typename Iterator, typename Callback>
void
level_order (Iterator begin, Iterator end, Callback&& cb) {
    auto count = static_cast<std::size_t>(std::distance (begin, end));
    if (!count) {
        return;
    }
    std::queue<sorted_range<Iterator>> queue;
    queue.push (make_sorted_range (begin, end, count));
    do {
        auto& tree = queue.front();
        auto step = tree.size >> 1;
        auto middle = std::next (tree.begin, step);
        cb (*middle);
        if (step) {
            queue.push (make_sorted_range (tree.begin, middle, step));
            auto rhsize = tree.size - (step + 1);
            if (rhsize) {
                queue.push (make_sorted_range (++middle, tree.end, rhsize));
            }
        }
        queue.pop();
    } while (!queue.empty());
}
*/

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
              
    /* BFS
     
    start = std::chrono::steady_clock::now();
    std::vector<DNSName> bfs;
    bfs.reserve (block_list.size());
    level_order (block_list.begin(), block_list.end(), 
                [&bfs](auto const& name) {
                    bfs.push_back (name);
                });
    stop = std::chrono::steady_clock::now();
    for (auto i = 0; i < 10; ++i){
        std::cout << bfs[i] << std::endl;
    }
    std::cout << "BFS took "
              << std::chrono::duration_cast<std::chrono::milliseconds>
                 (stop - start).count()
              << " ms" << std::endl;*/

    /* Lookup something
     */
    std::cout << "Enter a domain to lookup" << std::endl;
    {
        std::cout << "> ";
        std::getline (std::cin, line);
        DNSName name (std::move (line));

        decltype(find_root(name, block_list)) blocker;
        auto const iterations = 100000000ul;
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
