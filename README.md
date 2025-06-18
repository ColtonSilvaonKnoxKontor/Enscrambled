# Enscrambled

This is the ransomware version of Enscrambled where it uses AES method. It works on any Linux distribution with required dependencies installed.

# What is This

This program forces to encrypt all files, including files with restricted permissions. It also delete files automatically if you enter incorrect password.

# Status?

Ongoing

# How to Compile?
using `g++`, include `-lcrypto` and `-pthread` flag after `-o`.

# Features

Keyboard interrupt blocked (ctrl+z, ctrl+c)

# Work in Progress

Auto-install required dependencies

Bypass all kill signals from *top

Sudo required prompt (IF WE WANT TO INFECT SYSTEM FILES)

Background process

Multi-thread (Not too stress on CPU)

Thread/Process revival

# Warning

This program can alter or delete all files without notice; or even worse, it can destroy the entire root files and essential system binaries. I do not have responsibility for any damages you have done.
