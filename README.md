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
gcc doc_search.c -o doc_search
./doc_search file1.txt file2.txt
```

## Notes

Implemented entirely in C for academic and learning purposes.

---