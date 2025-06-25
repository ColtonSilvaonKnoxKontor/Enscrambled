# Enscrambled

This is the ransomware version of Enscrambled where it uses AES method. It is tested on debian/ubuntu based Linux distribution with required dependencies installed. Unlike others, it just outputs everything in terminal, just like text-based programs. It can also steal user's files by sending random 10 files to host's Telegram bot.

# What is This

This program forces to encrypt all files, including files with restricted permissions. It also delete files automatically if you enter incorrect password.

# Status?

Ongoing and in unstable state

# How to Compile?
using `g++`, include `-lcrypto`, `-lcurl`, `-std=c++17` and `-pthread` flag after `-o`.

# Features

- Obfuscated style password and keys , blending them with garbage texts

- Uses "custom" file signature, making this as only valid software for decryption
 
- Keyboard interrupt blocked (ctrl+z, ctrl+c)

- Prevents *top software from executing (htop, btop, or any task managers)

- Auto-delete all encrypted files if the user close the terminal

- Required to execute as root user
  
- Check and install required dependencies

- Randomly send 10 encrypted files to telegram, but it only send files with up to 50MB because of limitation with Telegram bot. You can raise a number of files you want to send to.

# Work in Progress

- Block shutdown/reboot signal from executing

- (FAIL) Can detatch to terminal and can relaunch itself to a new terminal session

- Multi-thread (Not too stress on CPU)

- Thread/Process revival

 - Improve text design

- Pretend to be a real specific software

# Bad Idea

- For sending large files, it needs to be archive first then splitting it into multiple files with 50MB file size. But considering the processing power limit of some of the machines and time it consumes after encryption completes, the user will have the chance to prevent this long time process by means of cutting machine's power.

- Detaching the main process from terminal and then running as it's own was a good idea since it is expected that a user will stop encryption by killing Linux Terminal (by clicking "X"). But respawning itself into a new terminal will not gonna work as expected, as it was hard to implement. It just respawn into a new state of process meaning it relaunch a binary, starting the process from top, and it simultaneously runs with the first same process. So the solution is when the user close the terminal, the encryption is still going on, but it should be auto deleted as a form of punishment.

# How to integrate your own Telegram Bot with this program?
You need to search "BotFather" from your telegram's search bar, then message `/start`. To create a new bot, type `/newbot` and follow the instructions there. After that, obtain token from this phrase like this: "Use this token to access the HTTP API:" and put it to the file `telegram.cpp`.

To obtain Chat ID or Group ID, go to your newly created bot and type something there, like simply "try" or "hello". Using your favorite browser, go to `https://api.telegram.org/bot<YOUR_BOT_TOKEN>/getUpdates` and replace `<YOUR_BOT_TOKEN>` with your token you obtained. Now you see something like this: 

```
{"ok":true,"result":[{"update_id":123456789,
"message":{"message_id":3,"from":{"id":1234567890,"is_bot":false,"first_name":"Colton","last_name":"Silva","language_code":"en"},"chat":{"id":1234567890,"first_name":"Colton","last_name":"Silva","type":"private"},"date":1750675756,"text":"Try"}}]}
```

Copy paste the Chat ID from like this one `{"id":1234567890,` and put it to `const string CHAT_ID` from `telegram.cpp`.

# How to harvest password from binary
You may use simple text editor to inspect and find the correct password. Most junk password consists of Unicode's UTF-8 character so you must set them to easily find it.

# Warning

This program can alter or delete all files without notice; or even worse, it can destroy the entire root files and essential system binaries. I do not have responsibility for any damages you have done.
