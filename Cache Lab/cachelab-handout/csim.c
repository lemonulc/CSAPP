#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h> /*exit() malloc()*/
#include <getopt.h>
#include <unistd.h> /*getopt()*/
#include <string.h> /*strcpy*/

int h, v, s, E, b, S;

int hits = 0;
int misses = 0;
int evictions = 0;

char t[100];

typedef struct _node {
	int tag;
	struct _node *pre;
	struct _node *next;
} Node;

typedef struct _lru {
	Node *head;
	Node *tail;
	int size;
} LRU;

LRU *lru;

/*帮助文档*/
void print_help() {
	printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n"
	       "Options:\n"
	       "  -h         Print this help message.\n"
	       "  -v         Optional verbose flag.\n"
	       "  -s <num>   Number of set index bits.\n"
	       "  -E <num>   Number of lines per set.\n"
	       "  -b <num>   Number of block offset bits.\n"
	       "  -t <file>  Trace file.\n\n"
	       "Examples:\n"
	       "  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n"
	       "  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");

	exit(-1);
}

/*参数初始化*/
void arg_init(int argc, char *argv[]) {
	if (argc < 9) {
		print_help();
	}
	int opt;
	while (-1 != (opt = getopt(argc, argv, "hvs:E:b:t:"))) {
		switch (opt) {
			case 'h':
				h = 1;
				print_help();
			case 'v':
				v = 1;
				break;
			case 's':
				s = atoi(optarg);
				break;
			case 'E':
				E = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
			case 't':
				strcpy(t, optarg);
				break;
		}
	}
	S = 1 << s;
}

/*S组LRU链表初始化*/
void init_lru(int i) {
	lru[i].size = 0;
	lru[i].head = malloc(sizeof(Node));
	lru[i].tail = malloc(sizeof(Node));
	lru[i].head->next = lru[i].tail;
	lru[i].tail->pre = lru[i].head;
}

/*初始化cache模拟器*/
void cache_init() {
	lru = malloc(S * sizeof(LRU));
	for (int i = 0; i < S; i++) {
		init_lru(i);	//初始化每个组的LRU
	}
}

void del_node(Node *node, int i) {
	node->next->pre = node->pre;
	node->pre->next = node->next;
	lru[i].size--;
}

void add2head(Node *node, int i) {
	node->next = lru[i].head->next;
	node->next->pre = node;
	node->pre = lru[i].head;
	lru[i].head->next = node;
	lru[i].size++;
}

/*更新LRU链表*/
void update(unsigned addr) {
	unsigned s_index = (-1u >> (64 - s)) & (addr >> b);
	int tag_index = (addr >> (s + b));

	Node *tmp = lru[s_index].head->next;
	while (tmp != lru[s_index].tail) {
		if (tmp->tag == tag_index) {
			del_node(tmp, s_index);
			add2head(tmp, s_index);
			hits++;
			if (v == 1)
				printf(" hit");
			return;
		}
		tmp = tmp->next;
	}
	if (lru[s_index].size == E) {
		tmp = lru[s_index].tail->pre;
		del_node(tmp, s_index);
		free(tmp);
		evictions++;
		if (v == 1)
			printf(" eviction");
	}
	Node *new_node = malloc(sizeof(Node));
	new_node->tag = tag_index;
	add2head(new_node, s_index);
	misses++;
	if (v == 1)
		printf(" miss");
}

/*进行缓存模拟*/
void simulate() {
	FILE *pFile;
	pFile = fopen(t, "r");
	if (pFile == NULL) {
		printf("open error!\n");
		exit(-1);
	}
	char identifier;
	unsigned addr;
	int size;
	while (fscanf(pFile, "	%c	%x,%d", &identifier, &addr, &size) > 0) {
		if (v == 1)
			printf(" %c %x,%d", identifier, addr, size);
		switch (identifier) {
			case 'L':
				update(addr);
				if (v == 1)
					printf("\n");
				break;
			case 'M':
				update(addr);
			case 'S':
				update(addr);
				if (v == 1)
					printf("\n");
				break;
		}
	}
	fclose(pFile);
}

/*释放内存*/
void free_last() {
	for (int i = 0; i < S; i++) {
		Node *tmp = lru[i].head->next;
		while (tmp != lru[i].tail) {
			Node *next = tmp->next;
			free(tmp);
			tmp = next;
		}
		free(lru[i].head);
		free(lru[i].tail);
	}
	free(lru);
}

int main(int argc, char *argv[]) {
	arg_init(argc, argv);
	cache_init();
	simulate();
	free_last();
	printSummary(hits, misses, evictions);
	return 0;
}
