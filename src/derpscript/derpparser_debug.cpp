#include "derpparser_internal.h"
using namespace std;

namespace ExPop {

    std::string getStringForNodeType(DerpExecNodeType type) {
      #define DT(x) { if(type == x) return #x; }

        DT(DERPEXEC_ERROROP);

        DT(DERPEXEC_ADD);
        DT(DERPEXEC_SUBTRACT);
        DT(DERPEXEC_MULTIPLY);
        DT(DERPEXEC_DIVIDE);
        DT(DERPEXEC_ASSIGNMENT);
        DT(DERPEXEC_REFASSIGNMENT);

        DT(DERPEXEC_BINARYNOT);

        DT(DERPEXEC_NOT);

        DT(DERPEXEC_LITERAL);

        DT(DERPEXEC_VARLOOKUP);
        DT(DERPEXEC_VARIABLEDEC);

        return "???";
    }

}

