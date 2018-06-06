#include <iostream>
#include <thread>
#include <string>
using namespace std;

int main()
{
    string str,str2;
    cin>>str;
    cerr<<"test2 get input "<<str<<endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::copy(str.rbegin(),str.rend(),std::back_inserter(str2));
    cout<<"from test2 "<<str2<<endl;
    return 0;
}
