// These had to be confined to their own header file due to editor
// funkiness.

#define TOKEN_TYPE(x) (getTokenTypeSafe(tokens, i + (x)))
#define LINE_NUMBER (getTokenLineSafe(tokens, i))
#define ERROR(x)                                    \
    do {                                            \
        ostringstream ostr;                         \
        ostr << LINE_NUMBER << ": " << (x) << endl; \
        errorStr = errorStr + ostr.str();           \
    } while(0);

