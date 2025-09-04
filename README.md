# Advanced Operating Systems – Assignment 2  
**POSIX Shell in C++**

**Name:** Satyajit Priyadarshi  
**Roll Number:** 2025201055  

---

## Project Overview
This project is a custom POSIX shell written in **C++**.  
It works like a basic version of bash, supporting command execution, process control, redirection, piping, and features such as history and autocomplete.  
The full logic is inside the single file **`main.cpp`**, with a **Makefile** for compilation.

---

## Features
- **Prompt:** `username@system:current_directory>`  
- **Execution:** Supports multiple (`;`) and background (`&`) commands  
- **Built-ins:** `cd`, `pwd`, `echo`, `ls (-a, -l)`, `pinfo`, `search`, `history`  
- **Redirection & Pipes:** `<`, `>`, `>>`, `|`, and combinations  
- **Interactive:**  
  - Signal handling (`Ctrl+C`, `Ctrl+Z`, `Ctrl+D`)  
  - Arrow key history navigation  
  - TAB autocomplete for commands and files  

---

## How to Run
```bash
make
./my_shell
```