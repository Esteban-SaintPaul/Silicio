#define	SYS_OPEN	4
#define SYS_READ	5
#define SYS_WRITE	6
#define SYS_WAIT	7
#define SYS_SEEK	8

#define SYS_REG		100
#define SYS_MEM		101
#define SYS_RET		102

#define SYS_DEBUG	200
#define SYS_PIPER	203
#define SYS_PIPEW	204
#define SYS_RINT	205
#define SYS_RPORT	206
#define SYS_URPORT	207
#define SYS_VBE_ADDR	208
#define SYS_VBE_X	209
#define SYS_VBE_Y	210
#define SYS_VBE_COLOR	211
#define SYS_HLT		212

#define SYS_SIZE	512

typedef struct {
	char *arch;		// Puntero a nombre de disp
	unsigned long call;	// Numero de llamada a sistema
	unsigned long size;	// Tamaño del buffer
	unsigned long count;	// cant transferida de bites
	int ret;		// valor retorno de llamada
	unsigned long id_drv;	// id del driver
	unsigned long id_ret;	// id del proceso llamador
	char *buf;		// puntero a buffer transferido
} sys_param;
