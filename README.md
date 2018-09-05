# CIBL
CIBL is two separate minimal Lisp implementation built in C and Python respectively as a personal linguistics
project. I intend to use CIBL for educational purposes as part of my work
teaching programming workshops for UW CSE Outreach. The python interpreter is ready to go but the C intepreter is still in progress!

## Naming and Influences (/ˈkɪbəl/)
CIBL Is Basically Lisp! :smile: (I couldn't resist the recursive acronym)

## Syntax and Design
Build on OSX. Support for Windows, OSX, and Linux

#### Polish Notation 
`` (+ 5 (* 8 2)) ``
#### Linked Lists?
Unlike most Lisps, which utilize the Linked List, CIBL's S-Expressions are 
instead built up using explicitly sized arrays and pointers.

`` (list 1 2 (quote foo)) ``
#### CIBL BNF
Backus–Naur Form (BNF) forthcoming...
