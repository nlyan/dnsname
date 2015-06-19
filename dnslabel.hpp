
#line 1 "dnslabel.hpp.rl"
#include <iterator>
#ifndef NDEBUG
#include <boost/utility/string_ref.hpp>
#include <cassert>
#endif


#line 56 "dnslabel.hpp.rl"



namespace {
    
inline auto
count_trailing_backslashes (std::string const& s) noexcept {
    auto const n = s.find_last_not_of('\\');
    if (n == std::string::npos) {
        return s.size();
    } else {
        return (s.size() - (n + 1));
    }
}


template <typename Iterator, typename LabelFun, typename DotFun> 
auto parse_dns_name_cstr_unsafe (
    Iterator p,
    LabelFun&& labelfun, 
    DotFun&& dotfun
){
#ifndef NDEBUG
    /* While using Ragels 'noend' directive this function is unsafe when
     * called on a string with an active trailing escape character. 
     * This means the caller needs to check for an odd number of trailing \'s 
     * before calling this function.
     */
    assert (0 == (count_trailing_backslashes(p) & 1ul));
#endif
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#pragma clang diagnostic ignored "-Wunused-variable"

#line 46 "dnslabel.hpp"
static const int escaped_dnsname_start = 1;
static const int escaped_dnsname_first_final = 10;
static const int escaped_dnsname_error = 0;

static const int escaped_dnsname_en_dnsname = 1;


#line 90 "dnslabel.hpp.rl"
    int cs;

#line 57 "dnslabel.hpp"
	{
	cs = escaped_dnsname_start;
	}

#line 92 "dnslabel.hpp.rl"
    unsigned nlen = 0;
    unsigned llen = 0;
    char decb = 0;

#line 67 "dnslabel.hpp"
	{
	switch ( cs )
	{
case 1:
	if ( (*p) == 92 )
		goto tr2;
	if ( (*p) > 45 ) {
		if ( 47 <= (*p) && (*p) <= 126 )
			goto tr0;
	} else if ( (*p) >= 33 )
		goto tr0;
	goto st0;
st0:
cs = 0;
	goto _out;
tr0:
#line 23 "dnslabel.hpp.rl"
	{
        llen = 0;
    }
#line 28 "dnslabel.hpp.rl"
	{
        ++llen;
        labelfun ((*p));
    }
	goto st2;
tr4:
#line 28 "dnslabel.hpp.rl"
	{
        ++llen;
        labelfun ((*p));
    }
	goto st2;
tr8:
#line 34 "dnslabel.hpp.rl"
	{
        ++nlen;
    }
#line 23 "dnslabel.hpp.rl"
	{
        llen = 0;
    }
#line 28 "dnslabel.hpp.rl"
	{
        ++llen;
        labelfun ((*p));
    }
	goto st2;
tr15:
#line 17 "dnslabel.hpp.rl"
	{
        ++llen;
        labelfun (decb);
    }
#line 28 "dnslabel.hpp.rl"
	{
        ++llen;
        labelfun ((*p));
    }
	goto st2;
st2:
	p += 1;
case 2:
#line 131 "dnslabel.hpp"
	switch( (*p) ) {
		case 0: goto tr3;
		case 46: goto tr5;
		case 92: goto st4;
	}
	if ( 33 <= (*p) && (*p) <= 126 )
		goto tr4;
	goto st0;
tr3:
#line 39 "dnslabel.hpp.rl"
	{
        nlen += llen;
        dotfun (llen, nlen);
    }
#line 55 "dnslabel.hpp.rl"
	{ {p++; cs = 10; goto _out;} }
	goto st10;
tr7:
#line 55 "dnslabel.hpp.rl"
	{ {p++; cs = 10; goto _out;} }
	goto st10;
tr14:
#line 17 "dnslabel.hpp.rl"
	{
        ++llen;
        labelfun (decb);
    }
#line 39 "dnslabel.hpp.rl"
	{
        nlen += llen;
        dotfun (llen, nlen);
    }
#line 55 "dnslabel.hpp.rl"
	{ {p++; cs = 10; goto _out;} }
	goto st10;
st10:
	p += 1;
case 10:
#line 170 "dnslabel.hpp"
	goto st0;
tr5:
#line 39 "dnslabel.hpp.rl"
	{
        nlen += llen;
        dotfun (llen, nlen);
    }
	goto st3;
tr16:
#line 17 "dnslabel.hpp.rl"
	{
        ++llen;
        labelfun (decb);
    }
#line 39 "dnslabel.hpp.rl"
	{
        nlen += llen;
        dotfun (llen, nlen);
    }
	goto st3;
st3:
	p += 1;
case 3:
#line 194 "dnslabel.hpp"
	switch( (*p) ) {
		case 0: goto tr7;
		case 92: goto tr9;
	}
	if ( (*p) > 45 ) {
		if ( 47 <= (*p) && (*p) <= 126 )
			goto tr8;
	} else if ( (*p) >= 33 )
		goto tr8;
	goto st0;
tr2:
#line 23 "dnslabel.hpp.rl"
	{
        llen = 0;
    }
	goto st4;
tr9:
#line 34 "dnslabel.hpp.rl"
	{
        ++nlen;
    }
#line 23 "dnslabel.hpp.rl"
	{
        llen = 0;
    }
	goto st4;
tr17:
#line 17 "dnslabel.hpp.rl"
	{
        ++llen;
        labelfun (decb);
    }
	goto st4;
st4:
	p += 1;
case 4:
#line 231 "dnslabel.hpp"
	if ( (*p) == 50 )
		goto tr11;
	if ( (*p) > 49 ) {
		if ( 51 <= (*p) && (*p) <= 57 )
			goto st0;
	} else if ( (*p) >= 48 )
		goto tr10;
	goto tr4;
tr10:
#line 12 "dnslabel.hpp.rl"
	{ decb = (*p) - '0'; }
	goto st5;
st5:
	p += 1;
case 5:
#line 14 "dnslabel.hpp.rl"
	{ decb *= 10; }
#line 13 "dnslabel.hpp.rl"
	{ decb += (*p) - '0'; }
#line 251 "dnslabel.hpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st6;
	goto st0;
st6:
	p += 1;
case 6:
#line 14 "dnslabel.hpp.rl"
	{ decb *= 10; }
#line 13 "dnslabel.hpp.rl"
	{ decb += (*p) - '0'; }
#line 262 "dnslabel.hpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st7;
	goto st0;
st7:
	p += 1;
case 7:
	switch( (*p) ) {
		case 0: goto tr14;
		case 46: goto tr16;
		case 92: goto tr17;
	}
	if ( 33 <= (*p) && (*p) <= 126 )
		goto tr15;
	goto st0;
tr11:
#line 12 "dnslabel.hpp.rl"
	{ decb = (*p) - '0'; }
	goto st8;
st8:
	p += 1;
case 8:
#line 14 "dnslabel.hpp.rl"
	{ decb *= 10; }
#line 13 "dnslabel.hpp.rl"
	{ decb += (*p) - '0'; }
#line 288 "dnslabel.hpp"
	if ( (*p) == 53 )
		goto st9;
	if ( 48 <= (*p) && (*p) <= 52 )
		goto st6;
	goto st0;
st9:
	p += 1;
case 9:
#line 14 "dnslabel.hpp.rl"
	{ decb *= 10; }
#line 13 "dnslabel.hpp.rl"
	{ decb += (*p) - '0'; }
#line 301 "dnslabel.hpp"
	if ( 48 <= (*p) && (*p) <= 53 )
		goto st7;
	goto st0;
	}

	_out: {}
	}

#line 96 "dnslabel.hpp.rl"
#pragma clang diagnostic pop

    if (cs < 10) {
        throw BadDNSName();
    }
    
    return nlen;
}

}
