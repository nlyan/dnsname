
#line 1 "dnsparse.hpp.rl"

#line 55 "dnsparse.hpp.rl"



namespace {

template <typename Iterator, typename CommitFun, typename LabelFun>
auto
parse_dnsname (
    Iterator p,
    Iterator const pe,
    CommitFun&& commit,
    LabelFun&& commit_label
){
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#pragma clang diagnostic ignored "-Wunused-variable"

#line 23 "dnsparse.hpp"
static const int escaped_dnsname_start = 1;
static const int escaped_dnsname_first_final = 7;
static const int escaped_dnsname_error = 0;

static const int escaped_dnsname_en_dnsname = 1;


#line 72 "dnsparse.hpp.rl"
    int cs;
    auto const eof = pe;

#line 35 "dnsparse.hpp"
	{
	cs = escaped_dnsname_start;
	}

#line 75 "dnsparse.hpp.rl"
    unsigned nlen = 0;
    unsigned llen = 0;
    char decb = 0;

#line 45 "dnsparse.hpp"
	{
	if ( p == pe )
		goto _test_eof;
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
#line 6 "dnsparse.hpp.rl"
	{
        llen = 0;
    }
#line 26 "dnsparse.hpp.rl"
	{
        ++llen;
        commit ({(*p)});
    }
	goto st7;
tr3:
#line 26 "dnsparse.hpp.rl"
	{
        ++llen;
        commit ({(*p)});
    }
	goto st7;
tr11:
#line 33 "dnsparse.hpp.rl"
	{
        ++nlen;
    }
#line 6 "dnsparse.hpp.rl"
	{
        llen = 0;
    }
#line 26 "dnsparse.hpp.rl"
	{
        ++llen;
        commit ({(*p)});
    }
	goto st7;
tr13:
#line 19 "dnsparse.hpp.rl"
	{
        ++llen;
        commit ({decb});
    }
#line 26 "dnsparse.hpp.rl"
	{
        ++llen;
        commit ({(*p)});
    }
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
#line 112 "dnsparse.hpp"
	switch( (*p) ) {
		case 46: goto tr9;
		case 92: goto st2;
	}
	if ( 33 <= (*p) && (*p) <= 126 )
		goto tr3;
	goto st0;
tr9:
#line 38 "dnsparse.hpp.rl"
	{
        nlen += llen;
        commit_label ({llen});
    }
	goto st8;
tr14:
#line 19 "dnsparse.hpp.rl"
	{
        ++llen;
        commit ({decb});
    }
#line 38 "dnsparse.hpp.rl"
	{
        nlen += llen;
        commit_label ({llen});
    }
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 143 "dnsparse.hpp"
	if ( (*p) == 92 )
		goto tr12;
	if ( (*p) > 45 ) {
		if ( 47 <= (*p) && (*p) <= 126 )
			goto tr11;
	} else if ( (*p) >= 33 )
		goto tr11;
	goto st0;
tr2:
#line 6 "dnsparse.hpp.rl"
	{
        llen = 0;
    }
	goto st2;
tr12:
#line 33 "dnsparse.hpp.rl"
	{
        ++nlen;
    }
#line 6 "dnsparse.hpp.rl"
	{
        llen = 0;
    }
	goto st2;
tr15:
#line 19 "dnsparse.hpp.rl"
	{
        ++llen;
        commit ({decb});
    }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 179 "dnsparse.hpp"
	if ( (*p) == 50 )
		goto tr5;
	if ( (*p) > 49 ) {
		if ( 51 <= (*p) && (*p) <= 57 )
			goto st0;
	} else if ( (*p) >= 48 )
		goto tr4;
	goto tr3;
tr4:
#line 12 "dnsparse.hpp.rl"
	{ decb = (*p) - '0'; }
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
#line 16 "dnsparse.hpp.rl"
	{ decb *= 10; }
#line 15 "dnsparse.hpp.rl"
	{ decb += (*p) - '0'; }
#line 200 "dnsparse.hpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st4;
	goto st0;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 16 "dnsparse.hpp.rl"
	{ decb *= 10; }
#line 15 "dnsparse.hpp.rl"
	{ decb += (*p) - '0'; }
#line 212 "dnsparse.hpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st9;
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	switch( (*p) ) {
		case 46: goto tr14;
		case 92: goto tr15;
	}
	if ( 33 <= (*p) && (*p) <= 126 )
		goto tr13;
	goto st0;
tr5:
#line 12 "dnsparse.hpp.rl"
	{ decb = (*p) - '0'; }
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
#line 16 "dnsparse.hpp.rl"
	{ decb *= 10; }
#line 15 "dnsparse.hpp.rl"
	{ decb += (*p) - '0'; }
#line 239 "dnsparse.hpp"
	if ( (*p) == 53 )
		goto st6;
	if ( 48 <= (*p) && (*p) <= 52 )
		goto st4;
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
#line 16 "dnsparse.hpp.rl"
	{ decb *= 10; }
#line 15 "dnsparse.hpp.rl"
	{ decb += (*p) - '0'; }
#line 253 "dnsparse.hpp"
	if ( 48 <= (*p) && (*p) <= 53 )
		goto st9;
	goto st0;
	}
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 7: 
#line 38 "dnsparse.hpp.rl"
	{
        nlen += llen;
        commit_label ({llen});
    }
	break;
	case 9: 
#line 19 "dnsparse.hpp.rl"
	{
        ++llen;
        commit ({decb});
    }
#line 38 "dnsparse.hpp.rl"
	{
        nlen += llen;
        commit_label ({llen});
    }
	break;
#line 290 "dnsparse.hpp"
	}
	}

	_out: {}
	}

#line 79 "dnsparse.hpp.rl"
#pragma clang diagnostic pop

    if (cs < 7) {
        throw BadDNSName();
    }
    
    return nlen;
}

}
