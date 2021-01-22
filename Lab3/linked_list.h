#define LINE_MAX 1024

typedef struct node_s {
  char *line;
  struct node_s *next;
} node_t;

typedef struct list_s {
  node_t *head;
  node_t *tail;
} list_t;
