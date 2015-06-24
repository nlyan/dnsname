#include "dnschar.hpp"
#include <string>
#include <iterator>                     // for begin, end, reverse_iterator
#include <algorithm>                    // for equal, lexicographical_compare
#include <stdexcept>                    // for runtime_error
#include <iosfwd>                       // for forward decl of ostream
#include <cassert>                      // for assert
#include <boost/utility/string_ref.hpp> 
#include <boost/iterator/iterator_facade.hpp>

using DNSLabel = boost::basic_string_ref<char, DNSCharTraits>;

class BadDNSName final: public std::runtime_error {
    public:
        BadDNSName () noexcept: std::runtime_error("Bad DNS name") {}
        BadDNSName (BadDNSName const&) = default;
        BadDNSName (BadDNSName &&) = default;
        ~BadDNSName() noexcept;
};


class DNSLabelIterator final: public boost::iterator_facade<
    DNSLabelIterator,
    DNSLabel,
    boost::iterators::bidirectional_traversal_tag,
    DNSLabel
>{
    friend class DNSName;
    friend class boost::iterator_core_access;
    private:
        DNSLabelIterator (char const* label, char len) noexcept:
            label_(label), len_(len) {
        }

        bool 
        equal (DNSLabelIterator const& o) const noexcept {
            return ((len_ == o.len_) && (label_ == o.label_));
        }
        
        DNSLabel
        dereference() const noexcept {
            return DNSLabel (label_, static_cast<std::size_t>(len_));
        }

        void increment() noexcept {
            assert (len_ != 0);
            std::advance (label_, len_);
            len_ ^= *label_++;
        }

        void decrement() noexcept {
            assert (len_ != 0);
            len_ ^= *--label_;
            std::advance (label_, -1 * len_);
        }
    private:
        char const* label_;
        char len_;
};


class DNSName final {
    public:
        DNSName (std::string);
        DNSName (char const*) = delete;
        
        DNSLabelIterator 
        begin() const noexcept {
            if (fll_) {
                return DNSLabelIterator (str_.c_str(), fll_);
            } else  {
                return end();
            }
        }

        DNSLabelIterator
        end() const noexcept {
            return DNSLabelIterator (str_.c_str() + str_.size() + 1, lll_);
        }

        std::reverse_iterator<DNSLabelIterator> 
        rbegin() const noexcept {
            return std::make_reverse_iterator(end());
        }

        std::reverse_iterator<DNSLabelIterator> 
        rend() const noexcept {
            return std::make_reverse_iterator(begin());
        }

        std::size_t
        label_count() const noexcept {
            return std::distance (begin(), end());
        }
    private:
        std::string str_;
        char fll_; /* First label length */
        char lll_; /* Last label length */
};


inline bool 
operator== (DNSName const& n1, DNSName const& n2) noexcept {
    using std::begin; using std::end;
    return std::equal (begin(n1), end(n1), begin(n2), end(n2));
}


inline bool 
operator< (DNSName const& n1, DNSName const& n2) noexcept {
    using std::rbegin; using std::rend;
    return std::lexicographical_compare 
           (rbegin(n1), rend(n1), rbegin(n2), rend(n2));
}

std::ostream& operator<< (std::ostream&, DNSLabel const&);
std::ostream& operator<< (std::ostream&, DNSName const&);

