#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//CONSTANTS

#define MAX_WORD 100
#define HASH_SIZE 1000
#define MAX_POS 100
#define MAX_DOCS 10
#define MAX_LINES 1000
#define MAX_LINE_LEN 256

//STOP WORDS

const char *stopwords[] = {
    "the","is","a","an","of","to","in","on","for","and",
    "with","as","by","at","from"
};
#define STOPWORD_COUNT 15

int isStopWord(char *w) {
    for (int i = 0; i < STOPWORD_COUNT; i++)
        if (strcmp(w, stopwords[i]) == 0)
            return 1;
    return 0;
}

//TRIE

typedef struct trienode {
    struct trienode *child[256];
    int endofword;
} TRIE;

TRIE *trie_root;

TRIE* getnode() {
    TRIE *t = malloc(sizeof(TRIE));
    for (int i = 0; i < 256; i++)
        t->child[i] = NULL;
    t->endofword = 0;
    return t;
}

void trie_insert(TRIE *root, char *key) {
    TRIE *curr = root;
    for (int i = 0; key[i]; i++) {
        unsigned char idx = key[i];
        if (!curr->child[idx])
            curr->child[idx] = getnode();
        curr = curr->child[idx];
    }
    curr->endofword = 1;
}

//INVERTED INDEX

typedef struct posting {
    int doc_id;
    int freq;
    int line[MAX_POS];
    int word_pos[MAX_POS];
    int count;
    struct posting *next;
} POSTING;

typedef struct hashnode {
    char word[MAX_WORD];
    POSTING *plist;
    struct hashnode *next;
} HASHNODE;

HASHNODE *htable[HASH_SIZE];

//FILE STORAGE

char *file_lines[MAX_DOCS][MAX_LINES];
int doc_line_count[MAX_DOCS];

//UTILITY

unsigned hash(char *s) {
    unsigned h = 0;
    while (*s)
        h = h * 31 + *s++;
    return h % HASH_SIZE;
}

void normalize(char *s) {
    for (int i = 0; s[i]; i++)
        s[i] = tolower(s[i]);
}

//ADD TO INDEX

void addIndex(char *word, int doc_id, int line_no, int word_no) {
    unsigned h = hash(word);
    HASHNODE *node = htable[h];

    while (node) {
        if (strcmp(node->word, word) == 0) {
            POSTING *p = node->plist;
            while (p) {
                if (p->doc_id == doc_id) {
                    if (p->count < MAX_POS) {
                        p->line[p->count] = line_no;
                        p->word_pos[p->count] = word_no;
                        p->count++;
                    }
                    p->freq++;
                    return;
                }
                p = p->next;
            }
            POSTING *np = malloc(sizeof(POSTING));
            np->doc_id = doc_id;
            np->freq = 1;
            np->line[0] = line_no;
            np->word_pos[0] = word_no;
            np->count = 1;
            np->next = node->plist;
            node->plist = np;
            return;
        }
        node = node->next;
    }

    HASHNODE *newnode = malloc(sizeof(HASHNODE));
    strcpy(newnode->word, word);

    POSTING *p = malloc(sizeof(POSTING));
    p->doc_id = doc_id;
    p->freq = 1;
    p->line[0] = line_no;
    p->word_pos[0] = word_no;
    p->count = 1;
    p->next = NULL;

    newnode->plist = p;
    newnode->next = htable[h];
    htable[h] = newnode;

    trie_insert(trie_root, word);
}

//FILE PROCESS

void processFile(char *fname, int doc_id) {
    FILE *fp = fopen(fname, "r");
    if (!fp) return;

    char line[MAX_LINE_LEN];
    int line_no = 0;

    while (fgets(line, sizeof(line), fp) && line_no < MAX_LINES) {
        file_lines[doc_id][line_no] = strdup(line);
        doc_line_count[doc_id]++;
        line_no++;

        char word[MAX_WORD];
        int idx = 0, wpos = 0;

        for (int i = 0; line[i]; i++) {
            if (isalnum(line[i])) {
                word[idx++] = tolower(line[i]);
            } else if (idx > 0) {
                word[idx] = '\0';
                idx = 0;
                wpos++;
                if (!isStopWord(word))
                    addIndex(word, doc_id, line_no, wpos);
            }
        }

        // Handles last word
        if (idx > 0) {
            word[idx] = '\0';
            wpos++;
            if (!isStopWord(word))
                addIndex(word, doc_id, line_no, wpos);
        }
    }
    fclose(fp);
}

//SEARCH

void searchWord(char *key);

char buffer[MAX_WORD];
int blen;

void collectPrefix(TRIE *node) {
    if (node->endofword) {
        buffer[blen] = '\0';
        searchWord(buffer);
    }
    for (int i = 0; i < 256; i++) {
        if (node->child[i]) {
            buffer[blen++] = i;
            collectPrefix(node->child[i]);
            blen--;
        }
    }
}

void prefixSearch(char *prefix) {
    TRIE *curr = trie_root;
    for (int i = 0; prefix[i]; i++) {
        unsigned char idx = prefix[i];
        if (!curr->child[idx]) {
            printf("No prefix matches found\n");
            return;
        }
        curr = curr->child[idx];
    }
    strcpy(buffer, prefix);
    blen = strlen(prefix);
    collectPrefix(curr);
}

void searchWord(char *key) {
    if (isStopWord(key)) return;

    unsigned h = hash(key);
    HASHNODE *node = htable[h];

    while (node) {
        if (strcmp(node->word, key) == 0) {
            POSTING *p = node->plist;
            printf("\nWord: %s\n", key);
            while (p) {
                printf("Document %d (freq %d)\n", p->doc_id + 1, p->freq);
                for (int i = 0; i < p->count; i++) {
                    printf("  Line %d, Word %d: %s",
                           p->line[i],
                           p->word_pos[i],
                           file_lines[p->doc_id][p->line[i] - 1]);
                }
                p = p->next;
            }
            return;
        }
        node = node->next;
    }
    printf("Word not found\n");
}

//MEMORY CLEANUP

void freeTrie(TRIE *node) {
    if (!node) return;
    for (int i = 0; i < 256; i++)
        freeTrie(node->child[i]);
    free(node);
}

void freeHashTable() {
    for (int i = 0; i < HASH_SIZE; i++) {
        HASHNODE *node = htable[i];
        while (node) {
            HASHNODE *next = node->next;
            POSTING *p = node->plist;
            while (p) {
                POSTING *pn = p->next;
                free(p);
                p = pn;
            }
            free(node);
            node = next;
        }
    }
}

//MAIN

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s file1.txt file2.txt\n", argv[0]);
        return 0;
    }

    memset(htable, 0, sizeof(htable));
    memset(doc_line_count, 0, sizeof(doc_line_count));

    trie_root = getnode();

    for (int i = 1; i < argc && i <= MAX_DOCS; i++)
        processFile(argv[i], i - 1);

    char key[MAX_WORD];
    while (1) {
        printf("\nEnter word/prefix (exit to quit): ");
        scanf("%s", key);
        if (!strcmp(key, "exit")) break;
        normalize(key);

        printf("\n--- Exact Search ---\n");
        searchWord(key);

        printf("\n--- Prefix Search ---\n");
        prefixSearch(key);
    }

    freeHashTable();
    freeTrie(trie_root);

    for (int d = 0; d < MAX_DOCS; d++)
        for (int l = 0; l < doc_line_count[d]; l++)
            free(file_lines[d][l]);

    return 0;
}