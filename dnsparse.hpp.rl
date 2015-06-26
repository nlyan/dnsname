%%{
    machine escaped_dnsname;
    alphtype char;
    
    # Invoked at the beginning of every label
    action label_init {
        llen = 0;
    }

    # Invoked at the beginning of every 3 character decimal escape sequence
    # (e.g. "\126")
    action dec_init     { decb = fc - '0'; }
    
    # Arithmetic decoding of the decimal escape sequence
    action dec_add      { decb += fc - '0'; }
    action dec_mul10    { decb *= 10; }

    # Invoked at the end of every decimal escape sequence
    action dec_fin {
        ++llen;
        commit ({decb});
    }

    # Invoked on every printable and/or escaped byte in a label (but not inside
    # decimal escape sequences)
    action label_char {
        ++llen;
        commit ({fc});
    }

    # Invoked on label separators e.g. for the first two periods in "x.y.z.",
    # but not the last
    action label_sep {
        ++nlen;
    }

    # Invoked at the end of every label
    action label_fin {
        nlen += llen;
        commit_label ({llen});
    }

    raw     = any - digit;
    plain   = ((0x21 .. 0x7e) - [.\\]) >label_char;
    escaped = '\\' (raw >label_char);
    escdecb = '\\' ((([01] digit{2}) | ('2' [0-4] digit) | ('2' '5' [0-5]))
                    >dec_init
                    <>*dec_mul10
                    <>*dec_add
                    %dec_fin
                );

    label = (plain | escaped | escdecb)+ >label_init %label_fin;
    dnsname := label ('.' label >label_sep)* '.'?;
}%%


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
%% write data;
    int cs;
    auto const eof = pe;
%% write init;
    unsigned nlen = 0;
    unsigned llen = 0;
    char decb = 0;
%% write exec;
#pragma clang diagnostic pop

    if (cs < %%{ write first_final; }%%) {
        throw BadDNSName();
    }
    
    return nlen;
}

}
