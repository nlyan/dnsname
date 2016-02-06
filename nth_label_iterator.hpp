#include "dnsname.hpp"
#include <boost/iterator/iterator_facade.hpp>
#include <iostream>

template <typename NameIterator>
struct nth_label_iterator: public boost::iterator_facade<
    nth_label_iterator<NameIterator>,   // Derived
    dns_string,                         // Value
    std::random_access_iterator_tag,    // CategoryOrTraversal
    DNSLabel                            // Reference
>{
    friend class boost::iterator_core_access;

    public:
        using name_reference = typename std::iterator_traits
                                    <NameIterator>::reference;

        template <typename... Args> explicit
        nth_label_iterator (std::size_t const n = 0, Args&&... args) noexcept:
            name_(std::forward<Args>(args)...),
            index_(n) {
        }

        auto name() const {
            return name_;
        }

        void bump() noexcept {
            ++index_;
            name_reference name = *name_;
            if (label_ != name.rend()) {
                ++label_;
            }
        }

    private:
        bool
        equal (nth_label_iterator<NameIterator> const& rhs) const noexcept {
            return ((name_ == rhs.name_) && (index_ == rhs.index_));
        }

        DNSLabel
        dereference() const noexcept {
            name_reference name = *name_;
            if (!ready_) {
                label_ = next (name.rbegin(), index_, name.rend());
                ready_ = true;
            }
            if (label_ == name.rend()) {
                return DNSLabel();
            }
            return *label_;
        }

        void increment() {
            ++name_;
            ready_ = false;
        }

        void decrement() {
            --name_;
            ready_ = false;
        }
        
        void
        advance (std::ptrdiff_t n) noexcept {
            std::advance (name_, n);
        }
        
        auto 
        distance_to 
        (nth_label_iterator<NameIterator> const& rhs) const noexcept {
            return std::distance (name_, rhs.name_);
        }

    private:
        NameIterator name_;
        std::reverse_iterator<DNSLabelIterator> mutable label_;
        std::uint8_t index_;
        bool mutable ready_ = false;
};


template <typename NameIterator> static inline
auto
make_nth_label_iterator (NameIterator&& name, std::size_t const n = 0) {
    using name_iterator_type = typename std::decay<NameIterator>::type;
    return nth_label_iterator<name_iterator_type>
                (n, std::forward<NameIterator>(name));
}

