<div align="center">
  <h1>GitC2</h1>
  <img src="https://raw.githubusercontent.com/N3agu/GitC2/refs/heads/main/Images/github.png" width="256">
  
  <p><b>An educational Proof of Concept demonstrating how to use the GitHub API as a C2 Server.</b></p>
</div>

## Overview
**GitC2** is a lightweight, C2 PoC with no dependencies. It demonstrates "Living off the Cloud" techniques by utilizing GitHub Issues as a dead-drop resolver to route commands and exfiltrate outputs.  By leveraging trusted domains (`api.github.com`), this project illustrates how modern evasion techniques bypass traditional network perimeter defenses.

<div align="center">
  <img src="https://raw.githubusercontent.com/N3agu/GitC2/refs/heads/main/Images/showcase.png" alt="GitC2 Showcase">
</div>

## Key Features

* **GitHub API Integration**
    * Leverages the GitHub REST API (specifically Issues and Comments) to act as a resilient, trusted-domain proxy for task dispatching and data exfiltration, blending C2 traffic with normal developer HTTPS traffic.
    * **Note:** While using a trusted domain provides initial stealth, this method has significant weaknesses. The constant polling interval (beaconing) is easily detectable by modern EDR/NDR solutions. Furthermore, GitHub's internal abuse-monitoring systems can identify and flag anomalous API activity, leading to account suspension.
    * **Risk:** Since the GitHub Personal Access Token (PAT) is required by the client to poll for tasks, a successful reverse-engineering of the client binary would grant them full control over the C2 infrastructure. This could allow defenders to "take over" the C2, intercepting all communications or issuing counter-commands.

* **No External C++ Dependencies**
    * The client relies entirely on standard C++ libraries (STL) and the OS's native `curl` and `_popen` functions.
    * *Note: While the STL is typically avoided in real-world malware development to minimize binary size and runtime dependencies, it is utilized in this PoC to keep the source code as accessible as possible.*

* **Encrypted Communication**
    * Implements a custom rolling XOR cipher paired with Base64 encoding to encrypt commands and outputs from plain-text inspection, using the GitHub Token as the key.
    * *Note: While XOR provides basic obfuscation suitable for a PoC, it is cryptographically weak. Real-world threat actors typically use more robust and efficient algorithms like RC4, AES, or ChaCha20 for secure command channels. This method was chosen to maintain a dependency-free, lightweight footprint for educational purposes.*

## How It Works
1. The Server (Python) creates a new GitHub Issue containing the command (encrypted or plain-text).
2. The Client (C++) continuously polls the repository for open issues.
3. Upon finding an open issue, the client parses the command, decrypts it (if enabled), and executes it locally.
4. The client encrypts the terminal output, posts it as a comment on the same issue, and changes the issue state to `closed`.
5. The server detects the closed issue, retrieves the comment, decrypts the output, and displays it to the user.

## Screenshots

<details open>
  <summary><strong>Screenshot of Tasks</strong></summary>
  
  ![](https://raw.githubusercontent.com/N3agu/GitC2/refs/heads/main/Images/tasks.png)
</details>

<details>
  <summary><strong>Screenshot of a Normal Task</strong></summary>
  
  ![](https://raw.githubusercontent.com/N3agu/GitC2/refs/heads/main/Images/task_normal.png)
</details>

<details>
  <summary><strong>Screenshot of an Encrypted Task</strong></summary>
  
  ![](https://raw.githubusercontent.com/N3agu/GitC2/refs/heads/main/Images/task_encrypted.png)
</details>

## Setup & Configuration

### 1. Prerequisites
* A GitHub account and a dedicated repository (can be private).
* A GitHub Personal Access Token (PAT Classic) with the `repo` scope (server contains step by step instructions on how to get one).
* A copy of this repository.

```sh
git clone https://github.com/N3agu/GitC2.git
```

### 2. Server Setup (Python)
The server requires the `requests` library.

```sh
pip install requests
python3 server.py
```

*On first run, the server will prompt you for your GitHub Token, Repository Owner, Repository Name, and whether you want to enable encryption. It will save these to a settings.json file automatically.*

You can make the configuration file manually by creating the file "settings.json" in the Server directory with the following structure:
```json
{
    "token": "YOUR_GITHUB_TOKEN",
    "owner": "YOUR_GITHUB_USERNAME",
    "repo": "YOUR_REPOSITORY_NAME",
    "encrypt_communication": true
}
```

### 3. Client Setup (C++)
Create the file 'settings.h' in the Client directory with the following structure and ensure they match the settings used on the Server:
```cpp
#pragma once
#include <string>

const std::string GITHUB_TOKEN = "YOUR_GITHUB_TOKEN"; 
const std::string REPO_OWNER = "YOUR_GITHUB_USERNAME";
const std::string REPO_NAME = "YOUR_REPOSITORY_NAME";
const bool ENCRYPT_COMMUNICATION = true;
```

*! Compilation was done using VS2022.*

## Disclaimer

***This project is an educational Proof of Concept designed to demonstrate how legitimate platforms can be utilized by threat actors. It is intended strictly for academic research, defensive analysis, and portfolio demonstration.***

***Using this code to violate GitHub's Terms of Service, or deploying it on systems without explicit, mutual, and documented permission, is strictly prohibited. The author assumes no liability and is not responsible for any misuse or damage caused by this program.***
