#include <iterator>
#include <type_traits>
#include <utility>
#include <cassert>

#ifndef NDEBUG
#include <typeinfo>
#include <algorithm>
#include <iostream>
#endif
extern std::size_t cmpcnt;

#define LIKELY(X) __builtin_expect((X), true)
#define UNLIKELY(X) __builtin_expect((X), false)


/*template <typename Iterator, typename Distance> static inline
__attribute__((warn_unused_result))
Distance
advance (Iterator& it, Distance n, Iterator const end,
         std::input_iterator_tag) noexcept {
    assert (n >= Distance(0));
    while (n) {
        if (UNLIKELY (it == end)) {
            break;
        }
        ++it;
        --n;
    }
    return n;
}*/


template <typename Iterator, typename Distance> static inline
__attribute__((warn_unused_result, noinline))
Distance
advance (Iterator& it, Distance const n, Iterator const end,
         std::random_access_iterator_tag) noexcept {
    auto const zero = Distance(0);
    if (UNLIKELY (!n)) {
        return zero;
    }
    auto const have = std::distance (it, end);
    if (LIKELY (n > zero)) {
        if (LIKELY (have >= n)) {
            it += n;
            return zero;
        }
    } else if (UNLIKELY (n < zero)) {
        if (LIKELY (have <= n)) {
            it += n;
            return zero;
        }
    }
    it += have;
    return n - have;
}


template <typename Iterator, typename Distance> static inline
__attribute__((warn_unused_result, noinline))
Distance
advance (Iterator& it, Distance n, Iterator const end,
         std::bidirectional_iterator_tag) noexcept {
    auto const zero = Distance(0);
    if (LIKELY (n > zero)) {
        do {
            if (UNLIKELY (it == end)) {
                break;
            }
            ++it;
        } while (--n);
    } else if (UNLIKELY (n < zero)) {
        do {
            if (UNLIKELY (it == end)) {
                break;
            }
            --it;
        } while (++n);
    }
    return n;
}


template <typename Iterator, typename Distance> static inline
__attribute__((warn_unused_result, noinline))
auto
advance (Iterator& it, Distance n, Iterator const end) noexcept {
    return advance (it, n, end,
        typename std::iterator_traits<Iterator>::iterator_category());
}


template <typename Iterator, typename Distance> static inline
__attribute__((noinline))
Iterator
next (Iterator it, Distance n, Iterator const end) noexcept {
    if (advance (it, n, end)) {
        assert (it == end);
    }
    return it;
}

#include <cstdio>

template <typename Iterator1, typename Iterator2, typename CmpChar>
static inline __attribute__((noinline))
auto
lexicographical_compare_3way (
    Iterator1 l, Iterator1 const lend,
    Iterator2 r, Iterator2 const rend,
    CmpChar&& compare
){
    using distance_type = typename std::common_type<
        typename std::iterator_traits<Iterator1>::difference_type,
        typename std::iterator_traits<Iterator2>::difference_type
    >::type;
    std::pair<decltype(compare(*l,*r)), distance_type> result;
    auto& diff = result.first = 0;
    auto& count = result.second = 0;

#ifndef NDEBUG
    std::cout << "cmp: " << ((l == lend) ? std::string("<null>") : std::string(l,lend))
                         << ", " << ((r == rend) ? std::string("<null>") : std::string(r,rend));
#endif

    if ((l == lend) && (r == rend)) {
        puts("\n");
    }

    ++cmpcnt;
    do {
        if (UNLIKELY (l == lend)) {
            if (r != rend) {
                diff = -1;
            }
            break;
        } else if (UNLIKELY (r == rend)) {
            diff = 1;
            break;
        }
        diff = compare (*l, *r);
        if (UNLIKELY (diff)) {
            break;
        }
        ++count;
        ++l;
        ++r;
    } while (true);

#ifndef NDEBUG
    std::cout << ((diff < 0) ? " < " : ((diff == 0) ? " == " : " > "))
              << std::endl;
#endif
    return result;
}


template <typename Iterator1, typename Iterator2, typename CmpChar>
static inline
auto
lexicographical_compare_3way (
    Iterator1 const l, Iterator1 const lend,
    Iterator2 const r, Iterator2 const rend,
    CmpChar&& compare,
    typename std::common_type<
        typename std::iterator_traits<Iterator1>::difference_type,
        typename std::iterator_traits<Iterator2>::difference_type
    >::type const pos
){
    auto result = lexicographical_compare_3way (
        next (l, pos, lend), lend,
        next (r, pos, rend), rend,
        std::forward<CmpChar>(compare)
    );
    result.second += pos;
    return result;
}


template <typename StrSetFwdIt, typename StrFwdIt, typename CmpChar>
static inline __attribute__((noinline)) auto
lexicographical_lower_bound_3way (
    StrSetFwdIt setbegin, StrSetFwdIt const setend,
    StrFwdIt const strbegin, StrFwdIt const strend,
    CmpChar&& compare,
    typename std::common_type<
        typename std::iterator_traits<StrSetFwdIt>::difference_type,
        typename std::iterator_traits<StrFwdIt>::difference_type
    >::type pos = 0
){
    using std::begin;
    using std::end;
    using distance_type = decltype(pos);

    auto size = std::distance (setbegin, setend);
    distance_type lb_prefix_len = pos;
    distance_type ub_prefix_len = pos;

#ifndef NDEBUG
    std::cout << "label = '";
    std::copy (strbegin, strend, std::ostream_iterator<char>(std::cout));
    std::cout << "'\n";
#endif

    while (size) {
        auto const prefix = std::min (lb_prefix_len, ub_prefix_len);
#ifndef NDEBUG
        std::cout << "prefix = '";
        std::copy_n (strbegin, prefix, std::ostream_iterator<char>(std::cout));
        std::cout << "'" << std::endl;
        std::cout << "lb, size = " << size << " ";
#endif
        auto const half = size >> 1;
        auto notless = std::next (setbegin, half);
        auto const result = lexicographical_compare_3way (
            begin (*notless), end (*notless),
            strbegin, strend,
            compare,
            prefix
        );
        if (result.first < 0) {
            setbegin = ++notless;
            size -= half + 1;
            lb_prefix_len = result.second;
        } else {
            size = half;
            ub_prefix_len = result.second;
        }
    }

    return setbegin;
}


template <typename StrSetFwdIt, typename Str, typename CmpChar> static inline
auto
lexicographical_lower_bound_3way (
    StrSetFwdIt&& setbegin, StrSetFwdIt&& setend,
    Str const& str,
    CmpChar&& compare
){
    using std::begin;
    using std::end;
    return lexicographical_lower_bound_3way (
        std::forward<StrSetFwdIt>(setbegin), std::forward<StrSetFwdIt>(setend),
        begin(str), end(str),
        std::forward<CmpChar>(compare)
    );
}


template <typename StrSet, typename Str, typename CmpChar> static inline
auto
lexicographical_lower_bound_3way
(StrSet const& set, Str const& str, CmpChar&& compare) {
    using std::begin;
    using std::end;
    return lexicographical_lower_bound_3way (
        begin(set), end(set),
        begin(str), end(str),
        std::forward<CmpChar>(compare)
    );
}


template <typename StrSetFwdIt, typename StrFwdIt, typename CmpChar> static inline
__attribute__((noinline))
auto
lexicographical_upper_bound_3way (
    StrSetFwdIt setbegin, StrSetFwdIt const setend,
    StrFwdIt const strbegin, StrFwdIt const strend,
    CmpChar&& compare,
    typename std::common_type<
        typename std::iterator_traits<StrSetFwdIt>::difference_type,
        typename std::iterator_traits<StrFwdIt>::difference_type
    >::type pos = 0
){
    using std::begin;
    using std::end;
    using distance_type = decltype(pos);

    auto size = std::distance (setbegin, setend);
    distance_type lb_prefix_len = pos;
    distance_type ub_prefix_len = pos;

#ifndef NDEBUG
    std::cout << "label = '";
    std::copy (strbegin, strend, std::ostream_iterator<char>(std::cout));
    std::cout << "'\n";
#endif

    while (size) {
        auto const prefix = std::min (lb_prefix_len, ub_prefix_len);
#ifndef NDEBUG
        std::cout << "prefix = '";
        std::copy_n (strbegin, prefix, std::ostream_iterator<char>(std::cout));
        std::cout << "'" << std::endl;
        std::cout << "ub, size = " << size << " ";
#endif
        auto const half = size >> 1;
        auto greater = std::next (setbegin, half);
        auto const result = lexicographical_compare_3way (
            begin (*greater), end (*greater),
            strbegin, strend,
            compare,
            prefix
        );
        if (result.first <= 0) {
            setbegin = ++greater;
            size -= half + 1;
            lb_prefix_len = result.second;
        } else {
            size = half;
            ub_prefix_len = result.second;
        }
    }

    return setbegin;
}


template <typename StrSetFwdIt, typename Str, typename CmpChar> static inline
auto
lexicographical_upper_bound_3way (
    StrSetFwdIt&& setbegin, StrSetFwdIt&& setend,
    Str const& str,
    CmpChar&& compare
){
    using std::begin;
    using std::end;
    return lexicographical_upper_bound_3way (
        std::forward<StrSetFwdIt>(setbegin), std::forward<StrSetFwdIt>(setend),
        begin(str), end(str),
        std::forward<CmpChar>(compare)
    );
}


template <typename StrSet, typename Str, typename CmpChar> static inline
auto
lexicographical_upper_bound_3way
(StrSet const& set, Str const& str, CmpChar&& compare) {
    using std::begin;
    using std::end;
    return lexicographical_upper_bound_3way (
        begin(set), end(set),
        begin(str), end(str),
        std::forward<CmpChar>(compare)
    );
}

template <typename StrSetFwdIt, typename StrFwdIt, typename CmpChar> static inline
__attribute__((noinline))
std::pair<StrSetFwdIt, StrSetFwdIt>
lexicographical_equal_range_3way (
    StrSetFwdIt setbegin, StrSetFwdIt const setend,
    StrFwdIt const strbegin, StrFwdIt const strend,
    CmpChar&& compare
){
    using std::begin;
    using std::end;
    using distance_type = typename std::common_type<
        typename std::iterator_traits<StrSetFwdIt>::difference_type,
        typename std::iterator_traits<StrFwdIt>::difference_type
    >::type;

#ifndef NDEBUG
    std::cout << typeid(typename std::iterator_traits<StrSetFwdIt>::iterator_category()).name() << std::endl;
#endif
    auto size = std::distance (setbegin, setend);
    distance_type lb_prefix_len = 0;
    distance_type ub_prefix_len = 0;

#ifndef NDEBUG
    std::cout << "label = '";
    std::copy (strbegin, strend, std::ostream_iterator<char>(std::cout));
    std::cout << "'\n";
#endif

    while (size) {
        auto const prefix = std::min (lb_prefix_len, ub_prefix_len);
#ifndef NDEBUG
        std::cout << "prefix = '";
        std::copy_n (strbegin, prefix, std::ostream_iterator<char>(std::cout));
        std::cout << "'" << std::endl;
        std::cout << "eqrng, size = " << size << " ";
#endif
        auto const half = size >> 1;
        auto middle = std::next (setbegin, half);
        auto const result = lexicographical_compare_3way (
            begin (*middle), end (*middle),
            strbegin, strend,
            compare,
            prefix
        );
        if (result.first < 0) {
            setbegin = ++middle;
            size -= half + 1;
            lb_prefix_len = result.second;
        } else if (result.first > 0) {
            size = half;
            ub_prefix_len = result.second;
        } else {
            auto notless = lexicographical_lower_bound_3way (
                setbegin, middle,
                strbegin, strend,
                compare,
                std::min (lb_prefix_len, result.second)
            );
            ++middle;
            size -= half + 1;
            auto greater = lexicographical_upper_bound_3way (
                middle, std::next(middle, size),
                strbegin, strend,
                compare,
                std::min (result.second, ub_prefix_len)
            );
            return {notless, greater};
        }
    }

    return {setbegin, setbegin};
}

