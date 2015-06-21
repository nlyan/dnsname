#include "dnsname.hpp"

#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE dnsname_test
#include <boost/test/included/unit_test.hpp>
#include <vector>
#include <numeric>
#include <sstream>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"

BOOST_AUTO_TEST_CASE (dots) {
    BOOST_CHECK_NO_THROW (DNSName (""));
    BOOST_CHECK_NO_THROW (DNSName ("."));
    BOOST_CHECK_EQUAL (DNSName (""), DNSName (""));
    BOOST_CHECK_EQUAL (DNSName ("."), DNSName ("."));
    BOOST_CHECK_EQUAL (DNSName (""), DNSName ("."));
    BOOST_CHECK_EQUAL (DNSName ("."), DNSName (""));
    BOOST_CHECK_EQUAL (DNSName ("com"), DNSName ("com."));
    BOOST_CHECK_EQUAL (DNSName ("com."), DNSName ("com"));
    BOOST_CHECK_EQUAL (DNSName ("example.com"), DNSName ("example.com."));
    BOOST_CHECK_EQUAL (DNSName ("example.com."), DNSName ("example.com"));
    BOOST_CHECK_EQUAL (DNSName ("mail.example.com"), DNSName ("mail.example.com"));
    BOOST_CHECK_EQUAL (DNSName ("mail.example.com"), DNSName ("mail.example.com."));
    BOOST_CHECK_EQUAL (DNSName ("mail.example.com."), DNSName ("mail.example.com"));
    BOOST_CHECK_THROW (DNSName (".com"), BadDNSName);
    BOOST_CHECK_THROW (DNSName ("..com"), BadDNSName);
    BOOST_CHECK_THROW (DNSName ("com.."), BadDNSName);
    BOOST_CHECK_THROW (DNSName (".example.com."), BadDNSName);
    BOOST_CHECK_THROW (DNSName ("..example.com."), BadDNSName);
    BOOST_CHECK_THROW (DNSName ("mail..example.com"), BadDNSName);
    BOOST_CHECK_THROW (DNSName ("mail.example..com"), BadDNSName);
    BOOST_CHECK_THROW (DNSName ("mail.example.com.."), BadDNSName);
}

BOOST_AUTO_TEST_CASE (escapes) {
    BOOST_CHECK_EQUAL (DNSName (R"(\com)"), DNSName (R"(\com)"));
    BOOST_CHECK_EQUAL (DNSName (R"(\com)"), DNSName ("com"));
    BOOST_CHECK_EQUAL (DNSName (R"(\c\o\m)"), DNSName ("com"));
    BOOST_CHECK_EQUAL (DNSName (R"(\c\o\m\.)"), DNSName (R"(com\..)"));
    /* Checks termination */
    BOOST_CHECK_THROW (DNSName (R"(\)"), BadDNSName);
    BOOST_CHECK_THROW (DNSName (R"(\\\)"), BadDNSName);
    BOOST_CHECK_THROW (DNSName (R"(\\\\..)"), BadDNSName);
    BOOST_CHECK_NO_THROW (DNSName (R"(\.)"));
    BOOST_CHECK_NO_THROW (DNSName (R"(\\)"));
}

BOOST_AUTO_TEST_CASE (case_insensitivity) {
    BOOST_CHECK_EQUAL (DNSName ("Com"), DNSName ("com"));
    BOOST_CHECK_EQUAL (DNSName ("com"), DNSName ("coM"));
    BOOST_CHECK_EQUAL (DNSName ("cOm"), DNSName ("CoM"));
    BOOST_CHECK_EQUAL (DNSName ("abcdefghijklmnopqrstuvwxyz"), 
                       DNSName ("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
    BOOST_CHECK_EQUAL (DNSName ("ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 
                       DNSName ("abcdefghijklmnopqrstuvwxyz"));
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

BOOST_AUTO_TEST_CASE (streaming) {
    std::ostringstream oss;
    std::string str ("www.google.com");
    oss << DNSName (str);
    BOOST_CHECK_EQUAL (oss.str(), str);

    oss.clear ();
    oss.str (std::string());
    str = "www.Goo\\032gle.com";
    oss << DNSName (str);
    BOOST_CHECK_EQUAL (oss.str(), str);
    
    oss.clear ();
    oss.str (std::string());
    str = "www.goo\\046gle.com";
    oss << DNSName (str);
    BOOST_CHECK_EQUAL (oss.str(), "www.goo\\.gle.com");
}

#pragma clang diagnostic pop
