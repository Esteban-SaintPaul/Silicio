
//-------------------------------------------------
// cabeceras para utilizar funciones de librería

#include <stdio.h>      /* printf()          */
#include <sys/types.h>  /* open() ; lseek()  */
#include <sys/stat.h>   /*   "               */
#include <fcntl.h>      /*   "               */
#include <unistd.h>     /* read() ; lseek()  */
#include <string.h>	/* strcmp()          */

#define SIZE_BOOT	1024
#define EXT2_N_BLOCKS	15
#define EXT2_NAME_LEN	255
#define GROUP_INI 0
#define INODE_ROOT 2

char *error[]={	"ok",
		"La ruta no es absoluta",
		"Error en superblock",
		"Archivo no encontrado"
}; 

typedef struct {
  unsigned long  s_inodes_count;
  unsigned long  s_blocks_count;
  unsigned long  s_r_blocks_count;
  unsigned long  s_free_blocks_count;
  unsigned long  s_free_inodes_count;
  unsigned long  s_first_data_block;
  unsigned long  s_log_block_size;
  long           s_log_frag_size;
  unsigned long  s_blocks_per_group;
  unsigned long  s_frags_per_group;
  unsigned long  s_inodes_per_group;
  unsigned long  s_mtime;
  unsigned long  s_wtime;
  unsigned short s_mnt_count;
  short          s_max_mnt_count;
  unsigned short s_magic;
  unsigned short s_state;
  unsigned short s_errors;
  unsigned short s_pad;
  unsigned long  s_lastcheck;
  unsigned long  s_checkinterval;
  unsigned long  s_creator_so;
  unsigned long  s_rev_level;
  unsigned short s_def_refuid;
  unsigned short s_def_refgid;
  unsigned long  s_first_inode;

  unsigned long  s_reserved[100];
} ext2_super_block;

typedef struct {
  unsigned long  bg_block_bitmap;
  unsigned long  bg_inode_bitmap;
  unsigned long  bg_inode_table;
  unsigned short bg_free_blocks_count;
  unsigned short bg_free_inodes_count;
  unsigned short bg_used_dirs_count;
  unsigned short bg_pad;
  unsigned long  bg_reserved[3];
} ext2_group_desc;

typedef struct {
  unsigned short i_mode;
  unsigned short i_uid;
  unsigned long  i_size;
  unsigned long  i_atime;
  unsigned long  i_ctime;
  unsigned long  i_mtime;
  unsigned long  i_dtime;
  unsigned short i_gid;
  unsigned short i_links_count;
  unsigned long  i_blocks;
  unsigned long  i_flags;
  unsigned long  i_reserved1;
  unsigned long  i_block[EXT2_N_BLOCKS];
  unsigned long  i_version;
  unsigned long  i_file_acl;
  unsigned long  i_dir_acl;
  unsigned long  i_faddr;
  unsigned char  i_frag;
  unsigned char  i_fsize;
  unsigned short i_pad1;
  unsigned long  i_reserved2[2];
} ext2_inode;

typedef struct  {
  unsigned long  inode;
  unsigned short rec_len;
  unsigned char name_len;
  unsigned char file_type;
  char           name[EXT2_NAME_LEN];
} ext2_dir_entry;

typedef struct {
	ext2_dir_entry de;
	ext2_inode ino;
}file_desc;

typedef char ext2_logic[4096];
unsigned long BLOCK_SIZE;


unsigned long block_size(unsigned long block); // determina el tamaño del bloque lògico
unsigned long block_jump(int arch, unsigned long size_block, unsigned long block);// lseek largo
unsigned long superblock(int arch, ext2_super_block *sb);//carga el superbloque
unsigned long group(int arch, ext2_group_desc *bg, unsigned long n_group);
unsigned long inode(int arch, ext2_inode *ino, unsigned long n_inode, ext2_group_desc *bg );
unsigned long block_logico(int arch, ext2_logic *lo,unsigned long n_logic);
unsigned long string2dir(ext2_dir_entry *de, char *file,ext2_dir_entry *de_file, unsigned long size);//
unsigned long string2file_desc(int arch, char *file,unsigned long inode_dir, file_desc *desc);

//--------------------------------------------------------------------
// Comienzo de programa

int main(int argc, char *argv[], char env[])
{
  int arch;
  file_desc desc;
  unsigned long i;
  char file[EXT2_NAME_LEN];

  if( argc != 3 ){
    printf("Error de parámetro :\n");
    printf("Use : \"%s <imagen_de_partición>  <fichero_buscado>\"\n",argv[0]);
    return 0;
  }

  arch=open(argv[1],O_RDONLY); // Abro el archivo
  if( arch <= 0 ){             // salgo si no lo puedo Abrir
    printf("Error en apertura\n");
    printf("%s no puede ser abierto\n",argv[1]);
    return 0;
  }
  
  for(i=0; argv[2][i+1]!= '/' && argv[2][i+1]!='0'; i++){
	file[i]=argv[2][i+1];
  }
  file[i]=0;
printf("%s\n",file);
  
  i=string2file_desc(arch, file,INODE_ROOT, &desc);
  if(i != 0){
	printf("Error %d :%s\n",i,error[(int)i]);
	return(i);
  }


  
  printf("inode %d\n",desc.de.inode);
  printf("file_type %d\n",desc.de.file_type);
  printf("Nombre : ");
  for(i=0; i < desc.de.name_len ; i++){
	printf("%c",desc.de.name[i]);
  }
  printf("\n");
  getchar();


  return( 0);
}

//-----------------------------------------------------------------------

unsigned long block_size(unsigned long block){

	return( ( (unsigned long) 0x1 << block) * 1024 );
}

unsigned long block_jump(int arch, unsigned long size_block, unsigned long block){
	unsigned long i;
	for(i=0 ; i < block ; i++){
		lseek(arch , size_block, SEEK_CUR);
	}
	return(i);
}


unsigned long superblock( int arch, ext2_super_block *sb){
	unsigned long ret=0;
	lseek(arch,SIZE_BOOT,SEEK_SET);
	read( arch , sb , sizeof(ext2_super_block) );
	if( sb->s_magic != 0xef53 ){
		ret=1;
	}
	return(ret);
}

unsigned long group(int arch, ext2_group_desc *bg, unsigned long n_group){
	unsigned long i;

	lseek(arch,SIZE_BOOT + BLOCK_SIZE +( sizeof(ext2_group_desc) * n_group ) ,SEEK_SET);
	read( arch , bg , sizeof(ext2_group_desc) );
	return(0);
}

unsigned long inode(	int arch,               \
			ext2_inode *ino,        \
			unsigned long n_inode,  \
			ext2_group_desc *bg       ) {

	lseek(arch, 0, SEEK_SET);
	block_jump(arch, BLOCK_SIZE, bg->bg_inode_table);
	lseek(arch, (128 * (n_inode-1)) ,SEEK_CUR);
	read( arch , ino , sizeof(ext2_inode) ); 
	
	return(0);
}

unsigned long block_logico(int arch, ext2_logic *lo,unsigned long n_logic){
	lseek(arch,0,SEEK_SET);
	block_jump(arch, BLOCK_SIZE, n_logic);
 	read( arch , lo ,  BLOCK_SIZE);
}

unsigned long string2dir(ext2_dir_entry *de, char *file, ext2_dir_entry *de_file,unsigned long size){
	unsigned int i,j=0,ret=0;
	char fichero[EXT2_NAME_LEN];
	while(j < size){

		for(i=0; i < de->name_len ; i++){
		fichero[i]=de->name[i];
		}
		fichero[i]=0;
		if( 0 == strcmp(file,fichero) ){
			de_file->inode=de->inode;
	  		de_file->rec_len=de->rec_len;
			de_file->name_len=de->name_len;
			de_file->file_type=de->file_type;
			for(i=0; i < de->name_len ; i++){
				de_file->name[i]=de->name[i];
			}
			ret=de->inode;
		}
		j+= de->rec_len;
		i=(unsigned long)de;
		i+= de->rec_len;
		de= (void *)i;
	}
	return(ret);
}


unsigned long string2file_desc(int arch, char *file,unsigned long inode_dir, file_desc *desc){
	unsigned long ret=0,i;
	ext2_super_block sb;
	ext2_group_desc bg;
	ext2_inode ino;
	unsigned long size_logic;
	ext2_logic lo;
	ext2_dir_entry de_file;


	if(0 != superblock(arch, &sb) ) {
		return(2);
	}
	BLOCK_SIZE = block_size(sb.s_log_block_size);
	group(arch, &bg, inode_dir / sb.s_inodes_per_group);
	inode(arch, &ino, inode_dir, &bg );
	size_logic= ino.i_blocks / (BLOCK_SIZE / 512);

	block_logico(arch, &lo,ino.i_block[0]);
	if ( 0 == string2dir( (ext2_dir_entry *) &lo,	\
				file,			\
				&(desc->de),		\
				ino.i_size )		){
		return(3);
	}
	group(arch,&bg,desc->de.inode % sb.s_inodes_per_group);
	inode(arch,&(desc->ino),desc->de.inode,&bg);	

	return(0);
}
