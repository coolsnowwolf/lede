#include <iostream>
#include "conv.h"

using namespace std;

int main(int argc, char *argv[])
{
    string src = "我是谁？";
    string dsc = "";
    
    dsc = string(u2g(src.c_str()));
    cout << dsc << endl;
    src = dsc;
    dsc = string(g2u(src.c_str()));
    cout << dsc << endl;
     
    return 0;
}
