//EXECUTION STEPS : 

	//gcc comb_new_block_newer.c -o comb_new_block_newer `pkg-config fuse --cflags --libs`
 	//mkdir mount
	//./comb_new_block_newer -f mount


#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>

#define SIZE 100
#define STR_SIZE 100
#define BLK_STR_SIZE 4
#define BLK_NO 10
#define BLK_SIZE 10


typedef struct file_content
{
	char *file_name;
	char **file_content;
	int block_index;
	int no_block;
	int inode;
}file_content;

typedef struct file_table
{
	file_content **table;
	int index;
	int size;
	int dirty_bit;	
}file_table;

typedef struct directory
{
	char *directory_name;
	int inode;
}directory;

typedef struct directory_table
{
	directory **table;
	int index;
	int size;
	int dirty_bit;
}directory_table;

file_table *files;
directory_table *directories;
int inode_number;
int file_open_mode; 




file_content *init_file()
{
	file_content *temp = (file_content *)malloc(sizeof(file_content));
	temp -> file_name = (char *)malloc(STR_SIZE * sizeof(char));
	temp -> file_content = (char **)malloc(BLK_NO * sizeof(char *));
	int i = 0;
	for(i = 0;i < BLK_NO;i++)
	{
		temp -> file_content[i] = (char *)malloc(STR_SIZE * sizeof(char));
	}
	temp -> no_block = BLK_NO;
	temp -> block_index = 0;
	return temp;
}

file_table *init_table()
{
	int i = 0;
	file_table *temp = (file_table *)malloc(sizeof(file_table));
	temp -> table = (file_content **)malloc(SIZE * sizeof(file_content *));
	temp -> index = 0;
	temp -> size = SIZE;
	for(i = 0;i < SIZE;i++)
	{
		temp -> table[i] = init_file();
	}
	temp -> dirty_bit = 0;
	return temp;
}

directory *init_directory()
{
	directory *temp = (directory *)malloc(sizeof(directory));
	temp -> directory_name = (char *)malloc(STR_SIZE * sizeof(char));
	//temp -> data = init_table();
	return temp;
}

directory_table *init_directory_table()
{
	directory_table *temp = (directory_table *)malloc(sizeof(directory_table));
	temp -> table = (directory **)malloc(SIZE * sizeof(directory *));
	temp -> index = 0;
	temp -> size = SIZE;
	int i = 0;
	for(i = 0;i < SIZE;i++)
	{
		temp -> table[i] = init_directory();
	}
	temp -> dirty_bit = 0;	
	return temp;	
}


int strcpy_2(char *a, char *b)
{
	int n = strlen(b);
	if(a == NULL)
	{
		a = (char *)malloc(STR_SIZE * sizeof(char));
	}
	printf("my strcopy called for string : %s of len : %d \n", b, n);
	int i = 0;
	for(i = 0;i < n;i++)
	{
		a[i] = b[i];
	}
	a[i] = '\0';
	return 0;
}

int strncpy_2(char *a, char *b, int n)
{
	if(a == NULL)
	{
		a = (char *)malloc(BLK_SIZE * sizeof(char));
	}

	int i = 0;
	for(i = 0;i < n;i++)
	{
		a[i] = b[i];
	}
	a[i] = '\0';	
}

int strncmp_2(const char *a , const char *b , int n)
{
	printf("\nSTRNCMP\n");
	printf("\n%s with %s" , a , b);
	if(a == NULL || b== NULL){
		return -1;
	}
//	printf("\nstrlen of a = %d , strlen of b = %d , int n = %d\n", strlen(a) , strlen(b) , n);
	if(strlen(a) < n || strlen(b) < n)
	{
		printf("\nweird\n");		
		return 1;
	}
	for(int i = 0 ; i < n ; i++)
	{
		printf("\n%c compared with %c\n" , a[i] , b[i]);
		if(a[i] != b[i]){
			return 1;
		}
	}
	return 0;
}

void concatenate(char *a, char *b)
{
	int i = strlen(a);
	int j = strlen(b);
	int index = 0;
	for(index = 0;index < j;index++)
	{
		a[i + index] = b[index];
	}
	a[i + index] = '\0';
}




char *block_copy_read(char **a, int index)
{

	char *t = *a;
	int d = files->table[files->index]->no_block;
	char *s = (char *)malloc(STR_SIZE * d * sizeof(char));
	s[0] = '\0';
	printf("number of blocks : %d\n",d);
	printf("blocknumber   block_content\n");
	int i=0;
	for(i=0;i<d;i++)
	{
		printf("%d   %s\n",(i+1),files -> table[index] -> file_content[i]);
		concatenate(s, files -> table[index] -> file_content[i]);
	}
	return s;
}


int block_copy_write( char *a[] , char *b, int index)
{
	int n = strlen(b);
	//int block_size = 4;
	int i;
	int d = n/BLK_STR_SIZE;
	d =d;
	printf("n : %d \nd: %d\n",n,d);
	if(a == NULL)
	{
		a = (char **)malloc((d) * sizeof(char *));
	}
	if(d > files -> table[index] ->  no_block)
	{
		int size = 2 * files -> table[index] ->  no_block;		
		files -> table[index] -> file_content = (char **)realloc(files -> table[index] -> file_content, size * sizeof(char *));
		for(i = (size / 2);i < size;i++)
		{
			files -> table[index] -> file_content[i] = (char *)malloc(d* sizeof(char));
		}
		files -> table[index] -> no_block = size;
		//will get an initial no_block of 10 for files.
	}
	d=d+1;
	for(i=0;i<d;i++)
	{
		if(a[i]==NULL)
		{
			//printf("it be null !!!!!!!!");
			a[i] = (char *)malloc(BLK_STR_SIZE * sizeof(char));
			//printf("malloc'd\n");
		}

	}

	printf("INITIATING COPY !");
	
	for(i=0;i<d;i++)
	{
		printf("block %d being copied \n-----------------\n",(i+1));
		printf("copying b :%s\n-----------------\n",b);
		printf("b4 copied a :%s\n-----------------\n",a[i]);
		strncpy_2(a[i],b,BLK_STR_SIZE);
		printf("copied a :%s\n-----------------\n",a[i]);
		b=b+BLK_STR_SIZE;
		
	}
	files -> table[index] -> block_index = d - 1; 

	
}

int file_belong_here(char *a)
{
//	int n = strlen(a);
	int i = 0;
	printf("file belong here check : %s\n", a);
	while(a[i] != '\0')
	{
		if(a[i] == '/')
		{
			return 0;
		}
		i++;
	}
	if(i!= 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void print_file_table()
{
	int i = 0;
	printf("FILE TABLE PRINTING :- \n\n\n\n");
	for(i = 0;i < files -> index;i++)
	{
		printf("%s : %s\n", files -> table[i] -> file_name, files -> table[i] -> file_content[0]);
	}
	return;
}


void insert_file(file_table *files, char *filename, char *content)
{
	printf("\nfilename :%s\ncontent:%s\n", filename, content);
	if(files -> index >= files -> size)
	{
		printf("HAVE TO INCREASE SIZE OF FILE TABLE\n");
		int size = 2 * files -> size; 
		files -> table = (file_content **)realloc(files, size * sizeof(file_content *)); 
		files -> size = size;
	}
	//block_copy(files -> table[files->index] -> file_name, filename);
	strcpy_2(files -> table[files->index] -> file_name, filename);

//	strcpy_2(files -> table[files->index] -> file_content, content);
	block_copy_write(files->table[files->index]->file_content , content,  files -> index);
	files -> table[files -> index] -> inode = inode_number;
	printf("\ninsert_file done : %d : %s : %s\n", files->index, files -> table[files->index] -> file_name, files -> table[files->index] -> file_content[0]);
	files -> index++;
	files -> dirty_bit = 1;
	inode_number++;
	print_file_table();
}

void insert_directory(directory_table *temp, char *directory_name)
{
	if(temp -> index >= temp -> size)
	{
		printf("HAVE TO INCREASE SIZE OF DIRECTORY TABLE\n");
		int size = 2 * temp -> size; 
		temp -> table = (directory **)realloc(temp, size * sizeof(directory *)); 
		temp -> size = size;
	}
//	directory_name = directory_name + 1;
	printf("directory_name : %s\n", directory_name);
	strcpy_2(temp -> table[temp -> index] -> directory_name, directory_name);
	temp -> table[temp -> index] -> inode = inode_number;
	printf("insert done of directory : %s\n", temp -> table[temp -> index] -> directory_name);
	temp -> index++;
	inode_number++;
	temp -> dirty_bit = 1;
}

int process_file_table_backup(FILE *f)
{
	char *s = (char *)malloc(500 * sizeof(char));
	char a;
	int count = 0;
	while(a != EOF)
	{
		int index = 0;
		char *file_content = (char *)malloc(500 * sizeof(char));
		file_content[0] = '\0';
		fscanf(f, "%s :", s);
		while(count < 3)
		{
			a = fgetc(f);
			if(a == EOF)
			{
				break;
			}
			if(a == '\n')
			{
				count++;
				if(count >= 3)
				{
					file_content[index - 2] = '\0';
				}
				else
				{
					file_content[index] = a;
					index++;
				}
//				printf("%c", a);
//				printf("count incremented : %d  \n", count);
			}
			else
			{
				count = 0;
				//printf("%c", a);
				file_content[index] = a;
				index++;
			}
		}
		count = 0;
		if(index > 0)
		{
			file_content++;
			printf("file name : %s\n", s);
			printf("file_content : %s\n", file_content);
			insert_file(files, s, file_content);
		}
		//fscanf("")
	}
	return 0;
}

int process_directory_table_backup(FILE *f)
{
	char a;
	a = fgetc(f);
	char *name = (char *)malloc(500 * sizeof(char));
	int index = 0;
	while(a != EOF)
	{
		if(a == '\n')
		{
			name[index] = '\0';
			insert_directory(directories, name);
			index = 0;
		}
		else
		{
			name[index] = a;
			index++;
		}
		a = fgetc(f);
	}
}

void delete(file_table *files, int index)
{
	int i = index;
	for(i = index;i < files -> index;i++)
	{
		//files -> table[i] = files -> table[i + 1];
		strcpy_2(files -> table[i] -> file_name, files -> table[i + 1] -> file_name);
		//files -> table[i] -> file_content = files -> table[i + 1] -> file_content;
		char *temp = block_copy_read(files -> table[i + 1] -> file_content, i + 1);
		printf("replacing content : %s at index : %d\n", temp, i);
		block_copy_write(files -> table[i] -> file_content, temp, i);
		files -> table[i] -> block_index = files -> table[i + 1] -> block_index;
		files -> table[i] -> no_block = files -> table[i + 1] -> no_block;
		files -> table[i] -> inode = files -> table[i + 1] -> inode;
	}
	files -> index--;
	print_file_table();
	files -> dirty_bit = 1;
}  

void write_file_table()
{
	int i = 0;
	char *final_filename = (char *)malloc(STR_SIZE * sizeof(char));
	strcpy_2(final_filename, "./backup/file_table.txt");
	FILE *f = fopen(final_filename, "w");
	printf("\nfile table files : %s\n", final_filename);
	for(i = 0;i < files -> index;i++)
	{
		fprintf(f, "%s :\n", files -> table[i] -> file_name);
		char *a = block_copy_read(files -> table[i] -> file_content, i);
		fprintf(f, "%s\n\n\n", a);	
	}
	fclose(f);
}

void write_directory_table()
{
	int i = 0;
	char *final_filename = (char *)malloc(STR_SIZE * sizeof(char));
	final_filename[0] = '\0';
	strcpy_2(final_filename, "backup/directory_table.txt");
	FILE *f = fopen(final_filename, "w");
	printf("\ndirectory table files : %s\n", final_filename);
	for(i = 0;i < directories -> index;i++)
	{
		fprintf(f, "%s\n", directories -> table[i] -> directory_name);	
		printf("%s\n", directories -> table[i] -> directory_name);
	}
	printf("\n\n\n\n");
	fclose(f);
}

 
void backup()
{
//	write_files();
	if(files -> dirty_bit == 1)
	{
		write_file_table();
	}
	if(directories -> dirty_bit == 1)
	{
		write_directory_table();
		printf("directory BACKUP \n\n\n");
	}
	files -> dirty_bit = 0;
	directories -> dirty_bit = 0;
}


 
static int do_getattr( const char *path, struct stat *st )
{
	printf( "[getattr] Called\n" );
	printf( "\tAttributes of %s requested\n", path );
	
	// GNU's definitions of the attributes (http://www.gnu.org/software/libc/manual/html_node/Attribute-Meanings.html):
	// 		st_uid: 	The user ID of the file’s owner.
	//		st_gid: 	The group ID of the file.
	//		st_atime: 	This is the last access time for the file.
	//		st_mtime: 	This is the time of the last modification to the contents of the file.
	//		st_mode: 	Specifies the mode of the file. This includes file type information (see Testing File Type) and the file permission bits (see Permission Bits).
	//		st_nlink: 	The number of hard links to the file. This count keeps track of how many directories have entries for this file. If the count is ever decremented to zero, then the file itself is discarded as soon 
	//						as no process still holds it open. Symbolic links are not counted in the total.
	//		st_size:	This specifies the size of a regular file in bytes. For files that are really devices this field isn’t usually meaningful. For symbolic links this specifies the length of the file name the link refers to.
	
	st->st_uid = getuid();  // The owner of the file/directory is the user who mounted the filesystem
	st->st_gid = getgid();  // The group of the file/directory is the same as the group of the user who mounted the filesystem
						    //st->st_atime = time( NULL ); // The last "a"ccess of the file/directory is right now
							//st->st_mtime = time( NULL ); // The last "m"odification of the file/directory is right now
	
	char *name = (char *)malloc(STR_SIZE * sizeof(char));
	char *file = (char *)malloc(STR_SIZE * sizeof(char));
	strcpy_2(name, (char *)path);
	strcpy_2(file, (char *)path);
	file = file + 1;
	int i = 0;
	printf("do_getattr : name : %s\n", name);
	for(i = 0;i < directories -> index;i++)
	{
		printf("directory comparision of %s with %s\n\n", name, directories -> table[i] -> directory_name);
		if( strcmp(name, directories -> table[i] -> directory_name ) == 0)
		{
			printf("directory found : %s\n", directories -> table[i] -> directory_name);
			st->st_mode = S_IFDIR | 0777;
			st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
			st->st_size = 8;
			return 0;
		}
	}
	for(i = 0;i < files -> index;i++)
	{
		printf("file comparision of %s with %s\n\n", file, files -> table[i] -> file_name);
		if( strcmp(file, files -> table[i] -> file_name) == 0)
		{
			printf("file found : %s\n", files -> table[i] -> file_name);
			printf("block index for the file found : %d\n\n", files -> table[i] -> block_index);
			printf("block size : %d\n\n", BLK_STR_SIZE);
			st->st_mode = S_IFREG | 0777;
			st->st_nlink = 1;
			st->st_size = (files -> table[i] -> block_index + 1) * BLK_STR_SIZE;
			st->st_blksize = BLK_STR_SIZE;
			st->st_blocks = files -> table[i] -> block_index + 1;
			return 0;
		}
	}
	if(strcmp(path, "") == 0)
	{
		return 0;
	}
	return -ENOENT;
}

static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
	printf( "--> Getting The List of Files of %s\n", path );
	filler( buffer, ".", NULL, 0 ); // Current Directory
	filler( buffer, "..", NULL, 0 ); // Parent Directory
	char *temp = (char *)malloc(STR_SIZE * sizeof(char));
	strcpy_2(temp, (char *)path);
	temp = temp + 1;
	int n = strlen(temp);
	printf("\nDO READ DIR : path : %s && temp : %s\n", path, temp);
	int i = 0;	
	if(strcmp(path, "/") == 0)
	{
		for(i = 0;i < files -> index;i++)
		{
			printf("\nfiller : %s\n", files -> table[i] -> file_name);
			if(file_belong_here(files -> table[i] -> file_name))
			{
				filler( buffer, files -> table[i] -> file_name, NULL, 0 );
			}
		}
		for(i = 1;i < directories -> index;i++)
		{
			printf("filler : directory : %s\n", directories -> table[i] -> directory_name + 1);
			if(file_belong_here(directories -> table[i] -> directory_name + 1))
			{
				filler( buffer, directories -> table[i] -> directory_name + 1, NULL, 0 );
			}
		}
	}
	else
	{
		char *directory_name = (char *)malloc(STR_SIZE * sizeof(char));
		for(i = 0;i < files -> index;i++)
		{
			printf("INITIAL file_name : %s\n", files -> table[i] -> file_name);
			int flag = strncpy_2(directory_name, files -> table[i] -> file_name, n);
			printf("FINAL directory_name : %s\n", directory_name);
			printf("COMPARING WITH : %s\n", temp);
			printf("WHILE PATH IS : %s\n", path);
			if(flag != -1 && (strcmp(directory_name, temp) == 0))
			{
				if(file_belong_here(files -> table[i] -> file_name + n + 1))
				{	
					printf("\nMATCH FILE FOUND : filler : %s : %s\n", files -> table[i] -> file_name + n + 1, files -> table[i] -> file_content[0]);
					filler( buffer, files -> table[i] -> file_name + n + 1, NULL, 0 );
				}
			}

		}
		directory_name = (char *)malloc(STR_SIZE * sizeof(char));
		for(i = 1;i < directories -> index;i++)
		{
			printf("INITIAL directory_name : %s\n", directories -> table[i] -> directory_name);
			int flag = strncpy_2(directory_name, directories -> table[i] -> directory_name, n + 1);
			printf("FINAL directory_name : %s\n", directory_name);
			printf("COMPARING WITH : %s\n", path);
			printf("WHILE TEMP IS : %s\n", temp);
			if(flag != -1 && (strcmp(directory_name, path) == 0))
			{
				if(file_belong_here(directories -> table[i] -> directory_name + n + 2))
				{
					printf("MATCH FOLDER FOUND : filler : directory : %s\n", directories -> table[i] -> directory_name + n);
					filler( buffer, directories -> table[i] -> directory_name + n + 2, NULL, 0);
				}
			}
		}
	}
	return 0;
}


	
static int do_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
	path = path + 1;
	//printf("hereeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee!");
	printf( "--> Trying to read %s, %lu, %lu\n", path, offset, size );
	char selectedText[100];
	int i = 0;	
	for(i = 0;i < files -> index;i++)
	{
		char *a = (char *)malloc((SIZE + 1) * sizeof(char));
		a = files -> table[i] -> file_name;
		printf("\nREAD : path : %s : a : %s\n", path, a);
		if(strcmp(path, a) == 0)
		{
			printf("MATCH FOUND\n");
			strcpy_2(selectedText, block_copy_read(files -> table[i] -> file_content, i));
			//strcpy_2(selectedText, files -> table[i] -> file_content);
			break;
		}
	}
	if(selectedText == NULL)
	{ 
		return -1;
	}

	//printf("selectedText : %s\n", selectedText);
	memcpy( buffer, selectedText + offset, size );
	printf("memcpy done\n");	
	return strlen( selectedText ) - offset;
}

static int do_truncate(const char *path, off_t size)
{
	return size;
}

static int do_mkdir(const char* path, mode_t mode)
{
	//path = path + 1;
	printf("\nmake directory was called with name : %s\n", path);
	insert_directory(directories, (char *)path);
	return 0;
}


static int do_write(const char *path, const char *buffer, size_t size, off_t offset,struct fuse_file_info *fi)
{
	path = path + 1;
	char selectedText[100];
	printf("write : path : %s : new buffer : %s\n", path, selectedText);
	strcpy_2(selectedText, (char *)buffer);
	int i = 0;	
	for(i = 0;i < files -> index;i++)
	{
		char *a = (char *)malloc((STR_SIZE + 1) * sizeof(char));
		//strcat(a, "/");
		strcpy_2(a, files -> table[i] -> file_name);
		printf("WRITE : path : %s && a : %s\n\n", path, a);
		if(strcmp(path, a) == 0)
		{
			//delete(files, i);
			//strcpy_2(files -> table[i] -> file_content, selectedText);
			//printf("\ndeleted old data\n");
			if(file_open_mode == 1)
			{
				block_copy_write(files -> table[i] -> file_content, selectedText, i);
			}
			else if(file_open_mode == 2)
			{
				char *temp = block_copy_read(files -> table[i] -> file_content, i);
				printf("\n\n\n\nold copy of temp : %s\n\n\n\n\n", temp);
				concatenate(temp, selectedText);
				printf("\n\n\n\nnew copy of temp : %s\n\n\n\n\n", temp);
				block_copy_write(files -> table[i] -> file_content, temp, i);
			}
			else
			{
				delete(files, i);
				insert_file(files, (char *)path, selectedText);
			}
			break;
		}
	}
	files -> dirty_bit = 1;
	return size;
}

static int do_open(const char *path, struct fuse_file_info *info)
{
	path = path + 1;
	printf( "--> Trying to open %s\n", path);
	char selectedText[100];
	int i = 0;	
	for(i = 0;i < files -> index;i++)
	{
		char *a = (char *)malloc((SIZE + 1) * sizeof(char));
		a = files -> table[i] -> file_name;
//		concatenate(a, files -> table[i] -> file_name);
		printf("\npath : %s : a : %s\n", path, a);
		if(strcmp(path, a) == 0)
		{
//			strcpy_2(selectedText, files -> table[i] -> file_content);
			break;
		}
	}
	if(i == files -> index)
	{ 
		return -ENOENT;
	}

	if ((info->flags & O_ACCMODE) == O_WRONLY || (info->flags & O_ACCMODE) == O_RDWR)
	{
		printf("\n\n\n\n\nFILE READ IN WRITE MODE\n\n\n\n\n\n");
		file_open_mode = 1;
	}
	if(info -> flags & O_APPEND)
	{
		printf("\n\n\n\n\nFILE READ IN APPEND MODE\n\n\n\n\n\n");
		file_open_mode = 2;
	}
	else
	{
		printf("\n\n\n\n\nFILE READ IN OTHER MODE\n\n\n\n\n\n");
		file_open_mode = 3;
	}
	return 0;
}

static int do_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	path = path + 1;
	printf("\n\ncreate file called for path : %s\n\n", path);
	insert_file(files, (char *)path, (char *)"");
	return 0;
}

static int do_remove(const char* path)
{
	path = path + 1;
	int i = 0;
	for(i = 0;i < files -> index;i++)
	{
		if(strcmp(path, files -> table[i] -> file_name) == 0)
		{
			break;
		}
	}
	if(i < files -> index)
	{
		delete(files, i);
	}
	else
	{
		return -1;
	}
	return 0;
}

static int do_release(const char *path, struct fuse_file_info *fi)
{
	printf("\n\n\n\nRELEASE FOR FILES IS CALLED AT PATH  : %s\n\n\n\n", path);
	backup();
	return 0;
}

static int do_releasedir(const char *path, struct fuse_file_info *fi)
{
	printf("\n\n\n\nRELEASE FOR DIRECTORY IS CALLED AT PATH  : %s\n\n\n\n\n", path);
	backup();
	return 0;
}


void copy_dir_entry(directory *a , directory *b)
{
	strcpy_2(a->directory_name , b->directory_name);
	a->inode = b->inode;
}

void remove_dir(const char*path , int index)
{
	int i = index;
	for(i = index;i < directories -> index;i++)
	{
		copy_dir_entry(directories -> table[i] , directories -> table[i + 1]);
	}
	directories -> index--;	
}


int do_rmdir(const char *path){
	int empty = 1;
	int n = strlen(path);
	int index = 0;
	for (int i = 0; i < files->index; ++i)
	{
		char *temp = (char *)malloc(100*sizeof(char));
		strcpy_2(temp , (char *)path);
		temp += 1;
		printf("\n********** temp : %s*********\n" , temp);
		if(strncmp_2(files->table[i]->file_name , temp , n-1) == 0)
		{
			printf("\ndirectory not empty.\n");
			return 0;
		}
	}
	for (int i = 0; i < directories->index; ++i)
	{
		if(strncmp_2(directories->table[i]->directory_name , path , n) == 0 )
		{
			if(strlen((directories->table)[i]->directory_name) == n){
				index = i;
				continue;
			}
			printf("\ndirectory not empty.\n");
			empty = 0;
			return 0;
		}
	}

	if(empty == 1){
		remove_dir(path , index);
		files -> dirty_bit = 1;
		directories -> dirty_bit = 1;
	}

	return 0;
}

static int do_rename(const char *path, const char *new)
{
	path = path + 1;
	printf("\n\n %s to %s\n\n", path, new);
	int i = 0;
	for(i = 0;i < files -> index;i++)
	{
		if(strcmp(path, files -> table[i] -> file_name) == 0)
		{
			strcpy_2(files -> table[i] -> file_name, (char *)new);
			break;
		}
	}
	if(i == files -> index)
	{
		return -ENOENT;
	}

	return 0;
}

static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir,
    .read		= do_read,
    .write      = do_write,
    .truncate   = do_truncate,
    .mkdir      = do_mkdir,
    .create     = do_create,
    .unlink     = do_remove,
    .open       = do_open,
    .rmdir      = do_rmdir,
    .release    = do_release,
    .releasedir = do_releasedir,
    .rename     = do_rename,


};


int main( int argc, char *argv[] )
{
	files = init_table();
	directories = init_directory_table();
	FILE *f = fopen("backup/file_table.txt", "r");
	if(f != 0)
	{
		process_file_table_backup(f);
	}
	f = fopen("backup/directory_table.txt", "r");
	if(f != 0)
	{
		process_directory_table_backup(f);
	}
//	insert_directory(directories, "/");
	return fuse_main( argc, argv, &operations, NULL );
}
