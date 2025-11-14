# Enscrambled

This is a blended ransomware where it uses hybrid RSA/AES method. It is tested on debian/ubuntu based Linux distribution with required dependencies installed. Unlike others, it just outputs everything in terminal, just like text-based programs. It can encrypts, deface, steals and do backdoors on a target machine.

# What is This

This program forces to encrypt all files, including files with restricted permissions. It also delete files automatically if you enter incorrect password.

# Status?

Immature, ongoing and in unstable state

# Tips:

This source code can encrypt only the current directory by this `fs::current_path()` for testing purpose, so you need to change the function `sendRandomEncryptedFiles()` from `int main()`, and `encryptDirectory()` from `void encryptAllFiles()` so that it can include entire root, or your chosen root directory. See this `// comment` for instructions on how.

# How to Compile?
Although this program can automatically install missing dependencies on target system, it should be compiled with host system with installed dependencies.

You need to install `libssl-dev`, `acpi` and `libcurl4-openssl-dev`.

Execute `make`

For lower versions of g++:

- version 8: Uncomment `#include <iomanip>` from main.cpp. To compile, you just need to add the flag `-lstdc++fs` to include the filesystem objects from the stdc++fs static lib.
- version 7 below: You have to modify the entire source code to support experimental filesystem, do manual quoting instead of `std::quoted()` and just a few syntax fixes.

Now some can analyze your code by executing `strings ./enscrambled`, which they can instantly see all of the text strings as obviously a type of malware.

We need to transform the executable file into a "packed" form of executable file in which the inexperienced computer security analyst cannot read them using `strings`

Install `upx-ucl`, then after that execute `upx` with `-9` for best compression result, `-o` as output binary and then the last should be the source binary.

And then to produce this on netwide, you have to make this a kind of Trojan and a way of distributing this to the victims.

# Features

- Hardcoded obfuscated style password and keys , blending them with garbage texts

- Uses "custom" file signature, making this as only valid software for decryption
 
- Keyboard interrupt blocked (ctrl+z, ctrl+c)

- Prevents *top software from executing (htop, btop, or any task managers)

- Auto-delete all encrypted files if the user close the terminal

- Launches another terminal window, showing matrix animation

- Automatically adds predefined user and password in sudo group. This can be used to access target's ssh.
  
- Check and install required dependencies

- Randomly send 10 encrypted files to telegram, but it only send files with up to 50MB because of limitation with Telegram bot. You can raise a number of files you want to send to.

- It can also send machine's info, internal and external IP Address, and the file signature if in case they replace it with someting else.

- Send single screenshot to telegram

- Can sing **Happy Birthday** "if the root user can communicate with ALSA" or just beeps.

- Replace the server's website into the malware's generated html file

- Change contents of motd and issue file into custom generated defined by enscrambled

# Work in Progress

- Make it like computer worm

- Add secondary passcode key coming from attacker's server with tor service
  
- Block shutdown/reboot signal from executing

- Tamper web server (Apache2 or nginx) to point generated "index.html"

- (IN PROGRESS) Can detatch to terminal and can relaunch itself to a new terminal session

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