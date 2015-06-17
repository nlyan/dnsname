#include <string>

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

