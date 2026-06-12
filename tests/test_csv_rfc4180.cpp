#include "csv_parser.h"
#include <cassert>
#include <iostream>

int main(){
std::cout<<"Testing RFC 4180 parser..."<<std::endl;
auto t1=parseCSV("a,b,c");
assert(t1.size()==1&&t1[0].size()==3);
auto t2=parseCSV("\"x\",\"y\",\"z\"");
assert(t2[0][0]=="x"&&t2[0][1]=="y"&&t2[0][2]=="z");
auto t3=parseCSV("\"a,b\",c");
assert(t3[0][0]=="a,b"&&t3[0][1]=="c");
auto t4=parseCSV("\"a\"\"b\",c");
assert(t4[0][0]=="a\"b");
auto t5=parseCSV("\"a\nb\",c");
assert(t5.size()==1&&t5[0][0]=="a\nb"&&t5[0][1]=="c");
auto t6=parseCSV("a,b\r\nc,d\r\n");
assert(t6.size()==2&&t6[0][1]=="b"&&t6[1][0]=="c"&&t6[1][1]=="d");
std::cout<<"All tests passed"<<std::endl;
return 0;
}
