/**
 * Simple test program for ParseToString function
 */
#include <iostream>
#include <fstream>
#include <string>
#include "V8File.h"
#include <boost/filesystem.hpp>

using namespace std;
namespace fs = boost::filesystem;

int main() {
    // Create test data directory
    string test_dir = "test_data";
    string v8_file = "test_file.cf";
    string out_dir = "test_output";

    // Clean up old test files
    fs::remove_all(test_dir);
    fs::remove_all(out_dir);

    // Create test directory with some data
    fs::create_directory(test_dir);

    // Create some test files
    ofstream test1(test_dir + "/file1.txt");
    test1 << "Content of file 1\n";
    test1.close();

    ofstream test2(test_dir + "/file2.txt");
    test2 << "Content of file 2 with some more text\n";
    test2.close();

    // Create subdirectory with files
    string sub_dir = test_dir + "/subdir";
    fs::create_directory(sub_dir);

    ofstream test3(sub_dir + "/file3.txt");
    test3 << "Content of file 3 in subdirectory\n";
    test3.close();

    cout << "Created test data in '" << test_dir << "'" << endl;

    // Build V8 file
    int build_result = v8unpack::BuildCfFile(test_dir, v8_file, false);
    if (build_result != 0) {
        cout << "Failed to build V8 file: " << build_result << endl;
        return 1;
    }

    cout << "Built V8 file: '" << v8_file << "'" << endl;

    // Test ParseToString function
    string parse_result;
    int parse_result_code = v8unpack::ParseToString(v8_file, {}, parse_result);

    if (parse_result_code != 0) {
        cout << "Failed to parse to string: " << parse_result_code << endl;
        return 1;
    }

    cout << "ParseToString result (" << parse_result.length() << " characters):" << endl;
    cout << "---------------" << endl;
    cout << parse_result << endl;
    cout << "---------------" << endl;

    // Also test regular Parse for comparison
    int regular_parse = v8unpack::Parse(v8_file, out_dir, {});
    if (regular_parse != 0) {
        cout << "Failed to parse regularly: " << regular_parse << endl;
        return 1;
    }

    cout << "Regular Parse succeeded to directory: '" << out_dir << "'" << endl;

    // Compare content
    if (parse_result.find("file1.txt") != string::npos &&
        parse_result.find("file2.txt") != string::npos &&
        parse_result.find("file3.txt") != string::npos) {
        cout << "SUCCESS: ParseToString contains expected content" << endl;
    } else {
        cout << "FAILURE: ParseToString missing expected content" << endl;
        return 1;
    }

    // Clean up
    fs::remove(v8_file);
    fs::remove_all(test_dir);
    fs::remove_all(out_dir);

    cout << "Test completed successfully!" << endl;
    return 0;
}
