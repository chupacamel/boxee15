//#ifndef _WIN32
#define ERROR 12345
// #endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct list_item {
	struct list_item* next;
	struct list_item* prev;
	unsigned long content;
};

struct list {
	struct list_item* head;
};

struct list* new_list(); 

struct list_item* new_list_item(unsigned long content);

void delete_list(struct list* list);
	
int compare(struct list_item* item1, struct list_item* item2);

int insert(struct list* lst, unsigned long content);

int is_in_list(struct list* lst, unsigned long content);

int is_list_empty( struct list* lst );

int head( struct list* lst, unsigned long* content, int remove );

#ifdef __cplusplus
};
#endif //__cplusplus
