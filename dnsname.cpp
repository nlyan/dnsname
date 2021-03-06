#include "dnsname.hpp"
#include "dnsparse.hpp"
#include <ostream>

BadDNSName::~BadDNSName() noexcept {
}

DNSName::DNSName
() noexcept: fll_(0), lll_(0), lcount_(0) {
}

DNSName::DNSName
(std::string str): str_(std::move (str)), fll_(0), lll_(0), lcount_(0) {
    if (str_ == ".") {
        str_.clear();
    }
    if (str_.empty()) {
        return;
    }

    auto out = std::begin (str_);
    char* llen_ptr = &fll_;

    auto const name_len = parse_dnsname (
        std::cbegin (str_),
        std::cend (str_),
        [&](char c) {
            *out++ = c;
        },
        [&](unsigned llen) {
            if (llen > 63) {
                throw BadDNSName ("DNS label is too long");
            }
            *llen_ptr = lll_ ^ static_cast<char>(llen);
            llen_ptr = std::addressof (*out++);
            lll_ = static_cast<char>(llen);
            ++lcount_;
        }
    );

    if (name_len > 253) {
        throw BadDNSName ("DNS name is too long");
    }

    str_.resize (name_len);
#ifndef NDEBUG
    check();
#endif
}

#ifndef NDEBUG
void
DNSName::check_not_empty() const {
    assert (label_count());
    assert (label_count() <= 127);
    assert (str_.size() >= (2 * label_count() - 1));
    assert (str_.size() <= 253);
    assert (fll_);
    assert (lll_);
    assert (fll_ <= 63);
    assert (lll_ <= 63);
    if (label_count() == 1) {
        assert (fll_ == lll_);
        assert (str_.size() == fll_);
    } else {
        auto min_size = 2 * label_count() - 1;
        min_size += fll_ - 1;
        min_size += lll_ - 1;
        assert (str_.size() >= min_size);
    }
    for (auto const& label: *this) {
        assert (label.size());
        assert (label.size() <= 63);
    }
}

void
DNSName::check_empty() const {
    assert (!label_count());
    assert (!fll_);
    assert (!lll_);
    assert (str_.empty());
}

void
DNSName::check() const {
    if (label_count()) {
        check_not_empty();
    } else {
        check_empty();
    }
}
#endif

static const char decimal_escape_seqs[162][3] = {
    {'1','2','7'},{'1','2','8'},{'1','2','9'},{'1','3','0'},{'1','3','1'},
    {'1','3','2'},{'1','3','3'},{'1','3','4'},{'1','3','5'},{'1','3','6'},
    {'1','3','7'},{'1','3','8'},{'1','3','9'},{'1','4','0'},{'1','4','1'},
    {'1','4','2'},{'1','4','3'},{'1','4','4'},{'1','4','5'},{'1','4','6'},
    {'1','4','7'},{'1','4','8'},{'1','4','9'},{'1','5','0'},{'1','5','1'},
    {'1','5','2'},{'1','5','3'},{'1','5','4'},{'1','5','5'},{'1','5','6'},
    {'1','5','7'},{'1','5','8'},{'1','5','9'},{'1','6','0'},{'1','6','1'},
    {'1','6','2'},{'1','6','3'},{'1','6','4'},{'1','6','5'},{'1','6','6'},
    {'1','6','7'},{'1','6','8'},{'1','6','9'},{'1','7','0'},{'1','7','1'},
    {'1','7','2'},{'1','7','3'},{'1','7','4'},{'1','7','5'},{'1','7','6'},
    {'1','7','7'},{'1','7','8'},{'1','7','9'},{'1','8','0'},{'1','8','1'},
    {'1','8','2'},{'1','8','3'},{'1','8','4'},{'1','8','5'},{'1','8','6'},
    {'1','8','7'},{'1','8','8'},{'1','8','9'},{'1','9','0'},{'1','9','1'},
    {'1','9','2'},{'1','9','3'},{'1','9','4'},{'1','9','5'},{'1','9','6'},
    {'1','9','7'},{'1','9','8'},{'1','9','9'},{'2','0','0'},{'2','0','1'},
    {'2','0','2'},{'2','0','3'},{'2','0','4'},{'2','0','5'},{'2','0','6'},
    {'2','0','7'},{'2','0','8'},{'2','0','9'},{'2','1','0'},{'2','1','1'},
    {'2','1','2'},{'2','1','3'},{'2','1','4'},{'2','1','5'},{'2','1','6'},
    {'2','1','7'},{'2','1','8'},{'2','1','9'},{'2','2','0'},{'2','2','1'},
    {'2','2','2'},{'2','2','3'},{'2','2','4'},{'2','2','5'},{'2','2','6'},
    {'2','2','7'},{'2','2','8'},{'2','2','9'},{'2','3','0'},{'2','3','1'},
    {'2','3','2'},{'2','3','3'},{'2','3','4'},{'2','3','5'},{'2','3','6'},
    {'2','3','7'},{'2','3','8'},{'2','3','9'},{'2','4','0'},{'2','4','1'},
    {'2','4','2'},{'2','4','3'},{'2','4','4'},{'2','4','5'},{'2','4','6'},
    {'2','4','7'},{'2','4','8'},{'2','4','9'},{'2','5','0'},{'2','5','1'},
    {'2','5','2'},{'2','5','3'},{'2','5','4'},{'2','5','5'},{'0','0','0'},
    {'0','0','1'},{'0','0','2'},{'0','0','3'},{'0','0','4'},{'0','0','5'},
    {'0','0','6'},{'0','0','7'},{'0','0','8'},{'0','0','9'},{'0','1','0'},
    {'0','1','1'},{'0','1','2'},{'0','1','3'},{'0','1','4'},{'0','1','5'},
    {'0','1','6'},{'0','1','7'},{'0','1','8'},{'0','1','9'},{'0','2','0'},
    {'0','2','1'},{'0','2','2'},{'0','2','3'},{'0','2','4'},{'0','2','5'},
    {'0','2','6'},{'0','2','7'},{'0','2','8'},{'0','2','9'},{'0','3','0'},
    {'0','3','1'},{'0','3','2'}
};

std::ostream&
operator<< (std::ostream& os, DNSLabel const& label) {
    std::ostream::sentry sentry (os);
    if (!sentry) {
        return os;
    }
    std::ostreambuf_iterator<char> out(os);
    bool const nouppercase = !(os.flags() & std::ios_base::uppercase);
    for (char c: label) {
        if ((c == '.') || (c == '\\')) {
            *out++ = '\\';
            *out++ = c;
        } else if ((c < 0x21) || (c > 0x7e)) {
            auto uc = static_cast<unsigned char>(c);
            uc += 0x81;
            *out++ = '\\';
            *out++ = decimal_escape_seqs[uc][0];
            *out++ = decimal_escape_seqs[uc][1];
            *out++ = decimal_escape_seqs[uc][2];
        } else {
            if (nouppercase) {
                DNSCharTraits::lower(c);
            }
            *out++ = c;
        }
    }
    return os;
}

std::ostream&
operator<< (std::ostream& os, DNSName const& name) {
    using std::begin;
    using std::end;
    std::ostream::sentry sentry (os);
    if (!sentry) {
        return os;
    }
    std::ostreambuf_iterator<char> out(os);
    auto l_it = begin (name);
    auto const l_end = end (name);
    if (l_it != l_end) {
        os << *l_it++;
        while (l_it != l_end) {
            *out++ = '.';
            os << *l_it++;
        }
    }
    if (os.flags() & std::ios_base::showpoint) {
        *out++ = '.';
    }
    return os;
}
