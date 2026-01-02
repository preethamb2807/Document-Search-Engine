# 📘 Document Search Engine (C)

## Overview

An **in-memory document search engine** implemented in C using core data structures. It indexes text files and supports fast keyword and prefix-based searching without using any database.

## Features

* Keyword search using **inverted index(hash table + singly linked list)**
* Prefix search using **Trie**
* Case-insensitive search
* Stop-word filtering
* Exact word locations (document, line, word position)
* Displays actual line text

## Data Structures

Trie, Hash Table, Singly Linked List, Arrays

## Usage

```bash
gcc main.c -o main.c
./main.c file1.txt file2.txt
```

**Note :** *file1.txt* and *file2.txt* are the input documents to be searched.

## Notes

Implemented entirely in C with potential applications in text search and information retrieval systems.

---
