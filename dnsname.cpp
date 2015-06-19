#include "dnsname.hpp"
#include "dnslabel.hpp"
#include <ostream>
#include <iostream>

BadDNSName::~BadDNSName
() noexcept {
}

DNSLabel
DNSLabelIterator::dereference() const noexcept {
    return DNSLabel (label_, static_cast<std::size_t>(len_));
}

bool 
DNSLabelIterator::equal (DNSLabelIterator const& o) const noexcept {
    return ((len_ == o.len_) && (label_ == o.label_));
}

void
DNSLabelIterator::increment() noexcept {
    std::advance (label_, len_);
    len_ ^= *label_++;
}

void
DNSLabelIterator::decrement() noexcept {
    len_ ^= *--label_;
    std::advance (label_, -1 * len_);
}

DNSName::DNSName
(std::string str): str_(std::move (str)), fll_(0), lll_(0) {   
    auto out = std::begin(str_);
    char* llen_ptr = &fll_;
    
    if (str_.empty()) {
        return;
    }
    if (str_ == ".") {
        return;
    }
    if (count_trailing_backslashes (str_) & 1ul) {
        throw BadDNSName();
    }
    
    auto name_len = parse_dns_name_cstr_unsafe(
        str_.c_str(),
        [&](char c) {
            *out++ = c;
        },
        [&](unsigned llen, unsigned nlen) {
            if (llen > 63) {
                throw std::runtime_error ("DNS label is too long");
            }
            if (nlen > 253) {
                throw std::runtime_error ("DNS name is too long");
            }
            *llen_ptr = lll_ ^ static_cast<char>(llen);
            llen_ptr = std::addressof (*out++);
            lll_ = static_cast<char>(llen);
        }
    );

    str_.resize (name_len);
}

DNSLabelIterator
DNSName::begin() const noexcept {
    return DNSLabelIterator (str_.c_str(), fll_);
}

DNSLabelIterator
DNSName::end() const noexcept {
    return DNSLabelIterator (str_.c_str() + str_.size() + 1, lll_);
}

std::reverse_iterator<DNSLabelIterator>
DNSName::rbegin() const noexcept {
    return std::make_reverse_iterator(end());
}

std::reverse_iterator<DNSLabelIterator> 
DNSName::rend() const noexcept {
    return std::make_reverse_iterator(begin());
}

bool 
operator== 
(DNSName const& na, DNSName const& nb) noexcept {
    using std::begin;
    using std::end;
    return std::equal (
        begin(na), end(na), begin(nb), end(nb),
        [](DNSLabel la, DNSLabel lb) {
            return std::equal (begin(la), end(la), begin(lb), end(lb),
                [](char a, char b) {
                    if ((a >= 'A') && (a <= 'Z')) { a ^= 0x20; }
                    if ((b >= 'A') && (b <= 'Z')) { b ^= 0x20; }
                    return (a == b);
                });
        }
    );
}

bool 
operator<
(DNSName const& na, DNSName const& nb) noexcept {
    using std::begin; using std::end;
    using std::rbegin; using std::rend;
    return std::lexicographical_compare (
        rbegin(na), rend(na), rbegin(nb), rend(nb),
        [](DNSLabel la, DNSLabel lb) {
            return std::lexicographical_compare (
                begin(la), end(la), begin(lb), end(lb),
                [](char a, char b) {
                    if ((a >= 'A') && (a <= 'Z')) { a ^= 0x20; }
                    if ((b >= 'A') && (b <= 'Z')) { b ^= 0x20; }
                    return (a < b);
                }
            );
        }
    );
}

std::ostream&
operator<< (std::ostream& os, DNSName const& name) {
    for (auto const& label: name) {
        os << label << ".";
    }
    return os;
}
