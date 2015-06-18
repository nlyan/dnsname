#include <stdexcept>

%%{
    machine escaped_dnsname;
    alphtype char;
    
    action begdec { decb = fc - '0'; }
    action add    { decb += fc - '0'; }
    action mul10  { decb *= 10; }
    
    action enddec {
        ++llen;
        labelfun (decb);
    }
    
    action beglabel {
        llen = 0;
    }
    
    action lchar {
        ++llen;
        labelfun (fc);
    }

    action inner_dot {
        ++nlen;
    }
    
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


template <typename Iterator, typename LabelFun, typename DotFun> 
auto
parse_dns_name_cstr (
    Iterator p,
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
        throw std::runtime_error ("Bad DNS name");
    }
    
    return nlen;
}
