#ifndef DEBUG_INTERFACE_H_
#define DEBUG_INTERFACE_H_

/**
* FH_DBI_Version
*@brief 按SDK的统一格式定义的版本信息
*@param [in] print_enable:是否打印版本信息
*@param [out] 无
*@return 是否成功
* - RETURN_OK(0):  成功
* - 其他：失败,错误详见错误号
*/
char *FH_DBI_Version(unsigned int print_enable);

typedef int (*dbi_send)(void* obj, unsigned char *buf, int size);
typedef int (*dbi_recv)(void* obj, unsigned char *buf, int size);



typedef struct DI_config
{
	void *		obj;
	dbi_send	send;
	dbi_recv	recv;


}DI_config;

struct Debug_Interface;

int DI_handle(struct Debug_Interface* di);

int DI_destroy(struct Debug_Interface *di);

/* object create */
struct Debug_Interface * DI_create(struct DI_config *cfg);

#endif

