#include <string>

#define dnsname_unlikely(X) __builtin_expect(!!(X), 0)

struct DNSCharTraits: std::char_traits<char> {
    static constexpr bool
    is_upper (char_type const c) noexcept {
        return ((c >= 'A') && (c <= 'Z'));
    }

    static constexpr bool
    is_lower (char_type const c) noexcept {
        return ((c >= 'a') && (c <= 'z'));
    }

    static char_type
    to_lower (char_type c) noexcept {
        if (is_upper(c)) { c |= 0x20; }
        return c;
    }

    static void
    lower (char_type& c1, char_type& c2) noexcept {
        c1 = to_lower(c1);
        c2 = to_lower(c2);
    }

    static int_type
    compare (char_type const* s1, char_type const* s2, std::size_t n) noexcept {
        while (n--) {
            auto c1 = static_cast<unsigned char>(*s1);
            auto c2 = static_cast<unsigned char>(*s2);
            int_type const d = { c1 - c2 };
            if ((!d) || dnsname_unlikely ((d == -0x20) && is_upper (*s1))
                     || dnsname_unlikely ((d ==  0x20) && is_lower (*s1))) {
                ++s1;
                ++s2;
                continue;
            }
            return d;
        }
        return 0;
    }

    static bool
    eq (char_type c1, char_type c2) noexcept {
        lower (c1, c2);
        return (c1 == c2);
    }

    static bool
    lt (char_type c1, char_type c2) noexcept {
        lower (c1, c2);
        return (static_cast<unsigned char>(c1) < static_cast<unsigned char>(c2));
    }
};

#undef dnsname_unlikely
