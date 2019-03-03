#include <iostream>
#include "map.hpp"
#include <map>
using namespace std;
int main () {
    sjtu::map<int,string> a;
    for (int i = 0; i <= 999; i++) {
        a[i]=to_string(i+1);
    }
    for (int i = 999; i >=499; i--) {
        auto j=a.find(i);
        a.erase(j);
        cout<<j->key<<endl;
    }
    for (int i = 0; i < 499; i++) {
        auto j=a.find(i);
        a.erase(j);
        cout<<j->key<<endl;
    }
    a[0]="1";
}