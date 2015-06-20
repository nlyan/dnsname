#include "dnsname.hpp"
#include "dnslabel.hpp"
#include <ostream>
#include <iostream>

BadDNSName::~BadDNSName() noexcept {
}


DNSName::DNSName
(std::string str): str_(std::move (str)), fll_(0), lll_(0) {   
    auto out = std::begin (str_);
    char* llen_ptr = &fll_;
    
    if ((str_.empty()) || (str_ == ".")) {
        return;
    }
    if (count_trailing_backslashes (str_) & 1ul) {
        throw BadDNSName();
    }
    
    auto name_len = parse_dns_name_cstr_unsafe (
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


/* TODO: escape the string again */
std::ostream&
operator<< (std::ostream& os, DNSName const& name) {
    for (auto const& label: name) {
        os << label.data() << ".";
    }
    return os;
}
