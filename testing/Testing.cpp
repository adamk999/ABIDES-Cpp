#include <iostream>
#include "../message/Message.h"
using namespace std;

int main()
{
    Message m1("This is a test");
    Message m2;

    cout << m1.body << m1.uniq_id << endl;
    cout << m2.uniq_id << endl;

    return 0;
}