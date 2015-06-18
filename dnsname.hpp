#include <string>
#include <boost/utility/string_ref.hpp> 
#include <boost/iterator/iterator_facade.hpp>

using DNSLabel = boost::string_ref;

class DNSLabelIterator final: public boost::iterator_facade<
    DNSLabelIterator, 
    DNSLabel,
    boost::iterators::bidirectional_traversal_tag,
    DNSLabel
>{
    friend class DNSName;
    public:
        DNSLabel dereference() const noexcept;
        bool equal (DNSLabelIterator const&) const noexcept;
        void increment() noexcept;
        void decrement() noexcept;
    private:
        char const* label_;
        char len_;
    private:
        DNSLabelIterator
        (char const* label, char len): label_(label), len_(len) {}
};

class DNSName final {
    public:
        explicit DNSName (std::string);
        void get_raw (std::string& s) const {
            s = str_;
        }
        DNSLabelIterator begin() const noexcept;
        DNSLabelIterator end() const noexcept;
    private:
        std::string str_;
        char fll_; // First label length
        char lll_; // Last label length
};

