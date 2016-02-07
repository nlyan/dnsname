#include "dnschar.hpp"
#include "lexicographical_3way.hpp"
#include <iterator>                     // for begin, end, reverse_iterator
#include <algorithm>                    // for equal, lexicographical_compare
#include <stdexcept>                    // for runtime_error
#include <iosfwd>                       // for forward decl of ostream
#include <cassert>                      // for assert
#include <string>
#include <boost/utility/string_ref.hpp>
#include <boost/container/string.hpp>
#include <boost/iterator/iterator_facade.hpp>

using dns_string = boost::container::basic_string<char, DNSCharTraits>;
using dns_string_ref = boost::basic_string_ref<char, DNSCharTraits>;


struct DNSLabel: public dns_string_ref {
    template <typename... Args> explicit
    DNSLabel (Args&&... args): dns_string_ref (std::forward<Args>(args)...) {}

    DNSLabel (DNSLabel&&) = default;
    DNSLabel (DNSLabel const&) = delete;
    DNSLabel& operator= (DNSLabel&&) = default;
    DNSLabel& operator= (DNSLabel const&) = delete;

    operator dns_string() const { return dns_string(data(), size()); }
    operator std::string() const { return std::string(data(), size()); }
};


struct BadDNSName final: public std::runtime_error {
    explicit
    BadDNSName (char const* msg) noexcept: std::runtime_error (msg) {}
    BadDNSName () noexcept: std::runtime_error ("Bad DNS name") {}
    BadDNSName (BadDNSName &&) = default;
    BadDNSName (BadDNSName const&) = default;
    ~BadDNSName() noexcept;
};


class DNSLabelIterator final: public boost::iterator_facade<
    DNSLabelIterator,
    dns_string,
    std::bidirectional_iterator_tag,
    DNSLabel
>{
    friend class DNSName;
    friend class boost::iterator_core_access;

    public:
        constexpr
        DNSLabelIterator() noexcept: label_(nullptr), len_(0) {}

    private:
        DNSLabelIterator (char const* label, char len) noexcept:
            label_(label), len_(len) {
        }

        bool
        equal (DNSLabelIterator const& rhs) const noexcept {
            return ((len_ == rhs.len_) &&
                    ((len_ == 0) || (label_ == rhs.label_)));
        }

        DNSLabel
        dereference() const noexcept {
            assert (len_ >= 0);
            return DNSLabel (label_, static_cast<std::size_t>(len_));
        }

        void
        increment() noexcept {
            assert (len_ != 0);
            std::advance (label_, len_);
            len_ ^= *label_++;
        }

        void
        decrement() noexcept {
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
        DNSName () noexcept;
        DNSName (std::string);
        DNSName (char const*) = delete;

        DNSLabelIterator
        begin() const noexcept {
            if (fll_) {
                return DNSLabelIterator (str_.c_str(), fll_);
            } else {
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

        auto
        label_count() const noexcept {
            return lcount_;
        }

        template <typename Label>
        void
        push_back (Label const& label) {
#ifndef NDEBUG
            check();
#endif
            assert (label.size() <= 63);
            if (label_count()) {
                str_ += static_cast<char>(lll_ ^ label.size());
            } else {
                fll_ = static_cast<char>(label.size());
            }
            str_.append (label.data(), label.size());
            lll_ = static_cast<char>(label.size());
            ++lcount_;
#ifndef NDEBUG
            check();
#endif
        }

        void
        pop_back() {
#ifndef NDEBUG
            check();
#endif
            str_.resize (str_.size() - lll_);
            --lcount_;
            if (label_count()) {
                lll_ ^= str_.back();
                str_.pop_back();
            } else {
                assert (str_.empty());
                fll_ = 0;
                lll_ = 0;
            }
#ifndef NDEBUG
            check();
#endif
        }

    private:
        std::string str_;
        char fll_;              /* First label length */
        char lll_;              /* Last label length */
        unsigned char lcount_;  /* Number of labels */

#ifndef NDEBUG
        void check_not_empty() const;
        void check_empty() const;
        void check() const;
#endif
};


inline bool
operator== (DNSName const& n1, DNSName const& n2) noexcept {
    using std::begin;
    using std::end;
    return std::equal (begin(n1), end(n1), begin(n2), end(n2));
}


inline bool
operator< (DNSName const& n1, DNSName const& n2) noexcept {
    using std::rbegin;
    using std::rend;
    return std::lexicographical_compare
           (rbegin(n1), rend(n1), rbegin(n2), rend(n2));
}

std::ostream& operator<< (std::ostream&, DNSLabel const&);
std::ostream& operator<< (std::ostream&, DNSName const&);

