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
    public:
        DNSLabel dereference() const noexcept;
        bool equal (DNSLabelIterator const&) const noexcept;
        void increment() noexcept;
        void decrement() noexcept;
    private:
        std::string::const_iterator str_it_;
        char llen_;
};

class DNSName final {
    public:
        explicit DNSName (std::string);
        void get_raw (std::string& s) const {
            s = str_;
        }
    private:
        std::string str_;
        char fll_; // First label length
        char lll_; // Last label length
};

