#include "V8File.h"
#include <iostream>

using namespace std;
using namespace v8unpack;

int main() {
    string result;
    string filter;

    cout << "Testing ParseToString with non-existent file..." << endl;
    int ret = ParseToString("test_file.cf", {}, result);
    cout << "Return code: " << ret << endl;

    if (ret == -1) {
        cout << "File not found, as expected." << endl;
    }

    return 0;
}
