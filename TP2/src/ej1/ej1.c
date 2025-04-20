#include "ej1.h"

char* strdup(const char* s) {
		char* copy = malloc(strlen(s) + 1);
		if (copy) strcpy(copy, s);
		return copy;
}
string_proc_list* string_proc_list_create(void){
	string_proc_list* list = (string_proc_list*) malloc(sizeof(string_proc_list));
	if (!list) return NULL; //reviso el malloc
	list->first = NULL;
	list->last = NULL;
	return list;

}


string_proc_node* string_proc_node_create(uint8_t type, char* hash){
	string_proc_node* node = (string_proc_node*) malloc(sizeof(string_proc_node));
	if (!node) return NULL;

	node->type = type;
	node->hash = hash;
	node->next = NULL;
	node->previous = NULL;
	return node;
}

void string_proc_list_add_node(string_proc_list* list, uint8_t type, char* hash){
	if (list == NULL) return;
	string_proc_node* node = string_proc_node_create(type, hash);
	if (!node) return;
	if (list->first == NULL) {
		//caso lista vacia
		list->first = node;
		list->last = node;
	} else {
		//caso lista no vacia
		node->previous = list->last;
		list->last->next = node;
		list->last = node;
	}
}


char* string_proc_list_concat(string_proc_list* list, uint8_t type , char* hash){
	//reviso los nulls
	if (list == NULL) return NULL;
	if (list->first == NULL) return NULL; 
	if (hash == NULL) return NULL; 
	if(!list->first-> next){
		//si la lista tiene un solo nodo
		if(list->first->type == type){
			char* result = str_concat(hash, list->first->hash);
			return result;
		}else{
			return hash;
		}
	}
	string_proc_node* node_actual = list->first;
	char* result = strdup(hash);
	while(node_actual != NULL){
		if(node_actual->type == type){
			char* aux = str_concat(result, node_actual->hash);
			free(result);
			result = aux;
		
		}
		node_actual = node_actual->next;
	}
	return result;
}


/** AUX FUNCTIONS **/

void string_proc_list_destroy(string_proc_list* list){
	/* borro los nodos: */
	string_proc_node* current_node	= list->first;
	string_proc_node* next_node		= NULL;
	while(current_node != NULL){
		next_node = current_node->next;
		string_proc_node_destroy(current_node);
		current_node	= next_node;
	}
	/*borro la lista:*/
	list->first = NULL;
	list->last  = NULL;
	free(list);
}
void string_proc_node_destroy(string_proc_node* node){
	node->next      = NULL;
	node->previous	= NULL;
	node->hash		= NULL;
	node->type      = 0;			
	free(node);
}


char* str_concat(char* a, char* b) {
	int len1 = strlen(a);
	int len2 = strlen(b);
	int totalLength = len1 + len2;
	char *result = (char *)malloc(totalLength + 1); 
	strcpy(result, a);
	strcat(result, b);
	return result;  
}

void string_proc_list_print(string_proc_list* list, FILE* file){
		uint32_t length = 0;
		string_proc_node* current_node  = list->first;
		while(current_node != NULL){
				length++;
				current_node = current_node->next;
		}
		fprintf( file, "List length: %d\n", length );
		current_node    = list->first;
		while(current_node != NULL){
				fprintf(file, "\tnode hash: %s | type: %d\n", current_node->hash, current_node->type);
				current_node = current_node->next;
		}
}

