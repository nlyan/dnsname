#include "dnsname.hpp"

#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE dnsname_test
#include <boost/test/included/unit_test.hpp>
#include <vector>
#include <numeric>                              /* For iota */
#include <sstream>                              /* For stringstream */
#include <fstream>                              /* For ifstream */
#include <algorithm>                            /* For next_permutation etc */
#include <locale>

using namespace std::literals::string_literals;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"

BOOST_AUTO_TEST_CASE (dots) {
    BOOST_CHECK_NO_THROW (DNSName (""s));
    BOOST_CHECK_NO_THROW (DNSName ("."s));
    BOOST_CHECK_EQUAL (DNSName (""s), DNSName (""s));
    BOOST_CHECK_EQUAL (DNSName ("."s), DNSName ("."s));
    BOOST_CHECK_EQUAL (DNSName (""s), DNSName ("."s));
    BOOST_CHECK_EQUAL (DNSName ("."s), DNSName (""s));
    BOOST_CHECK_EQUAL (DNSName ("com"s), DNSName ("com."s));
    BOOST_CHECK_EQUAL (DNSName ("com."s), DNSName ("com"s));
    BOOST_CHECK_EQUAL (DNSName ("example.com"s), DNSName ("example.com."s));
    BOOST_CHECK_EQUAL (DNSName ("example.com."s), DNSName ("example.com"s));
    BOOST_CHECK_EQUAL (DNSName ("mail.example.com"s), DNSName ("mail.example.com"s));
    BOOST_CHECK_EQUAL (DNSName ("mail.example.com"s), DNSName ("mail.example.com."s));
    BOOST_CHECK_EQUAL (DNSName ("mail.example.com."s), DNSName ("mail.example.com"s));
    BOOST_CHECK_EQUAL (DNSName (".com"s), DNSName ("com"s));
    BOOST_CHECK_EQUAL (DNSName (".example.com."s), DNSName ("example.com."s));
    BOOST_CHECK_THROW (DNSName ("..com"s), BadDNSName);
    BOOST_CHECK_THROW (DNSName ("com.."s), BadDNSName);
    BOOST_CHECK_THROW (DNSName ("..example.com."s), BadDNSName);
    BOOST_CHECK_THROW (DNSName ("mail..example.com"s), BadDNSName);
    BOOST_CHECK_THROW (DNSName ("mail.example..com"s), BadDNSName);
    BOOST_CHECK_THROW (DNSName ("mail.example.com.."s), BadDNSName);
}

BOOST_AUTO_TEST_CASE (escapes) {
    BOOST_CHECK_EQUAL (DNSName (R"(\com)"s), DNSName (R"(\com)"s));
    BOOST_CHECK_EQUAL (DNSName (R"(\com)"s), DNSName ("com"s));
    BOOST_CHECK_EQUAL (DNSName (R"(\c\o\m)"s), DNSName ("com"s));
    BOOST_CHECK_EQUAL (DNSName (R"(\c\o\m\.)"s), DNSName (R"(com\..)"s));
    /* Checks termination */
    BOOST_CHECK_THROW (DNSName (R"(\)"s), BadDNSName);
    BOOST_CHECK_THROW (DNSName (R"(\\\)"s), BadDNSName);
    BOOST_CHECK_THROW (DNSName (R"(\\\\..)"s), BadDNSName);
    BOOST_CHECK_NO_THROW (DNSName (R"(\.)"s));
    BOOST_CHECK_NO_THROW (DNSName (R"(\\)"s));
    BOOST_CHECK_THROW (DNSName ("www.google.com.\x00subdomain.domain.com"s),
                       BadDNSName);
}

BOOST_AUTO_TEST_CASE (case_insensitivity) {
    BOOST_CHECK_EQUAL (DNSName ("Com"s), DNSName ("com"s));
    BOOST_CHECK_EQUAL (DNSName ("com"s), DNSName ("coM"s));
    BOOST_CHECK_EQUAL (DNSName ("cOm"s), DNSName ("CoM"s));
    BOOST_CHECK_EQUAL (DNSName ("abcdefghijklmnopqrstuvwxyz"s), 
                       DNSName ("ABCDEFGHIJKLMNOPQRSTUVWXYZ"s));
    BOOST_CHECK_EQUAL (DNSName ("ABCDEFGHIJKLMNOPQRSTUVWXYZ"s), 
                       DNSName ("abcdefghijklmnopqrstuvwxyz"s));
}

BOOST_AUTO_TEST_CASE (basic_printable) {
    std::string printable;
    printable.resize ((0x7e - 0x21) + 1);
    std::iota (begin (printable), end (printable), 0x21);
    
    /* Split to avoid max label length */
    auto split = std::next(begin (printable), 64);
    std::string p1 (begin (printable), split);
    std::string p2 (split, end (printable));
    
    BOOST_CHECK_EQUAL (p1.front(), 0x21);
    BOOST_CHECK_EQUAL (p2.back(), 0x7e);
    BOOST_CHECK_NO_THROW (DNSName x(p1));
    BOOST_CHECK_NO_THROW (DNSName x(p2));
}
    
BOOST_AUTO_TEST_CASE (printable_equality) {
    /* All printables except uppercase characters */
    std::string printable_excl_uc;
    printable_excl_uc.resize ((0x7e - 0x21) + 1);
    std::iota (begin (printable_excl_uc), end (printable_excl_uc), 0x21);
    printable_excl_uc.erase (
        std::remove_if (begin (printable_excl_uc), end (printable_excl_uc), 
                        [](char c) { return ((c >= 'A') && (c <= 'Z')); }),
        end (printable_excl_uc)
    );
    
    /* Split to avoid max label length */
    auto split = std::next(begin (printable_excl_uc), 64);
    std::string p1 (begin (printable_excl_uc), split);
    std::string p2 (split, end (printable_excl_uc));
    
    BOOST_CHECK_EQUAL (p1.front(), 0x21);
    BOOST_CHECK_EQUAL (p2.back(), 0x7e);
    BOOST_CHECK_NO_THROW (DNSName x(p1));
    BOOST_CHECK_NO_THROW (DNSName x(p2));
    
    /* Permute such that every char is compared to at least every other char */
    std::string p1p (p1);
    std::string p2p (p2);
    for (auto i = 0u; i < p1p.size(); ++i) {
        std::next_permutation (begin (p1), end (p1));
        BOOST_CHECK_NE (DNSName (p1), DNSName (p1p));
    }
    for (auto i = 0u; i < p2p.size(); ++i) {
        std::next_permutation (begin (p2), end (p2));
        BOOST_CHECK_NE (DNSName (p2), DNSName (p2p));
    }
}

BOOST_AUTO_TEST_CASE (decimal_escapes) {
    /* 1 digit */
    for (auto i = 0; i < 10; ++i) {
        std::ostringstream oss;
        oss << "\\" << i;
        BOOST_CHECK_THROW (DNSName (oss.str()), BadDNSName);
    }
    /* 2 digits */
    for (auto i = 0; i < 100; ++i) {
        std::ostringstream oss;
        oss << "\\" << std::setw(2) << std::setfill('0') << i;
        BOOST_CHECK_THROW (DNSName (oss.str()), BadDNSName);
    }
    /* 3 digits */
    for (auto i = 0; i < 256; ++i) {
        std::ostringstream oss;
        oss << "\\" << std::setw(3) << std::setfill('0') << i;
        BOOST_CHECK_NO_THROW (DNSName x(oss.str()));
    }
    /* Equality with escaped, unencoded, raw bytes e.g. "\\\x00" == R"(\000)" */
    for (auto i = 0; i < 48; ++i) {
        std::ostringstream oss;
        oss << "\\" << std::setw(3) << std::setfill('0') << i;
        std::string s ("\\"s);
        s.append (1, static_cast<char>(i));
        BOOST_CHECK_EQUAL (DNSName (oss.str()), DNSName(s));
    }
    for (auto i = 58; i < 256; ++i) {
        std::ostringstream oss;
        oss << "\\" << std::setw(3) << std::setfill('0') << i;
        std::string s ("\\"s);
        s.append (1, static_cast<char>(i));
        BOOST_CHECK_EQUAL (DNSName (oss.str()), DNSName(s));
    }
}

BOOST_AUTO_TEST_CASE (streaming) {
    std::ostringstream oss;
    std::string str (R"(www.google.com)"s);
    oss << DNSName (str);
    BOOST_CHECK_EQUAL (oss.str(), str);

    oss.clear ();
    oss.str (std::string());
    oss << DNSName (R"(WwW.Goo\ GLe.cOm)"s);
    BOOST_CHECK_EQUAL (oss.str(), R"(www.goo\032gle.com)"s);
    
    oss.clear ();
    oss.str (std::string());
    auto flags = oss.flags();
    oss << std::showpoint << std::uppercase << DNSName (R"(WwW.Goo\ GLe.cOm)"s);
    oss.flags (flags);
    BOOST_CHECK_EQUAL (oss.str(), R"(WwW.Goo\032GLe.cOm.)"s);

    oss.clear ();
    oss.str (std::string());
    oss << DNSName (R"(www.goo\046gle.com.)"s);
    BOOST_CHECK_EQUAL (oss.str(), R"(www.goo\.gle.com)"s);
}

BOOST_AUTO_TEST_CASE (alexa) {
    std::ifstream csv ("testdata/top-1m-201506201230UTC.csv",
                       std::ios::in | std::ios::binary);
    csv.imbue (std::locale::classic());
    std::vector<DNSName> top1m;
    top1m.reserve (1000000);
    std::string line;

    for (std::size_t n = 1; n <= top1m.capacity(); ++n) {
        std::size_t l;
        csv >> l;
        assert (l == n);

        csv.ignore (1, ',');
        std::getline (csv, line, '\n');
        BOOST_CHECK_NO_THROW (top1m.emplace_back (line));

        std::ostringstream oss;
        oss << top1m.back();
        BOOST_CHECK_EQUAL (line, oss.str());
    }

    std::sort (begin(top1m), end(top1m));

    std::ifstream txt ("testdata/top-1m-201506201230UTC-sorted.txt",
                       std::ios::in | std::ios::binary);
    txt.imbue (std::locale::classic());
    std::vector<std::string> presorted;
    presorted.reserve (top1m.capacity());
    presorted.assign (std::istream_iterator<std::string>(txt),
                      std::istream_iterator<std::string>());
    BOOST_CHECK (std::equal(begin(top1m), end(top1m), 
                            begin(presorted), end(presorted)));
}

#pragma clang diagnostic pop
