# Enscrambled

This is the ransomware version of Enscrambled where it uses AES method. It is tested on debian/ubuntu based Linux distribution with required dependencies installed.

# What is This

This program forces to encrypt all files, including files with restricted permissions. It also delete files automatically if you enter incorrect password.

# Status?

Ongoing

# How to Compile?
using `g++`, include `-lcrypto` and `-pthread` flag after `-o`.

# Features

- Uses "custom" file signature, making this as only valid software for decryption
 
- Keyboard interrupt blocked (ctrl+z, ctrl+c)

- Prevents *top software from executing (htop, btop, or any task managers)

- Auto-delete all encrypted files if the user close the terminal

- Required to execute as root user
  
- Check and install required dependencies



# Work in Progress

- Self-replication (Like computer worm)

- Block shutdown/reboot signal from executing

- (FAIL) Can detatch to terminal and can relaunch itself to a new terminal session

- Multi-thread (Not too stress on CPU)

- Thread/Process revival

 - Improve text design

- Pretend to be a real specific software

# How to harvest password from binary
You may use simple text editor to inspect and find the correct password. Most junk password consists of Unicode's UTF-8 character so you must set them to easily find it.

# Warning

This program can alter or delete all files without notice; or even worse, it can destroy the entire root files and essential system binaries. I do not have responsibility for any damages you have done.
