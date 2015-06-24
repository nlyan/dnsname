#include "dnsname.hpp"
#include <fstream>
#include <iostream>
#include <tuple>
#include <iterator>
#include <iomanip>
#include <cstdint>

struct DNSNameRef {
    unsigned linenum;
    DNSName name;
    std::uint8_t shared_sufflen = 0;
    std::uint8_t prev_labelnum;
    unsigned prev_linenum;
};

inline bool
operator< (DNSNameRef const& x, DNSNameRef const& y) noexcept {
    return (x.name < y.name);
}

inline std::uint8_t
shared_suffix_length (DNSName const& n1, DNSName const& n2) noexcept {
    auto mismatched = std::mismatch (n1.rbegin(), n1.rend(),
                                     n2.rbegin(), n2.rend());
    return static_cast<std::uint8_t>(std::distance (n1.rbegin(),
                                                    mismatched.first));
}

inline void
update_backref (DNSNameRef& cur, DNSNameRef const& prev) {
    assert (prev.linenum < cur.linenum);
    auto const shared_sufflen = shared_suffix_length (cur.name, prev.name);

    /* Does the previous node have a longer common suffix with us than any
     * other node we know about? If so, point to it.
     */
    if (shared_sufflen > cur.shared_sufflen) {
        cur.shared_sufflen = shared_sufflen;
        if (shared_sufflen == prev.shared_sufflen) {
            /* The name we'd like to point into shares exactly the same suffix
             * that it shares with us, with an even earlier node. Bypass the
             * middle-man by copying its backreference forward.
             */
            cur.prev_linenum = prev.prev_linenum;
            cur.prev_labelnum = prev.prev_labelnum;
        } else {
            cur.prev_linenum = prev.linenum;
            cur.prev_labelnum = prev.name.label_count() - shared_sufflen;
        }
    }
}

int main() {
    std::vector<DNSNameRef> seen;
    decltype(seen)::iterator notlower;
    decltype(notlower) greater;
    std::string line;

    for (unsigned linenum = 0; std::getline (std::cin, line); ++linenum) {
        std::cout << std::setw(2) << linenum << ": ";
        DNSNameRef nameref { linenum, std::move (line) };
        std::tie (notlower, greater) = std::equal_range (begin(seen), end(seen),
                                                         nameref);
        if (notlower != greater) {
            /* We've seen this name before */
            std::cout << "^[" << notlower->linenum << ":0]";
        } else {
            /* We've not seen this name before, but... */
            if (notlower != begin (seen)) {
                /* ...we've seen a lexicographically lower name */
                update_backref (nameref, *std::prev(notlower));
            }
            if (greater != end (seen)) {
                /* ...and/or we've seen a lexicographically higher name */
                update_backref (nameref, *greater);
            }
            
            /* ...and no other names we've seen before can share a longer 
             * suffix 8) Dump it.
             */
            std::copy_n (nameref.name.begin(),
                         nameref.name.label_count() - nameref.shared_sufflen,
                         std::ostream_iterator<DNSLabel>(std::cout, "."));
            if (nameref.shared_sufflen) {
                std::cout << "^[" << nameref.prev_linenum
                          << ":"  << static_cast<unsigned>(nameref.prev_labelnum) << "]";
            }

            seen.insert (notlower, std::move(nameref));
        }
        std::cout << std::endl;
    }
}
