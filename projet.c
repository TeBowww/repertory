/*
Projet N°1  UE :Algorithmique et Structure de Données
Codé par Thibault Delavoux  groupe 2B
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/time.h>

#define NAME_LENGTH_MIN 3
#define NAME_LENGTH_MAX 10
#define TELEPHONE_LENGTH 8 
#define MAX(a,b) (((a)>(b))?(a):(b))
#define FNV_PRIME_32 16777619
#define OFFSET_BASIS 2166136261
#define BUFSIZE 2


  //---------------------- Definition des Structures -----------------------------------//


struct directory_data {
	char last_name[NAME_LENGTH_MAX + 1];
	char first_name[NAME_LENGTH_MAX + 1];
	char telephone[TELEPHONE_LENGTH + 1];
};

struct directory{
	struct directory_data **data;
	size_t size;
	size_t capacity;	
};

typedef size_t (*index_hash_func_t)(const struct directory_data *data); 

struct index_bucket{
	const struct directory_data *data;
	struct index_bucket *next;	
};

struct index{
	struct index_bucket **buckets;
	size_t count;
	size_t size;
	index_hash_func_t funct;	
};



//------------ Definition des Fonctions traitant le repertoire------------------------ //


//affichage des information d'un contact sous forme NOM PRENOM TELEPHONE
void directory_data_print(const struct directory_data *data){
	printf("%s %s %s\n", data->last_name, data->first_name, data->telephone);
}

//initialisation aléatoire d'une chaine nom ou prénom
void directory_random_name(char *t){
	char voyelle[] = {'A','E','I','O','U','Y'};
	char consone[] = {'B', 'C', 'D', 'F', 'G', 'H', 'J', 'L', 'M', 'N', 'P', 'R', 'S', 'T', 'V'};
	int longueur = (rand()%(NAME_LENGTH_MAX - NAME_LENGTH_MIN)) + NAME_LENGTH_MIN;
	for(int i = 0 ; i < longueur; ++i){
		int indice; 
		if(i%2 == 0){
			indice = rand() %14;
			t[i] = consone[indice];
		}	
		else{
			indice = rand() %5;
			t[i] = voyelle[indice];
		}
	}	
	t[longueur] = '\0';
}

//initialisation aléatoire du numero de téléphone
void directory_random_num(char *t){
	for(int i = 0 ; i < TELEPHONE_LENGTH ; ++i){
		t[i] = (rand()%9) + '0';
	}
	t[TELEPHONE_LENGTH] = '\0';
}

//fonction principale d'initialisation aléatoire d'un individu
void directory_data_random(struct directory_data *data){	
	directory_random_name(data->first_name);
	directory_random_name(data->last_name);
	directory_random_num(data->telephone);	
}

//initialisation d'un répertoire vide
void directory_create(struct directory *self){
	self->data = NULL;
	self->size = 0;
	self->capacity = 0;
}

//destruction du répertoire et ses entrées
void directory_destroy(struct directory *self){
	
	for(size_t i = 0 ; i < self->size ; ++i){
		struct directory_data *temp = self->data[i];
		free(temp);
	}

	self->capacity = 0;
	self->size = 0;
	free(self->data);
}

//Ajout d'un élément dans le répertoire
void directory_add(struct directory *self, struct directory_data *data){
	if(self->capacity == 0){
		self->capacity = 1;
		self->data = malloc(sizeof(struct directory_data));
	}
	else if (self->capacity == self->size){
		self->capacity = self->capacity *2;
		self->data = realloc(self->data, self->capacity * sizeof(struct directory_data));
	}
	self->data[self->size] = data;
	++self->size;
}

//Ajout de n éléments dans le répertoire
void directory_random(struct directory *self, size_t n){
	for(size_t i = 0 ; i < n ; ++i){
		struct directory_data *new = malloc(sizeof(struct directory_data));
		directory_data_random(new);
		directory_add(self, new);
		//directory_data_print(new);
	}
}

//Calcul la longueur d'une chaine de charactere
size_t char_size(const char *self){
	size_t size = 0;
	while(self[size] != '\0'){
		++size;
	}
	return size;
}

//Analyse si deux tableaux de charactères sont identiques
bool char_equals(const char *self, const char *last_name){
	size_t size = char_size(self);
	if(size != char_size(last_name)){
		return false;
	}
	else{
		for(size_t i = 0 ; i < size ; ++i){
			if(self[i] != last_name[i]){
				return false;
			}
		}
	}
  return true;
}

//recherche et affichage d'une entrée par Last_name
void directory_search(const struct directory *self, const char *last_name){
	for(size_t i = 0 ; i < self->size ; ++i){
		if(char_equals(self->data[i]->last_name, last_name))
			directory_data_print(self->data[i]);
	}
	printf("-----Fin de recherche-----\n");
}

//echange la place de deux entrées dans le répertoire
void directory_swap(struct directory *self, size_t i, size_t j) {
	struct directory_data *temp;
	temp = self->data[i];
	self->data[i] = self->data[j];
	self->data[j] = temp;
}

//détermine l'ordre alphabétique de deux chaines de charactères
bool name_inf(const char *self, const char *pivot){
	size_t size = MAX(char_size(self),  char_size(pivot));
	for(size_t i = 0 ; i < size; ++i){
		if((int)self[i] < (int)pivot[i]){
			return true;
		}
		else if((int)self[i] > (int)pivot[i])
			return false;
	}
	return true;
}

//partition du répertoire pour la fonction de tri Quick sort
ssize_t directory_partition(struct directory *self, ssize_t i, ssize_t j) {
	ssize_t pivot_index = i;
	const char *pivot = self->data[pivot_index]->last_name;
	directory_swap(self, pivot_index, j);
	ssize_t p = i;
	for(ssize_t k = i; k < j; ++k) {
		if(name_inf(self->data[k]->last_name, pivot)) {
		directory_swap(self, k, p);
		p++;
		}
	}
	directory_swap(self, p, j);
	return p;
}

//tri par quick_sort du répertoire
void directory_qs(struct directory *self, ssize_t i, ssize_t j){
	if (i < j) {
		ssize_t p = directory_partition(self, i, j);
		directory_qs(self, i, p - 1);
		directory_qs(self, p + 1, j);
	}
}

//tri des entrées en fonction du Last_name
void directory_sort(struct directory *self){
	directory_qs(self, 0, (self->size-1));
}

//algorithme de recherche dichotomique
void dicho_search(const struct directory *self, size_t debut, size_t fin, const char *last_name){
	if(fin < debut)
		return;
	size_t i = (fin + debut)/2;
	if(char_equals(self->data[i]->last_name, last_name)){
		directory_data_print(self->data[i]);
		dicho_search(self, debut, i -1, last_name);
		dicho_search(self, i +1, fin, last_name);
	}
	else if(name_inf(self->data[i]->last_name, last_name)!= true){
		dicho_search(self, debut, i -1, last_name);
	}
	else if(name_inf(self->data[i]->last_name, last_name)){
		dicho_search(self, i +1, fin, last_name);
	}
}

//recherche d'un nom dans un répertoire trié de manière optimale
void directory_search_opt(const struct directory *self, const char *last_name){
	assert(self->data != NULL);
	assert(char_size(last_name) > 0);
	dicho_search(self, 0, (self->size-1), last_name);
	printf("-----Fin de recherche-----\n");
}



//---------------------Definition des Fonctions traitant des index ---------------------------//



//Ajout d'un élément à une liste chainée et renvoie de la nouvelle liste.
struct index_bucket *index_bucket_add(struct index_bucket *self, const struct directory_data *data){
	struct index_bucket *new = malloc(sizeof(struct index_bucket));
	new->data = data;
	new->next = self;
	return new;
}

//Destruction d'une liste chaînée
void index_bucket_destroy(struct index_bucket *self){
	
	while(self != NULL){
		struct index_bucket *temp = self;
		self = self->next;
		free(temp);
	}
}

//calcul d'un hash FNV 32bits
size_t fnv_hash(const char *key){
	size_t hash = OFFSET_BASIS;
	size_t i = 0;
	while(key[i] != '\0'){
		hash = hash * FNV_PRIME_32;
		hash ^= key[i];
		++i;
	}	
	return hash;
}

//Hash sur le prénom
size_t index_first_name_hash(const struct directory_data *data){
	return fnv_hash(data->first_name);
}

//hash sur le numéro de téléphone
size_t index_telephone_hash(const struct directory_data * data){
	return fnv_hash(data->telephone);
}

//Initialisation d'un index vide et initialisation de la fonction de hachage
void index_create(struct index *self, index_hash_func_t funct){
	self->buckets = NULL;
	self->size = 1;
	self->buckets = malloc(sizeof(struct index_bucket));
	self->buckets[0] = NULL;
	self->count = 0;
	self->funct = funct;
}

//destruction d'un index
void index_delete(struct index *self){
	for(size_t i = 0; i < self->size; ++i){
		index_bucket_destroy(self->buckets[i]);
	}
	self->size = 0;
	self->count = 0;
	free(self->buckets);
}

//Rehash d'un index
void index_rehash(struct index *self){
	//on double la taille du tableau
	self->size *= 2;
	struct index_bucket **index_temp = calloc(self->size, sizeof(struct index_bucket));

	//on recalcule tous les éléments déja présents
	for(size_t i = 0; i < self->size/2 ; ++i){	
		while(self->buckets[i] != NULL){
			struct index_bucket *temp = self->buckets[i];
			self->buckets[i] = temp->next;
			size_t hash_code = self->funct(temp->data) % self->size;
			index_temp[hash_code] = index_bucket_add(index_temp[hash_code], temp->data);
			free(temp);
		
		}
	}
	free(self->buckets);
	self->buckets = index_temp;
}

//ajout d'un élément dans l'index
void index_add(struct index *self, const struct directory_data *data){

	//si le risque de collision est impotant, on rehash
	if((self->count+1) / self->size >= 0.5){
		index_rehash(self);
	}

	//On ajoute l'élément dans la liste correspondant
	size_t hash_code = self->funct(data) % self->size;
	self->buckets[hash_code] = index_bucket_add(self->buckets[hash_code], data);
	self->count += 1;

}

//remplissage d'un index avec répertoire
void index_fill_with_directory(struct index *self, const struct directory *dir){
	for(size_t i = 0; i < dir->size; ++i){
		index_add(self, dir->data[i]);
	}
}

//Recherche dans l'index par prénom
void index_search_by_first_name(const struct index *self, const char *first_name){
	size_t idx = fnv_hash(first_name) % self->size;
	struct index_bucket *temp = self->buckets[idx];
	while(temp != NULL){
		if(char_equals(temp->data->first_name, first_name)){
			directory_data_print(temp->data);
			
		}
		temp = temp->next;
	}
	printf("-----Fin de recherche-----\n");
}

void index_search_by_telephone(const struct index *self, const char *telephone){
	size_t idx = fnv_hash(telephone) % self->size;
	struct index_bucket *temp = self->buckets[idx];
	if(temp == NULL)
	while(temp != NULL){
		if(char_equals(temp->data->telephone, telephone)){
			directory_data_print(temp->data);
		}
		temp = temp->next;
	}
	printf("-----Fin de recherche-----\n");
}



//---------------------Fonctions relatif à l'interactivité de l'application-------------------------//


//fonction supprimant le charactère de fin de ligne
void clean_newline(char *buf, size_t size){
	for(size_t i = 0; i < size; ++i){
		if(buf[i] == '\n'){
			buf[i] = '\0';
			return;
		}
	}
	buf[size] = '\0';
	//vide le buffer pour éviter les erreurs de rétention. Seul le premier charactere sera prit en compte
	int c = 0;
	while(c != EOF && c != '\n'){
		c= getchar();
	}
}

//Fonction de Menu
void menu(const struct directory *rep, const struct index *index_name, const struct index *index_tel){

	//definition des indices de temps pour le calcul des temps d'execution
	struct timeval pre_time, post_time;

	//boucle de saisie
	char buf[BUFSIZE];
	for(;;){

		//impression du menu
		printf("WHAT DO YOU WANT TO DO ?\n");
		printf("\t1: Search by last name(non optimized)\n");
		printf("\t2: Search by last name (optimized)\n");
		printf("\t3: Search by first name (indexed)\n");
		printf("\t4: Search by telephpne (indexed)\n");
		printf("\tq : Quit\n");
		printf("YOUR CHOICE :");

	
		printf("\n> ");
		fgets(buf, BUFSIZE,stdin);
		clean_newline(buf, BUFSIZE);

		if(char_equals(buf, "q"))
			return;
		else if(char_equals(buf, "1")){
			gettimeofday(&pre_time, NULL);
			char buf_name[NAME_LENGTH_MAX];
			printf("Entrer un nom de longueur max %d charactères\n-", NAME_LENGTH_MAX);
			fgets(buf_name, NAME_LENGTH_MAX,stdin);
			clean_newline(buf_name, NAME_LENGTH_MAX);
			directory_search(rep, buf_name);
			gettimeofday(&post_time, NULL);
			printf("Recherche non optimisé réalisé en %ld mili secondes\n\n", (post_time.tv_sec - pre_time.tv_sec) * 1000);
		}
		else if(char_equals(buf, "2")){
			gettimeofday(&pre_time, NULL);
			char buf_name[NAME_LENGTH_MAX];
			printf("Entrer un nom de longueur max %d charactères\n-", NAME_LENGTH_MAX);
			fgets(buf_name, NAME_LENGTH_MAX,stdin);
			clean_newline(buf_name, NAME_LENGTH_MAX);
			directory_search_opt(rep, buf_name);
			gettimeofday(&post_time, NULL);
			printf("Recherche optimisé réalisé en %ld mili secondes\n\n", (post_time.tv_sec - pre_time.tv_sec) * 1000);
		}
		else if(char_equals(buf, "3")){
			gettimeofday(&pre_time, NULL);
			char buf_name[NAME_LENGTH_MAX];
			printf("Entrer un prénom de longueur max %d charactères\n-", NAME_LENGTH_MAX);
			fgets(buf_name, NAME_LENGTH_MAX,stdin);
			clean_newline(buf_name, NAME_LENGTH_MAX);
			index_search_by_first_name(index_name, buf_name);
			gettimeofday(&post_time, NULL);
			printf("Recherche indexé par prénom réalisé en %ld mili secondes\n\n", ((post_time.tv_sec - pre_time.tv_sec) * 1000 + (post_time.tv_usec - pre_time.tv_usec) / 1000));
		}
		else if(char_equals(buf, "4")){
			gettimeofday(&pre_time, NULL);
			char buf_name[NAME_LENGTH_MAX];
			printf("Entrer un numéro de longueur %d charactères\n-", TELEPHONE_LENGTH);
			fgets(buf_name, NAME_LENGTH_MAX,stdin);
			clean_newline(buf_name, NAME_LENGTH_MAX);
			index_search_by_telephone(index_tel, buf_name);
			gettimeofday(&post_time, NULL);
			printf("Recherche indexé par numero réalisé en %ld mili secondes\n\n", ((post_time.tv_sec - pre_time.tv_sec) * 1000 + (post_time.tv_usec - pre_time.tv_usec) / 1000));
		
		}

	}
}


//----------------------------------Fonction Main-----------------------------------//

int main(int argc, char *argv[]){

	//changement de graine de la fonction rand
	//srand(time(NULL));

	/*
	Nb d'entrées à ajouter aurépertoire récuperé en paramètre. 
	Si trop ou aucun paramètres ne sont donnés, utilisation d'une valeur pas défault
	*/
	size_t n = 100000;
	if(argc == 2)
		n = atoi(argv[1]);
	else if(argc != 2)
		printf("Erreur de paramètres, utilisation d'une valeur par défault\n\n");

	//initialisation d'un nouveau répertoire
	struct directory repertoire;
	directory_create(&repertoire);

	//ajout de n éléments aléatoires dans le répertoires et tri du répertoire
	directory_random(&repertoire, n);
	directory_sort(&repertoire);
	
	//Initialisation d'un index sur les prénoms et remplissage avec le répertoire
	struct index index_name;
	index_create(&index_name, &index_first_name_hash);
	index_fill_with_directory(&index_name, &repertoire);
	
	//Initialisation d'un index sur les numéros de telephone et remplissage avec le répertoire
	struct index index_tel;
	index_create(&index_tel, &index_telephone_hash);
	index_fill_with_directory(&index_tel, &repertoire);
	
	//boucle principale de l'application
	menu(&repertoire, &index_name, &index_tel);
	printf("\n----Merci et à Bientôt :)----\n");
	
	//effacement du répertoire et liberation de la mémoire allouée dynamiquement
	index_delete(&index_name);
	index_delete(&index_tel);
	directory_destroy(&repertoire);
	return 0;
}
