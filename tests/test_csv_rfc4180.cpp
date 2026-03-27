#include "../include/csv_parser.h"
#include <cassert>
#include <iostream>

int main(){
std::cout<<"Testing RFC 4180 parser..."<<std::endl;
auto t1=parseCSVRFC4180("a,b,c");
assert(t1.size()==1&&t1[0].size()==3);
auto t2=parseCSVRFC4180("\"x\",\"y\",\"z\"");
assert(t2[0][0]=="x"&&t2[0][1]=="y"&&t2[0][2]=="z");
auto t3=parseCSVRFC4180("\"a,b\",c");
assert(t3[0][0]=="a,b"&&t3[0][1]=="c");
auto t4=parseCSVRFC4180("\"a\"\"b\",c");
assert(t4[0][0]=="a\"b");
std::cout<<"All tests passed"<<std::endl;
return 0;
}
