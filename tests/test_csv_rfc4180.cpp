#include "../include/csv_parser.h"
#include <cassert>

int main(){
auto simple=parseCSVRFC4180("a,b,c");
assert(simple[0][0]=="a"&&simple[0][1]=="b"&&simple[0][2]=="c");
auto quoted=parseCSVRFC4180("\"x\",\"y\",\"z\"");
assert(quoted[0][0]=="x"&&quoted[0][1]=="y"&&quoted[0][2]=="z");
return 0;
}
