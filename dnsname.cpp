#include "dnsname.hpp"
#include "dnslabel.hpp"
#include <iostream>
#include <iterator>

DNSLabel
DNSLabelIterator::dereference() const noexcept {
    return DNSLabel (&*str_it_, llen_);
}

bool 
DNSLabelIterator::equal (DNSLabelIterator const& o) const noexcept {
    return ((llen_ == o.llen_) && (str_it_ == o.str_it_));
}

void
DNSLabelIterator::increment() noexcept {
    std::advance (str_it_, llen_);
    llen_ ^= *str_it_++;
}

void
DNSLabelIterator::decrement() noexcept {
    llen_ ^= *--str_it_;
    std::advance (str_it_, -1 * llen_);
}

DNSName::DNSName 
(std::string str): str_(std::move (str)), fll_(0), lll_(0) {   
    auto out = begin(str_);
    char* prev_llen_ptr = &fll_;
    
    auto name_len = parse_dns_name_cstr(
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
            *prev_llen_ptr ^= llen;
            prev_llen_ptr = std::addressof(*out++);
            *prev_llen_ptr = static_cast<char>(llen);
            lll_ = static_cast<char>(llen);
        }
    );
    
    str_.resize (name_len);
}


#include <vector>

int main() {
    std::vector<DNSName> names;
    names.emplace_back ("www1.cdn3x.microsoft.com");
}

