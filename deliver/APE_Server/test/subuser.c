#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef struct subuser_{
	int num;
	struct subuser_ *next;
} subuser;

subuser* init_sub(int num)
{
	subuser *sub = (subuser*)calloc(1, sizeof(subuser));
	sub->num = num;
	sub->next = NULL;
	return sub;
}

void del_sub(subuser **cur)
{
	subuser *del = *cur;

	*cur = (*cur)->next;

	//del->next = NULL;

	free(del);
}

int main()
{
	subuser *list, *node;
	int i;

	list = node = NULL;
	for (i = 0; i < 4; i++) {
		node = init_sub(i);
		node->next = list;
		list = node;
	}

	node = list;
	while (node != NULL) {
		printf("%d\n", node->num);
		node = node->next;
	}

	printf("\n\n");
	
	node = list->next;
	del_sub(&node);
	
	node = list;
	while (node != NULL) {
		printf("%d\n", node->num);
		node = node->next;
	}
	
	return 0;
}
