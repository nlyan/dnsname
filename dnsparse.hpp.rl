#include <string>

%%{
    machine escaped_dnsname;
    alphtype char;
    
    # Invoked at the beginning of every 3 character decimal escape sequence (e.g. "\126")
    action begdec { decb = fc - '0'; }
    action add    { decb += fc - '0'; }
    action mul10  { decb *= 10; }
    
    # Invoked at the end of every decimal escape sequence
    action enddec {
        ++llen;
        labelfun (decb);
    }
    
    # Invoked at the beginning of *every* label
    action beglabel {
        llen = 0;
    }
    
    # Invoked on every unescaped and decoded character in a label
    action lchar {
        ++llen;
        labelfun (fc);
    }

    # Invoked on inner dots e.g. for the first 2 dots in "foo.bar.com."
    action inner_dot {
        ++nlen;
    }
    
    # Invoked at the end of *every* label
    action endlabel {
        nlen += llen;
        dotfun (llen, nlen);
    }

    raw     = any - digit;
    safe    = ((0x21 .. 0x7e) - [.\\]) >lchar;
    escaped = '\\' (raw >lchar);
    decbyte = '\\' ((([01] digit{2}) | ('2' [0-4] digit) | ('2' '5' [0-5]))
                    >begdec
                    <>*mul10
                    <>*add
                    %enddec
                );

    label = (safe | escaped | decbyte)+ >beglabel %endlabel;
    dnsname := label ('.' label >inner_dot)* '.'? 0 @{ fbreak; };
}%%


namespace {
    
inline auto
count_trailing_backslashes (std::string& s) noexcept {
    auto const n = s.find_last_not_of('\\');
    if (n == std::string::npos) {
        return s.size();
    } else {
        return (s.size() - (n + 1));
    }
}

/* DANGER: While using Ragels 'noend' directive this function is unsafe when
 * called on a null terminated string with an active trailing escape character. 
 * This means the caller needs to check for an odd number of trailing \'s 
 * before calling this function.
 */
template <typename LabelFun, typename DotFun> 
auto parse_dns_name_cstr_unsafe (
    char const* p,
    LabelFun&& labelfun, 
    DotFun&& dotfun
){
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#pragma clang diagnostic ignored "-Wunused-variable"
%% write data;
    int cs;
%% write init;
    unsigned nlen = 0;
    unsigned llen = 0;
    char decb = 0;
%% write exec noend;
#pragma clang diagnostic pop

    if (cs < %%{ write first_final; }%%) {
        throw BadDNSName();
    }
    
    return p;
}

}
