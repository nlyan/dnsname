#include <iterator>
#include <string>
#include <stdexcept>
#include <iosfwd>
#include <boost/utility/string_ref.hpp> 
#include <boost/iterator/iterator_facade.hpp>

using DNSLabel = boost::string_ref;

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
        void increment() noexcept;
        void decrement() noexcept;
        bool equal (DNSLabelIterator const&) const noexcept;
        DNSLabel dereference() const noexcept;
    private:
        char const* label_;
        char len_;
};


class DNSName final {
    public:
        DNSName (std::string);
        DNSLabelIterator begin() const noexcept;
        DNSLabelIterator end() const noexcept;
        std::reverse_iterator<DNSLabelIterator> rbegin() const noexcept;
        std::reverse_iterator<DNSLabelIterator> rend() const noexcept;
    private:
        std::string str_;
        char fll_; /* First label length */
        char lll_; /* Last label length */
};

bool operator== (DNSName const&, DNSName const&) noexcept;
bool operator< (DNSName const&, DNSName const&) noexcept;
std::ostream& operator<< (std::ostream&, DNSName const&);

