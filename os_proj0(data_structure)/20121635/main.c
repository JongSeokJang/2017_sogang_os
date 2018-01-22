#include "bitmap.h"
#include "hash.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_SIZE	1024
#define TRUE			1
#define LIST			1
#define HASH			2
#define BITMAP			3

typedef struct _LIST_DATA{
	int val;
	struct list_elem elem;
}LIST_DATA;

typedef struct _HASH_DATA{
	int val;
	struct hash_elem elem;
}HASH_DATA;

typedef struct _DS_SET{
	char	name[32+1];
	int		type;
	void	*dummy;
	struct list_elem elem; // connect prev, next
}DS_SET;

typedef struct _LIST_SET{
	char	name[32+1];
	int		type;
	struct list *list_master; // information about head and tail
	struct list_elem elem; // connect prev, next
}LIST_SET;

typedef struct _HASH_SET{
	char	name[32+1];
	int 	type;
	struct  hash *hash_master;
	struct list_elem elem; // connect prev, next
}HASH_SET;

typedef struct _BITMAP_SET{
	char	name[32+1];
	int 	type;
	void	*bitmap_master;
	struct list_elem elem; // connect prev, next
}BITMAP_SET;

// hash action_func
void hash_clear_func(struct hash_elem *e, void *aux);
void hash_square(struct hash_elem *e, void *aux);
void hash_triple(struct hash_elem *e, void *aux);
bool hash_less (const struct hash_elem *a, const struct hash_elem *b, void *aux);
unsigned hash_hash (const struct hash_elem *e, void *aux);

// list action_func
bool list_less (const struct list_elem *a,const struct list_elem *b,void *aux);
struct list_elem *list_nth(struct list *list_set, int findex);

// misc
int MakeTokenTable(char* inputCmd, char*** tokenTable);
int SystemPrompt(struct list *data_set);
void* FindDSHead(char *name, struct list *data_set);


int main(void){

	struct list data_set;
	int result = 0;

	list_init(&data_set);

	while(!result) {
		result = SystemPrompt(&data_set);
	}

	return 0;

}

int SystemPrompt(struct list *data_set)
{
	
	char	inputCmd[MAX_INPUT_SIZE];
	char	**tokenArray	= NULL;
	char	*token			= NULL;
	int		pCnt	= 0;
	int		ii		= 0 ;

	memset(inputCmd,	0x00, sizeof(inputCmd)	);

	fgets(inputCmd, sizeof(inputCmd), stdin);	
	
	if(inputCmd[0] == '\n' || inputCmd[0] == '\0')
		return -1;
	
	inputCmd[ strlen(inputCmd) -1 ]  = '\0';

	// make token to token Array
	pCnt = MakeTokenTable(inputCmd, &tokenArray);	
	
	if( !strcmp(tokenArray[0], "create") ){

		if( !strcmp(tokenArray[1], "list") ){

			LIST_SET *temp = (LIST_SET*)malloc( sizeof(LIST_SET) );

			temp->list_master = (struct list*)malloc(sizeof(struct list));
			list_init( temp->list_master );

			strcpy(	temp->name, tokenArray[2] );
			temp->type = LIST;
			list_push_back( data_set, &(temp->elem) );

		}
		else if( !strcmp(tokenArray[1], "hashtable") ){

			HASH_SET *temp = (HASH_SET*)malloc( sizeof(HASH_SET) );

			temp->hash_master = (struct hash*)malloc(sizeof(struct hash));
			hash_init( temp->hash_master, hash_hash, hash_less, NULL);

			strcpy(	temp->name, tokenArray[2] );
			temp->type = HASH;
			list_push_back( data_set, &(temp->elem) );
	
		}
		else if( !strcmp(tokenArray[1], "bitmap") ){
		
			BITMAP_SET *temp = (BITMAP_SET*)malloc( sizeof(BITMAP_SET) );		

			temp->bitmap_master = (void*)bitmap_create( atoi(tokenArray[3]) );

			strcpy( temp->name, tokenArray[2] );
			temp->type = BITMAP;
			list_push_back( data_set, &(temp->elem) );

		}
		else{
			printf("4. %s\n", inputCmd);
		}

	}
	else if( !strcmp(tokenArray[0], "delete") ){

		DS_SET *head = (DS_SET*)FindDSHead(tokenArray[1], data_set);

		if( head->type == LIST ){

			LIST_SET *spec_list = (LIST_SET*)head;
			LIST_SET *temp;
			struct list_elem *e;

			for( e = list_begin( spec_list->list_master );
				 e != list_end( spec_list->list_master );
				 e =list_remove(e));

			for (   e = list_begin(data_set);
					e != list_end(data_set);
					e = list_next(e) ){

				temp = list_entry(e, LIST_SET, elem);
				if( !strcmp(temp->name, tokenArray[1]) ){
					list_remove(e);
					free(temp);
				}
			}

		}
		else if( head->type == HASH ){
		
			HASH_SET *spec_hash	= (HASH_SET*)head;
			hash_destroy(spec_hash->hash_master, hash_clear_func);

		}
		else if( head->type == BITMAP ){

			BITMAP_SET *spec_bitmap = (BITMAP_SET*)head;
			bitmap_destroy( spec_bitmap->bitmap_master );

		}
		else{

		}
	}
	else if( !strcmp(tokenArray[0], "quit") ){

		return -1;	

	}
	else if( !strcmp(tokenArray[0], "dumpdata") ){
		
		if( pCnt != 2 )
			return 0;

		DS_SET *temp = (DS_SET*)FindDSHead(tokenArray[1], data_set);
		
		if( temp->type == LIST ){

			LIST_SET *spec_list = (LIST_SET*)temp;

			if( spec_list != NULL ){

				struct list_elem *e;
				for( e = list_begin( spec_list->list_master );
					 e != list_end( spec_list->list_master );
					 e = list_next(e) ){

					LIST_DATA *temp = list_entry(e, LIST_DATA, elem);	
					printf("%d ", temp->val );

				}
				printf("\n");	

			}
		}
		else if( temp->type == HASH ){

			HASH_SET *spec_hash = (HASH_SET*)temp;
			struct hash_iterator iterator;

			if( hash_empty(spec_hash->hash_master) )
				return 0;

			hash_first( &iterator, spec_hash->hash_master );
	
			for(iterator; hash_next(&iterator)!= NULL; ){
                printf("%d ", hash_entry( iterator.elem, HASH_DATA,elem)->val );
			}
			printf("\n");

		}
		else if ( temp->type == BITMAP ){

			BITMAP_SET *spec_bitmap = (BITMAP_SET*)temp;
			int bitmapSize = bitmap_size(spec_bitmap->bitmap_master);

			for( ii = 0; ii< bitmapSize; ii++ )
				if( bitmap_test(spec_bitmap->bitmap_master, ii) == 1 )
					printf("1");
				else
					printf("0");

			printf("\n");

		}
		else{
			return 0;
		}
	}
	else{

		DS_SET *temp = (DS_SET*)FindDSHead(tokenArray[1], data_set);

		if ( !strcmp(tokenArray[0], "list_insert") ){

			struct list_elem *e;
			int index = 0;
			int insert_index = atoi(tokenArray[2]);
			LIST_SET	*spec_list	= (LIST_SET*)temp;
            LIST_DATA	*temp		= (LIST_DATA*)malloc( sizeof(LIST_DATA) );

            temp->val = atoi(tokenArray[3]);

			if( list_begin( spec_list->list_master) == list_end( spec_list->list_master) 
				|| insert_index == 0 ){
				 list_push_front( spec_list->list_master, &(temp->elem));
			}
			else{
				for( e = list_begin( spec_list->list_master );
					 e != list_end( spec_list->list_master );
					 e = list_next(e), index++ ){

					if( index + 1 == insert_index ){
						list_insert( list_next(e), &(temp->elem) );
						break;
					}	
				}

			}
		}
		else if( !strcmp(tokenArray[0], "list_splice") ){
		
			LIST_SET *insert_list = (LIST_SET*)FindDSHead(tokenArray[1], data_set);		
			LIST_SET *domain_list = (LIST_SET*)FindDSHead(tokenArray[3], data_set);		
			int insert_list_index = atoi(tokenArray[2]);
			int domain_list_start = atoi(tokenArray[4]);
			int domain_list_end   = atoi(tokenArray[5]);
			struct list_elem *a, *b, *c;

			a=list_nth(insert_list->list_master, insert_list_index);
			b=list_nth(domain_list->list_master, domain_list_start);
			c=list_nth(domain_list->list_master, domain_list_end);

			list_splice(a,b,c);	
	
		}
		else if( !strcmp(tokenArray[0], "list_push_front") ){

			LIST_SET	*spec_list	= (LIST_SET*)temp;
			LIST_DATA	*temp		= (LIST_DATA*)malloc( sizeof(LIST_DATA) );
			temp->val = atoi(tokenArray[2]);
			list_push_front( spec_list->list_master, &(temp->elem));

		}
		else if( !strcmp(tokenArray[0], "list_push_back") ){

			LIST_SET	*spec_list	= (LIST_SET*)temp;
			LIST_DATA	*temp		= (LIST_DATA*)malloc( sizeof(LIST_DATA) );
			temp->val = atoi(tokenArray[2]);
			list_push_back( spec_list->list_master, &(temp->elem));

		}
		else if( !strcmp(tokenArray[0], "list_remove") ){

			LIST_SET	*spec_list	= (LIST_SET*)temp;
            struct list_elem *e;
			int  index = 0;

            for( e = list_begin( spec_list->list_master );
                 e != list_end( spec_list->list_master );
                 e = list_next(e), index++ ){

				if( index == atoi(tokenArray[2]) ){
					LIST_DATA *temp = list_entry(e, LIST_DATA, elem); 
					list_remove(e);
					free(temp);
					return 0;
				}
			}
		}
		else if( !strcmp(tokenArray[0], "list_pop_front") ){

			LIST_SET	*spec_list	= (LIST_SET*)temp;
			struct list_elem *e = list_pop_front( spec_list->list_master );

			LIST_DATA	*temp		= list_entry(e, LIST_DATA, elem);

		}
		else if( !strcmp(tokenArray[0], "list_pop_back") ){

			LIST_SET	*spec_list	= (LIST_SET*)temp;
			struct list_elem *e = list_pop_back( spec_list->list_master );

			LIST_DATA	*temp		= list_entry(e, LIST_DATA, elem);

		}
		else if( !strcmp(tokenArray[0], "list_front") ){

			LIST_SET	*spec_list	= (LIST_SET*)temp;
			struct list_elem *e = list_front( spec_list->list_master );

			LIST_DATA	*temp		= list_entry(e, LIST_DATA, elem);
            printf("%d\n", temp->val );

		}
		else if( !strcmp(tokenArray[0], "list_back") ){

			LIST_SET	*spec_list	= (LIST_SET*)temp;
			struct list_elem *e = list_back( spec_list->list_master );

			LIST_DATA	*temp		= list_entry(e, LIST_DATA, elem);
            printf("%d\n", temp->val );
	
		}
		else if( !strcmp(tokenArray[0], "list_size") ){

			LIST_SET	*spec_list	= (LIST_SET*)temp;
			int listSize = list_size( spec_list->list_master );
			printf("%d\n", listSize);
	
		}
		else if( !strcmp(tokenArray[0], "list_empty") ){

			LIST_SET	*spec_list = (LIST_SET*)temp;
			bool isEmpty = list_empty( spec_list->list_master );
			if(isEmpty)
				printf("true\n");
			else
				printf("false\n");
		}
		else if( !strcmp(tokenArray[0], "list_reverse") ){

			LIST_SET	*spec_list = (LIST_SET*)temp;
			list_reverse( spec_list->list_master );
	
		}
		else if( !strcmp(tokenArray[0], "list_sort") ){
		
			LIST_SET	*spec_list = (LIST_SET*)temp;
			list_sort( spec_list->list_master, list_less, NULL );
			
		}
		else if( !strcmp(tokenArray[0], "list_insert_ordered") ){

			LIST_SET	*spec_list	= (LIST_SET*)temp;
			LIST_DATA	*temp		= (LIST_DATA*)malloc( sizeof(LIST_DATA) );
            temp->val = atoi(tokenArray[2]);
			
			list_sort( spec_list->list_master, list_less, NULL );	
			list_insert_ordered( spec_list->list_master, &(temp->elem), list_less, NULL);
			
		}
		else if( !strcmp(tokenArray[0], "list_unique") ){

			if( pCnt == 2){
				LIST_SET *a_list = (LIST_SET*)FindDSHead(tokenArray[1], data_set);
				list_unique(a_list->list_master, NULL, list_less, NULL);
			}
			else{
				LIST_SET *a_list = (LIST_SET*)FindDSHead(tokenArray[1], data_set);
				LIST_SET *b_list = (LIST_SET*)FindDSHead(tokenArray[2], data_set);
				
				list_unique(a_list->list_master, b_list->list_master, list_less, NULL);
			}

		}
		else if( !strcmp(tokenArray[0], "list_max") ){

			LIST_SET	*spec_list = (LIST_SET*)temp;
			struct list_elem *e = list_max( spec_list->list_master, list_less, NULL );

            LIST_DATA *temp = list_entry(e, LIST_DATA, elem);
			printf("%d\n", temp->val);
	
		}
		else if( !strcmp(tokenArray[0], "list_min") ){

			LIST_SET	*spec_list = (LIST_SET*)temp;
			struct list_elem *e = list_min( spec_list->list_master, list_less, NULL );

            LIST_DATA *temp = list_entry(e, LIST_DATA, elem);
			printf("%d\n", temp->val);
	
		}
		else if( !strcmp(tokenArray[0], "list_swap") ){
	
			LIST_SET	*spec_list = (LIST_SET*)temp;
			int index1 = atoi(tokenArray[2]);
			int index2 = atoi(tokenArray[3]);
			struct list_elem *a, *b;

			a=list_nth(spec_list->list_master, index1);
			b=list_nth(spec_list->list_master, index2);
		
			list_swap(a,b);	
				
		}
		else if( !strcmp(tokenArray[0], "list_shuffle") ){
			
			LIST_SET	*spec_list = (LIST_SET*)temp;

			list_shuffle( spec_list->list_master );

		}
		else if( !strcmp(tokenArray[0], "hash_insert") ){

			HASH_SET *spec_hash = (HASH_SET*)temp;
			HASH_DATA *temp		= (HASH_DATA*)malloc( sizeof(HASH_DATA) );
			struct hash_elem *e;		
			temp->val = atoi(tokenArray[2]);

			e = hash_insert( spec_hash->hash_master, &(temp->elem) );
				
		}
		else if( !strcmp(tokenArray[0], "hash_replace") ){

			HASH_SET *spec_hash = (HASH_SET*)temp;
			HASH_DATA *temp		= (HASH_DATA*)malloc( sizeof(HASH_DATA) );
			struct hash_elem *e;		
			temp->val = atoi(tokenArray[2]);

			e = hash_replace( spec_hash->hash_master, &(temp->elem) );
	
		}
		else if( !strcmp(tokenArray[0], "hash_find") ){

			HASH_SET *spec_hash = (HASH_SET*)temp;
			HASH_DATA *temp		= (HASH_DATA*)malloc( sizeof(HASH_DATA) );
			struct hash_elem *e;		
			temp->val = atoi(tokenArray[2]);

			e = hash_find( spec_hash->hash_master, &(temp->elem) );
			
			if( e != NULL)
				printf("%d\n", hash_entry(e, HASH_DATA,elem)->val);
		
		}
		else if( !strcmp(tokenArray[0], "hash_delete") ){

			HASH_SET *spec_hash = (HASH_SET*)temp;
			HASH_DATA *temp		= (HASH_DATA*)malloc( sizeof(HASH_DATA) );
			struct hash_elem *e;		
			temp->val = atoi(tokenArray[2]);

			e = hash_delete( spec_hash->hash_master, &(temp->elem) );
	
		}
		else if( !strcmp(tokenArray[0], "hash_clear") ){

			HASH_SET *spec_hash = (HASH_SET*)temp;

			hash_clear (spec_hash->hash_master, hash_clear_func);	

		}
		else if( !strcmp(tokenArray[0], "hash_size") ){

			HASH_SET *spec_hash = (HASH_SET*)temp;

			int hashSize = hash_size( spec_hash->hash_master );
			printf("%d\n", hashSize);

		}
		else if( !strcmp(tokenArray[0], "hash_empty") ){
	
			HASH_SET *spec_hash = (HASH_SET*)temp;
			bool check;
			check = hash_empty( spec_hash->hash_master);
			if( check == true )
				printf("true\n");
			else
				printf("false\n");

		}
		else if( !strcmp(tokenArray[0], "hash_apply") ){

			HASH_SET *spec_hash = (HASH_SET*)temp;
			char *apply_param = tokenArray[2];

			void hash_square(struct hash_elem *e, void *aux);

			if( !strcmp(apply_param, "square") ){
				hash_apply( spec_hash->hash_master, hash_square );
			}
			else if( !strcmp(apply_param, "triple") ){
				hash_apply( spec_hash->hash_master, hash_triple );
			}
			else{

			}

		}
		else if( !strcmp(tokenArray[0], "bitmap_size") ){

			BITMAP_SET *spec_bitmap = (BITMAP_SET*)temp;

			int bitmapSize = bitmap_size( spec_bitmap->bitmap_master );	
			printf("%d\n", bitmapSize);
	
		}
		else if( !strcmp(tokenArray[0], "bitmap_set") ){

			BITMAP_SET *spec_bitmap = (BITMAP_SET*)temp;
			int bitmap_index = atoi(tokenArray[2]);
			char *set_check = tokenArray[3];
			if( !strcmp(set_check, "true") )
				bitmap_set( spec_bitmap->bitmap_master, bitmap_index, 1 );
			else
				bitmap_set( spec_bitmap->bitmap_master, bitmap_index, 0 );
	
		}
		else if( !strcmp(tokenArray[0], "bitmap_mark") ){

			BITMAP_SET *spec_bitmap = (BITMAP_SET*)temp;
			int bitmap_index = atoi(tokenArray[2]);

			bitmap_set( spec_bitmap->bitmap_master, bitmap_index, 1 );
	
		}
		else if( !strcmp(tokenArray[0], "bitmap_reset") ){

			BITMAP_SET *spec_bitmap = (BITMAP_SET*)temp;
			int bitmap_index = atoi(tokenArray[2]);

			bitmap_set( spec_bitmap->bitmap_master, bitmap_index, 0 );
	
		}
		else if( !strcmp(tokenArray[0], "bitmap_flip") ){

			BITMAP_SET *spec_bitmap = (BITMAP_SET*)temp;
			int bitmap_index = atoi(tokenArray[2]);
			
			bitmap_flip( spec_bitmap->bitmap_master, bitmap_index);
		}
		else if( !strcmp(tokenArray[0], "bitmap_test") ){

			BITMAP_SET *spec_bitmap = (BITMAP_SET*)temp;
			int bitmap_index = atoi(tokenArray[2]);
			
			bool test = bitmap_test( spec_bitmap->bitmap_master, bitmap_index);
			if(test == true)
				printf("true\n");
			else
				printf("false\n");

		}
		else if( !strcmp(tokenArray[0], "bitmap_set_all") ){

			BITMAP_SET *spec_bitmap = (BITMAP_SET*)temp;
			char *set_check = tokenArray[2];
		
			if( !strcmp(set_check, "true") )
				bitmap_set_all( spec_bitmap->bitmap_master, 1 );
			else
				bitmap_set_all( spec_bitmap->bitmap_master, 0 );
	
	
		}
		else if( !strcmp(tokenArray[0], "bitmap_set_multiple") ){
	
			BITMAP_SET *spec_bitmap = (BITMAP_SET*)temp;
			int bitmap_sindex = atoi(tokenArray[2]);
			int bitmap_set_cnt = atoi(tokenArray[3]);
			char *set_check = tokenArray[4];

			if( !strcmp(set_check, "true") )
				bitmap_set_multiple( spec_bitmap->bitmap_master, bitmap_sindex,bitmap_set_cnt, 1);
			else
				bitmap_set_multiple( spec_bitmap->bitmap_master, bitmap_sindex,bitmap_set_cnt, 0);

		}
		else if( !strcmp(tokenArray[0], "bitmap_count") ){
	
			BITMAP_SET *spec_bitmap = (BITMAP_SET*)temp;
			int bitmap_sindex = atoi(tokenArray[2]);
			int bitmap_set_cnt = atoi(tokenArray[3]);
			char *set_check = tokenArray[4];
			int result = 0;

			if( !strcmp(set_check, "true") )
				result = bitmap_count( spec_bitmap->bitmap_master, bitmap_sindex,bitmap_set_cnt, 1);
			else
				result = bitmap_count( spec_bitmap->bitmap_master, bitmap_sindex,bitmap_set_cnt, 0);
		
			printf("%d\n",result);

		}
		else if( !strcmp(tokenArray[0], "bitmap_contains") ){

			BITMAP_SET *spec_bitmap = (BITMAP_SET*)temp;
			int bitmap_sindex = atoi(tokenArray[2]);
			int bitmap_set_cnt = atoi(tokenArray[3]);
			char *set_check = tokenArray[4];
			bool result;

			if( !strcmp(set_check, "true") )
				result = bitmap_count( spec_bitmap->bitmap_master, bitmap_sindex,bitmap_set_cnt, 1);
			else
				result = bitmap_count( spec_bitmap->bitmap_master, bitmap_sindex,bitmap_set_cnt, 0);

			if( result == true )
				printf("true\n");
			else
				printf("false\n");
	
		}
		else if( !strcmp(tokenArray[0], "bitmap_any") ){

			BITMAP_SET *spec_bitmap = (BITMAP_SET*)temp;
			int bitmap_sindex = atoi(tokenArray[2]);
			int bitmap_set_cnt = atoi(tokenArray[3]);
			bool result;

			result = bitmap_any( spec_bitmap->bitmap_master, bitmap_sindex,bitmap_set_cnt);
			
			if( result == true )
				printf("true\n");
			else
				printf("false\n");

		}
		else if( !strcmp(tokenArray[0], "bitmap_none") ){

			BITMAP_SET *spec_bitmap = (BITMAP_SET*)temp;
			int bitmap_sindex = atoi(tokenArray[2]);
			int bitmap_set_cnt = atoi(tokenArray[3]);
			bool result;

			result = bitmap_none( spec_bitmap->bitmap_master, bitmap_sindex,bitmap_set_cnt);
			
			if( result == true )
				printf("true\n");
			else
				printf("false\n");
	
		}
		else if( !strcmp(tokenArray[0], "bitmap_all") ){
	
			BITMAP_SET *spec_bitmap = (BITMAP_SET*)temp;
			int bitmap_sindex = atoi(tokenArray[2]);
			int bitmap_set_cnt = atoi(tokenArray[3]);
			
			bool result;

			result = bitmap_all( spec_bitmap->bitmap_master, bitmap_sindex,bitmap_set_cnt);
			
			if( result == true )
				printf("true\n");
			else
				printf("false\n");

		}
		else if( !strcmp(tokenArray[0], "bitmap_scan") ){

			BITMAP_SET *spec_bitmap = (BITMAP_SET*)temp;
			int bitmap_sindex = atoi(tokenArray[2]);
			int bitmap_set_cnt = atoi(tokenArray[3]);
			char *set_check = tokenArray[4];
			size_t result;

			if( !strcmp(set_check, "true") )
				result = bitmap_scan( spec_bitmap->bitmap_master, bitmap_sindex, bitmap_set_cnt, true);
			else
				result = bitmap_scan( spec_bitmap->bitmap_master, bitmap_sindex, bitmap_set_cnt, false);
			
			printf("%u\n", result);
		}
		else if( !strcmp(tokenArray[0], "bitmap_scan_and_flip") ){

			BITMAP_SET *spec_bitmap = (BITMAP_SET*)temp;
			int bitmap_sindex = atoi(tokenArray[2]);
			int bitmap_set_cnt = atoi(tokenArray[3]);
			char *set_check = tokenArray[4];
			size_t result;

			if( !strcmp(set_check, "true") )
				result = bitmap_scan_and_flip( spec_bitmap->bitmap_master, bitmap_sindex, bitmap_set_cnt, true);
			else
				result = bitmap_scan_and_flip( spec_bitmap->bitmap_master, bitmap_sindex, bitmap_set_cnt, false);
			
			printf("%u\n", result);
	
		}
		else if( !strcmp(tokenArray[0], "bitmap_dump") ){

			BITMAP_SET *spec_bitmap = (BITMAP_SET*)temp;
			bitmap_dump(spec_bitmap->bitmap_master);	

		}
		else if( !strcmp(tokenArray[0], "bitmap_expand") ){
			
			BITMAP_SET *spec_bitmap = (BITMAP_SET*)temp;
			int expand_bitmap_size = atoi(tokenArray[2]);
			
			spec_bitmap->bitmap_master = bitmap_expand( spec_bitmap->bitmap_master, expand_bitmap_size);
		}
		else{

		}

	}

	return 0;

}

int MakeTokenTable(char* inputCmd, char*** tokenTable)
{
	char	tempCmd[MAX_INPUT_SIZE];
	char	*temp	= NULL;
	int		ii		= 0;
	int		pCnt	= 0;

	memset(tempCmd,		0x00,	sizeof(tempCmd) );

	for( ii = 0; ii < (int)strlen(inputCmd) ; ii++ ){
		if( inputCmd[ii] != ' ' && 
				( inputCmd[ii+1] == ' ' || inputCmd[ii+1] == '\0' ) )
			pCnt++;	
	}
	
	*tokenTable = (char**)malloc(sizeof(char*) * pCnt);	
	strcpy(tempCmd, inputCmd);
	
	for( temp = strtok(tempCmd, " "), ii = 0; temp != NULL; temp = strtok(NULL, " ") ){
		(*tokenTable)[ii] = (char*)malloc(sizeof(char) * ( strlen(temp) + 1 ) );
		strcpy((*tokenTable)[ii++], temp);
	}

	return pCnt;
}
	
void* FindDSHead(char *name, struct list *data_set)
{
	struct list_elem *e;
	for(e = list_begin(data_set); e != list_end(data_set); 
		e = list_next(e)){

		DS_SET *temp = list_entry(e, DS_SET, elem);
		if( !strcmp(temp->name, name) ){
			return temp;	
		}
			
	}
	return NULL;
}

struct list_elem *list_nth(struct list *list_set, int findex)
{

	struct list_elem *e;
	int index =0;

	for( e = list_begin( list_set );
		 e != list_end( list_set );
		 e = list_next(e), index++ ){

		if( index == findex )
			return e;
	}
	return NULL;
	
}

bool list_less (const struct list_elem *a,const struct list_elem *b,void *aux)
{
    LIST_DATA *list_data_a = list_entry(a, LIST_DATA, elem);
    LIST_DATA *list_data_b = list_entry(b, LIST_DATA, elem);

    if( list_data_a->val < list_data_b->val)
        return true;
    else
        return false;
}

unsigned hash_hash (const struct hash_elem *e, void *aux)
{
	return hash_int(hash_entry(e, HASH_DATA, elem)->val);
	//return hash_int_2(hash_entry(e, HASH_DATA, elem)->val);
}

bool hash_less (const struct hash_elem *a, const struct hash_elem *b, void *aux)
{
    HASH_DATA *hash_data_a = hash_entry(a, HASH_DATA, elem);
    HASH_DATA *hash_data_b = hash_entry(b, HASH_DATA, elem);

    if( hash_data_a->val < hash_data_b->val)
        return true;
    else
		return false;

}

void hash_square(struct hash_elem *e, void *aux)
{
	HASH_DATA *hash_data = hash_entry(e, HASH_DATA, elem);
	hash_data->val = (hash_data->val) * (hash_data->val);

}

void hash_triple(struct hash_elem *e, void *aux)
{
	HASH_DATA *hash_data = hash_entry(e, HASH_DATA, elem);
	hash_data->val = (hash_data->val) * (hash_data->val) * (hash_data->val);

}

void hash_clear_func(struct hash_elem *e, void *aux)
{
	HASH_DATA *hash_data = hash_entry(e, HASH_DATA, elem);
	free(hash_data);
}
