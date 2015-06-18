#include "dnsname.hpp"
#include "dnslabel.hpp"
#include <iterator>

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
    
    auto name_len = parse_dns_name_cstr (
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
    /* This looks dangerous but we use the null terminator, hence
     * the 'end' is actually the byte after it. 
     */
    return DNSLabelIterator (str_.c_str() + str_.size() + 1, lll_);
}

#include <vector>
#include <iostream>

int main() {
    std::vector<DNSName> names;
    names.emplace_back ("www1.cdn3x.microsoft.museum");

    std::copy (names[0].begin(), names[0].end(),
        std::ostream_iterator<DNSLabel>(std::cout, "\n"));
}


