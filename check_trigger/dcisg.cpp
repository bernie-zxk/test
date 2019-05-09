// Dci.cpp: implementation of the CDci class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdarg.h>
#include "dci.h"
#include "dcidefine.h"
#include "dcisg.h"
#include "lst.h"
#ifdef WIN32
#pragma warning(disable:4786)
#include <windows.h>
typedef	CRITICAL_SECTION	os_mutex_t;
#else
typedef	pthread_mutex_t		os_mutex_t;
#endif	//WIN32
#define CONF_FILE_NAME		"dcisg.conf"		//配置文件路径
static int g_log_flag = 0;
#define LOG_FLAG_NO_LOG					0
#define LOG_FLAG_PRINT_LOG				1
#define LOG_FLAG_FILE_LOG				2
#define LOG_FLAG_PRINTF_AND_FILE_LOG	3
#define LOG_DIR			"LOG_DIR"			//日志文件保存的目录
#define LOG_FLAG			"LOG_FLAG"				//日志的输出格式，0为不输出，1为输出到屏幕，2为输出到文件，3为输出到屏幕和文件

static FILE *g_verlog = NULL;
static int get_config_info(char* cfg_name, char* cfg_value);
#ifndef WIN32
#include <unistd.h>
#include <ctype.h>
#define stricmp			strcasecmp
#define strnicmp		strncasecmp
#define strstr	strcasestr
#define _strlwr(pstr)  do \
{\
	char * _str = (char*)pstr;\
	while (*_str) \
	{ \
	if (isupper(*_str)) \
	*_str = tolower(*_str);\
	_str++;\
	}\
} while(0)

#endif
#define SET_ERROR(pError, ERROR_NO, ERR_INFO) \
	SetError(errhp, pError, ERROR_NO, ERR_INFO, DCI_HTYPE_ERROR, __FILE__, (sb4)__LINE__);

#define DCI_CHECK(pError, DCIfunc) \
	if (DCI_SUCCESS != (retcode = (DCIfunc))) \
	{ \
		SetError(errhp, pError, 0, "", DCI_HTYPE_ERROR, __FILE__, (sb4)__LINE__); \
		if (retcode != DCI_SUCCESS_WITH_INFO) {\
			DCIHandleFree((dvoid *)stmthp, DCI_HTYPE_STMT);\
			DCITransRollback(svchp, NULL, (ub4)DCI_DEFAULT);\
			DCIHandleFree((dvoid *)errhp, DCI_HTYPE_ERROR);\
			return FALSE;\
		}\
	}

#define DCI_CHECK_EX(pError, DCIfunc) \
	if (DCI_SUCCESS != (retcode = (DCIfunc))) \
	{ \
		if (retcode != DCI_NO_DATA)\
			SetError(errhp, pError, 0, "", DCI_HTYPE_ERROR, __FILE__, (sb4)__LINE__); \
	}

#define DCI_CHECK1(pError, DCIfunc) \
	if (DCI_SUCCESS != (retcode = (DCIfunc))) \
	{ \
		SetError(errhp, pError, 0, "", DCI_HTYPE_ERROR, __FILE__, (sb4)__LINE__); \
		if (retcode != DCI_SUCCESS_WITH_INFO) \
		{\
			DCIHandleFree((dvoid *)errhp, DCI_HTYPE_ERROR);\
			return FALSE;\
		}\
	}

#define DCI_CHECK1_EX(pError, Error_no, Error_info, DCIfunc) \
	if (DCI_SUCCESS != (retcode = (DCIfunc))) \
	{ \
		SetError(errhp, pError, Error_no, Error_info, DCI_HTYPE_ERROR, __FILE__, (sb4)__LINE__);\
	}

#define  RETURN_TRUE	DCIHandleFree((dvoid *)errhp, DCI_HTYPE_ERROR);\
	DCIHandleFree((dvoid *)stmthp, DCI_HTYPE_STMT);\
	return TRUE;
#define  RETURN_FALSE	DCIHandleFree((dvoid *)errhp, DCI_HTYPE_ERROR);\
	DCIHandleFree((dvoid *)stmthp, DCI_HTYPE_STMT);\
	return FALSE;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
static void mutex_create(os_mutex_t*   os_mutex)
{
	if (os_mutex == NULL)
		return;
#ifdef WIN32
	InitializeCriticalSection(os_mutex);
#else
	pthread_mutex_init(os_mutex, NULL);
#endif // WIN32
};

static void mutex_free(os_mutex_t* os_mutex)
{
	if (os_mutex == NULL)
		return;
#ifdef WIN32
	DeleteCriticalSection(os_mutex);
#else
	pthread_mutex_destroy(os_mutex);
#endif	// WIN32
}

static void mutex_enter(os_mutex_t* os_mutex)
{
	if (os_mutex == NULL)
		return;
#ifdef WIN32
	EnterCriticalSection(os_mutex);
#else
	pthread_mutex_lock(os_mutex);
#endif // WIN32
}

static void mutex_exit(os_mutex_t*   os_mutex)
{
	if (os_mutex == NULL)
		return;
#ifdef WIN32
	LeaveCriticalSection(os_mutex);
#else
	pthread_mutex_unlock(os_mutex);
#endif // WIN32
}

#ifdef DEBUG_MEMOFY
static int g_os_memory_mutex_init_flag = 0;
os_mutex_t g_os_memory_mutex;
typedef struct dcisg_malloc_struct dcisg_malloc_struct_t;
struct dcisg_malloc_struct
{
	void *data;
	char filename[300];
	int line;
	DM_LIST_NODE_T(dcisg_malloc_struct_t) link;
};
DM_LIST_BASE_NODE_T(dcisg_malloc_struct) lst_dcisg_memory;
void *dcisg_malloc_ex(size_t size, int line, const char* file_name)
{
	if (g_os_memory_mutex_init_flag == 0)
	{
		g_os_memory_mutex_init_flag = 1;
		mutex_create(&g_os_memory_mutex);
		DM_LIST_INIT(lst_dcisg_memory);
	}
	dcisg_malloc_struct_t *memory_temp = NULL;
	memory_temp = (dcisg_malloc_struct_t*)malloc(sizeof(dcisg_malloc_struct_t));
	if (memory_temp)
	{
		memset(memory_temp, 0, sizeof(dcisg_malloc_struct_t));
		strcpy(memory_temp->filename, file_name);
		memory_temp->line = line;
		memory_temp->data = malloc(size);
		if (memory_temp->data == NULL)
		{
			free(memory_temp);
			return NULL;
		}
		mutex_enter(&g_os_memory_mutex);
		DM_LIST_ADD_FIRST(link, lst_dcisg_memory, memory_temp);
		mutex_exit(&g_os_memory_mutex);
		return memory_temp->data;
	}
	return NULL;
}
void dcisg_free(void *data)
{
	if (g_os_memory_mutex_init_flag == 0)
	{
		g_os_memory_mutex_init_flag = 1;
		mutex_create(&g_os_memory_mutex);
		DM_LIST_INIT(lst_dcisg_memory);
	}
	mutex_enter(&g_os_memory_mutex);
	dcisg_malloc_struct_t *memory_temp = DM_LIST_GET_FIRST(lst_dcisg_memory);
	while(memory_temp)
	{
		if (memory_temp->data == data)
		{
			DM_LIST_REMOVE(link, lst_dcisg_memory, memory_temp);
			mutex_exit(&g_os_memory_mutex);
			free(memory_temp->data);
			free(memory_temp);
			return;
		}
		memory_temp = DM_LIST_GET_NEXT(link, memory_temp);
	}
	mutex_exit(&g_os_memory_mutex);
}
void show_memory()
{
	if (g_os_memory_mutex_init_flag == 0)
	{
		g_os_memory_mutex_init_flag = 1;
		mutex_create(&g_os_memory_mutex);
		DM_LIST_INIT(lst_dcisg_memory);
	}
	mutex_enter(&g_os_memory_mutex);
	dcisg_malloc_struct_t *memory_temp = DM_LIST_GET_FIRST(lst_dcisg_memory);
	while(memory_temp)
	{
		log_report("%s(%d)\r\n", memory_temp->filename, memory_temp->line);
		memory_temp = DM_LIST_GET_NEXT(link, memory_temp);
	}
	mutex_exit(&g_os_memory_mutex);
}
#else
void show_memory()
{
	return;
}
void* dcisg_malloc_ex(size_t size, int a, const char* b)
{
	return malloc(size);
}

void dcisg_free(void *p)
{
    if(p != NULL)
    {
        free(p);
    }
}
#endif // _DEBUG

static int InitMid();
CDci::CDci()
{
	//danath 20111025 add 此处如果没有会导致在登录不存在的情况下程序coredown
	memset(username,0,128);
	memset(pwd,0,128);
	memset(servername,0,128);

	InitMid();
	InitHandle();
	is_connect = FALSE;
	DCIInitialize((ub4) DCI_DEFAULT, (dvoid *)0,
		(dvoid * (*)(dvoid *, size_t)) 0,
		(dvoid * (*)(dvoid *, dvoid *, size_t))0,
		(void (*)(dvoid *, dvoid *)) 0 );
}

CDci::~CDci()
{
	ErrorInfo_t error;
	if (is_connect == TRUE)
	{
		DisConnect(&error);
	}
}
static void convert_datetime(char* out_datetime, tm &tm_time)
{
	sprintf(out_datetime, "%04d-%02d-%02d %02d:%02d:%02d", tm_time.tm_year, tm_time.tm_mon, tm_time.tm_mday,
		tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
}
static int g_log_first_in = TRUE;
static void log_report(const char*	format,	...)
{
	va_list		argptr;
	int		len;
	time_t ltime;
	if (g_log_flag == LOG_FLAG_NO_LOG)
		return;
	len = (int)strlen(format);
	if (len == 0)
		return;
	time(&ltime);
	tm tm_time = *localtime(&ltime);
	char temp_time[36];
	tm_time.tm_year += 1900;
	tm_time.tm_mon += 1;
	convert_datetime(temp_time, tm_time);
	// print directly if necessary
	if (g_log_flag == LOG_FLAG_PRINT_LOG || g_log_flag == LOG_FLAG_PRINTF_AND_FILE_LOG)
	{
		printf("%s: ", temp_time);
		va_start(argptr, format);
		vprintf(format, argptr);
		printf("\r\n");
		va_end(argptr);
	}
	if (g_log_first_in == TRUE && g_verlog == NULL)
	{
		char tempstr[1024];
		char dir[1024];
		get_config_info(LOG_DIR, tempstr);
		sprintf(dir, "%s/dcisg.log", tempstr);
		g_verlog = fopen(dir,"a+");
		g_log_first_in = FALSE;
	}
	// write to file
	if (g_verlog != NULL && (g_log_flag == LOG_FLAG_FILE_LOG || g_log_flag == LOG_FLAG_PRINTF_AND_FILE_LOG))
	{
		fprintf(g_verlog, "%s: ", temp_time);
		va_start(argptr, format);
		vfprintf(g_verlog, format, argptr);
		fprintf(g_verlog, "\r\n");
		va_end(argptr);
		fflush(g_verlog);
	}
}
/*******************************************************************************
**
** Routine:     Connect
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: request the connection from database
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
bool CDci::Connect(const char* server, const char* user_name, const char* pasword, bool is_init_format_tables, ErrorInfo_t* error)
{
	is_init_schema = is_init_format_tables;
	if(Connect(server, user_name, pasword, error))
	{
		if (is_init_format_tables)
		{
			InitSchemaTableColAttr();		//初始化配置文件中的表模板列属性
		}
		return TRUE;
	}
	return FALSE;
}
bool CDci::Connect(const char* server, const char* user_name, const char* pasword, ErrorInfo_t* error)
{
	int retcode;
	DCIError   *errhp = NULL;
	DCIStmt    *stmthp = NULL;
//	dschp = NULL;
	//		DCIStmt 	stmthp = NULL;
	memset(error, 0, sizeof(ErrorInfo_t));
	strcpy(error->error_info, "还未连接数据服务器");
	error->error_no = 3113;
	is_init_schema = FALSE;
	is_connect = FALSE;
#ifndef WIN32//让出CPU，以免在多线程的情况下，某个线程正在重连时，其它线程会调用接口做语句操作
	sleep(1);
#endif
	if (envhp)
	{
		//如果环境句柄不是NULL，说明连接已经建立，但是上层忘记释放就再次建立连接
		DisConnect(error);
	}
	errhp = NULL;
	srvhp = NULL;
	svchp = NULL;
	authp = NULL;

	DCI_CHECK1(error,
		DCIEnvInit((DCIEnv **)&envhp, DCI_DEFAULT, (size_t)0, (dvoid **)0));

	/* allocate error handles */
	DCI_CHECK1(error,
		DCIHandleAlloc((dvoid *)envhp,  (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));

	/* server contexts */
	DCI_CHECK1(error,
		DCIHandleAlloc((dvoid *)envhp,	(dvoid **)&srvhp, DCI_HTYPE_SERVER, (size_t)0, (dvoid **)0));

	DCI_CHECK1(error,
		DCIHandleAlloc((dvoid *)envhp, (dvoid **)&svchp, DCI_HTYPE_SVCCTX, (size_t)0, (dvoid **)0));

	DCI_CHECK1(error,
		DCIServerAttach(srvhp, errhp,
		(DciText*)server,
		(sb4)strlen((const char *)server),
		DCI_DEFAULT));

	/* set attribute server context in the service context */
	DCI_CHECK1(error,
		DCIAttrSet((dvoid *)svchp, DCI_HTYPE_SVCCTX,
		(dvoid *)srvhp, (ub4)0, DCI_ATTR_SERVER,
		errhp));

	/* begin a session */
	DCI_CHECK1(error,
		DCIHandleAlloc((dvoid *)envhp, (dvoid **)&authp, (ub4)DCI_HTYPE_SESSION, (size_t)0, (dvoid **)0));

	DCI_CHECK1(error,
		DCIAttrSet((dvoid *)authp, (ub4)DCI_HTYPE_SESSION,
		(dvoid *)user_name,
		(ub4)strlen((char *)user_name),
		(ub4)DCI_ATTR_USERNAME, errhp));

	DCI_CHECK1(error,
		DCIAttrSet((dvoid *)authp, (ub4)DCI_HTYPE_SESSION,
		(dvoid *)pasword,
		(ub4)strlen((char *)pasword),
		(ub4)DCI_ATTR_PASSWORD, errhp));

	DCI_CHECK1(error,
		DCISessionBegin(svchp, errhp, authp,
		DCI_CRED_RDBMS, (ub4)DCI_DEFAULT));

	/* set authentication context into service context */
	DCI_CHECK1(error,
		DCIAttrSet((dvoid *)svchp, (ub4)DCI_HTYPE_SVCCTX,
		(dvoid *)authp, (ub4)0, (ub4)DCI_ATTR_SESSION,
		errhp));

		/** allcocate the statement handle
		DCI_CHECK1(error,
		DCIHandleAlloc((dvoid *)envhp, (dvoid **)&stmthp,
		(ub4)DCI_HTYPE_STMT, (size_t)0, (dvoid **)0));
    **********/

	/* describe spatial object types */
// 	DCI_CHECK1(error,
// 		DCIHandleAlloc(envhp, (dvoid **)&dschp,
// 		(ub4)DCI_HTYPE_DESCRIBE, (size_t)0,
// 		(dvoid **)0));

	is_connect = TRUE;
	ErrorInfo_t errorEx;
	//下面是针对达梦数据库做的优化，先执行查询字典表的操作，判断当前连接的是否是达梦数据库
	is_dm_database = ExecSingle("select 1 from sysdba.systables where 1=0", &errorEx);
	strncpy(servername, server, 128);
	strncpy(username, user_name, 128);
	strncpy(pwd, pasword, 128);
	// if (stricmp(username, "HISDB") == 0 || stricmp(username, "STATICS") == 0 )
	{
	//	if(InitializeCurveInfo())
		{
			InitSchemaTableColAttr();
		}
	}
//	InitSchemaTableColAttr();		//初始化配置文件中的表模板列属性
	RETURN_TRUE;
}
/*******************************************************************************
**
** Routine: SetError
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: set the error
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/

void CDci::SetError(DCIError *errhp1, ErrorInfo_t *error, int error_no, const char *error_info, ub4 htype, const char* file_name, int file_line)
{
    sb4  errcode;

	if (error == NULL)
		return;

	error->error_info[ERROR_INFO_LEN] = '\0';
	strcpy(error->file_name, file_name);
	error->file_line = file_line;
	if (error_no != 0)
	{
		error->error_no = error_no;
		if (error_info)
		{
			strncpy(error->error_info, error_info, ERROR_INFO_LEN);
		}
		return;
	}
    DCIErrorGet(errhp1, (ub4) 1, (DciText *) NULL, &errcode,
		(DciText*)error->error_info, ERROR_INFO_LEN, htype);
	error->error_no = errcode;
	//printf("code:%d\r\nerror:%s\r\nfile_line:%d\r\nfile_name:%s\r\n", error->error_no, error->error_info, error->file_line, error->file_name);
}
/*******************************************************************************
**
** Routine: CleanError
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: clean the error information from struct ErrorInfo
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
void CDci::CleanError(ErrorInfo_t *error)
{
	if (error)
	{
		memset(error, 0, sizeof(ErrorInfo_t));
	}
}
/*******************************************************************************
**
** Routine: DisConnect
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: disconnect from the database
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
bool CDci::DisConnect(ErrorInfo_t *error)
{
//	DCIHandleFree((dvoid *)dschp, DCI_HTYPE_DESCRIBE);
	is_connect = FALSE;
	memset(error, 0, sizeof(ErrorInfo_t));
    DCIServerDetach(srvhp, NULL, DCI_DEFAULT);
	DCIHandleFree((dvoid *)authp, DCI_HTYPE_SESSION);
    DCIHandleFree((dvoid *)srvhp, DCI_HTYPE_SERVER);
	DCIHandleFree((dvoid *)svchp, DCI_HTYPE_SVCCTX);
    DCIHandleFree((dvoid *)envhp, DCI_HTYPE_ENV);
	envhp = NULL;
	srvhp = NULL;
	svchp = NULL;
	authp = NULL;
//	dschp = NULL;
	//stmthp = NULL;

	return TRUE;
}

#define RETURN_FALSE_DPLS(R) \
	if(R != DCI_SUCCESS) \
{ \
	FreeDirPathHandle(colLstDesc, dpctx, dpca, dpstr); \
	DCIHandleFree((dvoid *)errhp, DCI_HTYPE_ERROR);\
	DCITransRollback(svchp, NULL, (ub4)DCI_DEFAULT);\
	errhp = NULL;\
	return FALSE;\
}
//这个接口的数据后面，必需要跟两个字节来指定当前的值是否为NULL，这两个字节当sb2类型，置为-1为表示NULL
bool CDci::DirPathLoadStreamByRowBinds(const char *schema, const char *table, ColAttr_t *col, ub2 cols, ub4 rows, void *data, ub1 input_type, ErrorInfo_t *error)
{
	sword	retcode;
	ub4		rowoff;
	ub4		rowoff_all = 0;
	ub2		coloff;
	ub4		nrows;
	ub4		times = 0;
	ub4		rows_temp;
	DCIParam           *colLstDesc = NULL;        /* column list parameter handle */
	DCIDirPathCtx      *dpctx = NULL;                      /* direct path context */
	DCIDirPathColArray *dpca = NULL;           /* direct path column array handle */
	DCIDirPathStream   *dpstr = NULL;                /* direct path stream handle */
	DCIError           *errhp = NULL;
	ub1 *tempdata = NULL;
	memset(error, 0, sizeof(ErrorInfo_t));
	DCI_CHECK1(error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));
	CleanError(error);

	if (table == NULL || col == NULL)
	{
		SET_ERROR(error, NORMAL_ERR, "非法的空指针");
	}
	if (cols == 0)
	{
		SET_ERROR(error, NORMAL_ERR, "非法的列数");
	}
	if (!SetTableInfo(schema, table, col, cols, rows, &nrows, input_type, &dpctx, &dpca, &dpstr, &colLstDesc, errhp, error))
	{
		RETURN_FALSE_DPLS(DCI_ERROR);
	}

	tempdata = (ub1*)data; //danath 20111207 add
	while (rows)
	{
		if (rows > nrows)
		{
			rows_temp = nrows;
			rows -= nrows;
		}
		else
		{
			rows_temp = rows;
			rows = 0;
		}

		DCI_CHECK_EX(error, DCIDirPathColArrayReset(dpca, errhp));
		RETURN_FALSE_DPLS(retcode);

		DCI_CHECK_EX(error, DCIDirPathStreamReset(dpstr, errhp));
		RETURN_FALSE_DPLS(retcode);
		// tempdata = (ub1*)data; danath 20111207 delete 该处导致数组长度超长时重复插入前nrows个数据
		for (rowoff = 0; rowoff < rows_temp; rowoff++)
		{
			for (coloff = 0; coloff < cols; coloff++)
			{
				if (*(sb2*)(tempdata + col[coloff].data_size) == -1)
				{
					DCI_CHECK_EX(error,
						DCIDirPathColArrayEntrySet(dpca, errhp,	rowoff, coloff,
						tempdata, col[coloff].data_size, DCI_DIRPATH_COL_NULL));
					RETURN_FALSE_DPLS(retcode);
				}
				else
				{
					DCI_CHECK_EX(error,
						DCIDirPathColArrayEntrySet(dpca, errhp,	rowoff, coloff,
						tempdata, col[coloff].data_size, DCI_DIRPATH_COL_COMPLETE));
					RETURN_FALSE_DPLS(retcode);
				}
				tempdata += col[coloff].data_size + sizeof(sb2);
			}
			rowoff_all++;
		}

		/* Convert Col Array to Stream */
		DCI_CHECK_EX(error,
			DCIDirPathColArrayToStream(dpca, dpctx,
			dpstr, errhp, rows_temp, 0));
		RETURN_FALSE_DPLS(retcode);

		/* Load Stream */
		DCI_CHECK_EX(error,
			DCIDirPathLoadStream(dpctx, dpstr, errhp));
		RETURN_FALSE_DPLS(retcode);
	}

	DCI_CHECK_EX(error,
		DCIDirPathDataSave(dpctx, errhp, (ub4)DCI_DIRPATH_DATASAVE_FINISH));
	RETURN_FALSE_DPLS(retcode);

	/* free up server data structures for the load. */
	DCI_CHECK_EX(error,
		DCIDirPathFinish(dpctx, errhp));
	RETURN_FALSE_DPLS(retcode);

	FreeDirPathHandle(colLstDesc, dpctx, dpca, dpstr);
	DCIHandleFree((dvoid *)errhp, DCI_HTYPE_ERROR);
	return TRUE;
}
/*******************************************************************************
**
** Routine: DirPathLoadStream
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: insert the data decording the schema and the table information
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
bool CDci::DirPathLoadStream(const char *schema, const char *table, ColAttr_t *col, ub2 cols, ub4 rows, void *data, ub1 input_type, ErrorInfo_t *error)
{
	sword	retcode;
	ub4		rowoff;
	ub4		rowoff_all = 0;
	ub2		coloff;
	ub4		record_size = 0;
	ub4		nrows;
	ub4		times = 0;
	ub4		rows_temp;
	DCIParam           *colLstDesc = NULL;        /* column list parameter handle */
	DCIDirPathCtx      *dpctx = NULL;                      /* direct path context */
	DCIDirPathColArray *dpca = NULL;           /* direct path column array handle */
	DCIDirPathStream   *dpstr = NULL;                /* direct path stream handle */
	DCIError           *errhp = NULL;
	memset(error, 0, sizeof(ErrorInfo_t));
	DCI_CHECK1(error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));
	CleanError(error);

	if (table == NULL || col == NULL)
	{
		SET_ERROR(error, NORMAL_ERR, "非法的空指针");
	}
	if (cols == 0)
	{
		SET_ERROR(error, NORMAL_ERR, "非法的列数");
	}

	InitColOffset(col, cols, &record_size);
	if (!SetTableInfo(schema, table, col, cols, rows, &nrows, input_type, &dpctx, &dpca, &dpstr, &colLstDesc, errhp, error))
	{
		RETURN_FALSE_DPLS(DCI_ERROR);
	}

	while (rows)
	{
		if (rows > nrows)
		{
			rows_temp = nrows;
			rows -= nrows;
		}
		else
		{
			rows_temp = rows;
			rows = 0;
		}

		DCI_CHECK_EX(error, DCIDirPathColArrayReset(dpca, errhp));
		RETURN_FALSE_DPLS(retcode);

		DCI_CHECK_EX(error, DCIDirPathStreamReset(dpstr, errhp));
		RETURN_FALSE_DPLS(retcode);

		for (rowoff = 0; rowoff < rows_temp; rowoff++)
		{
			for (coloff = 0; coloff < cols; coloff++)
			{
				if (col[coloff].indp && col[coloff].indp[rowoff_all] == -1)
				{
					DCI_CHECK_EX(error,
						DCIDirPathColArrayEntrySet(dpca, errhp,	rowoff, coloff,
						(ub1*)data + (rowoff_all * record_size) + col[coloff].data_offset,
						col[coloff].data_size, DCI_DIRPATH_COL_NULL));
					RETURN_FALSE_DPLS(retcode);
				}
				else
				{
					DCI_CHECK_EX(error,
						DCIDirPathColArrayEntrySet(dpca, errhp,
						rowoff, coloff,
						(ub1*)data + (rowoff_all * record_size) + col[coloff].data_offset,
						col[coloff].data_size, DCI_DIRPATH_COL_COMPLETE));
					RETURN_FALSE_DPLS(retcode);
				}
			}
			rowoff_all++;
		}

		/* Convert Col Array to Stream */
		DCI_CHECK_EX(error,
			DCIDirPathColArrayToStream(dpca, dpctx,
			dpstr, errhp, rows_temp, 0));
		RETURN_FALSE_DPLS(retcode);

		/* Load Stream */
		DCI_CHECK_EX(error,
			DCIDirPathLoadStream(dpctx, dpstr, errhp));
		RETURN_FALSE_DPLS(retcode);
	}

	DCI_CHECK_EX(error,
		DCIDirPathDataSave(dpctx, errhp, (ub4)DCI_DIRPATH_DATASAVE_FINISH));
	RETURN_FALSE_DPLS(retcode);

	/* free up server data structures for the load. */
	DCI_CHECK_EX(error,
		DCIDirPathFinish(dpctx, errhp));
	RETURN_FALSE_DPLS(retcode);

	FreeDirPathHandle(colLstDesc, dpctx, dpca, dpstr);
	DCIHandleFree((dvoid *)errhp, DCI_HTYPE_ERROR);
	errhp = NULL;
	return TRUE;
}

/*******************************************************************************
**
** Routine: InitColOffset
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: init the offset in the struct of column's information
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
void CDci::InitColOffset(ColAttr_t *col, ub2 cols, ub4 *record_size)
{
	ub4 offset = 0;
	int i;

	for (i = 0; i < cols; i++)
	{
		col[i].data_offset = offset;
		offset += col[i].data_size;
	}
	*record_size = offset;
}

/*******************************************************************************
**
** Routine: SetTableInfo
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: set the information of table
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
bool CDci::SetTableInfo(const char *schema, const char *table, ColAttr_t *col, ub2 cols, ub4 rows, ub4 *nrows, ub1 input_type,
						DCIDirPathCtx **dpctxpp, DCIDirPathColArray **dpcapp,
						DCIDirPathStream   **dpstrpp, DCIParam  **colLstDescpp, DCIError *errhp, ErrorInfo_t *error)
{
	sword	retcode;
	ub2		i, pos;
	DCIParam	*colDesc;                  /* column parameter descriptor */
	ub4		buf_size;
	ub1		dirpathinput = input_type;//DCI_DIRPATH_INPUT_STREAM//DCI_DIRPATH_INPUT_TEXT
	ub2		dtype = SQLT_CHR;
	char	dfltdatemask_tbl[100] = "YYYY-MM-DD HH24:MI:SS";
	DCIParam           *colLstDesc = NULL;        /* column list parameter handle */
	DCIDirPathCtx      *dpctx = NULL;                      /* direct path context */
	DCIDirPathColArray *dpca = NULL;           /* direct path column array handle */
	DCIDirPathStream   *dpstr = NULL;                /* direct path stream handle */
	DCIStmt    *stmthp = NULL;		//这个参数无作用，只是为了可以使用宏DCI_CHECK
	*dpctxpp = NULL;
	*dpcapp = NULL;
	*dpstrpp = NULL;
	*colLstDescpp = NULL;
	errhp = NULL;
	DCI_CHECK1(error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));
	DCI_CHECK1(error,
		DCIHandleAlloc((dvoid *)envhp,
		(dvoid **)&dpctx,
		(ub4)DCI_HTYPE_DIRPATH_CTX,
		(size_t)0, (dvoid **)0));
	*dpctxpp = dpctx;
	DCI_CHECK(error,
		DCIAttrSet((dvoid *)dpctx, (ub4)DCI_HTYPE_DIRPATH_CTX,
		(dvoid *)table,
		(ub4)strlen((const char *)table),
		(ub4)DCI_ATTR_NAME, errhp));
	if (schema && strlen(schema) != 0)
	{
		DCI_CHECK(error,
			DCIAttrSet((dvoid *)dpctx, (ub4)DCI_HTYPE_DIRPATH_CTX,
			(dvoid *)schema,
			(ub4)strlen((const char *)schema),
			(ub4)DCI_ATTR_SCHEMA_NAME, errhp));
	}

	/* Note: setting tbl default datemask will not trigger client library
	* to check strings for dates - only setting column datemask will.
	*/
	DCI_CHECK(error,
		DCIAttrSet((dvoid *)dpctx, (ub4)DCI_HTYPE_DIRPATH_CTX,
		(dvoid *)dfltdatemask_tbl,
		(ub4)strlen((const char *)dfltdatemask_tbl),
		(ub4)DCI_ATTR_DATEFORMAT, errhp));

	/* set the data input type to be text */
	DCI_CHECK(error,
		DCIAttrSet ((dvoid *)dpctx, DCI_HTYPE_DIRPATH_CTX,
		(dvoid *)&dirpathinput, (ub4)0,
		DCI_ATTR_DIRPATH_INPUT, errhp));
	/* set number of columns to be loaded */
	DCI_CHECK(error,
		DCIAttrSet((dvoid *)dpctx, (ub4)DCI_HTYPE_DIRPATH_CTX,
		(dvoid *)&cols,
		(ub4)0, (ub4)DCI_ATTR_NUM_COLS, errhp));

	/* get the column parameter list */
	DCI_CHECK(error,
		DCIAttrGet((dvoid *)dpctx,
		DCI_HTYPE_DIRPATH_CTX,
		(dvoid *)&colLstDesc, (ub4 *)0,
		DCI_ATTR_LIST_COLUMNS, errhp));

	for (i = 0, pos = 1; i<cols; i++, pos++)
	{
		/* get parameter handle on the column */
		DCI_CHECK_EX(error,
			DCIParamGet((CONST dvoid *)colLstDesc,
			(ub4)DCI_DTYPE_PARAM, errhp,
			(dvoid **)&colDesc, pos));
		*colLstDescpp = colLstDesc;

		/* column name */
		DCI_CHECK_EX(error,
			DCIAttrSet((dvoid *)colDesc, (ub4)DCI_DTYPE_PARAM,
			(dvoid *)col[i].col_name,
			(ub4)strlen(col[i].col_name),
			(ub4)DCI_ATTR_NAME, errhp));

		/* column type */
		if (dirpathinput == DCI_DIRPATH_INPUT_STREAM)
		{
			DCI_CHECK_EX(error,
				DCIAttrSet((dvoid *)colDesc, (ub4)DCI_DTYPE_PARAM,
				(dvoid *)&col[i].data_type, (ub4)0,
				(ub4)DCI_ATTR_DATA_TYPE, errhp));
		}
		else
		{
			DCI_CHECK_EX(error,
				DCIAttrSet((dvoid *)colDesc, (ub4)DCI_DTYPE_PARAM,
				(dvoid *)&dtype, (ub4)0,
				(ub4)DCI_ATTR_DATA_TYPE, errhp));
		}

		/* max data size */
		DCI_CHECK_EX(error,
			DCIAttrSet((dvoid *)colDesc, (ub4)DCI_DTYPE_PARAM,
			(dvoid *)&col[i].data_size, (ub4)0,
			(ub4)DCI_ATTR_DATA_SIZE, errhp));

		/* free the parameter handle to the column descriptor */
		DCI_CHECK1_EX(error, NORMAL_ERR, "释放列描述句柄失败",
			DCIDescriptorFree((dvoid *)colDesc, DCI_DTYPE_PARAM));
		*colLstDescpp = NULL;
	}
	buf_size = 64 *1024;
	DCI_CHECK_EX(error,
		DCIAttrSet((dvoid *)dpctx, (ub4)DCI_HTYPE_DIRPATH_CTX,
		(dvoid *)&buf_size,
		(ub4)0, (ub4)DCI_ATTR_BUF_SIZE, errhp));

	/* prepare the load */
	DCI_CHECK(error,
		DCIDirPathPrepare(dpctx, svchp, errhp));
		/*
		if (retcode == -1 && error->error_no == 604)//表不存在
		{
		//可以在这里建表
		char *sql = NULL;
		if (!CreateTable(schema, table, col, cols, &sql, error))
		{
		if(sql)
		{
		dcisg_free(sql);
		}
		return FALSE;
		}
		dcisg_free(sql);
		DCI_CHECK(error,//再次准备一次
		DCIDirPathPrepare(dpctx, svchp, errhp));
		}
		else if (retcode == -1)
		{
		return FALSE;
		}
	*/
	DCI_CHECK1(error,    //分配句柄的时候用
		DCIHandleAlloc((dvoid *)dpctx, (dvoid**)&dpca,
		(ub4)DCI_HTYPE_DIRPATH_COLUMN_ARRAY,
		(size_t)0, (dvoid **)0));
	*dpcapp = dpca;
	DCI_CHECK_EX(error,
		DCIAttrGet((CONST dvoid *)dpca,
		DCI_HTYPE_DIRPATH_COLUMN_ARRAY,
		(dvoid *)(nrows), (ub4 *)0,
		DCI_ATTR_NUM_ROWS, errhp));
	DCI_CHECK_EX(error,
		DCIHandleAlloc((dvoid *)dpctx,(dvoid**)&dpstr,
		(ub4)DCI_HTYPE_DIRPATH_STREAM,
		(size_t)0, (dvoid **)0));
	*dpstrpp = dpstr;
	DCIHandleFree((dvoid *)errhp, DCI_HTYPE_ERROR);
	return TRUE;
}
/*******************************************************************************
**
** Routine: FreeDirPathHandle
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: free the handle of direct path handle
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
bool CDci::FreeDirPathHandle(
							 DCIParam           *colLstDesc,        /* column list parameter handle */
							 DCIDirPathCtx      *dpctx,                      /* direct path context */
							 DCIDirPathColArray *dpca,           /* direct path column array handle */
							 DCIDirPathStream   *dpstr                /* direct path stream handle */)
{
	if (colLstDesc != NULL)
	{
		DCIDescriptorFree((dvoid *)colLstDesc, DCI_DTYPE_PARAM);
	}
	if (dpca != NULL)
	{
		DCIHandleFree((dvoid *)dpca, DCI_HTYPE_DIRPATH_COLUMN_ARRAY);
	}
	if (dpstr != NULL)
	{
		DCIHandleFree((dvoid *)dpstr, DCI_HTYPE_DIRPATH_STREAM);
	}
	if (dpctx != NULL)
	{
		DCIHandleFree((dvoid *)dpctx, DCI_HTYPE_DIRPATH_CTX);
	}

	return TRUE;
}

/*******************************************************************************
**
** Routine: CreateTableEx
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: create the table according the schema and table
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
bool CDci::CreateTableEx(const char *schema, const char *table, ColAttr_t *col, ub2 cols, char**ppsql, ErrorInfo_t *error)
{
	sword	retcode;
	ub4		sql_size;
	char	*sql;
	char	*sql_temp;
	ub2		i;
	char	type_name[128];
	DCIStmt	*stmthp;

	*ppsql = NULL;
	sql_size = (MAX_NAME_LEN + 30) * cols;
	sql = (char*)dcisg_malloc(sql_size);
	if (sql == NULL)
	{
		return FALSE;
	}
	*ppsql = sql;
	sql_temp = sql;
	strcpy(sql_temp, "CREATE TABLE ");
	sql_temp += strlen(sql_temp);
	if (schema && schema[0] != 0)
	{
		strcat(sql_temp, schema);
		strcat(sql_temp, ".");
		sql_temp += strlen(sql_temp);
	}
	strcat(sql_temp, table);
	strcat(sql_temp, "(");
	sql_temp += strlen(sql_temp);
	for (i = 0; i < cols; i++)
	{
		if (!GetDataTypeName(col[i].data_type, col[i].data_size, type_name, error))
		{
			return FALSE;
		}
		if (i == cols -1)
		{
			sprintf(sql_temp, "%s %s)", col[i].col_name, type_name);
		}
		else
		{
			sprintf(sql_temp, "%s %s,", col[i].col_name, type_name);
		}
		sql_temp += strlen(sql_temp);
	}
	DCIError           *errhp = NULL;
	DCI_CHECK1(error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));
	DCI_CHECK1(error, DCIHandleAlloc(envhp, (void**)&stmthp, DCI_HTYPE_STMT, 0, 0));

	if (schema != NULL && strlen(schema)!=0)
	{
		char schemasql[1024];

		sprintf(schemasql, "CREATE SCHEMA %s", schema);
		DCI_CHECK_EX(error,
			DCIStmtPrepare(stmthp, errhp, (DciText*)schemasql, strlen(schemasql), DCI_NTV_SYNTAX, DCI_DEFAULT));
		DCI_CHECK_EX(error,
			DCIStmtExecute(svchp, stmthp, errhp, 1, 0, NULL, NULL, DCI_DEFAULT));
	}

	DCI_CHECK(error,
		DCIStmtPrepare(stmthp, errhp, (DciText*)sql, strlen(sql), DCI_NTV_SYNTAX, DCI_DEFAULT));
	DCI_CHECK(error,
		DCIStmtExecute(svchp, stmthp, errhp, 1, 0, NULL, NULL, DCI_DEFAULT));

	RETURN_TRUE;
}

/*******************************************************************************
**
** Routine: GetDataTypeName
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: translate the data type from the DCI datatype to china database
**              factory datatype.
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
bool CDci::GetDataTypeName(ub2 dtype, ub4 data_size, char* type_name, ErrorInfo_t *error)
{
	DCIError *errhp = NULL;
	*type_name = 0;
	switch(dtype)
	{
	case DCI_CHR:
	case DCI_STR:
		sprintf(type_name, "VARCHAR(%d)", data_size);
		break;
	case DCI_NUM:
		strcpy(type_name, "NUMERIC");
		break;
	case DCI_INT:
		strcpy(type_name, "INT");
		break;
	case DCI_FLT:
		strcpy(type_name, "FLOAT");
		break;
	case DCI_ODT:
	case DCI_DAT:
		strcpy(type_name, "DATE");
		break;
	default:
		SET_ERROR(error, NORMAL_ERR, "暂不支持的数据类型");
		return FALSE;
	}
	return TRUE;
}

/*******************************************************************************
**
** Routine: CreateTable
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: create table
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/

bool CDci::CreateTable(const char *schema, const char *table, ColAttr_t *col, ub2 cols, ErrorInfo_t *error)
{
	char *sql = NULL;
	DCIError *errhp = NULL;
	memset(error, 0, sizeof(ErrorInfo_t));
	if(!is_connect)
	{
		SET_ERROR(error, NORMAL_ERR, "数据库连接未建立");
		return FALSE;
	}
	if (!CreateTableEx(schema, table, col, cols, &sql, error))
	{
		if(sql)
			dcisg_free(sql);

		return FALSE;
	}
	dcisg_free(sql);

	return TRUE;
}

/*******************************************************************************
**
** Routine: DirPathLoadStreamByColumnBinds
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: insert the data into the database using columns binding technology
**              according the schema and table
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
bool CDci::DirPathLoadStreamByColumnBinds(const char *schema, const char *table, ColAttr_t *col, ub2 cols, ub4 rows, ub1 input_type, ErrorInfo_t *error)
{
	DCIParam           *colLstDesc;        /* column list parameter handle */
	DCIDirPathCtx      *dpctx;                      /* direct path context */
	DCIDirPathColArray *dpca;           /* direct path column array handle */
	DCIDirPathStream   *dpstr;                /* direct path stream handle */
	ub4 rowoff;
	ub4 rowoff_all = 0;
	ub2 coloff;
	ub4 nrows;
	ub4 times = 0;
	ub4 rows_temp;
	DCIError           *errhp = NULL;
	sword	retcode;
	memset(error, 0, sizeof(ErrorInfo_t));
	DCI_CHECK1(error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));
	CleanError(error);

	if (table == NULL || col == NULL)
	{
		SET_ERROR(error, NORMAL_ERR, "非法的空指针");
	}
	if (cols == 0)
	{
		SET_ERROR(error, NORMAL_ERR, "非法的列数");
	}

	if (!SetTableInfo(schema, table, col, cols, rows, &nrows, input_type, &dpctx, &dpca, &dpstr, &colLstDesc,  errhp, error))
	{
		RETURN_FALSE_DPLS(DCI_ERROR);
	}
	while (rows)
	{
		if (rows > nrows)
		{
			rows_temp = nrows;
			rows -= nrows;
		}
		else
		{
			rows_temp = rows;
			rows = 0;
		}
		DCI_CHECK_EX(error, DCIDirPathColArrayReset(dpca, errhp));
		RETURN_FALSE_DPLS(retcode);
		DCI_CHECK_EX(error, DCIDirPathStreamReset(dpstr, errhp));
		RETURN_FALSE_DPLS(retcode);
		for (rowoff = 0; rowoff < rows_temp; rowoff++)
		{
			for (coloff = 0; coloff < cols; coloff++)
			{
				if (col[coloff].indp && col[coloff].indp[rowoff_all] == -1)
				{
					DCI_CHECK_EX(error,
						DCIDirPathColArrayEntrySet(dpca, errhp,
						rowoff, coloff,
						(ub1*)col[coloff].data + (rowoff_all * col[coloff].data_size),
						col[coloff].data_size, DCI_DIRPATH_COL_NULL));
					RETURN_FALSE_DPLS(retcode);
				}
				else
				{
					DCI_CHECK_EX(error,
						DCIDirPathColArrayEntrySet(dpca, errhp,
						rowoff, coloff,
						(ub1*)col[coloff].data + (rowoff_all * col[coloff].data_size),
						col[coloff].data_size, DCI_DIRPATH_COL_COMPLETE));
					RETURN_FALSE_DPLS(retcode);
				}
			}
			rowoff_all++;
		}
		DCI_CHECK_EX(error,
			DCIDirPathColArrayToStream(dpca, dpctx,
			dpstr, errhp, rows_temp, 0));
		RETURN_FALSE_DPLS(retcode);

		DCI_CHECK_EX(error,
			DCIDirPathLoadStream(dpctx, dpstr, errhp));
		RETURN_FALSE_DPLS(retcode);
	}
	DCI_CHECK_EX(error,
		DCIDirPathDataSave(dpctx, errhp, (ub4)DCI_DIRPATH_DATASAVE_FINISH));
	RETURN_FALSE_DPLS(retcode);

	/* free up server data structures for the load. */
	DCI_CHECK_EX(error,
		DCIDirPathFinish(dpctx, errhp));
	RETURN_FALSE_DPLS(retcode);

	FreeDirPathHandle(colLstDesc, dpctx, dpca, dpstr);
	DCIHandleFree((dvoid *)errhp, DCI_HTYPE_ERROR);
	errhp = NULL;
	return TRUE;
}

#define RETURN_FALSE_ES(R) \
	if (R != DCI_SUCCESS) \
{ \
	if(ppbnd) \
	dcisg_free(ppbnd);\
	ppbnd = NULL;\
	DCIHandleFree(stmthp, DCI_HTYPE_STMT);\
	DCITransRollback(svchp, NULL, (ub4)DCI_DEFAULT);\
	DCIHandleFree((dvoid *)errhp, DCI_HTYPE_ERROR);\
	errhp = NULL;\
	return FALSE;\
} \
else

/*******************************************************************************
**
** Routine: ExecuteSql
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: execute the sql statement
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
bool CDci::ExecuteSql(const char *sql, ParamAttr_t *in_param, ub2 in_param_nums, ub4 iters, ErrorInfo_t *error)
{
	sword	retcode;
	DCIBind **ppbnd = NULL;
	DCIStmt * stmthp = NULL;
	DCIError           *errhp = NULL;
	memset(error, 0, sizeof(ErrorInfo_t));
	if (sql == NULL)
	{
		SET_ERROR(error, NORMAL_ERR, "NULL 串");
		return FALSE;
	}
	if (in_param == NULL && in_param_nums != 0)
	{
		SET_ERROR(error, NORMAL_ERR, "非法的输入参数结构，指定的参数的个数，但是未指定参数信息(in_param值为NULL)");
		return FALSE;
	}

	if (!is_connect)
	{
		SET_ERROR(error, 3113, "数据库连接未建立");
		return FALSE;
	}

	DCI_CHECK1(error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));
	DCI_CHECK1(error, DCIHandleAlloc(envhp, (void**)&stmthp, DCI_HTYPE_STMT, 0, 0));
	DCI_CHECK(error, DCIStmtPrepare(stmthp, errhp, (DciText*)sql, strlen(sql), DCI_NTV_SYNTAX, DCI_DEFAULT));

	if (in_param)
	{
		ppbnd = (DCIBind **)dcisg_malloc(sizeof(DCIBind *) * (in_param_nums));
		if (ppbnd == NULL)
		{
			SET_ERROR(error, NORMAL_ERR, "内存不足");
			RETURN_FALSE_ES(DCI_ERROR);
		}
		memset(ppbnd, 0, sizeof(DCIBind *) * in_param_nums);
		if(!BindParam(stmthp, ppbnd, in_param, in_param_nums, errhp, error))
		{
			RETURN_FALSE_ES(DCI_ERROR);
		}
	}
	DCI_CHECK_EX(error, StmtExecute(svchp, stmthp, errhp, iters, DCI_DEFAULT));
	RETURN_FALSE_ES(retcode);

	errhp = NULL;
	if(ppbnd)
		dcisg_free(ppbnd);
		ppbnd = NULL;
	DCITransCommit(svchp, NULL, DCI_DEFAULT);
	RETURN_TRUE;
}

#define RETURN_FALSE_EP(R) \
	if (R != DCI_SUCCESS) \
{ \
	DCIHandleFree(stmtp_sql, DCI_HTYPE_STMT);\
	DCIHandleFree(stmtp_result, DCI_HTYPE_STMT);\
	DCIHandleFree((dvoid *)errhp, DCI_HTYPE_ERROR);\
	DCITransRollback(svchp, NULL, (ub4)DCI_DEFAULT);\
	errhp = NULL;\
	if(ppbnd) \
	dcisg_free(ppbnd);\
	return FALSE;\
} \
else

/*******************************************************************************
**
** Routine: ExecuteProcedure
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: execute the procedure
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
bool CDci::ExecuteProcedure(const char *package_name,const char *procedure_name,
							ParamAttr_t *in_param,ub2 in_param_nums, ErrorInfo_t *error)
{
	char	sql[1024];
	int		i;
	int		curnum = 0;
	DCIStmt *stmtp_sql = NULL;
	DCIStmt *stmtp_result = NULL;
	DCIBind **ppbnd = NULL;
	ResultAttr_t *resultp = NULL;
	DCIError           *errhp = NULL;
	sword	retcode;
	memset(error, 0, sizeof(ErrorInfo_t));
	if (!is_connect)
	{
		SET_ERROR(error, 3113, "数据库连接未建立");
		return FALSE;
	}

	if (in_param == NULL && in_param_nums != 0)
	{
		SET_ERROR(error, NORMAL_ERR, "非法的输入参数结构，指定的参数的个数，但是未指定参数信息(in_param值为NULL)");
		RETURN_FALSE_EP(DCI_ERROR);
	}
	CreateProcedureSql(sql,package_name,procedure_name, in_param, in_param_nums);
	DCIHandleAlloc(envhp, (void**)&stmtp_sql, DCI_HTYPE_STMT, 0, 0);
	DCIHandleAlloc(envhp, (void**)&stmtp_result, DCI_HTYPE_STMT, 0, 0);

	DCI_CHECK1(error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));
	//	strcpy(sql, "p1(1, '22222')");
	DCI_CHECK_EX(error,
		DCIStmtPrepare(stmtp_sql, errhp, (DciText*)sql, strlen(sql), DCI_NTV_SYNTAX, DCI_DEFAULT));
	RETURN_FALSE_EP(retcode);

	ppbnd = (DCIBind **)dcisg_malloc(sizeof(DCIBind *) * (in_param_nums));
	if (ppbnd == NULL)
	{
		SET_ERROR(error, NORMAL_ERR, "内存不足");
		RETURN_FALSE_EP(DCI_ERROR);
	}
	memset(ppbnd, 0, sizeof(DCIBind *) * in_param_nums);

	for (i = 0; i < in_param_nums; i++)
	{
		if (in_param[i].data_type == DCI_CUR)
		{
			if (curnum > 0)  //不允许有多个输出结果集
			{
				SET_ERROR(error, NORMAL_ERR, "过程暂只支持最多一个游标参数");
				RETURN_FALSE_EP(DCI_ERROR);
			}
			DCI_CHECK_EX(error,
				DCIBindByPos(stmtp_sql, &ppbnd[i], errhp, i+1, &stmtp_result, 0,
				DCI_RSET, NULL, NULL, NULL, 0, NULL, DCI_DEFAULT));
			resultp = (ResultAttr_t*)in_param[i].data;
			if (resultp == NULL)
			{
				SET_ERROR(error, NORMAL_ERR, "未绑定游标结果集结构");
				RETURN_FALSE_EP(DCI_ERROR);
			}
			memset(resultp, 0, sizeof(ResultAttr_t));
			curnum++;
		}
		else
		{
			DCI_CHECK_EX(error,
				DCIBindByPos(stmtp_sql, &ppbnd[i], errhp, i+1, in_param[i].data, in_param[i].data_size,
				in_param[i].data_type, in_param[i].indp, NULL, NULL, 0, NULL, DCI_DEFAULT));
		}
		RETURN_FALSE_EP(retcode);
	}

	DCI_CHECK_EX(error, StmtExecute(svchp, stmtp_sql, errhp, 1, DCI_DEFAULT));
	RETURN_FALSE_EP(retcode);

	if (!GetStmtResult(stmtp_result, resultp, errhp, error))
	{
		RETURN_FALSE_EP(DCI_ERROR);
	}
	DCITransCommit(svchp, NULL, DCI_DEFAULT);
	DCIHandleFree(stmtp_sql, DCI_HTYPE_STMT);
	DCIHandleFree(stmtp_result, DCI_HTYPE_STMT);
	DCIHandleFree((dvoid *)errhp, DCI_HTYPE_ERROR);
	errhp = NULL;
	dcisg_free(ppbnd);
	return TRUE;
}

/*******************************************************************************
**
** Routine: CreateProcedureSql
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: create the procedure and sql
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
void CDci::CreateProcedureSql(char *sql,const char *package_name,const char *procedure_name, ColAttr_t *in_param, ub2 in_param_nums)
{
	char	*sqltemp;
	char	packageName[200];
	int		i;
	sql[0] = '\0';
	sqltemp = sql;
	packageName[0] = '\0';

	if (package_name && strlen(package_name) > 0)
	{
		sprintf(packageName, "%s.", package_name);
	}
	if (in_param_nums == 0)
	{
		sprintf(sqltemp, "begin %s%s; end;", packageName,procedure_name);
		return;
	}
	sprintf(sqltemp, "begin %s%s(", packageName,procedure_name);
	sqltemp = sqltemp + strlen(sqltemp);
	for (i = 1; i <= in_param_nums; i++)
	{
		if (i == in_param_nums)
		{
			sprintf(sqltemp, ":%d); end;", i);
		}
		else
		{
			sprintf(sqltemp, ":%d,", i);
		}
		sqltemp = sqltemp + strlen(sqltemp);
	}
}

/*******************************************************************************
**
** Routine: GetResultColAttr
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: get the result of column's attribute.
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
bool CDci::GetResultColAttr(DCIStmt *stmtp_result, ColAttr_t *col_attr, ub2 cols, DCIError *errhp, ErrorInfo_t *error)
{
	DCIParam	*col_param;
	char		*name=NULL;
	ub4			size;
	sword	retcode;
	for (ub2 pos = 0; pos < cols; pos++)
	{
		DCI_CHECK_EX(error,
			DCIParamGet(stmtp_result, DCI_HTYPE_STMT, errhp, (void **) &col_param, pos+1));
		if(retcode != DCI_SUCCESS)
		{
			return FALSE;
		}
        DCI_CHECK_EX(error,
			DCIAttrGet(col_param, DCI_DTYPE_PARAM,(dvoid*) &name, (ub4 *) &size, DCI_ATTR_NAME,errhp));
		memcpy(col_attr[pos].col_name, name, size);
		col_attr[pos].col_name[size] ='\0';
		DCI_CHECK_EX(error,
			DCIAttrGet(col_param, DCI_DTYPE_PARAM, &col_attr[pos].data_type, 0, DCI_ATTR_DATA_TYPE, errhp));
        DCI_CHECK_EX(error,
			DCIAttrGet(col_param, DCI_DTYPE_PARAM,&col_attr[pos].data_size , 0, DCI_ATTR_DATA_SIZE, errhp));
		DCI_CHECK_EX(error,
			DCIAttrGet(col_param, DCI_DTYPE_PARAM, &col_attr[pos].precision, 0, DCI_ATTR_PRECISION, errhp));
		DCI_CHECK_EX(error,
			DCIAttrGet(col_param, DCI_DTYPE_PARAM, &col_attr[pos].scale, 0, DCI_ATTR_SCALE, errhp));

		if (col_attr[pos].data_type == SQLT_NUM)
		{
			if (col_attr[pos].scale == 0)
			{
				col_attr[pos].data_type = DCI_INT;
				col_attr[pos].data_size = sizeof(int);
			}
			else
			{
				col_attr[pos].data_type = DCI_FLT;//float
				col_attr[pos].data_size = sizeof(float);
			}
		}
		if (col_attr[pos].data_type == SQLT_TIMESTAMP ||
			col_attr[pos].data_type == SQLT_DATE ||
			col_attr[pos].data_type == SQLT_DAT ||
			col_attr[pos].data_type == SQLT_TIME)
		{
			col_attr[pos].data_type = SQLT_ODT;
			col_attr[pos].data_size = sizeof(DCIDate);
		}
	}

	return TRUE;
}

/*******************************************************************************
**
** Routine: AllocResultMemory
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: allocate the memory for result
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
bool CDci::AllocResultMemory(void **ppresult, void **ppind, ColAttr_t *col_attr, ub2 cols, ErrorInfo_t *error)
{
	DCIError *errhp = NULL;
	for (int i = 0; i < cols; i++)
	{
		ppresult[i] = dcisg_malloc(MAX_FETCH_ROWS * col_attr[i].data_size);
		if (ppresult[i] == NULL)
		{
			SET_ERROR(error, NORMAL_ERR, "内存不足");
			return FALSE;
		}
		ppind[i] = dcisg_malloc(MAX_FETCH_ROWS * sizeof(sb2));
		if (ppind[i] == NULL)
		{
			SET_ERROR(error, NORMAL_ERR, "内存不足");
			return FALSE;
		}
		memset(ppresult[i], 0, MAX_FETCH_ROWS * col_attr[i].data_size);
		memset(ppind[i], 0, MAX_FETCH_ROWS * sizeof(sb2));
	}

	return TRUE;
}

/*******************************************************************************
**
** Routine: FreeResultMemory
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: dcisg_free the memory for data result
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
bool CDci::FreeResultMemory(void **ppresult, void **ppind, ub2 cols)
{
	if (ppresult)
	{
		for (int i = 0; i < cols; i++)
		{
			if(ppresult[i] != NULL)
			{
				dcisg_free(ppresult[i]);
				ppresult[i] = NULL;
			}
		}
		dcisg_free(ppresult);
		ppresult = NULL;
	}
	if (ppind)
	{
		for (int i = 0; i < cols; i++)
		{
			if (ppind[i] != NULL)
			{
				dcisg_free(ppind[i]);
				ppind[i] = NULL;
			}
		}
		dcisg_free(ppind);
		ppind = NULL;
	}

	return TRUE;
}

/*******************************************************************************
**
** Routine: BindResult
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: Bind the data result.
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
bool CDci::BindResult(DCIStmt *stmtp_result, DCIDefine **ppdefn, void **ppresult, void** ppind, ColAttr_t *col_attr, ub2 cols, DCIError *errhp, ErrorInfo_t *error)
{
	sword	retcode;
	for (ub2 pos = 0; pos < cols; pos++)
	{
		DCI_CHECK_EX(error,
			DCIDefineByPos(stmtp_result, &ppdefn[pos], errhp, pos+1, ppresult[pos],
			col_attr[pos].data_size, col_attr[pos].data_type, ppind[pos], NULL, NULL, DCI_DEFAULT));
		DCI_CHECK_EX(error,
			DCIDefineArrayOfStruct(ppdefn[pos], errhp, col_attr[pos].data_size, sizeof(sb2), 0, 0));
	}

	return TRUE;
}

/*******************************************************************************
**
** Routine: MergerProcedureResult
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: merge the data result.
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
bool CDci::MergerProcedureResult(void **ppresult, void **ppind, ColAttr_t *col_attr, ub2 cols, ub4 copy_rows, ub4 last_rows, ErrorInfo_t *error)
{
	void *tempp;
	DCIError *errhp = NULL;
	for (ub2 pos = 0; pos < cols; pos++)
	{
		tempp = dcisg_malloc((copy_rows + last_rows) * col_attr[pos].data_size);
		if (tempp == NULL)
		{
			SET_ERROR(error, NORMAL_ERR, "内存不足");
			return FALSE;
		}
		if (col_attr[pos].data)
		{
			memcpy(tempp, col_attr[pos].data, last_rows * col_attr[pos].data_size);
			memcpy((char*)tempp + last_rows * col_attr[pos].data_size, ppresult[pos], copy_rows*col_attr[pos].data_size);
			dcisg_free(col_attr[pos].data);
			col_attr[pos].data = NULL;
		}
		else
		{
			memcpy((char*)tempp + last_rows * col_attr[pos].data_size, ppresult[pos], copy_rows*col_attr[pos].data_size);
		}
		col_attr[pos].data = tempp;

		tempp = dcisg_malloc((copy_rows + last_rows) * sizeof(ub2));
		if (tempp == NULL)
		{
			SET_ERROR(error, NORMAL_ERR, "内存不足");
			return FALSE;
		}

		if (col_attr[pos].indp)
		{
			memcpy(tempp, col_attr[pos].indp, last_rows * sizeof(sb2));
			memcpy((char*)tempp + last_rows * sizeof(sb2), ppind[pos], copy_rows * sizeof(sb2));
			dcisg_free(col_attr[pos].indp);
		}
		else
		{
			memcpy((char*)tempp + last_rows * sizeof(sb2), ppind[pos], copy_rows * sizeof(sb2));
		}
		col_attr[pos].indp = (sb2*)tempp;
		memset(ppresult[pos], 0, MAX_FETCH_ROWS * col_attr[pos].data_size);
		memset(ppind[pos], 0, MAX_FETCH_ROWS * sizeof(sb2));
	}

	return TRUE;
}

/*******************************************************************************
**
** Routine: SqltLen
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: get the datatype's length.
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
int CDci::SqltLen(int sqlt, int len)
{
	int length = 0;

	switch (sqlt)
	{
	case 1:	//SQLT_CHAR DCI_CHAR
		length = len + 1;
		break;
	case 2:	//SQLT_ NUM DCI_NUM
		length = 22;
		break;
	case 3:	//SQLT_INT DCI_INT
		length = len;
		break;
	case 4:	//SQLT_FLT DCI_FLT
		length = len;
		break;
	case 5:	//SQLT_STR DCI_STR
		length = len + 1;
		break;
	case 6:	//SQLT_VNU
		length = len;
		break;
	case 8:	//SQLT_LNG
		length = 22;
		break;
	case 9:	//SQLT_VCS
		length = len + 1;
		break;
	case 12:	//SLQT_DAT
		length = 7+1;//7字节表示是ORACLE时间类型的长度，加1是为了取四字节对齐
		break;
	case 96:	//SLQT_AFC
		length = len +1;
		break;
	case 97:	//SLQT_AVC
		length = len +1;
		break;
	case 104://SLQT_RDD
		length = len +1;
		break;
	default:
		//length = -1；
		length = len; //danath 20120223 除去已处理的类型长度，其它的类型长度如返回-1会导致程序崩溃
		break;
	}

	return length;
}

/*******************************************************************************
**
** Routine:     ReadData
**
** Author: xiemei
**
** DATE :2009/03/11
**
** Description: read the data from database according the query.
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
int CDci::ReadData (IN const char *query, OUT int *rec_num, OUT int *attr_num,OUT struct ColAttr ** attrs,OUT char **buf,ErrorInfo_t* error)
{
	memset(error, 0, sizeof(ErrorInfo_t));
	return ReadData(query, -1, rec_num, attr_num, attrs, buf, error);
}

//在这里是针对达梦数据库的语法做的特殊处理，用来返回TOP前多少条的记录
void CDci::ReplaceTopSql(const char* query, char **sql, int top_number)
{
	int len = strlen(query);
	const char* query_temp = query;
	const char* query_end = NULL;
	char *sql_temp = NULL;
	int flag = 0;
	sql_temp = (char*)dcisg_malloc(len + 200);
	if (sql_temp == NULL || !is_dm_database)
	{
		return;
	}
	memset(sql_temp, 0, len + 200);
	while (*query_temp != '\0' && (*query_temp == ' ' || *query_temp == '\n' || *query_temp == '\r' || *query_temp == '\t'))
		query_temp++;
	if (strlen(query_temp) <= 10 || strstr(query_temp, "union") != NULL)
	{
		dcisg_free(sql_temp);
		return;
	}
	memcpy(sql_temp, query_temp, 6);//select
	_strlwr(sql_temp);
	if (stricmp(sql_temp, "select") == 0 && (query_temp[6] == ' ' || query_temp[6] == '\n' || query_temp[6] == '\r' || query_temp[6] == '\t'))
	{
		const char* query_temp1 = query_temp + 6;
		char top[100];
		memset(top, 0, 100);

		while (*query_temp1 != '\0' && (*query_temp1 == ' ' || *query_temp1 == '\n' || *query_temp1 == '\r' || *query_temp1 == '\t'))
			query_temp1++;
		memcpy(top, query_temp1, 3);//top
		_strlwr(top);
		top[3] = '\0';
		if (stricmp(top, "top") == 0 && (query_temp1[3] == ' ' || query_temp1[3] == '\n' || query_temp1[3] == '\r' || query_temp1[3] == '\t'))
		{
			dcisg_free(sql_temp);
			return;
		}
		memcpy(top, query_temp1, 5);//count
		_strlwr(top);
		top[5] = '\0';
		if (stricmp(top, "count") == 0 && (query_temp1[5] == '(' || query_temp1[5] == ' ' || query_temp1[5] == '\n' || query_temp1[5] == '\r' || query_temp1[5] == '\t'))
		{
			*sql = sql_temp;
			strcpy(sql_temp, "SET TRANSACTION ISOLATION LEVEL READ UNCOMMITTED;");
			sql_temp += strlen(sql_temp);
			const char* query_temp2 = query_temp1 + 5;
			while (*query_temp2 != '\0' && (*query_temp2 == '(' || *query_temp2 == '*' || *query_temp2 == ')' || *query_temp2 == ' ' || *query_temp2 == '\n' || *query_temp2 == '\r' || *query_temp2 == '\t'))
				query_temp2++;
			memcpy(top, query_temp2, 4);//from
			_strlwr(top);
			top[4] = '\0';
			if (stricmp(top, "from") == 0 && (query_temp2[4] == ' ' || query_temp2[4] == '\n' || query_temp2[4] == '\r' || query_temp2[4] == '\t'))
			{
				query_temp2 += 4;
				while (*query_temp2 != '\0' && (*query_temp2 == ' ' || *query_temp2 == '\n' || *query_temp2 == '\r' || *query_temp2 == '\t'))
					query_temp2++;
				if (query_temp2[0] == '(')
				{
					query_temp2 += 1;
					while (*query_temp2 != '\0' && (*query_temp2 == ' ' || *query_temp2 == '\n' || *query_temp2 == '\r' || *query_temp2 == '\t'))
						query_temp2++;
					memcpy(top, query_temp2, 6);//select
					_strlwr(top);
					top[6] = '\0';
					if (stricmp(top, "select") == 0 && (query_temp2[6] == ' ' || query_temp2[6] == '\n' || query_temp2[6] == '\r' || query_temp2[6] == '\t'))
					{
						query_temp2 += 6;
		//				_strlwr(query_temp2);
						query_temp2 = strstr(query_temp2, " from ");
						if (query_temp2)
						{
							int len_temp = strlen(query_temp2);
							char* xxx_temp = NULL;
							strcpy(sql_temp, "select count(*)");
							memcpy(sql_temp + 15, query_temp2, len_temp);
							xxx_temp = sql_temp + 15 + len_temp -1;
							while (*xxx_temp == ' ' || *xxx_temp == '\n' || *xxx_temp == '\r' || *xxx_temp == '\t' || *xxx_temp == ';')
								xxx_temp--;
							if (xxx_temp[0] == ')')
							{
								xxx_temp[0] = '\0';
								flag = 1;
								sql_temp += strlen(sql_temp);
							}
						}
					}
				}
			}
			if (flag == 0)
			{
				memcpy(sql_temp, query, len);
				sql_temp += len;
			}

			query_end = sql_temp-1;
			while (*query_end == ' ' || *query_end == '\n' || *query_end == '\r' || *query_end == '\t')
				query_end--;
			if (*query_end == ';')
			{
				strcpy(sql_temp, " SET TRANSACTION ISOLATION LEVEL READ COMMITTED;");
			}
			else
			{
				strcpy(sql_temp, ";SET TRANSACTION ISOLATION LEVEL READ COMMITTED;");
			}
			return;
		}
// 		if (top_number == -1)
// 		{
// 			dcisg_free(sql_temp);
// 			return;
// 		}
// 		*sql = sql_temp;
// 		sprintf(sql_temp + 6, " top %d ", top_number);
// 		sql_temp += strlen(sql_temp);
// 		strcpy(sql_temp, query_temp + 7);
// 		return;
	}
	dcisg_free(sql_temp);
	sql_temp = NULL;
}
/*******************************************************************************
**
** Routine:     ReadData
**
** Author: xiemei
**
** DATE :2009/05/23
**
** Description: read the data from database according the query.
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
int CDci::ReadData (IN const char *query,IN int top_number, OUT int *rec_num, OUT int *attr_num,OUT struct ColAttr ** attrs,OUT char **buf,ErrorInfo_t* error)
{
	int		i, reclen = 0,	col_num = 0, col_name_len = 0;
	DCIDefine	*defhp[COL_NUM];
	sword		ind[COL_NUM];
	char		*pdata = NULL;
	sb4	parm_status;
	DCIParam	*mypard = (DCIParam *) 0;
	ub4	dtype = 0;
	ub4	col_width =0 ;
	ub4     dscale = 0;
	ub4		dprecision = 0;
	DciText	*col_name;
	char *tempp=NULL,*tempp_last=NULL;
	int rows=0,last_rows=0;
	int g_AttrNum; // 属性个数
	ColAttr_t *g_AryAttr = NULL;
	char *data_addr = NULL;
	int retcode;
	DCIStmt          *stmthp = NULL;
	DCIError           *errhp = NULL;
	char * sql_temp = NULL;
	*rec_num = 0;
	*attr_num = 0;
	*attrs = NULL;
	*buf = NULL;
	memset(error, 0, sizeof(ErrorInfo_t));
	if (is_connect == FALSE)
	{
		SET_ERROR(error, 3113, "服务器已经断开");
		return FALSE;
	}
//	ReplaceTopSql(query, &sql_temp, top_number);//在这里是针对达梦数据库的语法做的特殊处理，用来返回TOP前多少条的记录
	if (top_number == -1)
	{
		top_number = 0x7FFFFFFF;
	}

	if (rec_num == NULL || attr_num == NULL || attrs ==NULL || buf == NULL || error == NULL)
	{
		SET_ERROR(error, -1, "传入了非法的空指针");
		return FALSE;
	}
	DCI_CHECK1(error, DCIHandleAlloc(envhp, (void**)&stmthp, DCI_HTYPE_STMT, 0, 0));
	DCI_CHECK1(error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));
	if (sql_temp != NULL)
	{
		//在这里是针对达梦数据库的语法做的特殊处理，用来返回TOP前多少条的记录
		DCI_CHECK(error, DCIStmtPrepare(stmthp, errhp, (DciText *)sql_temp, (ub4)strlen(sql_temp), (ub4)DCI_NTV_SYNTAX, (ub4)DCI_DEFAULT));
		dcisg_free(sql_temp);
		sql_temp = NULL;
	}
	else
	{
		DCI_CHECK(error, DCIStmtPrepare(stmthp, errhp, (DciText *)query, (ub4)strlen(query), (ub4)DCI_NTV_SYNTAX, (ub4)DCI_DEFAULT));
	}

	DCI_CHECK_EX(error, StmtExecute(svchp, stmthp, errhp, (ub4)0, (ub4)DCI_DEFAULT));
	if (retcode ==DCI_ERROR)
	{
		DCIHandleFree(stmthp, DCI_HTYPE_STMT);
		DCIHandleFree(errhp, DCI_HTYPE_ERROR);
		DCITransRollback(svchp, NULL, (ub4)DCI_DEFAULT);
		return FALSE;
	}

	DCI_CHECK(error, DCIAttrGet(stmthp, DCI_HTYPE_STMT, &col_num, 0, DCI_ATTR_PARAM_COUNT, errhp));
	if (col_num > COL_NUM)
	{
		DCIHandleFree(stmthp, DCI_HTYPE_STMT);
		DCIHandleFree(errhp, DCI_HTYPE_ERROR);
		SET_ERROR(error, -1, "查询结果集列数太多，目前最大支持1500列");
		return FALSE;
	}
	//g_AryAttr = (ColAttrS *)calloc(col_num, sizeof(ColAttr));
	g_AryAttr = (struct ColAttr *)calloc(col_num, sizeof(ColAttr));
	g_AttrNum = 0;

	/* Request a parameter descriptor for position 1 in the select-list */
	int counter = 1;
	parm_status = DCIParamGet((dvoid *)stmthp, DCI_HTYPE_STMT, errhp, (dvoid **)&mypard, (ub4) counter);
	/* Loop only if a descriptor was successfully retrieved for
	* current position, starting at 1.
	*/
	reclen = 0;
	while (parm_status == DCI_SUCCESS)
	{
		/* Retrieve the column name attribute */
		DCI_CHECK_EX(error, DCIAttrGet((dvoid*) mypard, (ub4) DCI_DTYPE_PARAM,
			(dvoid**) &col_name, (ub4 *) &col_name_len,
			(ub4) DCI_ATTR_NAME,(DCIError *) errhp ));

		/* Retrieve the datatype attribute */
		DCI_CHECK_EX(error, DCIAttrGet((dvoid*) mypard, (ub4) DCI_DTYPE_PARAM,
			(dvoid*) &dtype, (ub4 *) 0, (ub4) DCI_ATTR_DATA_TYPE,
			(DCIError *) errhp));

		//col_width = 0;
		/* Retrieve the length semantics for the column */
		//char_semantics = 0;


		/* Retrieve the column width in bytes */
		DCI_CHECK_EX(error, DCIAttrGet((dvoid*) mypard, (ub4) DCI_DTYPE_PARAM,
			(dvoid*) &col_width,(ub4 *) 0, (ub4) DCI_ATTR_DATA_SIZE,
			(DCIError *) errhp ));

		if (dtype == DCI_NUM)
		{
			DCI_CHECK_EX(error, DCIAttrGet((dvoid*) mypard, (ub4) DCI_DTYPE_PARAM,
				(dvoid*) &dscale,(ub4 *) 0, (ub4) DCI_ATTR_SCALE,
				(DCIError *) errhp ));
			DCI_CHECK_EX(error, DCIAttrGet((dvoid*) mypard, (ub4) DCI_DTYPE_PARAM,
				(dvoid*) &dprecision,(ub4 *) 0, (ub4) DCI_ATTR_PRECISION,
				(DCIError *) errhp ));

		}

		/* added to attr struct */
		memset(g_AryAttr[g_AttrNum].col_name,0, MAX_NAME_LEN);
		strncpy(g_AryAttr[g_AttrNum].col_name, (char *)col_name, col_name_len);
		//printf(" the column name is %s and type is %d and size is %d, precision is %d, scale is %d \n",
		//			col_name,dtype,col_width,dprecision,dscale);

		ConvertDciType((ub2 &)dtype, (ub2 &)col_width, dprecision, dscale);

		if (dtype == DCI_BLOB || dtype == DCI_CLOB)
		{
			DCIHandleFree(stmthp, DCI_HTYPE_STMT);
			DCIHandleFree(errhp, DCI_HTYPE_ERROR);
			DCITransRollback(svchp, NULL, (ub4)DCI_DEFAULT);
			dcisg_free(g_AryAttr);
			SET_ERROR(error, -1, "查询结果集中存在大字段，请使用ReadDateEx来获取这个结果集");
			return FALSE;
		}

		g_AryAttr[g_AttrNum].data_type = dtype;
		g_AryAttr[g_AttrNum].data_size = SqltLen(dtype, col_width);
		reclen = reclen + g_AryAttr[g_AttrNum].data_size;
		g_AttrNum++;

		/* increment counter and get next descriptor, if there is one */
		counter++;
		parm_status = DCIParamGet((dvoid *)stmthp, DCI_HTYPE_STMT, errhp,
			(dvoid **)&mypard, (ub4) counter);
	}

	// dcisg_malloc space
	pdata = data_addr = (char*)dcisg_malloc(RECNUM * reclen);

	// added by Lijunliang for dcisg_malloc error 2008/07/04
	if (data_addr == NULL)
	{
		DCIHandleFree(stmthp,DCI_HTYPE_STMT);
		DCIHandleFree(errhp, DCI_HTYPE_ERROR);
		DCITransRollback(svchp, NULL, (ub4)DCI_DEFAULT);
		dcisg_free(g_AryAttr);
		return FALSE;
	}
	memset((char *)data_addr,0, RECNUM*reclen);

	/* define */
	for (i = 1;i <= g_AttrNum;i++)
	{
		DCI_CHECK_EX(error, DCIDefineByPos(stmthp, &defhp[i-1], errhp,
			i, (ub1 *)pdata, g_AryAttr[i-1].data_size,
			g_AryAttr[i-1].data_type, &ind[i-1],/*(dvoid *)*/0,(ub2 *)0,DCI_DEFAULT));
		pdata = pdata + g_AryAttr[i-1].data_size;
	}
	for (i = 1;i <= g_AttrNum;i++)
	{
		DCI_CHECK_EX(error, DCIDefineArrayOfStruct(defhp[i-1], errhp,
												reclen, 0, 0, 0));
	}
	int temp_rows = 0;
	temp_rows = top_number>RECNUM?RECNUM:top_number;
	DCI_CHECK_EX(error,
		DCIStmtFetch(stmthp, errhp,temp_rows, DCI_FETCH_NEXT, DCI_DEFAULT));
	while (retcode == DCI_SUCCESS)
	{
		DCI_CHECK_EX(error,
			DCIAttrGet(stmthp, DCI_HTYPE_STMT, &rows, 0, DCI_ATTR_ROW_COUNT, errhp));


		tempp = (char*)dcisg_malloc(rows*reclen);
		if (tempp == NULL)
		{
			SET_ERROR(error, NORMAL_ERR, "内存不足");
			DCIHandleFree(stmthp,DCI_HTYPE_STMT);
			DCITransRollback(svchp, NULL, (ub4)DCI_DEFAULT);
			DCIHandleFree(errhp, DCI_HTYPE_ERROR);
			dcisg_free(g_AryAttr);
			dcisg_free(data_addr);
			return FALSE;
		}
		if(tempp_last != NULL)
		{
			memcpy(tempp,tempp_last,last_rows*reclen);
			memcpy(tempp+last_rows*reclen,data_addr,(rows-last_rows)*reclen);
			dcisg_free(tempp_last);
			tempp_last = NULL;
		}
		else
		{
			memcpy(tempp+last_rows*reclen,data_addr,(rows-last_rows)*reclen);
		}
		last_rows = rows;
		tempp_last = tempp;
		temp_rows = (top_number - last_rows)>RECNUM?RECNUM:(top_number - last_rows);
		if(temp_rows == 0)
		{
			DCIHandleFree(stmthp,DCI_HTYPE_STMT);
			DCITransCommit(svchp, errhp, 0);
			DCIHandleFree(errhp, DCI_HTYPE_ERROR);
			dcisg_free(data_addr);
			data_addr = NULL;
			*attr_num = g_AttrNum;
			*attrs = g_AryAttr;
			*rec_num = rows;
			*buf = tempp;
			return TRUE;
		}
		memset((char *)data_addr,0, RECNUM*reclen);
		DCI_CHECK_EX(error,
			DCIStmtFetch(stmthp, errhp, temp_rows, DCI_FETCH_NEXT, DCI_DEFAULT));
	}

	if (retcode != DCI_NO_DATA)
	{
		DCIHandleFree(stmthp,DCI_HTYPE_STMT);
		DCITransRollback(svchp, NULL, (ub4)DCI_DEFAULT);
		DCIHandleFree(errhp, DCI_HTYPE_ERROR);
		dcisg_free(g_AryAttr);
		dcisg_free(data_addr);
		return FALSE;
    }
    // retcode==DCI_NO_DATA
	DCI_CHECK_EX(error,
		DCIAttrGet(stmthp, DCI_HTYPE_STMT, &rows, 0, DCI_ATTR_ROW_COUNT, errhp));
	if (last_rows < rows)
	{
		tempp = (char*)dcisg_malloc(rows*reclen);
		if (tempp == NULL)
		{
			SET_ERROR(error, NORMAL_ERR, "内存不足");
			DCIHandleFree(stmthp,DCI_HTYPE_STMT);
			DCITransRollback(svchp, NULL, (ub4)DCI_DEFAULT);
			DCIHandleFree(errhp, DCI_HTYPE_ERROR);
			dcisg_free(g_AryAttr);
			dcisg_free(data_addr);
			return FALSE;
		}
		if(tempp_last != NULL)
		{
			memcpy(tempp,tempp_last,last_rows*reclen);
			memcpy(tempp+last_rows*reclen,data_addr,(rows-last_rows)*reclen);
			dcisg_free(tempp_last);
			tempp_last = NULL;
		}
		else
		{
			memcpy(tempp+last_rows*reclen,data_addr,(rows-last_rows)*reclen);
		}
	}
	else
	{
		tempp = tempp_last;
	}

	*attr_num = g_AttrNum;
	*attrs = g_AryAttr;
	*rec_num = rows;
	*buf = tempp;
    /**********************************
	for (int ll=0;ll < *rec_num;ll++)
	for (int kk=0;kk< *attr_num;kk++)
	{
	if(kk==0)
	{
	//printf(" the dci data is %d \n", *(int *)(data_addr + (ll * (*attr_num) + kk) * g_AryAttr[kk].data_size));
	printf(" the dci data is %d and reclen is %d\n", *(int *)(*buf + ll *reclen),reclen);
	}
	}
	***********************/
	dcisg_free(data_addr);
	data_addr = NULL;
	DCIHandleFree((dvoid *)stmthp, DCI_HTYPE_STMT);
	DCITransCommit(svchp, errhp, 0);
	DCIHandleFree((dvoid *)errhp, DCI_HTYPE_ERROR);
	return TRUE;
}
int CDci::FreeReadDataNext(IN OUT void **stmthpp, IN OUT void **errhpp, IN struct ColAttr ** attrs, OUT char **buf)
{
	if (*stmthpp)
	{
		DCIHandleFree(*stmthpp, DCI_HTYPE_STMT);
	}
	DCITransCommit(svchp, NULL, (ub4)DCI_DEFAULT);
	if (*errhpp)
	{
		DCIHandleFree((dvoid *)*errhpp, DCI_HTYPE_ERROR);
	}
	dcisg_free(*attrs);
	dcisg_free(*buf);
	*attrs = NULL;
	*buf = NULL;
	return TRUE;
}
int CDci::ReadDataNext (IN OUT void **stmthpp, IN OUT void **errhpp, IN const char *query, OUT struct ColAttr ** attrs, OUT int *attr_num, OUT char **buf, ErrorInfo_t* error)
{
	int		i, reclen = 0,	col_num = 0, col_name_len = 0;
	DCIDefine	*defhp[COL_NUM];
	char		*pdata = NULL;
	sb4	parm_status;
	DCIParam	*mypard = (DCIParam *) 0;
	ub2		dtype, col_width;
	ub1     dscale;
	ub2		dprecision;
	DciText	*col_name;
	int g_AttrNum; // 属性个数
	ColAttr_t *g_AryAttr = NULL;
	int retcode;
	DCIStmt          *stmthp = NULL;
	DCIError           *errhp = NULL;
	memset(error, 0, sizeof(ErrorInfo_t));
	if (stmthpp == NULL || errhpp == NULL || attr_num == NULL || attrs ==NULL || buf == NULL || error == NULL)
	{
		SET_ERROR(error, -1, "传入了非法的空指针");
		return FALSE;
	}
	if (*stmthpp == NULL)
	{
		DCI_CHECK1(error, DCIHandleAlloc(envhp, (void**)&stmthp, DCI_HTYPE_STMT, 0, 0));
		DCI_CHECK1(error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));
		DCI_CHECK(error, DCIStmtPrepare(stmthp, errhp, (DciText *)query, (ub4)strlen(query), (ub4)DCI_NTV_SYNTAX, (ub4)DCI_DEFAULT));
		DCI_CHECK(error, StmtExecute(svchp, stmthp, errhp, (ub4)0, (ub4)DCI_DEFAULT));
		DCI_CHECK(error, DCIAttrGet(stmthp, DCI_HTYPE_STMT, &col_num, 0, DCI_ATTR_PARAM_COUNT, errhp));
		if (col_num > COL_NUM)
		{
			DCIHandleFree(stmthp, DCI_HTYPE_STMT);
			DCIHandleFree(errhp, DCI_HTYPE_ERROR);
			SET_ERROR(error, -1, "查询结果集列数太多，目前最大支持1500列");
			return FALSE;
		}
		g_AryAttr = (struct ColAttr *)calloc(col_num, sizeof(ColAttr));
		g_AttrNum = 0;

		/* Request a parameter descriptor for position 1 in the select-list */
		int counter = 1;
		parm_status = DCIParamGet((dvoid *)stmthp, DCI_HTYPE_STMT, errhp, (dvoid **)&mypard, (ub4) counter);
		/* Loop only if a descriptor was successfully retrieved for
		* current position, starting at 1.
		*/
		reclen = 0;
		while (parm_status == DCI_SUCCESS)
		{
			/* Retrieve the column name attribute */
			DCI_CHECK_EX(error, DCIAttrGet((dvoid*) mypard, (ub4) DCI_DTYPE_PARAM,
				(dvoid**) &col_name, (ub4 *) &col_name_len,
				(ub4) DCI_ATTR_NAME,(DCIError *) errhp ));

			/* Retrieve the datatype attribute */
			DCI_CHECK_EX(error, DCIAttrGet((dvoid*) mypard, (ub4) DCI_DTYPE_PARAM,
				(dvoid*) &dtype, (ub4 *) 0, (ub4) DCI_ATTR_DATA_TYPE,
				(DCIError *) errhp));

			//col_width = 0;
			/* Retrieve the length semantics for the column */
			//char_semantics = 0;


			/* Retrieve the column width in bytes */
			DCI_CHECK_EX(error, DCIAttrGet((dvoid*) mypard, (ub4) DCI_DTYPE_PARAM,
				(dvoid*) &col_width,(ub4 *) 0, (ub4) DCI_ATTR_DATA_SIZE,
				(DCIError *) errhp ));

			if (dtype == DCI_NUM)
			{
				DCI_CHECK_EX(error, DCIAttrGet((dvoid*) mypard, (ub4) DCI_DTYPE_PARAM,
					(dvoid*) &dscale,(ub4 *) 0, (ub4) DCI_ATTR_SCALE,
					(DCIError *) errhp ));
				DCI_CHECK_EX(error, DCIAttrGet((dvoid*) mypard, (ub4) DCI_DTYPE_PARAM,
					(dvoid*) &dprecision,(ub4 *) 0, (ub4) DCI_ATTR_PRECISION,
					(DCIError *) errhp ));

			}

			/* added to attr struct */
			memset(g_AryAttr[g_AttrNum].col_name,0, MAX_NAME_LEN);
			strncpy(g_AryAttr[g_AttrNum].col_name, (char *)col_name, col_name_len);
			//printf(" the column name is %s and type is %d and size is %d, precision is %d, scale is %d \n",
			//			col_name,dtype,col_width,dprecision,dscale);

			ConvertDciType(dtype, col_width, dprecision, dscale);

			if (dtype == DCI_BLOB || dtype == DCI_CLOB)
			{
				DCIHandleFree(stmthp, DCI_HTYPE_STMT);
				DCIHandleFree(errhp, DCI_HTYPE_ERROR);
				dcisg_free(g_AryAttr);
				SET_ERROR(error, -1, "查询结果集中存在大字段，请使用ReadDateEx来获取这个结果集");
				return FALSE;
			}

			g_AryAttr[g_AttrNum].data_type = dtype;
			g_AryAttr[g_AttrNum].data_size = SqltLen(dtype, col_width);
			reclen = reclen + g_AryAttr[g_AttrNum].data_size + sizeof(sb2);
			g_AttrNum++;

			/* increment counter and get next descriptor, if there is one */
			counter++;
			parm_status = DCIParamGet((dvoid *)stmthp, DCI_HTYPE_STMT, errhp,
				(dvoid **)&mypard, (ub4) counter);
		}
		pdata = (char*)dcisg_malloc( reclen);
		if (pdata == NULL)
		{
			DCIHandleFree(stmthp,DCI_HTYPE_STMT);
			DCIHandleFree(errhp, DCI_HTYPE_ERROR);
			dcisg_free(g_AryAttr);
			return FALSE;
		}
		*buf = pdata;
		memset((char *)pdata,0, reclen);
		/* define */
		for (i = 1;i <= g_AttrNum;i++)
		{
			DCI_CHECK_EX(error, DCIDefineByPos(stmthp, &defhp[i-1], errhp, i, (ub1 *)pdata, g_AryAttr[i-1].data_size,
				g_AryAttr[i-1].data_type, pdata + g_AryAttr[i-1].data_size,/*(dvoid *)*/0,(ub2 *)0,DCI_DEFAULT));
			pdata = pdata + g_AryAttr[i-1].data_size + sizeof(sb2);
		}
		for (i = 1;i <= g_AttrNum;i++)
		{
			DCIDefineArrayOfStruct(defhp[i-1], errhp, reclen, reclen, 0, 0);
		}
		*attrs = g_AryAttr;
		*attr_num = g_AttrNum;
		*stmthpp = stmthp;
		*errhpp = errhp;
	}
	else
	{
		stmthp = *(DCIStmt**)stmthpp;
		errhp = *(DCIError**)errhpp;
	}
	ub4 lastrows, nextrows;
	DCI_CHECK_EX(error,
		DCIAttrGet(stmthp, DCI_HTYPE_STMT, &lastrows, 0, DCI_ATTR_ROW_COUNT, errhp));
	DCI_CHECK_EX(error,
		DCIStmtFetch(stmthp, errhp, 1, DCI_FETCH_NEXT, DCI_DEFAULT));
	DCI_CHECK_EX(error,
		DCIAttrGet(stmthp, DCI_HTYPE_STMT, &nextrows, 0, DCI_ATTR_ROW_COUNT, errhp));
	if (lastrows < nextrows)
	{
		return TRUE;
	}
	else
	{
		DCIHandleFree((dvoid *)stmthp, DCI_HTYPE_STMT);
		DCITransCommit(svchp, errhp, 0);
		DCIHandleFree((dvoid *)errhp, DCI_HTYPE_ERROR);
		dcisg_free(*buf);
		dcisg_free(*attrs);
		*stmthpp = NULL;
		*errhpp = NULL;
		*buf = NULL;
		*attrs = NULL;
		*attr_num = 0;
		return FALSE;
	}
}
/*******************************************************************************
**
** Routine:     ReadDataEx
**
** Author: xiemei
**
** DATE :2009/05/23
**
** Description: 特别要注意，这个函数读出来的数据，每列值后面都带两个字节的空批示符位置，记录整行记录大小时，要累加每个列的两字节指示符空间
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
struct OneBlobData
{
	ub4 len;
	bool null_flag;
	void *data;
	struct OneBlobData *next_col;
};
struct OneRowBlobData		//该结构体用来缓存结果集中的大字段数据
{
	ub4 len;
	OneBlobData *blob_data;
	struct OneRowBlobData *next_row;
};
static void FreeBlobData(OneRowBlobData *head)
{
	OneRowBlobData *temp;
	OneBlobData* tempEx;
	while(head != NULL)
	{
		temp = head->next_row;
		while (head->blob_data != NULL)
		{
			tempEx = head->blob_data->next_col;
			if (head->blob_data->data)
			{
				dcisg_free(head->blob_data->data);
				head->blob_data->data = NULL;
			}
			dcisg_free(head->blob_data);
			head->blob_data = NULL;
			head->blob_data = tempEx;
		}
		dcisg_free(head);
		head = NULL;
		head = temp;
	}

}
static void FreeLobHandle(DCILobLocator **lobp)
{
	int i=0;
	for (i=0; i<COL_NUM; i++)
	{
		if (lobp[i] == NULL)
		{
			break;
		}
		DCIDescriptorFree(lobp[i], DCI_DTYPE_LOB);
	}
}
#define RETURN_FALSE_RDE(RET) if(RET != DCI_SUCCESS){\
DCIHandleFree(stmthp,DCI_HTYPE_STMT);\
DCITransRollback(svchp, NULL, (ub4)DCI_DEFAULT);\
DCIHandleFree(errhp, DCI_HTYPE_ERROR);\
if(g_AryAttr) dcisg_free(g_AryAttr);\
	g_AryAttr = NULL;\
if(data_addr) dcisg_free(data_addr);\
	data_addr = NULL;\
FreeBlobData(pblob);\
FreeLobHandle(lobp);\
return FALSE;\
}

int CDci::ReadDataEx (IN const char *query,IN int top_number, OUT int *rec_num, OUT int *attr_num,OUT struct ColAttr ** attrs,OUT char **buf, OUT ub4 *buf_size, ErrorInfo_t* error)
{
	int		i, reclen = 0,	col_num = 0, col_name_len = 0;
	ub4		counter;
//	sb2		ind[COL_NUM];
	DCIDefine	*defhp[COL_NUM];
	DCILobLocator *lobp[COL_NUM];
	int lob_index = 0;
	char		*pdata = NULL;
	sb4	parm_status;
	DCIParam	*mypard = (DCIParam *) 0;
	ub4	dtype = 0;
	ub4	col_width = 0;
	ub4     dscale = 0;
	ub4	dprecision = 0;
	DciText	*col_name = NULL;
	char *tempp=NULL,*tempp_last=NULL;
	int rows=0,last_rows=0;
	int g_AttrNum = 0; // 属性个数
	ColAttr_t *g_AryAttr = NULL;
	char *data_addr = NULL;
	int retcode;
	char *sql_temp = NULL;
	DCIStmt          *stmthp = NULL;
	DCIError           *errhp = NULL;
	OneRowBlobData *pblob = NULL;
	OneRowBlobData *pblob_temp = NULL;
	OneBlobData *temp_blob = NULL;
	int blob_size = 0;
	*rec_num = 0;
	*attr_num = 0;
	*attrs = NULL;
	*buf = NULL;
	*buf_size = 0;
	if (is_connect == FALSE)
	{
		SET_ERROR(error, 3113, "服务器已经断开");
		return FALSE;
	}
	memset(error, 0, sizeof(ErrorInfo_t));
	memset(lobp, 0, sizeof(lobp));
	//ReplaceTopSql(query, &sql_temp, top_number); //此处如果连达梦的库一切正常，但是如果连金仓的话会产生严重的内存泄露
	if (top_number == -1)
	{
		top_number = 0x7FFFFFFF;
	}
	if (rec_num == NULL || attr_num == NULL || attrs ==NULL || buf == NULL || buf_size == NULL || error == NULL)
	{
		SET_ERROR(error, -1, "传入了非法的空指针");
		return FALSE;
	}
	memset(error, 0, sizeof(ErrorInfo_t));
	DCI_CHECK1(error, DCIHandleAlloc(envhp, (void**)&stmthp, DCI_HTYPE_STMT, 0, 0));
	DCI_CHECK1(error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));
	if (sql_temp != NULL)
	{
		//在这里是针对达梦数据库的语法做的特殊处理，用来返回TOP前多少条的记录
		DCI_CHECK(error, DCIStmtPrepare(stmthp, errhp, (DciText *)sql_temp, (ub4)strlen(sql_temp), (ub4)DCI_NTV_SYNTAX, (ub4)DCI_DEFAULT));
		dcisg_free(sql_temp);
		sql_temp = NULL;
	}
	else
	{
		DCI_CHECK(error, DCIStmtPrepare(stmthp, errhp, (DciText *)query, (ub4)strlen(query), (ub4)DCI_NTV_SYNTAX, (ub4)DCI_DEFAULT));
	}

	DCI_CHECK_EX(error, StmtExecute(svchp, stmthp, errhp, (ub4)0, (ub4)DCI_DEFAULT));
	RETURN_FALSE_RDE(retcode);
	/* define GID*/
	DCI_CHECK(error, DCIAttrGet(stmthp, DCI_HTYPE_STMT, &col_num, 0, DCI_ATTR_PARAM_COUNT, errhp));
	if (col_num > COL_NUM)
	{
		SET_ERROR(error, -1, "查询结果集列数太多，目前最大支持1500列");
		RETURN_FALSE_RDE(DCI_ERROR);
	}
	//g_AryAttr = (ColAttrS *)calloc(col_num, sizeof(ColAttr));
	g_AryAttr = (struct ColAttr *)calloc(col_num, sizeof(ColAttr));
	if (g_AryAttr == NULL)
	{
		SET_ERROR(error, -1, "内存不足");
		RETURN_FALSE_RDE(DCI_ERROR);
	}
	g_AttrNum = 0;

	/* Request a parameter descriptor for position 1 in the select-list */
	counter = 1;
	parm_status = DCIParamGet((dvoid *)stmthp, DCI_HTYPE_STMT, errhp, (dvoid **)&mypard, (ub4) counter);
	/* Loop only if a descriptor was successfully retrieved for
	* current position, starting at 1.
	*/
	reclen = 0;
	while (parm_status == DCI_SUCCESS)
	{
		/* Retrieve the column name attribute */
		DCIAttrGet((dvoid*) mypard, (ub4) DCI_DTYPE_PARAM,
			(dvoid**) &col_name, (ub4 *) &col_name_len,
			(ub4) DCI_ATTR_NAME,(DCIError *) errhp );

		/* Retrieve the datatype attribute */
		DCIAttrGet((dvoid*) mypard, (ub4) DCI_DTYPE_PARAM,
			(dvoid*) &dtype, (ub4 *) 0, (ub4) DCI_ATTR_DATA_TYPE,
			(DCIError *) errhp);

		//col_width = 0;
		/* Retrieve the length semantics for the column */
		//char_semantics = 0;


		/* Retrieve the column width in bytes */
		DCIAttrGet((dvoid*) mypard, (ub4) DCI_DTYPE_PARAM,
			(dvoid*) &col_width,(ub4 *) 0, (ub4) DCI_ATTR_DATA_SIZE,
			(DCIError *) errhp );

		if (dtype == DCI_NUM)
		{
			DCIAttrGet((dvoid*) mypard, (ub4) DCI_DTYPE_PARAM,
				(dvoid*) &dscale,(ub4 *) 0, (ub4) DCI_ATTR_SCALE,
				(DCIError *) errhp );
			DCIAttrGet((dvoid*) mypard, (ub4) DCI_DTYPE_PARAM,
				(dvoid*) &dprecision,(ub4 *) 0, (ub4) DCI_ATTR_PRECISION,
				(DCIError *) errhp );

		}

		/* added to attr struct */
		memset(g_AryAttr[g_AttrNum].col_name,0, MAX_NAME_LEN);
		strncpy(g_AryAttr[g_AttrNum].col_name, (char *)col_name, col_name_len);
		//printf(" the column name is %s and type is %d and size is %d, precision is %d, scale is %d \n",
		//			col_name,dtype,col_width,dprecision,dscale);

		ConvertDciType((ub2 &)dtype, (ub2 &)col_width, dprecision, dscale);

		g_AryAttr[g_AttrNum].data_type = dtype;
		if(dtype == DCI_BLOB || dtype == DCI_CLOB)
		{
			g_AryAttr[g_AttrNum].data_size = 8;
			lob_index++;
		}
		else
		{
			g_AryAttr[g_AttrNum].data_size = SqltLen(dtype, col_width);
		}
		reclen = reclen + g_AryAttr[g_AttrNum].data_size + sizeof(sb2);
		g_AttrNum++;

		/* increment counter and get next descriptor, if there is one */
		counter++;
		parm_status = DCIParamGet((dvoid *)stmthp, DCI_HTYPE_STMT, errhp,
			(dvoid **)&mypard, (ub4) counter);
	}
	// dcisg_malloc space
	pdata = data_addr = (char*)dcisg_malloc(RECNUM * reclen);

	// added by Lijunliang for dcisg_malloc error 2008/07/04
	if (NULL == data_addr)
	{
		RETURN_FALSE_RDE(DCI_ERROR);
	}
	memset((char *)data_addr,0, RECNUM*reclen);

	/* define */
	int j=0;
	for (i = 1;i <= g_AttrNum;i++)
	{
		if(g_AryAttr[i-1].data_type == DCI_BLOB || g_AryAttr[i-1].data_type == DCI_CLOB)
		{
			DCIDescriptorAlloc(envhp, (dvoid **) &lobp[j],	(ub4) DCI_DTYPE_LOB, (size_t) 0, (dvoid **) 0);
			DCI_CHECK_EX(error, DCIDefineByPos(stmthp, &defhp[i-1], errhp, i, &lobp[j], 4,
				g_AryAttr[i-1].data_type, pdata + g_AryAttr[i-1].data_size, 0, (ub2 *)0,DCI_DEFAULT));
			j++;
		}
		else
		{
			DCI_CHECK_EX(error, DCIDefineByPos(stmthp, &defhp[i-1], errhp, i, (ub1 *)pdata, g_AryAttr[i-1].data_size,
				g_AryAttr[i-1].data_type, pdata + g_AryAttr[i-1].data_size,/*(dvoid *)*/0,(ub2 *)0,DCI_DEFAULT));
		}
		pdata = pdata + g_AryAttr[i-1].data_size + sizeof(sb2);
	}
	for (i = 1;i <= g_AttrNum;i++)
	{
		if(!(g_AryAttr[i-1].data_type == DCI_BLOB || g_AryAttr[i-1].data_type == DCI_CLOB))
		{
			DCIDefineArrayOfStruct(defhp[i-1], errhp, reclen, reclen, 0, 0);
		}

	}
	int temp_rows = 0;
	temp_rows = top_number>RECNUM?RECNUM:top_number;
	if (lob_index > 0)
	{
		temp_rows = 1;
	}
	DCI_CHECK_EX(error, DCIStmtFetch(stmthp, errhp,temp_rows, DCI_FETCH_NEXT, DCI_DEFAULT));
	while (retcode == DCI_SUCCESS)
	{
		temp_blob = NULL;
		DCI_CHECK_EX(error, DCIAttrGet(stmthp, DCI_HTYPE_STMT, &rows, 0, DCI_ATTR_ROW_COUNT, errhp));
		tempp = (char*)dcisg_malloc(rows*reclen);
		if (tempp == NULL)
		{
			SET_ERROR(error, NORMAL_ERR, "内存不足");
			RETURN_FALSE_RDE(DCI_ERROR);
		}
		if(tempp_last != NULL)
		{
			memcpy(tempp,tempp_last,last_rows*reclen);
			memcpy(tempp+last_rows*reclen,data_addr,(rows-last_rows)*reclen);
			dcisg_free(tempp_last);
			tempp_last = NULL;
		}
		else
		{
			memcpy(tempp+last_rows*reclen,data_addr,(rows-last_rows)*reclen);
		}
		if (lob_index > 0)
		{
			//开始提取当前行的大对像，首选应该获取行中每一列的大对像的长度，以便申请足够多的内存来存放每行的大对像
			if (pblob_temp == NULL)
			{
				pblob_temp = (OneRowBlobData*)dcisg_malloc(sizeof(OneRowBlobData));
				if (pblob_temp == NULL)
				{
					SET_ERROR(error, NORMAL_ERR, "内存不足");
					RETURN_FALSE_RDE(DCI_ERROR);
				}
				memset(pblob_temp, 0, sizeof(OneRowBlobData));
				pblob = pblob_temp;
			}
			else
			{
				pblob_temp->next_row = (OneRowBlobData*)dcisg_malloc(sizeof(OneRowBlobData));
				pblob_temp = pblob_temp->next_row;
				if (pblob_temp == NULL)
				{
					SET_ERROR(error, NORMAL_ERR, "内存不足");
					RETURN_FALSE_RDE(DCI_ERROR);
				}
				memset(pblob_temp, 0, sizeof(OneRowBlobData));
			}
			for (j=0; j<lob_index; j++)
			{
				ub4 lenp;
				retcode = DCILobGetLength(svchp, errhp, lobp[j], &lenp);
				if (retcode != DCI_SUCCESS)
				{
					SET_ERROR(error, NORMAL_ERR, "未能获取大对像字段的数据流长度");
					RETURN_FALSE_RDE(DCI_ERROR);
				}
				if (temp_blob == NULL)
				{
					temp_blob = (OneBlobData*)dcisg_malloc(sizeof(OneBlobData));
					if (temp_blob == NULL)
					{
						SET_ERROR(error, NORMAL_ERR, "内存不足");
						RETURN_FALSE_RDE(DCI_ERROR);
					}
					memset(temp_blob, 0, sizeof(OneBlobData));
					pblob_temp->blob_data = temp_blob;
				}
				else
				{
					temp_blob->next_col = (OneBlobData*)dcisg_malloc(sizeof(OneBlobData));
					temp_blob = temp_blob->next_col;
					if (temp_blob == NULL)
					{
						SET_ERROR(error, NORMAL_ERR, "内存不足");
						RETURN_FALSE_RDE(DCI_ERROR);
					}
					memset(temp_blob, 0, sizeof(OneBlobData));
				}
				if (lenp == 0)
				{
					temp_blob->len = 0;
					temp_blob->null_flag = TRUE;
				}
				else
				{
					temp_blob->len = lenp;
				}
				if (temp_blob->len > 0)
				{
    	//<XM_20110630> ===> BEGIN
					/*
					temp_blob->data = dcisg_malloc(lenp+1);//这里加1是为了给CLOB字符串类型空一个空结尾
					if (temp_blob->data == NULL)
					{
						SET_ERROR(error, NORMAL_ERR, "内存不足");
						RETURN_FALSE_RDE(DCI_ERROR);
					}
					ub4 amtp = lenp+1;
					retcode = DCILobRead(svchp, errhp, lobp[j], &amtp, 0, (dvoid *) temp_blob->data, amtp+1, (dvoid *) 0,
						(sb4 (*)(dvoid *, const dvoid *, ub4, ub1)) 0, (ub2) 0, (ub1) SQLCS_IMPLICIT);
					printf(" the reccode is %d and amtp is %d and temp_blob->len is %d \n",retcode,amtp,temp_blob->len);
					if (retcode != DCI_SUCCESS || amtp != temp_blob->len)
					{
						SET_ERROR(error, NORMAL_ERR, "未能读取全部的大字段数据");
						RETURN_FALSE_RDE(DCI_ERROR);
					}*/
					ub4 amtp = 0;
					ub4 total_len = 0;
					void *old_datap;
					//***
					lenp+=1;
					//****
					amtp = lenp;
					if (lenp > 512*1024)
					{
						lenp = 512*1024;
					}

					temp_blob->data = malloc(lenp);//这里加1是为了给CLOB字符串类型空一个空结尾

					retcode = DCILobRead(svchp, errhp, lobp[j], &amtp, 1, (dvoid *) temp_blob->data, lenp, (dvoid *) 0,
						(sb4 (*)(dvoid *, const dvoid *, ub4, ub1)) 0, (ub2) 0, (ub1) SQLCS_IMPLICIT);
					while (retcode == DCI_NEED_DATA)
					{
						total_len += amtp;
						old_datap = temp_blob->data;
						temp_blob->data = realloc(temp_blob->data, total_len + lenp);
						if (temp_blob->data == NULL)
						{
							free(old_datap);
							SET_ERROR(error, NORMAL_ERR, "内存不足");
							RETURN_FALSE_RDE(DCI_ERROR);
						}
						amtp = lenp + 1;
						//这里DCILobRead参数offset置为1就是忽略offset参数,下次读的偏移从上次读完的开始,OREACLE上的做法也是如此
						retcode = DCILobRead(svchp, errhp, lobp[j], &amtp, 1, ((char*)temp_blob->data + total_len), lenp, (dvoid *) 0,
							(sb4 (*)(dvoid *, const dvoid *, ub4, ub1)) 0, (ub2) 0, (ub1) SQLCS_IMPLICIT);
					}
					total_len += amtp;
					temp_blob->len = total_len;
					printf(" the reccode is %d and amtp is %d and temp_blob->len is %d \n", retcode, amtp, temp_blob->len);
					if (retcode != DCI_SUCCESS && retcode != DCI_NO_DATA)
					{
						SET_ERROR(error, NORMAL_ERR, "未能读取全部的大字段数据");
						RETURN_FALSE_RDE(DCI_ERROR);
					}
	//<XM_20110630> ===> END>
				}

				pblob_temp->len += temp_blob->len;
			}
		}
		last_rows = rows;
		tempp_last = tempp;
		temp_rows = (top_number - last_rows)>RECNUM?RECNUM:(top_number - last_rows);
		if(temp_rows == 0)
		{
			goto SUCCESS;
		}
		if (lob_index > 0)
		{
			temp_rows = 1;
		}
		memset((char *)data_addr,0, RECNUM*reclen);
		DCI_CHECK_EX(error, DCIStmtFetch(stmthp, errhp, temp_rows, DCI_FETCH_NEXT, DCI_DEFAULT));
	}

	if (retcode != DCI_NO_DATA)
	{
		RETURN_FALSE_RDE(DCI_ERROR);
    }
    // retcode==DCI_NO_DATA
	DCI_CHECK_EX(error,
		DCIAttrGet(stmthp, DCI_HTYPE_STMT, &rows, 0, DCI_ATTR_ROW_COUNT, errhp));
	if (last_rows < rows)
	{
		tempp = (char*)dcisg_malloc(rows*reclen);
		if (tempp == NULL)
		{
			SET_ERROR(error, NORMAL_ERR, "内存不足");
			RETURN_FALSE_RDE(DCI_ERROR);
		}
		if(tempp_last != NULL)
		{
			memcpy(tempp,tempp_last,last_rows*reclen);
			memcpy(tempp+last_rows*reclen,data_addr,(rows-last_rows)*reclen);
			dcisg_free(tempp_last);
			tempp_last = NULL;
		}
		else
		{
			memcpy(tempp+last_rows*reclen,data_addr,(rows-last_rows)*reclen);
		}
	}
	else
	{
		tempp = tempp_last;
	}


SUCCESS:
	if (lob_index > 0 && rows > 0)
	{
		pblob_temp = pblob;
		//记算大字段的总长度
		while(pblob_temp != NULL)
		{
			blob_size += pblob_temp->len;
			pblob_temp = pblob_temp->next_row;
		}
		tempp_last = tempp;
		tempp = (char*)dcisg_malloc(reclen*rows + blob_size);
		if (tempp == NULL)
		{
			SET_ERROR(error, NORMAL_ERR, "内存不足");
			SET_ERROR(error, NORMAL_ERR, "内存不足");
			dcisg_free(tempp_last);
			tempp_last = NULL;
			RETURN_FALSE_RDE(DCI_ERROR);
		}
		memcpy(tempp, tempp_last, reclen*rows);
		char *data_offset = NULL;
		char *data_temp = NULL;
		data_temp = tempp + reclen*rows;
		pblob_temp = pblob;
		for (i=0; i<rows; i++)
		{
			data_offset = tempp + reclen * i;
			temp_blob = pblob_temp->blob_data;
			for (j=0; j<g_AttrNum; j++)
			{
				if (g_AryAttr[j].data_type == DCI_BLOB || g_AryAttr[j].data_type == DCI_CLOB)
				{
					if (temp_blob->null_flag == TRUE)
					{
						*(sb2*)(data_offset + g_AryAttr[j].data_size) = -1;
					}
					else
					{
						memcpy(data_temp, temp_blob->data, temp_blob->len);
						*(ub4*)data_offset = data_temp - tempp;
						*(ub4*)(data_offset + sizeof(ub4)) = temp_blob->len;
						data_temp += temp_blob->len;
					}
					temp_blob = temp_blob->next_col;
				}
				data_offset += g_AryAttr[j].data_size + sizeof(sb2);
			}
			pblob_temp = pblob_temp->next_row;
		}
		dcisg_free(tempp_last);
		tempp_last = NULL;
	}
	*attr_num = g_AttrNum;
	*attrs = g_AryAttr;
	*rec_num = rows;
	*buf = tempp;
	*buf_size = reclen * rows + blob_size;

    FreeBlobData(pblob);
	FreeLobHandle(lobp);
	dcisg_free(data_addr);
	data_addr = NULL;
	DCIHandleFree((dvoid *)stmthp, DCI_HTYPE_STMT);
	DCITransCommit(svchp, errhp, 0);
	DCIHandleFree((dvoid *)errhp, DCI_HTYPE_ERROR);
	return TRUE;
}
/*******************************************************************************
**
** Routine:     WriteData
**
** Author: Lijunliang
**
** DATE :2008/03/19
**
** Description: write the data into database according the query
**
** Return: DCI_SUCCESS on success ,DCI_ERROR on error
*******************************************************************************/
bool CDci::WriteData(IN const char *query, IN const char *buf, IN const int rec_num, IN const int attr_num, IN const struct ColAttr* attrs,ErrorInfo_t* error)
{
	int		query_len = strlen(query);
	int		i = 0;
	int		ret = 0;
	const char	*pbuf = buf;
	int retcode;
	DCIStmt          *stmthp = NULL;
	DCIError           *errhp = NULL;
	if (is_connect == FALSE)
	{
		SET_ERROR(error, 3113, "服务器已经断开");
		return FALSE;
	}
	memset(error, 0, sizeof(ErrorInfo_t));
	DCI_CHECK1(error, DCIHandleAlloc(envhp, (void**)&stmthp, DCI_HTYPE_STMT, 0, 0));
	DCI_CHECK1(error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));
	/* prepare the sql sentence*/
	DCI_CHECK(error, DCIStmtPrepare(stmthp, errhp, (unsigned char *)query, (ub4)query_len,
		(ub4) DCI_NTV_SYNTAX, (ub4) DCI_DEFAULT));
	/* Bind the placeholder */
	DCIBind *bndhp[COL_NUM];
	int rec_len = 0;

	/* Bind of the pos */
	for (i = 0; i < attr_num; i++)
	{
		DCI_CHECK_EX(error, DCIBindByPos(stmthp, &bndhp[i], errhp, (ub4) i+1,
			(dvoid *) pbuf, (sb4) attrs[i].data_size, (ub2)attrs[i].data_type,
			(dvoid *) 0, (ub2 *)0, (ub2 *)0,
			(ub4) 0, (ub4 *) 0, (ub4) DCI_DEFAULT));
		pbuf += attrs[i].data_size;
		rec_len += attrs[i].data_size;
	}

	/* Bind of the struct*/
	for(i = 0; i < attr_num; i++)
	{
		DCI_CHECK(error, DCIBindArrayOfStruct(bndhp[i],errhp, rec_len, 0, 0, 0));
	}

	/* Execute */
	DCI_CHECK( error, StmtExecute(svchp, stmthp, errhp, (ub4) rec_num, (ub4) DCI_DEFAULT));

	//printf("write data ok!\n");

	DCIHandleFree((dvoid *)stmthp, DCI_HTYPE_STMT);
	DCITransCommit(svchp, errhp, 0);
	DCIHandleFree((dvoid *)errhp, DCI_HTYPE_ERROR);
	return TRUE;
}

/*******************************************************************************
**
** Routine:     WriteDataNoCmit
**
** Author: Lijunliang
**
** DATE :2008/03/19
**
** Description: write the data into database according the query, no commit
**
** Return: DCI_SUCCESS on success ,DCI_ERROR on error
*******************************************************************************/
bool CDci::WriteDataNoCmit(IN const char *query, IN const char *buf, IN const int rec_num, IN const int attr_num, IN const struct ColAttr* attrs,ErrorInfo_t* error)
{
	int		query_len = strlen(query);
	int		i = 0;
	int		ret = 0;
	const char	*pbuf = buf;
	int retcode;
	DCIStmt          *stmthp = NULL;
	DCIError           *errhp = NULL;
	if (is_connect == FALSE)
	{
		SET_ERROR(error, 3113, "服务器已经断开");
		return FALSE;
	}
	memset(error, 0, sizeof(ErrorInfo_t));
	DCI_CHECK1(error, DCIHandleAlloc(envhp, (void**)&stmthp, DCI_HTYPE_STMT, 0, 0));
	DCI_CHECK1(error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));
	/* prepare the sql sentence*/
	DCI_CHECK(error, DCIStmtPrepare(stmthp, errhp, (unsigned char *)query, (ub4)query_len,
		(ub4) DCI_NTV_SYNTAX, (ub4) DCI_DEFAULT));
	/* Bind the placeholder */
	DCIBind *bndhp[COL_NUM];
	int rec_len = 0;

	/* Bind of the pos */
	for (i = 0; i < attr_num; i++)
	{
		DCI_CHECK_EX(error, DCIBindByPos(stmthp, &bndhp[i], errhp, (ub4) i+1,
			(dvoid *) pbuf, (sb4) attrs[i].data_size, (ub2)attrs[i].data_type,
			(dvoid *) 0, (ub2 *)0, (ub2 *)0,
			(ub4) 0, (ub4 *) 0, (ub4) DCI_DEFAULT));
		pbuf += attrs[i].data_size;
		rec_len += attrs[i].data_size;
	}

	/* Bind of the struct*/
	for(i = 0; i < attr_num; i++)
	{
		DCI_CHECK(error, DCIBindArrayOfStruct(bndhp[i],errhp, rec_len, 0, 0, 0));
	}

	/* Execute */
	DCI_CHECK( error, StmtExecute(svchp, stmthp, errhp, (ub4) rec_num, (ub4) DCI_DEFAULT));

	//printf("write data ok!\n");

	DCIHandleFree((dvoid *)stmthp, DCI_HTYPE_STMT);
	DCIHandleFree((dvoid *)errhp, DCI_HTYPE_ERROR);
	return TRUE;
}


int CDci::CheckMalloc(void *ptr)
{
	if (NULL == ptr)
	{

		return FALSE;
	}

	return TRUE;
}

/*******************************************************************************
**
** Routine:     InsertWithBind
**
** Author: Lijunliang
**
** DATE :2008/03/19
**
** Description: insert data into the db with define op.
**
*******************************************************************************/
bool CDci::InsertWithBind(const char *query, const char *buf, int rec_num, int attr_num, const struct ColAttr* attrs ,ErrorInfo_t* error)
{

	return WriteData(query, buf, rec_num, attr_num, attrs,error);
}

/*******************************************************************************
**
** Routine:     UpdateWithBind
**
** Author: Lijunliang
**
** DATE :2008/03/19
**
** Description: update data into the db with define op.
**
*******************************************************************************/
bool CDci::UpdateWithBind(const char *query, const char *buf, const int rec_num, const int attr_num, const struct ColAttr* attrs,ErrorInfo_t* error)
{
	return WriteData(query, buf, rec_num, attr_num, attrs,error);
}

/*******************************************************************************
**
** Routine:     ExecNoBind
**
** Author: Lijunliang
**
** DATE :2008/03/19
**
** Description: execute the single insert/delete/update without define op.
**
*******************************************************************************/
bool CDci::ExecNoBind(IN const char *sqlstr,ErrorInfo_t* error)
{
	return ExecSingle(sqlstr,error);
}

/*******************************************************************************
**
** Routine:     ExecNoBndAndCmit
**
** Author: Lijunliang
**
** DATE :2008/03/19
**
** Description: execute the single insert/delete/update without define op and commit.
**
*******************************************************************************/
bool CDci::ExecNoBindAndCmit(IN const char *sqlstr,ErrorInfo_t* error)
{
	int retcode;
	DCIStmt          *stmthp = NULL;
	DCIError           *errhp = NULL;
	if (is_connect == FALSE)
	{
		SET_ERROR(error, 3113, "服务器已经断开");
		return FALSE;
	}
	DCI_CHECK1(error, DCIHandleAlloc(envhp, (void**)&stmthp, DCI_HTYPE_STMT, 0, 0));
	DCI_CHECK1(error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));
	DCI_CHECK(error, DCIStmtPrepare(stmthp, errhp, (DciText *)sqlstr, (ub4)strlen(sqlstr), (ub4)DCI_NTV_SYNTAX, (ub4)DCI_DEFAULT));
	DCI_CHECK(error, StmtExecute(svchp, stmthp, errhp, 1, (ub4) DCI_DEFAULT));
	DCI_CHECK(error, DCITransCommit(svchp, errhp, DCI_DEFAULT));
	RETURN_TRUE;
}

/*******************************************************************************
**
** Routine:     ExecCommit
**
** Author: Lijunliang
**
** DATE :2008/03/19
**
** Description: execute the commit op.
**
*******************************************************************************/
bool CDci::ExecCommit(ErrorInfo_t* error)
{
	/* commit */
	DCIError *errhp = NULL;
	memset(error, 0, sizeof(ErrorInfo_t));
	return DCI_SUCCESS == DCITransCommit(svchp, errhp, (ub4)DCI_DEFAULT);
}

/*******************************************************************************
**
** Routine:     ExecRollback
**
** Author: Lijunliang
**
** DATE :2008/03/19
**
** Description: execute the rollback op.
**
*******************************************************************************/
bool CDci::ExecRollback(ErrorInfo_t* error)
{
	if (error)
	{
		memset(error, 0, sizeof(ErrorInfo_t));
	}
	return DCI_SUCCESS == DCITransRollback(svchp, NULL, (ub4)DCI_DEFAULT);
}

/*******************************************************************************
**
** Routine:     ExecSingle
**
** Author: Lijunliang
**
** DATE :2008/03/19
**
** Description: execute the sql (update, insert, delete) with no return
**
*******************************************************************************/
bool CDci::ExecSingle(IN const char *sqlstr,ErrorInfo_t* error)
{
	int retcode = 0;
	DCIStmt          *stmthp = NULL;
	DCIError           *errhp = NULL;
	if (is_connect == FALSE)
	{
		SET_ERROR(error, 3113, "服务器已经断开");
		return FALSE;
	}
	memset(error, 0, sizeof(ErrorInfo_t));
	DCI_CHECK1(error, DCIHandleAlloc(envhp, (void**)&stmthp, DCI_HTYPE_STMT, 0, 0));
	DCI_CHECK1(error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));
	DCI_CHECK(error, DCIStmtPrepare(stmthp, errhp, (DciText *)sqlstr, (ub4)strlen(sqlstr), (ub4)DCI_NTV_SYNTAX, (ub4)DCI_DEFAULT));

	DCI_CHECK(error, StmtExecute(svchp, stmthp, errhp, 0, DCI_COMMIT_ON_SUCCESS));
	RETURN_TRUE;
}

/*******************************************************************************
**
** Routine:     ExecSingleNoCmit
**
** Author: Lijunliang
**
** DATE :2008/03/19
**
** Description: execute the sql (update, insert, delete) with no return no commit
**
*******************************************************************************/
bool CDci::ExecSingleNoCmit(IN const char *sqlstr,ErrorInfo_t* error)
{
	int retcode = 0;
	DCIStmt          *stmthp = NULL;
	DCIError           *errhp = NULL;
	if (is_connect == FALSE)
	{
		SET_ERROR(error, 3113, "服务器已经断开");
		return FALSE;
	}
	memset(error, 0, sizeof(ErrorInfo_t));
	DCI_CHECK1(error, DCIHandleAlloc(envhp, (void**)&stmthp, DCI_HTYPE_STMT, 0, 0));
	DCI_CHECK1(error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));
	DCI_CHECK(error, DCIStmtPrepare(stmthp, errhp, (DciText *)sqlstr, (ub4)strlen(sqlstr), (ub4)DCI_NTV_SYNTAX, (ub4)DCI_DEFAULT));

	DCI_CHECK(error, StmtExecute(svchp, stmthp, errhp, 0, DCI_DEFAULT));
	RETURN_TRUE;
}

/*******************************************************************************
**
** Routine:     ReadAttrs
**
** Author: Lijunliang
**
** DATE :2008/03/19
**
** Description: read all attrs of the table.
**
*******************************************************************************/
bool CDci::ReadAttrs(IN char * tablename, OUT int * attr_num, OUT struct ColAttr * * attrs,ErrorInfo_t* error)
{
	char query[100];
	memset(error, 0, sizeof(ErrorInfo_t));
	memset(query, 0, 100);
	sprintf(query, "select * from %s", tablename);
	GetAttrs(query, attr_num, attrs,error);

	return TRUE;
}

/*******************************************************************************
**
** Routine:     ReadWithBind
**
** Author: Lijunliang
**
** DATE :2008/03/19
** Changed: 20090311 :add the readdata's reload function by xiemei
**
** Description: select data from the db with bind op.
**
*******************************************************************************/
bool CDci::ReadWithBind(char *query, char **buf, int *rec_num, int *attr_num, struct ColAttr** attrs,ErrorInfo_t* error)
{
	//GetAttrs(query, attr_num, attrs,error);
	//printf("******************  the attr_num is %d****************** \n",*attr_num);
	//GetRows(query, rec_num,error);
	//printf("******************  the rec_num is %d****************** \n",*rec_num);

	//ReadData(query, *rec_num, buf,error);
	ReadData(query, rec_num, attr_num,attrs,buf,error);

	return true;
}

/*******************************************************************************
**
** Routine:     GetRows
**
** Author: Lijunliang
**
** DATE :2008/03/19
**
** Description: get the rows of the sql prehand
**
** Return: DCI_SUCCESS on success ,DCI_ERROR on error
*******************************************************************************/
int CDci::GetRows(IN const char *query, OUT int *rec_num,ErrorInfo_t* error)
{
	//sword     status;
	/*make the sql(count) */
	//char * fromLocation, *attrLocation;
	int		col_width = 22;
	int		prefetch = 1;
	char	pquery[200];
	DCIDefine	*hDefine = NULL;
	const int	COUNTADD = 35;
	sprintf(pquery, "select count(*) from %s",  query);
	int retcode;
	DCIStmt          *stmthp = NULL;
	DCIError           *errhp = NULL;
	if (is_connect == FALSE)
	{
		SET_ERROR(error, 3113, "服务器已经断开");
		return FALSE;
	}
	DCI_CHECK1(error, DCIHandleAlloc(envhp, (void**)&stmthp, DCI_HTYPE_STMT, 0, 0));
	DCI_CHECK1(error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));
	//printf("count sql : %s\n", cntquery);
	/* parse query */
	DCI_CHECK(error, DCIStmtPrepare(stmthp, errhp, (DciText *)pquery, (ub4)strlen(pquery),	(ub4)DCI_NTV_SYNTAX, (ub4)DCI_DEFAULT));

	DCI_CHECK(error, DCIAttrSet(stmthp, DCI_HTYPE_STMT,
								&prefetch, 0,
								DCI_ATTR_PREFETCH_ROWS, errhp));

	/* define GID */
	DCI_CHECK(error, DCIDefineByPos(stmthp, &hDefine, errhp,
		1, (ub1 *)rec_num, 4, SQLT_INT, 0, /*(dvoid *)*/0, (ub2 *)0, DCI_DEFAULT));

	DCI_CHECK(error, StmtExecute(svchp, stmthp, errhp, (ub4)prefetch, (ub4)DCI_EXACT_FETCH));
	DCITransCommit(svchp, NULL, DCI_DEFAULT);
	RETURN_TRUE;
}

/*******************************************************************************
**
** Routine:     GetAttrs
**
** Author: Lijunliang
**
** DATE :2008/03/19
**
** Description: get the attrs of the sql prehand
** changed : xiemei at 20090306 for datatype of java because java hasn't the numric data type
**
** Return: DCI_SUCCESS on success ,DCI_ERROR on error
*******************************************************************************/
bool CDci::GetAttrs(IN const char *query, OUT int *attr_num, OUT struct ColAttr** attrs,ErrorInfo_t* error)
{
	int		i, 	reclen = 0,	col_num = 0;
	ub4	counter,  col_name_len;
	sb4	parm_status;
	DCIParam	*mypard = (DCIParam *) 0;
	ub2	dtype, col_width;
	ub1     dscale;
	ub2		dprecision;
	DciText	*col_name;
	int g_AttrNum; // 属性个数
	ColAttr_t *g_AryAttr;
	i = 0;
	*attr_num = 0;

	int retcode;
	DCIStmt          *stmthp = NULL;
	DCIError           *errhp = NULL;
	if (is_connect == FALSE)
	{
		SET_ERROR(error, 3113, "服务器已经断开");
		return FALSE;
	}
	DCI_CHECK1(error, DCIHandleAlloc(envhp, (void**)&stmthp, DCI_HTYPE_STMT, 0, 0));
	DCI_CHECK1(error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));

	DCI_CHECK(error, DCIStmtPrepare(stmthp, errhp, (DciText *)query, (ub4)strlen(query), (ub4)DCI_NTV_SYNTAX, (ub4)DCI_DEFAULT));

	/* define GID*/
	DCI_CHECK(error, StmtExecute(svchp, stmthp, errhp, 0, (ub4)DCI_DESCRIBE_ONLY));

	DCI_CHECK(error, DCIAttrGet(stmthp, DCI_HTYPE_STMT,	&col_num, 0, DCI_ATTR_PARAM_COUNT, errhp));

	//g_AryAttr = (ColAttrS *)calloc(col_num, sizeof(ColAttr));
	g_AryAttr = (struct ColAttr *)calloc(col_num, sizeof(ColAttr));
	g_AttrNum = 0;

	/* Request a parameter descriptor for position 1 in the select-list */
	counter = 1;
	parm_status = DCIParamGet((dvoid *)stmthp, DCI_HTYPE_STMT, errhp,
								(dvoid **)&mypard, (ub4) counter);
								/* Loop only if a descriptor was successfully retrieved for
								* current position, starting at 1.
	*/
	reclen = 0;
	while (parm_status == DCI_SUCCESS)
	{
		/* Retrieve the column name attribute */
		DCI_CHECK_EX(error, DCIAttrGet((dvoid*) mypard, (ub4) DCI_DTYPE_PARAM,
			(dvoid**) &col_name, (ub4 *) &col_name_len,
			(ub4) DCI_ATTR_NAME,(DCIError *) errhp ));
		//if( ret == ERROR)
		//{
		//dcisg_free(g_AryAttr);
		//return DCI_ERROR;
		//}
//		printf("the column name is %s \n",col_name);
		/* Retrieve the datatype attribute */
		DCI_CHECK_EX(error, DCIAttrGet((dvoid*) mypard, (ub4) DCI_DTYPE_PARAM,
			(dvoid*) &dtype, (ub4 *) 0, (ub4) DCI_ATTR_DATA_TYPE,
			(DCIError *) errhp));

//		printf("the column name is %d \n",dtype);
		col_width = 0;
		/* Retrieve the column width in bytes */
		DCI_CHECK_EX(error, DCIAttrGet((dvoid*) mypard, (ub4) DCI_DTYPE_PARAM,
			(dvoid*) &col_width,(ub4 *) 0, (ub4) DCI_ATTR_DATA_SIZE,
			(DCIError *) errhp ));
		/*** change for java no numric datatype  ******/
		if (dtype == DCI_NUM)
		{
			DCI_CHECK_EX(error, DCIAttrGet((dvoid*) mypard, (ub4) DCI_DTYPE_PARAM,
				(dvoid*) &dscale,(ub4 *) 0, (ub4) DCI_ATTR_SCALE,
				(DCIError *) errhp ));
			DCI_CHECK_EX(error, DCIAttrGet((dvoid*) mypard, (ub4) DCI_DTYPE_PARAM,
				(dvoid*) &dprecision,(ub4 *) 0, (ub4) DCI_ATTR_PRECISION,
				(DCIError *) errhp ));

		}
		/* added to attr struct */
		memset(g_AryAttr[g_AttrNum].col_name,0, MAX_NAME_LEN);
		strncpy(g_AryAttr[g_AttrNum].col_name, (char *)col_name, col_name_len);

		ConvertDciType(dtype, col_width, dprecision, dscale);

		g_AryAttr[g_AttrNum].data_type = dtype;
		g_AryAttr[g_AttrNum].data_size = SqltLen(dtype, col_width);
		reclen = reclen + g_AryAttr[g_AttrNum].data_size;
		g_AttrNum++;


		/* increment counter and get next descriptor, if there is one */
		counter++;
		parm_status = DCIParamGet((dvoid *)stmthp, DCI_HTYPE_STMT, errhp,
			(dvoid **)&mypard, (ub4) counter);
	}

	*attr_num = g_AttrNum;
	*attrs = g_AryAttr;
	//DCITransCommit(svchp, NULL, DCI_DEFAULT); danath 20111128 此处不需要
	RETURN_TRUE;
}

#define RETURN_FALSE_GETSTMTRESULT(R) \
	if (R != DCI_SUCCESS) \
	{ \
	if (ppdefn) \
	dcisg_free(ppdefn); \
		ppdefn = NULL;\
	if (out_result) \
	dcisg_free(out_result); \
		out_result = NULL;\
	FreeResultMemory(ppresult, ppind, cols); \
	return FALSE;\
} \
else

/*******************************************************************************
**
** Routine: GetStmtResult
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: get the statement's result
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
bool CDci::GetStmtResult(DCIStmt *stmtp_result, ResultAttr_t *resultp, DCIError* errhp, ErrorInfo_t *error)
{
	ub4		cols = 0;
	ub4		rows = 0;
	ub4		last_rows = 0;
	void	**ppresult = NULL;
	void	**ppind = NULL;
	DCIDefine	**ppdefn = NULL;
	ColAttr_t	*out_result = NULL;
	sword	retcode;
	DCI_CHECK_EX(error,
		DCIAttrGet(stmtp_result, DCI_HTYPE_STMT, &cols, NULL, DCI_ATTR_PARAM_COUNT, errhp));
	RETURN_FALSE_GETSTMTRESULT(retcode);
	if (cols != 0 && resultp != NULL)
	{
		ppdefn = (DCIDefine **)dcisg_malloc(sizeof(DCIDefine *) * cols);
		if (ppdefn == NULL)
		{
			SET_ERROR(error, NORMAL_ERR, "内存不足");
			RETURN_FALSE_GETSTMTRESULT(DCI_ERROR);
		}
		memset(ppdefn, 0, sizeof(DCIDefine *) * cols);

		out_result = (ColAttr_t*)dcisg_malloc(sizeof(ColAttr_t) * cols);
		if (out_result == NULL)
		{
			SET_ERROR(error, NORMAL_ERR, "内存不足");
			RETURN_FALSE_GETSTMTRESULT(DCI_ERROR);
		}
		memset(out_result, 0, sizeof(ColAttr_t) * cols);
		if(!GetResultColAttr(stmtp_result, out_result, cols, errhp, error))
		{
			RETURN_FALSE_GETSTMTRESULT(DCI_ERROR);
		}

		ppresult = (void**)dcisg_malloc(sizeof(void *) * cols);
		if (ppresult == NULL)
		{
			SET_ERROR(error, NORMAL_ERR, "内存不足");
			RETURN_FALSE_GETSTMTRESULT(DCI_ERROR);
		}
		memset(ppresult, 0, sizeof(void*) * cols);

		ppind = (void**)dcisg_malloc(sizeof(void *) * cols);
		if (ppind == NULL)
		{
			SET_ERROR(error, NORMAL_ERR, "内存不足");
			RETURN_FALSE_GETSTMTRESULT(DCI_ERROR);
		}
		memset(ppind, 0, sizeof(void*) * cols);

		if (!AllocResultMemory(ppresult, ppind, out_result, cols, error))
		{
			RETURN_FALSE_GETSTMTRESULT(DCI_ERROR);
		}
		if (!BindResult(stmtp_result, ppdefn, ppresult, ppind, out_result, cols, errhp, error))
		{
			RETURN_FALSE_GETSTMTRESULT(DCI_ERROR);
		}

		DCI_CHECK_EX(error,
			DCIStmtFetch(stmtp_result, errhp, MAX_FETCH_ROWS, DCI_FETCH_NEXT, DCI_DEFAULT));
		while (retcode == DCI_SUCCESS)
		{
			DCI_CHECK_EX(error,
				DCIAttrGet(stmtp_result, DCI_HTYPE_STMT, &rows, 0, DCI_ATTR_ROW_COUNT, errhp));
			RETURN_FALSE_GETSTMTRESULT(retcode);
			if (!MergerProcedureResult(ppresult, ppind, out_result, cols, rows - last_rows, last_rows, error))
			{
				RETURN_FALSE_GETSTMTRESULT(DCI_ERROR);
			}
			last_rows = rows;
			DCI_CHECK_EX(error,
				DCIStmtFetch(stmtp_result, errhp, MAX_FETCH_ROWS, DCI_FETCH_NEXT, DCI_DEFAULT));
		}
		if (retcode != DCI_NO_DATA)
		{
			RETURN_FALSE_GETSTMTRESULT(DCI_ERROR);
		}
		DCI_CHECK_EX(error,
			DCIAttrGet(stmtp_result, DCI_HTYPE_STMT, &rows, 0, DCI_ATTR_ROW_COUNT, errhp));
		RETURN_FALSE_GETSTMTRESULT(retcode);

		if (!MergerProcedureResult(ppresult, ppind, out_result, cols, rows - last_rows, last_rows, error))
		{
			RETURN_FALSE_GETSTMTRESULT(DCI_ERROR);
		}
		dcisg_free(ppdefn);
		ppdefn = NULL;
		FreeResultMemory(ppresult, ppind, cols);
		resultp->col_attr = out_result;
		resultp->cols = cols;
		resultp->rows = rows;
	}

	return TRUE;
}

#define RETURN_FALSE_ESR(R) \
	if (R != DCI_SUCCESS) \
{ \
	if (ppbnd) \
	dcisg_free(ppbnd); \
	ppbnd = NULL;\
	DCIHandleFree(stmthp, DCI_HTYPE_STMT);\
	DCITransRollback(svchp, NULL, (ub4)DCI_DEFAULT);\
	DCIHandleFree(errhp, DCI_HTYPE_ERROR);\
	return FALSE;\
} \
else

/*******************************************************************************
**
** Routine: ExecuteSqlWithResult
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: execute the sql ,and the sql have the return's data result
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
bool CDci::ExecuteSqlWithResult(const char *sql, ParamAttr_t *in_param, ub2 in_param_nums, ResultAttr_t *resultp, ErrorInfo_t *error)
{
	sword	retcode;
	DCIBind **ppbnd = NULL;
	DCIStmt          *stmthp = NULL;
	DCIError           *errhp = NULL;
	memset(error, 0, sizeof(ErrorInfo_t));
	if (is_connect == FALSE)
	{
		SET_ERROR(error, 3113, "服务器已经断开");
		return FALSE;
	}
	DCI_CHECK1(error, DCIHandleAlloc(envhp, (void**)&stmthp, DCI_HTYPE_STMT, 0, 0));
	DCI_CHECK1(error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));

	if (resultp)
	{
		memset(resultp, 0, sizeof(ResultAttr_t));
	}

	DCI_CHECK_EX(error,
		DCIStmtPrepare(stmthp, errhp, (DciText*)sql, strlen(sql), DCI_NTV_SYNTAX, DCI_DEFAULT));
	RETURN_FALSE_ESR(retcode);

	if (in_param)
	{
		ppbnd = (DCIBind **)dcisg_malloc(sizeof(DCIBind *) * (in_param_nums));
		if (ppbnd == NULL)
		{
			SET_ERROR(error, NORMAL_ERR, "内存不足");
			RETURN_FALSE_ESR(DCI_ERROR);
		}
		memset(ppbnd, 0, sizeof(DCIBind *) * in_param_nums);

		if (!BindParam(stmthp, ppbnd, in_param, in_param_nums, errhp, error))
		{
			RETURN_FALSE_ESR(DCI_ERROR);
		}
	}
	DCI_CHECK_EX(error,
		StmtExecute(svchp, stmthp, errhp, 0, DCI_DEFAULT));
	RETURN_FALSE_ESR(retcode);
	if (!GetStmtResult(stmthp, resultp, errhp, error))
	{
		RETURN_FALSE_ESR(DCI_ERROR);
	}
	if (ppbnd)
		dcisg_free(ppbnd);
		ppbnd = NULL;
	DCITransCommit(svchp, NULL, DCI_DEFAULT);
	RETURN_TRUE;
}
bool CDci::ExecuteSqlWithRowBind(const char *sql, char* data, ParamAttr_t *in_param, ub2 in_param_nums, ub4 rows, ub4 &row_count, ErrorInfo_t *error)
{
	sword	retcode;
	DCIBind **ppbnd = NULL;
	DCIStmt          *stmthp = NULL;
	DCIError           *errhp = NULL;
	memset(error, 0, sizeof(ErrorInfo_t));
	row_count = 0;
	if (is_connect == FALSE)
	{
		SET_ERROR(error, 3113, "服务器已经断开");
		return FALSE;
	}
	DCI_CHECK1(error, DCIHandleAlloc(envhp, (void**)&stmthp, DCI_HTYPE_STMT, 0, 0));
	DCI_CHECK1(error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));

	DCI_CHECK_EX(error,
		DCIStmtPrepare(stmthp, errhp, (DciText*)sql, strlen(sql), DCI_NTV_SYNTAX, DCI_DEFAULT));
	RETURN_FALSE_ESR(retcode);

	if (in_param)
	{
		ppbnd = (DCIBind **)dcisg_malloc(sizeof(DCIBind *) * (in_param_nums));
		if (ppbnd == NULL)
		{
			SET_ERROR(error, NORMAL_ERR, "内存不足");
			RETURN_FALSE_ESR(DCI_ERROR);
		}
		memset(ppbnd, 0, sizeof(DCIBind *) * in_param_nums);

		if (!BindParamByRowBind(stmthp, data, ppbnd, in_param, in_param_nums, errhp, error))
		{
			RETURN_FALSE_ESR(DCI_ERROR);
		}
	}
	DCI_CHECK_EX(error,
		StmtExecute(svchp, stmthp, errhp, rows, DCI_COMMIT_ON_SUCCESS));
	RETURN_FALSE_ESR(retcode);
	DCI_CHECK(error, DCIAttrGet(stmthp, DCI_HTYPE_STMT, &row_count, 0, DCI_ATTR_ROW_COUNT, errhp));
	if (ppbnd)
		dcisg_free(ppbnd);

	RETURN_TRUE;
}

/*******************************************************************************
**
** Routine: BindParam
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: bind the para for execute statement
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
bool CDci::BindParam(DCIStmt *stmtp_sql, DCIBind **ppbnd, ParamAttr_t *param, ub2 nums, DCIError *errhp, ErrorInfo_t *error)
{
	int		i;
	sword	retcode;
	for (i = 0; i < nums; i++)
	{
		if (param[i].data_type == DCI_CUR)
		{
			SET_ERROR(error, NORMAL_ERR, "不允许指定游标类型参数，如果执行的是存贮过程，请使用专用接口ExecuteProcedure");
			return FALSE;
		}
		else
		{
			DCI_CHECK_EX(error,
				DCIBindByPos(stmtp_sql, &ppbnd[i], errhp, i+1, param[i].data, param[i].data_size,
				param[i].data_type, param[i].indp, NULL, NULL, 0, NULL, DCI_DEFAULT));
		}
	}

	return TRUE;
}
bool CDci::BindParamByRowBind(DCIStmt *stmtp_sql, char*data, DCIBind **ppbnd, ParamAttr_t *param, ub2 nums, DCIError *errhp, ErrorInfo_t *error)
{
	int		i;
	sword	retcode;
	int rec_size = 0;
	for (i = 0; i < nums; i++)
	{
		if (param[i].data_type == DCI_CUR)
		{
			SET_ERROR(error, NORMAL_ERR, "不允许指定游标类型参数，如果执行的是存贮过程，请使用专用接口ExecuteProcedure");
			return FALSE;
		}
		else
		{
			DCI_CHECK_EX(error,
				DCIBindByPos(stmtp_sql, &ppbnd[i], errhp, i+1, data, param[i].data_size,
				param[i].data_type, data+param[i].data_size, NULL, NULL, 0, NULL, DCI_DEFAULT));
		}
		data += param[i].data_size+sizeof(sb2);
		rec_size += param[i].data_size+sizeof(sb2);
	}
	for (i = 0; i < nums; i++)
	{
		DCIBindArrayOfStruct(ppbnd[i], errhp, rec_size, rec_size, 0, 0);
	}
	return TRUE;
}
/*******************************************************************************
**
** Routine: InitHandle
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: init the handle
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/

void CDci::InitHandle()
{
	envhp = NULL;                       /* environment handle */
	srvhp = NULL;                            /* server handle */
	svchp = NULL;                          /* service context */
	authp = NULL;                   /* authentication context */
}

/*******************************************************************************
**
** Routine:IsConnected
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: department the connection's status
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
bool CDci::IsConnected()
{
	return is_connect;
}

/*******************************************************************************
**
** Routine:FreeColAttrDataEx
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: dcisg_free the column's attr from extern
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
void CDci::FreeColAttrDataEx(ColAttr_t *col_attr, int cols)
{
	for (int i = 0; i < cols; i++)
	{
		if (col_attr[i].data)
		{
			if (col_attr[i].data_type == DCI_CUR)
			{
				ResultAttr *resultp = (ResultAttr *)col_attr[i].data;
				FreeColAttrData(resultp->col_attr, resultp->cols);
			}
			dcisg_free(col_attr[i].data);
			col_attr[i].data = NULL;
		}
		if (col_attr[i].indp)
		{
			dcisg_free(col_attr[i].indp);
			col_attr[i].indp = NULL;
		}
	}
}

/*******************************************************************************
**
** Routine:FreeColAttrData
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: dcisg_free the column's attr
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
void CDci::FreeColAttrData(ColAttr_t *col_attr, int cols)
{
	if (col_attr == NULL)
	{
		return;
	}
	FreeColAttrDataEx(col_attr, cols);
	dcisg_free(col_attr);
	col_attr = NULL;
}

/*******************************************************************************
**
** Routine:FreeReadData
**
** Author: Xiemei
**
** DATE :2009/09/14
**
** Description: dcisg_free the column's attr and the data buf
**
*******************************************************************************/
void CDci::FreeReadData(ColAttr_t *col_attr, int colnum,char *databuf)
{
	if (col_attr == NULL)
	{
		return;
	}
	if(databuf == NULL)
	{
		return;
	}
	FreeColAttrDataEx(col_attr, colnum);
	dcisg_free(col_attr);
	dcisg_free(databuf);
	col_attr = NULL;
	databuf = NULL;
}
/*******************************************************************************
**
** Routine:FreeColAttrData
**
** Author: Xiemei
**
** DATE :2008/12/24
**
** Description: 调试用的函数，用来打印col_attr中的结果集
**
** Return: DCI_SUCCESS on success, DCI_ERROR on error
*******************************************************************************/
void CDci::PrintResultData(ColAttr_t *col_attr, ub2 cols, ub4 rows)
{
	char	cdata[MAX_STRING_LEN + 1];
	float	fdata;
	double	ddata;
	int		idata;
	int		idata_size;
	ResultAttr *resultp;
#ifdef WIN32
	__int64	bigidata;
#else
	sb8 bigidata;
#endif
	DCIDate	dtdata;

	//打印出得到的结果集
	for (ub4 i = 0; i < rows; i++)
	{
		for (int col = 0; col < cols; col++)
		{
			idata_size = col_attr[col].data_size;
			if (col_attr[col].indp)
			{
				if (col_attr[col].indp[i] == -1)
				{
					printf("NULL	");
					continue;
				}
			}
			switch(col_attr[col].data_type)
			{
			case DCI_FLT:
				if (col_attr[col].data_size == 4)
				{
					memcpy(&fdata, (char*)col_attr[col].data + i * idata_size, idata_size);
					printf("%f	", fdata);
				}
				else
				{
					memcpy(&ddata, (char*)col_attr[col].data + i * idata_size, idata_size);
					printf("%llf	", ddata);
				}
				break;
			case DCI_INT:
				if (col_attr[col].data_size == 4)
				{
					memcpy(&idata, (char*)col_attr[col].data + i * idata_size, idata_size);
					printf("%d	", idata);
				}
				else
				{
					memcpy(&bigidata, (char*)col_attr[col].data + i * idata_size, idata_size);
					printf("%lld	", bigidata);
				}
				break;
			case DCI_STR:
			case DCI_AFC:
			case DCI_AVC:
			case DCI_CHR:
				memset(cdata, '\0', MAX_STRING_LEN + 1);
				if (idata_size > MAX_STRING_LEN)
				{
					memcpy(cdata, (char*)col_attr[col].data + i * col_attr[col].data_size, MAX_STRING_LEN);
					printf("%s	", cdata);
				}
				else
				{
					memcpy(cdata, (char*)col_attr[col].data + i * col_attr[col].data_size, idata_size);
					printf("%s	", cdata);
				}
				break;
			case DCI_ODT:
				memcpy(&dtdata, (char*)col_attr[col].data + i * col_attr[col].data_size, idata_size);
				printf("%d-%d-%d %d:%d:%d ", dtdata.DCIDateYYYY,
					dtdata.DCIDateMM, dtdata.DCIDateDD,
					dtdata.DCIDateTime.DCITimeHH,
					dtdata.DCIDateTime.DCITimeMI,
					dtdata.DCIDateTime.DCITimeSS);
				break;
			case DCI_DAT:

				break;
			case DCI_CUR:
				printf("\n结果集 --> 开始\n");
				resultp = (ResultAttr_t*)col_attr[col].data;
				PrintResultData(resultp->col_attr, resultp->cols, resultp->rows);
				printf("结果集 --> 结束\n");
				break;
			default:
				printf("未知的格式  ");
				break;;
			}
		}
		printf("\n");
	}
}

bool CDci::ReConnect(ErrorInfo *error)
{
	return Connect(servername, username, pwd, is_init_schema, error);
}

static
int
dmtime_is_leap_year(
					int year
					)
{

    /**<p>如果输入的年是负的,那让它取绝对值</p>*/
	if (year < 0)
		year = -year;

	/**<p>判断输入的年能不能被4整除</p>*/
	if (year % 4 == 0)
	{
		/**<p>如果年能被100整除,并且不能被400整除,那返回FALSE</p>*/
		if (year % 100 == 0 && year % 400 != 0)
			return FALSE;

		/**<p>返回TRUE</p>*/
		return TRUE;
	}

	/**<p>年不能被4整除,返回FALSE</p>*/
	return FALSE;
}
static
unsigned int dmtime_n_days_from_year(struct tm *tm_datetime,	int year)
{
	int nYears,nLeapyears;
	int r[13];
	unsigned int nDays=0;
	int m_year;
	int i;
	unsigned int j;
	unsigned int month,day;

	r[0] = 0;
    r[1] = 31;
    r[2] = 28;
    r[3] = 31;
    r[4] = 30;
    r[5] = 31;
    r[6] = 30;
    r[7] = 31;
    r[8] = 31;
    r[9] = 30;
    r[10] = 31;
    r[11] = 30;
    r[12] = 31;
	m_year = tm_datetime->tm_year + 1900;
	month = tm_datetime->tm_mon + 1;
	day = tm_datetime->tm_mday;

	nYears = m_year - year;

	nLeapyears = 0;

	for(i = year; i < m_year; i++)
	{
		if (dmtime_is_leap_year(i))
			nLeapyears++;
	}

	nDays = nYears * 365 + nLeapyears;

	for (j = 1; j < month; j++)
		nDays += r[j];

	nDays += day - 1;

	if (dmtime_is_leap_year(m_year) && month > 2)
		nDays++;

	return nDays;
}
static int
dmtime_diff_day(tm *tm_end, tm *tm_start)
{

	int flag=0;
	int year2;
	int nDays,nDay1,nDay2;

	year2 = tm_start->tm_year + 1900;
	nDay1 = dmtime_n_days_from_year(tm_end, year2);
	nDay2 = dmtime_n_days_from_year(tm_start, year2);
	nDays = nDay1 - nDay2;

	return nDays;
}
static
int datatime_diff(
	tm *tm_start,
	tm *tm_end,
	char flag
)
{
	int year, month;
	int year1, month1, n;
	int hour, hour1;
	int min, min1;

	switch (flag) {
	case FUN_DATE_YEAR:
		year = tm_end->tm_year;
		year1 = tm_start->tm_year;
		n = year - year1;
		break;
	case FUN_DATE_QUARTER:
		year = tm_end->tm_year;
		year1 = tm_start->tm_year;
		month = tm_end->tm_mon;
		month1 = tm_start->tm_mon;
		n = 0;
		if (month < month1)
		{
			year --;
			month += 12;
		}
		n += (year - year1) * 4 + (month - month1 + 2) / 3;
		break;
	case FUN_DATE_MONTH:
		year = tm_end->tm_year;
		year1 = tm_start->tm_year;
		month = tm_end->tm_mon;;
		month1 = tm_start->tm_mon;
		n = year - year1;
		n = n * 12 + month - month1;
		break;
	case FUN_DATE_DAY:
		n = dmtime_diff_day(tm_end, tm_start);
		break;
	case FUN_DATE_HOUR:
		n = dmtime_diff_day(tm_end, tm_start);
		hour = tm_end->tm_hour;
		hour1 = tm_start->tm_hour;
		n = n * 24 + hour - hour1;
		break;
	case FUN_DATE_MIN:
		n = dmtime_diff_day(tm_end, tm_start);
		hour = tm_end->tm_hour;
		hour1 = tm_start->tm_hour;
		min = tm_end->tm_min;
		min1 = tm_start->tm_min;
		n = n * 24 + hour - hour1;
		n = n*60 + min - min1;
		break;
	default:
		n = 0;
		break;
	}
	return n;
}

int month_end_day(int year, int month)
{
	int day;
	switch (month)
	{
	case 4:
	case 6:
	case 9:
	case 11:
		day = 30;
		break;
	case 2:
		if (dmtime_is_leap_year(year))
			day = 29;
		else day = 28;
		break;
	default:
		day = 31;
		break;
	}
	return day;
}

int total_month_end_day(int year, int month, int months)
{
	int days = 0;
	for (int i=0; i<months; i++)
	{
		days += month_end_day(year, month);
		month++;
		if(month>12)
		{
			month = 1;
			year ++;
		}
	}
	return days;
}

int year_end_day(int year)
{
	int days = 0;
	for (int i=1; i<=12; i++)
	{
		days += month_end_day(year, i);
	}
	return days;
}
static
void format_date_sign(int *year, int *month)
{
    int    tmp_year;
    if ((*year >= 0 && *month >=0) || (*year <= 0 && *month <= 0))
        return;

    tmp_year = (*year * 12 + *month) / 12;
    *month = (*year * 12 + *month) % 12;
    *year = tmp_year;
}

static
void format_date(int *year, int *month)
{
	// 月进位到年
	if(*month >= 0)
	{
		*year += *month/12;
		*month %= 12;
	}
	else
	{
		*year -= -*month/12;
		*month = *month % 12;
	}

	if (*month == 0)
	{
		(*year) --;
		*month = 12;
	}
}
static
void month_add(int *year, int *month, int n)
{
	*month += n;
    format_date_sign(year, month);
    format_date(year, month);
}
static
void date_add(int *year, int *month, int *day, int n)
{
	int tmp;
	int tmp1 = (*day) + n;

	while (tmp1 <= 0)
	{
		(*month) --;
		tmp = month_end_day(*year, *month);
		tmp1 += tmp;
		if ((*month) < 1)
		{
			(*month) += 12;
			(*year) --;
		}
	}
	tmp = month_end_day(*year, *month);
	while (tmp < tmp1 )
	{
		tmp1 -= tmp;
		(*month) ++;
		if ((*month) > 12)
		{
			(*year) ++;
			(*month) = 1;
		}
		tmp = month_end_day(*year, *month);
	}
	(*day) = tmp1;

    format_date_sign(year, month);

    format_date(year, month);
}

void convert_dat_to_time(char* date_ptr,time_t *timep)
{
	tm tt;
    tt.tm_year=(*date_ptr - 100)*100 + (unsigned char)*(date_ptr + 1) - 100 - 1900;
    tt.tm_mon=*(date_ptr + 2) - 1;
    tt.tm_mday=*(date_ptr + 3);
    tt.tm_hour=*(date_ptr + 4) - 1;
    tt.tm_min=*(date_ptr + 5) - 1;
    tt.tm_sec=*(date_ptr + 6) - 1;
    *timep = mktime(&tt);
}

static
void second_add(int*month, int *day, int *hour, int *minute, int *second, int n)
{
	(*second) += n;
	while (*second >= 60)
	{
		*minute += 1;
		*second -= 60;
	}
	while (*minute >= 60)
	{
		*hour += 1;
		*minute -= 60;
	}
	while (*hour >= 24)
	{
		*day += 1;
		*hour -= 24;
	}
	while(*day >= 32)
	{
		*month += 1;
		*day -= 31;
	}
}

static
void trim_str(const char* p, const char* q, char* name)
{
	name[0] = 0;

	if (p >= q)
		return;

	while (p < q && (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t'))
		p++;

	if (p == q)
		return;

	q--;
	while (q > p && (*q == ' ' || *q == '\n' || *q == '\r' || *q == '\t'))
		q--;

	strncpy(name, p, q - p + 1);
	name[q - p + 1] = 0;
}
static
void trim_str_ex(const char* p, const char* q, char* name)
{
	name[0] = 0;

	if (p >= q)
		return;

	while (p < q && (*p == ' ' || *p == '\n' || *p == '\r' || *p == ',' || *p == '\t'))
		p++;

	if (p == q)
		return;

	q--;
	while (q > p && (*q == ' ' || *q == '\n' || *q == '\r' || *q == ',' || *p == '\t'))
		q--;

	strncpy(name, p, q - p + 1);
	name[q - p + 1] = 0;
}
static void replace_char(char* src, char old_char, char new_char)
{
	while(*src)
	{
		if (*src == old_char)
		{
			*src = new_char;
		}
		src ++;
	}
}
static int get_config_info(char* cfg_name, char* cfg_value)
{
#define MAX_CFG_FILE_LEN		2048
	FILE*	fp;
	char	buf[MAX_CFG_FILE_LEN];
	char	name[MAX_CFG_FILE_LEN];
	char	value[MAX_CFG_FILE_LEN];
	char	*p, *q;
	char	full_conf_file[300];
	if (cfg_value == NULL)
		return FALSE;
#ifndef WIN32
	char *d5000_home;
    d5000_home = getenv("D5000_HOME");
	if (d5000_home)
	{
		strcpy(full_conf_file,d5000_home);
		strcat(full_conf_file,"/conf/");
		strcat(full_conf_file, CONF_FILE_NAME);
	}
    else
	{
		printf("未取到环境变量D5000_HOME，使用默认的配置文件路径\r\n");
		strcpy(full_conf_file, CONF_FILE_NAME);
	}
#else
	strcpy(full_conf_file, CONF_FILE_NAME);
#endif
	strcpy(cfg_value, "");
	fp = fopen(full_conf_file, "rt");
	// open file

	if (fp == NULL)
	{
		printf("配置文件(%s) 未找到", full_conf_file);
		return FALSE;
	}

	// read file
	memset(buf, 0, MAX_CFG_FILE_LEN);
	if (fread(buf, 1, MAX_CFG_FILE_LEN - 1, fp) == 0)
	{
		fclose(fp);
		printf("配置文件(%s) 读取失败", full_conf_file);
		return FALSE;
	}

	// search name and value
	p = buf;
	while (*p != 0)
	{
		q = strchr(p, '=');
		if (q == NULL || q == p)
			break;

		trim_str(p, q, name);
		if (strlen(name) == 0)
			break;

		p = strchr(q, '{');
		if (p == NULL)
			break;

		q = strchr(p, '}');
		if (q == NULL)
			break;

		trim_str(p + 1, q, value);
		if (strlen(value) == 0)
			break;

		// check name
		if (stricmp(name, cfg_name) == 0)
		{
			strcpy(cfg_value, value);

			fclose(fp);
			return TRUE;
		}

		p = q + 1;
	}
	printf("配置文件(%s)中未找到'%s'配置信息", full_conf_file, cfg_name);
	fclose(fp);

	return FALSE;
}
//配置文件读取函数
static
int get_config_info_ex(char* pdata, char* cfg_name, char* cfg_value)
{
	char	name[OBJECT_NAME_LEN];
	char	value[MAX_CONF_VALUE_LEN];
	char	*p, *q;
	int		value_len;
	strcpy(cfg_value, "");
	// search name and value
	p = pdata;
	while (*p != 0)
	{
		q = strchr(p, '=');
		if (q == NULL || q == p)
			break;

		trim_str(p, q, name);
		if (strlen(name) == 0)
			break;

		p = strchr(q, '(');
		while(p != NULL)
		{
			if (*(p-1) == '\\')
			{
				q = p+1;
			}
			else
			{
				break;
			}
			p = strchr(q, '(');
		}
		if (p == NULL)
			break;

		q = strchr(p, ')');
		while(q != NULL)
		{
			if (*(q-1) == '\\')
			{
				q = q+1;
			}
			else
			{
				break;
			}
			q = strchr(q, ')');
		}
		if (q == NULL)
			break;

		trim_str(p + 1, q, value);
		value_len = strlen(value);
// 		if (value_len == 0)
// 			break;

		// check name
		if (stricmp(name, cfg_name) == 0)
		{
			if (value_len >= OBJECT_NAME_LEN && stricmp(cfg_name, COL_NAME_FT) != 0)
			{
				printf("配置项(%s)的值过长，应该限定在%d个字符以内", cfg_name, OBJECT_NAME_LEN);
				break;
			}
			strcpy(cfg_value, value);
			replace_char(cfg_value, '\\', ' ');
			return TRUE;
		}
		p = q + 1;
	}

	return FALSE;
}
static Curveinfo *g_curveinfop = NULL;
static int		g_has_curveinfo_init = 0;
//static Curveinfo *g_sheetinfop = NULL;
static
int InitMid()
{
	FILE *fp;
    int finished = 0;
	sb4 strLen;
	char *pfiledata = NULL;
	char *pfiledata2 = NULL;
	char *ptemp1 = NULL;
	char *ptemp2 = NULL;
	Curveinfo *temp_info = NULL;
	char tempstr[MAX_CONF_VALUE_LEN];
	char *d5000_home;
    char full_conf_file[400];

	if (g_curveinfop != NULL)
	{
		return 1;
	}
	else
	{
		get_config_info(LOG_FLAG, tempstr);
		g_log_flag = atoi(tempstr);
	}
	memset(full_conf_file, 0, 400);
//	memset(full_sheet_file, 0, 400);
#ifdef WIN32
	strcpy(full_conf_file, CONF_FILE_NAME_CURVE);
//	strcpy(full_sheet_file, CONF_FILE_NAME_SHEET);
#else
	memset(full_conf_file,0,400);
//	memset(full_sheet_file,0,400);
    d5000_home = getenv("D5000_HOME");
	if (d5000_home)
	{
		strcpy(full_conf_file, d5000_home);
		strcat(full_conf_file, "/conf/");
	}
	strcat(full_conf_file, CONF_FILE_NAME_CURVE);
// 	strcpy(full_sheet_file, d5000_home);
//     strcat(full_sheet_file, "/conf/");
// 	strcat(full_sheet_file, CONF_FILE_NAME_SHEET);
#endif
//	printf("配置文件路径: %s\n", full_conf_file);
//	printf("配置表格文件路径: %s\n", full_sheet_file);

	fp = fopen(full_conf_file, "r");
    if( fp == NULL)
    {
		g_curveinfop = (Curveinfo*) new Curveinfo;
		g_curveinfop->next = NULL;
        perror("Open configure file error!\n");
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    strLen = ftell(fp);
    pfiledata = (char *)dcisg_malloc(strLen + 1);
	if(pfiledata == NULL)
	{
		fclose(fp);
		return 0;
	}
    memset(pfiledata, 0, strLen + 1);
    rewind(fp);
    fread(pfiledata, 1, strLen, fp);
    fclose(fp);
	ptemp2 = pfiledata;
	ptemp1 = strchr(pfiledata, '}');
	while(ptemp1)
	{
		*ptemp1 = '\0';
		ptemp2 = strchr(ptemp2, '{');
		if (ptemp2 == NULL)
		{
			break;
		}
		ptemp2 ++;
		temp_info = new Curveinfo;
		temp_info->next = NULL;
		if(g_curveinfop == NULL)
		{
			g_curveinfop = temp_info;
		}
		else
		{
			temp_info->next = g_curveinfop;
			g_curveinfop = temp_info;
		}
		get_config_info_ex(ptemp2, CONFIG_ID, g_curveinfop->conf_id);
		get_config_info_ex(ptemp2, CURVE_ID, g_curveinfop->curve_id);
		get_config_info_ex(ptemp2, TABLE_NAME_FT, g_curveinfop->table_name_ft);
		get_config_info_ex(ptemp2, TABLE_NAME_MODEL, g_curveinfop->table_name_model);
		get_config_info_ex(ptemp2, STATE_TABLE_NAME_FT, g_curveinfop->state_table_name_ft);
		get_config_info_ex(ptemp2, ADDITION_COL_NAME_FT, g_curveinfop->addition_column_name_ft);
		get_config_info_ex(ptemp2, COL_NAME_FT, g_curveinfop->column_name_ft);
		get_config_info_ex(ptemp2, DEV_KEY_COL_NAME, g_curveinfop->dev_key_col_name);
		get_config_info_ex(ptemp2, DATE_COL_NAME, g_curveinfop->date_col_name);
		get_config_info_ex(ptemp2, HOUR_COL_NAME, g_curveinfop->hour_col_name);
		get_config_info_ex(ptemp2, DAY_COL_NAME, g_curveinfop->day_col_name);
		get_config_info_ex(ptemp2, MONTH_COL_NAME, g_curveinfop->month_col_name);
		get_config_info_ex(ptemp2, MINUTE_COL_NAME, g_curveinfop->minute_col_name);
		get_config_info_ex(ptemp2, YEAR_COL_NAME, g_curveinfop->year_col_name);
		get_config_info_ex(ptemp2, WHERE_NOR, g_curveinfop->where_nor);
		get_config_info_ex(ptemp2, INSERT_EXP, g_curveinfop->insert_exp);
		get_config_info_ex(ptemp2, INDEX_STR, g_curveinfop->index_str);
		if (strlen(g_curveinfop->insert_exp) == 0)
		{
			strcat(g_curveinfop->insert_exp, "%s) %s)");
		}
		memset(tempstr, 0, MAX_CONF_VALUE_LEN);
		g_curveinfop->base_col_index = 0;
		get_config_info_ex(ptemp2, BASE_COL_INDEX, tempstr);
		if (strlen(tempstr) > 0)
		{
			g_curveinfop->base_col_index = atoi(tempstr);
		}
		memset(tempstr, 0, MAX_CONF_VALUE_LEN);
		g_curveinfop->row_time_span = 0;
		get_config_info_ex(ptemp2, ROW_TIME_SPAN, tempstr);
		if (strlen(tempstr) > 0)
		{
			g_curveinfop->row_time_span = atoi(tempstr);
		}
		memset(tempstr, 0, MAX_CONF_VALUE_LEN);
		g_curveinfop->time_pace = 0;
		get_config_info_ex(ptemp2, TIME_PACE, tempstr);
		if (strlen(tempstr) > 0)
		{
			g_curveinfop->time_pace = atoi(tempstr);
		}
		ptemp2 = ptemp1+1;
		ptemp1 = strchr(ptemp2, '}');
	}

    dcisg_free(pfiledata);
	dcisg_free(pfiledata2);
	return 1;
}
#define RETURN_InitializeCurveInfo_FALSE log_report("初始化曲线配置项出错：%s(%d)", error.error_info, error.error_no);dcisg_free(src_data);dcisg_free(src_attr);return FALSE
bool CDci::InitializeCurveInfo()
{
	int src_cols = 0;
	int src_rows = 0;
	int col = 0;
	char *src_data = NULL;
	ColAttr *src_attr = NULL;
	ub4	src_buf_size = 0;
	char *temp_data = NULL;
	char sql[1024];
	ErrorInfo error;
	Curveinfo *temp_info = NULL;
	if (g_curveinfop != NULL)
	{
		return TRUE;
	}
	memset(&error, 0, sizeof(error));
	sprintf(sql, "SELECT CONF_ID,CURVE_ID,TABLE_NAME_FT,STATE_TABLE_NAME_FT,COLUMN_NAME_FT,\
		ADDITION_COLUMN_NAME_FT,DEV_KEY_COL_NAME,MINUTE_COL_NAME,HOUR_COL_NAME,DAY_COL_NAME,\
		MONTH_COL_NAME,YEAR_COL_NAME,DATE_COL_NAME,BASE_COL_INDEX,ROW_TIME_SPAN,TIME_PACE,\
		WHERE_NOR,INSERT_EXP,INDEX_STR,TABLE_NAME_MODEL \
		FROM CURVE_CONFIG");
	if(!ReadDataEx(sql, -1, &src_rows, &src_cols, &src_attr, &src_data, &src_buf_size, &error))
	{
		RETURN_InitializeCurveInfo_FALSE;
	}
	if (src_rows == 0)
	{
		sprintf(error.error_info, "曲线配置表中的配置项是空记录");
		RETURN_InitializeCurveInfo_FALSE;
	}
	temp_data = src_data;
	while(src_rows-->0)
	{
		col = 0;
		temp_info = new Curveinfo;
		temp_info->next = NULL;
		if(g_curveinfop == NULL)
		{
			g_curveinfop = temp_info;
		}
		else
		{
			temp_info->next = g_curveinfop;
			g_curveinfop = temp_info;
		}
		strcpy(temp_info->conf_id, "");			//配置项号
		strcpy(temp_info->curve_id, "");			//科东为应用号，该号可以跟MIDHIS服务器的数据库连接相对应
		strcpy(temp_info->table_name_ft, "");		//表名表达式
		strcpy(temp_info->table_name_model, "");		//模板表名，该名称和table_name_ft所指的表达式应该是相对应的
		strcpy(temp_info->state_table_name_ft, "");		//采样表对应的状态表名表达式
//		strcpy(temp_info->state_table_name_model, "");		//模板表名，该名称和table_name_ft所指的表达式应该是相对应的
		strcpy(temp_info->column_name_ft, "");			//列名表达式
		strcpy(temp_info->addition_column_name_ft, "");	//需要加在结果集中的附加列表达式,这个选项只对GetSheetData接口有效
		strcpy(temp_info->dev_key_col_name, "");	//设备KEYID所在的列名
		strcpy(temp_info->date_col_name, "");		//以小时为采样单位的表结构(TABLE_TYPE_COL_HOUR24表结构类型)，竖向日期列名
		strcpy(temp_info->hour_col_name, "");		//如果存在小时列，用来竖向指定某个小时某个设备的采样，那么这边存放该小时列的列名
		strcpy(temp_info->minute_col_name, "");	//如果存在分钟列，用来竖向指定某个分钟某个设备的采样，那么这边存放该分钟列的列名
		strcpy(temp_info->day_col_name, "");		//表示日期中日数的列名
		strcpy(temp_info->month_col_name, "");	//表示日期中月数的列名
		strcpy(temp_info->year_col_name, "");		//表示日期中年数的列名
		temp_info->base_col_index = 0;						//当以%d的格式来表达列名命名格式时，该值为格式的起始值
		temp_info->row_time_span = 0;						//单行跨度时间，单位秒
		temp_info->time_pace =0;							//采样单位步长，单位秒
		strcpy(temp_info->where_nor, "");			//为上下限的曲线服务
		strcpy(temp_info->insert_exp, "");		//更新曲线在执行UPDATE时，如果未更新行，那么用这个表达式来做INSERT
		strcpy(temp_info->index_str, "");
		//////////////////////////////////////////////////////////////////////////
		if (src_attr[col].data_type != DCI_STR || src_attr[col].data_size >= OBJECT_NAME_LEN)
		{
			sprintf(error.error_info, "%s的数据类型或长度跟预期的不一致", src_attr[col].col_name);
			RETURN_InitializeCurveInfo_FALSE;
		}
		if (*(sb2*)(temp_data + src_attr[col].data_size) == 0)
		{
			strncpy(temp_info->conf_id, temp_data, OBJECT_NAME_LEN);
		}
		temp_data += src_attr[col].data_size + sizeof(sb2);
		col++;
		//////////////////////////////////////////////////////////////////////////
		if (src_attr[col].data_type != DCI_STR || src_attr[col].data_size >= OBJECT_NAME_LEN)
		{
			sprintf(error.error_info, "%s的数据类型或长度跟预期的不一致", src_attr[col].col_name);
			RETURN_InitializeCurveInfo_FALSE;
		}
		if (*(sb2*)(temp_data + src_attr[col].data_size) == 0)
		{
			strncpy(temp_info->curve_id, temp_data, OBJECT_NAME_LEN);
		}
		temp_data += src_attr[col].data_size + sizeof(sb2);
		col++;
		//////////////////////////////////////////////////////////////////////////
		if (src_attr[col].data_type != DCI_STR || src_attr[col].data_size >= OBJECT_NAME_LEN)
		{
			sprintf(error.error_info, "%s的数据类型或长度跟预期的不一致", src_attr[col].col_name);
			RETURN_InitializeCurveInfo_FALSE;
		}
		if (*(sb2*)(temp_data + src_attr[col].data_size) == 0)
		{
			strncpy(temp_info->table_name_ft, temp_data, OBJECT_NAME_LEN);
		}
		temp_data += src_attr[col].data_size + sizeof(sb2);
		col++;
		//////////////////////////////////////////////////////////////////////////
		if (src_attr[col].data_type != DCI_STR || src_attr[col].data_size >= OBJECT_NAME_LEN)
		{
			sprintf(error.error_info, "%s的数据类型或长度跟预期的不一致", src_attr[col].col_name);
			RETURN_InitializeCurveInfo_FALSE;
		}
		if (*(sb2*)(temp_data + src_attr[col].data_size) == 0)
		{
			strncpy(temp_info->state_table_name_ft, temp_data, OBJECT_NAME_LEN);
		}
		temp_data += src_attr[col].data_size + sizeof(sb2);
		col++;
		//////////////////////////////////////////////////////////////////////////
		if (src_attr[col].data_type != DCI_STR || src_attr[col].data_size >= 2048)
		{
			sprintf(error.error_info, "%s的数据类型或长度跟预期的不一致", src_attr[col].col_name);
			RETURN_InitializeCurveInfo_FALSE;
		}
		if (*(sb2*)(temp_data + src_attr[col].data_size) == 0)
		{
			strncpy(temp_info->column_name_ft, temp_data, 2048);
		}
		temp_data += src_attr[col].data_size + sizeof(sb2);
		col++;
		//////////////////////////////////////////////////////////////////////////
		if (src_attr[col].data_type != DCI_STR || src_attr[col].data_size >= OBJECT_NAME_LEN)
		{
			sprintf(error.error_info, "%s的数据类型或长度跟预期的不一致", src_attr[col].col_name);
			RETURN_InitializeCurveInfo_FALSE;
		}
		if (*(sb2*)(temp_data + src_attr[col].data_size) == 0)
		{
			strncpy(temp_info->addition_column_name_ft, temp_data, OBJECT_NAME_LEN);
		}
		temp_data += src_attr[col].data_size + sizeof(sb2);
		col++;
		//////////////////////////////////////////////////////////////////////////
		if (src_attr[col].data_type != DCI_STR || src_attr[col].data_size >= OBJECT_NAME_LEN)
		{
			sprintf(error.error_info, "%s的数据类型或长度跟预期的不一致", src_attr[col].col_name);
			RETURN_InitializeCurveInfo_FALSE;
		}
		if (*(sb2*)(temp_data + src_attr[col].data_size) == 0)
		{
			strncpy(temp_info->dev_key_col_name, temp_data, OBJECT_NAME_LEN);
		}
		temp_data += src_attr[col].data_size + sizeof(sb2);
		col++;
		//////////////////////////////////////////////////////////////////////////
		if (src_attr[col].data_type != DCI_STR || src_attr[col].data_size >= OBJECT_NAME_LEN)
		{
			sprintf(error.error_info, "%s的数据类型或长度跟预期的不一致", src_attr[col].col_name);
			RETURN_InitializeCurveInfo_FALSE;
		}
		if (*(sb2*)(temp_data + src_attr[col].data_size) == 0)
		{
			strncpy(temp_info->minute_col_name, temp_data, OBJECT_NAME_LEN);
		}
		temp_data += src_attr[col].data_size + sizeof(sb2);
		col++;
		//////////////////////////////////////////////////////////////////////////
		if (src_attr[col].data_type != DCI_STR || src_attr[col].data_size >= OBJECT_NAME_LEN)
		{
			sprintf(error.error_info, "%s的数据类型或长度跟预期的不一致", src_attr[col].col_name);
			RETURN_InitializeCurveInfo_FALSE;
		}
		if (*(sb2*)(temp_data + src_attr[col].data_size) == 0)
		{
			strncpy(temp_info->hour_col_name, temp_data, OBJECT_NAME_LEN);
		}
		temp_data += src_attr[col].data_size + sizeof(sb2);
		col++;
		//////////////////////////////////////////////////////////////////////////
		if (src_attr[col].data_type != DCI_STR || src_attr[col].data_size >= OBJECT_NAME_LEN)
		{
			sprintf(error.error_info, "%s的数据类型或长度跟预期的不一致", src_attr[col].col_name);
			RETURN_InitializeCurveInfo_FALSE;
		}
		if (*(sb2*)(temp_data + src_attr[col].data_size) == 0)
		{
			strncpy(temp_info->day_col_name, temp_data, OBJECT_NAME_LEN);
		}
		temp_data += src_attr[col].data_size + sizeof(sb2);
		col++;
		//////////////////////////////////////////////////////////////////////////
		if (src_attr[col].data_type != DCI_STR || src_attr[col].data_size >= OBJECT_NAME_LEN)
		{
			sprintf(error.error_info, "%s的数据类型或长度跟预期的不一致", src_attr[col].col_name);
			RETURN_InitializeCurveInfo_FALSE;
		}
		if (*(sb2*)(temp_data + src_attr[col].data_size) == 0)
		{
			strncpy(temp_info->month_col_name, temp_data, OBJECT_NAME_LEN);
		}
		temp_data += src_attr[col].data_size + sizeof(sb2);
		col++;
		//////////////////////////////////////////////////////////////////////////
		if (src_attr[col].data_type != DCI_STR || src_attr[col].data_size >= OBJECT_NAME_LEN)
		{
			sprintf(error.error_info, "%s的数据类型或长度跟预期的不一致", src_attr[col].col_name);
			RETURN_InitializeCurveInfo_FALSE;
		}
		if (*(sb2*)(temp_data + src_attr[col].data_size) == 0)
		{
			strncpy(temp_info->year_col_name, temp_data, OBJECT_NAME_LEN);
		}
		temp_data += src_attr[col].data_size + sizeof(sb2);
		col++;
		//////////////////////////////////////////////////////////////////////////
		if (src_attr[col].data_type != DCI_STR || src_attr[col].data_size >= OBJECT_NAME_LEN)
		{
			sprintf(error.error_info, "%s的数据类型或长度跟预期的不一致", src_attr[col].col_name);
			RETURN_InitializeCurveInfo_FALSE;
		}
		if (*(sb2*)(temp_data + src_attr[col].data_size) == 0)
		{
			strncpy(temp_info->date_col_name, temp_data, OBJECT_NAME_LEN);
		}
		temp_data += src_attr[col].data_size + sizeof(sb2);
		col++;
		//////////////////////////////////////////////////////////////////////////
		if (src_attr[col].data_type != DCI_INT || src_attr[col].data_size != sizeof(int))
		{
			sprintf(error.error_info, "%s的数据类型或长度跟预期的不一致", src_attr[col].col_name);
			RETURN_InitializeCurveInfo_FALSE;
		}
		if (*(sb2*)(temp_data + src_attr[col].data_size) == 0)
		{
			memcpy(&temp_info->base_col_index, temp_data, sizeof(int));
		}
		temp_data += src_attr[col].data_size + sizeof(sb2);
		col++;
		//////////////////////////////////////////////////////////////////////////
		if (src_attr[col].data_type != DCI_INT || src_attr[col].data_size != sizeof(int))
		{
			sprintf(error.error_info, "%s的数据类型或长度跟预期的不一致", src_attr[col].col_name);
			RETURN_InitializeCurveInfo_FALSE;
		}
		if (*(sb2*)(temp_data + src_attr[col].data_size) == 0)
		{
			memcpy(&temp_info->row_time_span, temp_data, sizeof(int));
		}
		temp_data += src_attr[col].data_size + sizeof(sb2);
		col++;
		//////////////////////////////////////////////////////////////////////////
		if (src_attr[col].data_type != DCI_INT || src_attr[col].data_size != sizeof(int))
		{
			sprintf(error.error_info, "%s的数据类型或长度跟预期的不一致", src_attr[col].col_name);
			RETURN_InitializeCurveInfo_FALSE;
		}
		if (*(sb2*)(temp_data + src_attr[col].data_size) == 0)
		{
			memcpy(&temp_info->time_pace, temp_data, sizeof(int));
		}
		temp_data += src_attr[col].data_size + sizeof(sb2);
		col++;
		//////////////////////////////////////////////////////////////////////////
		if (src_attr[col].data_type != DCI_STR || src_attr[col].data_size >= OBJECT_NAME_LEN)
		{
			sprintf(error.error_info, "%s的数据类型或长度跟预期的不一致", src_attr[col].col_name);
			RETURN_InitializeCurveInfo_FALSE;
		}
		if (*(sb2*)(temp_data + src_attr[col].data_size) == 0)
		{
			strncpy(temp_info->where_nor, temp_data, OBJECT_NAME_LEN);
		}
		temp_data += src_attr[col].data_size + sizeof(sb2);
		col++;
		//////////////////////////////////////////////////////////////////////////
		if (src_attr[col].data_type != DCI_STR || src_attr[col].data_size >= OBJECT_NAME_LEN)
		{
			sprintf(error.error_info, "%s的数据类型或长度跟预期的不一致", src_attr[col].col_name);
			RETURN_InitializeCurveInfo_FALSE;
		}
		if (*(sb2*)(temp_data + src_attr[col].data_size) == 0)
		{
			strncpy(temp_info->insert_exp, temp_data, OBJECT_NAME_LEN);
		}
		temp_data += src_attr[col].data_size + sizeof(sb2);
		col++;
		//////////////////////////////////////////////////////////////////////////
		if (src_attr[col].data_type != DCI_STR || src_attr[col].data_size >= OBJECT_NAME_LEN)
		{
			sprintf(error.error_info, "%s的数据类型或长度跟预期的不一致", src_attr[col].col_name);
			RETURN_InitializeCurveInfo_FALSE;
		}
		if (*(sb2*)(temp_data + src_attr[col].data_size) == 0)
		{
			strncpy(temp_info->index_str, temp_data, OBJECT_NAME_LEN);
		}
		temp_data += src_attr[col].data_size + sizeof(sb2);
		col++;
		//////////////////////////////////////////////////////////////////////////
		if (src_attr[col].data_type != DCI_STR || src_attr[col].data_size >= OBJECT_NAME_LEN)
		{
			sprintf(error.error_info, "%s的数据类型或长度跟预期的不一致", src_attr[col].col_name);
			RETURN_InitializeCurveInfo_FALSE;
		}
		if (*(sb2*)(temp_data + src_attr[col].data_size) == 0)
		{
			strncpy(temp_info->table_name_model, temp_data, OBJECT_NAME_LEN);
		}
		temp_data += src_attr[col].data_size + sizeof(sb2);
		col++;
		//////////////////////////////////////////////////////////////////////////

		if (col != src_cols)
		{
			sprintf(error.error_info, "曲线配置项非法的列数");
			RETURN_InitializeCurveInfo_FALSE;
		}
	}
	dcisg_free(src_attr);
	dcisg_free(src_data);
	return TRUE;
}
static
int CreateName(const char* name_ft, char* name, int year, int month, int day, int hour, int minute, int second)
{
	char temp[OBJECT_NAME_LEN];
	const char* p;
	const char* p1;
	memset(temp, 0, OBJECT_NAME_LEN);
	p1 = name_ft;
	p = strchr(name_ft, '@');
	if (p == NULL)
	{
		strcpy(name, name_ft);
	}
	while(p != NULL)
	{
		memset(temp, 0, OBJECT_NAME_LEN);
		memcpy(name, p1, p - p1);
		name += (p - p1);
		p++;
		switch (*p)
		{
		case 'Y':
			sprintf(temp, "%d", year);
			if (memcmp(p, "YYYY", 4) == 0)
			{
				memcpy(name, temp, 4);
				name += 4;
				p += 4;
			}
			else
			{
				memcpy(name, temp+2, 2);
				name += 2;
				p += 2;
			}
			break;
		case 'M':
			if (memcmp(p, "MM", 2) == 0)
			{
				sprintf(temp, "%02d", month);
				p += 2;
			}
			else
			{
				sprintf(temp, "%d", month);
				p += 1;
			}
			strcpy(name, temp);
			name += strlen(temp);
			break;
		case 'D':
			if (memcmp(p, "DD", 2) == 0)
			{
				sprintf(temp, "%02d", day);
				p += 2;
			}
			else
			{
				sprintf(temp, "%d", day);
				p += 1;
			}
			strcpy(name, temp);
			name += strlen(temp);
			break;
		case 'H':
			if (memcmp(p, "HH", 2) == 0)
			{
				sprintf(temp, "%02d", hour);
				p += 2;
			}
			else
			{
				sprintf(temp, "%d", hour);
				p += 1;
			}
			strcpy(name, temp);
			name += strlen(temp);
			break;
		case 'I':
			if (memcmp(p, "II", 2) == 0)
			{
				sprintf(temp, "%02d", minute);
				p += 2;
			}
			else
			{
				sprintf(temp, "%d", minute);
				p += 1;
			}
			strcpy(name, temp);
			name += strlen(temp);
			break;
		case 'S':
			if (memcmp(p, "SS", 2) == 0)
			{
				sprintf(temp, "%02d", second);
				p += 2;
			}
			else
			{
				sprintf(temp, "%d", second);
				p += 1;
			}
			strcpy(name, temp);
			name += strlen(temp);
			break;
		case '%':
			if (memcmp(p+1, "d", 1) == 0)
			{
				sprintf(temp, "%02d", minute/15);
				p += 2;
			}
			strcpy(name, temp);
			name += strlen(temp);
			break;
		default:
			return 0;
		}
		p1 = p;
		p = strchr(p, '@');
	}
	return 1;
}

//table_name_ft参数格式：表名中@YYYY表示年份，@YY表示后两位年份，@MM表示月份，@M表示月份，但是小于10月的月分前面不加0，@DD表示日号，@D表示日号，但是小于10的日号前面不加0
int GetTableName(const char* table_name_ft, char **data_time, char **table_name, int &count, tm *tm_start, tm *tm_end)
{
	int year, month, day, hour, minute, second;
	int days = 0, months = 0, years = 0;
	count = 0;
	*table_name = NULL;
	*data_time = NULL;
	if (strlen(table_name_ft) == 0)
	{
		return FALSE;
	}
	year = tm_start->tm_year + 1900;
	month = tm_start->tm_mon + 1;
	day = tm_start->tm_mday;
	hour = tm_start->tm_hour;
	minute = tm_start->tm_min;
	second = tm_start->tm_sec;
	if (strstr(table_name_ft, "@D") != NULL)
	{
		days = datatime_diff(tm_start, tm_end, FUN_DATE_DAY);
		days ++;
		*table_name = (char*)dcisg_malloc(days * OBJECT_NAME_LEN);
		*data_time = (char*)dcisg_malloc(days* OBJECT_NAME_LEN);
		if (*table_name == NULL || *data_time == NULL)
		{
			return 0;
		}
		memset(*table_name, 0, days * OBJECT_NAME_LEN);
		memset(*data_time, 0, days * OBJECT_NAME_LEN);
		while(days-- > 0 )
		{
			if(!CreateName(table_name_ft, *table_name + count*OBJECT_NAME_LEN, year, month, day, 0, 0, 0))
			{
				dcisg_free(*table_name);
				return 0;
			}
			sprintf(*data_time + count*OBJECT_NAME_LEN, "%d-%02d-%02d 00:00:00", year, month, day);
			date_add(&year, &month, &day, 1);
			count++;
		}
	}
	else if (strstr(table_name_ft, "@M") != NULL)
	{
		months = datatime_diff(tm_start, tm_end, FUN_DATE_MONTH);
		months ++;
		*table_name = (char*)dcisg_malloc(months * OBJECT_NAME_LEN);
		*data_time = (char*)dcisg_malloc(months* OBJECT_NAME_LEN);
		if (*table_name == NULL || *data_time == NULL)
		{
			return 0;
		}
		memset(*table_name, 0, months * OBJECT_NAME_LEN);
		memset(*data_time, 0, months * OBJECT_NAME_LEN);
		day = 1;
		while(months-- > 0 )
		{
			if(!CreateName(table_name_ft, *table_name + count*OBJECT_NAME_LEN, year, month, day, 0, 0, 0))
			{
				dcisg_free(*table_name);
				return 0;
			}
			sprintf(*data_time + count*OBJECT_NAME_LEN, "%d-%02d-%02d 00:00:00", year, month, day);
			month_add(&year, &month, 1);
			count++;
		}
	}
	else if (strstr(table_name_ft, "@YY") != NULL)
	{
		years = datatime_diff(tm_start, tm_end, FUN_DATE_YEAR);
		years ++;
		*table_name = (char*)dcisg_malloc(years * OBJECT_NAME_LEN);
		*data_time = (char*)dcisg_malloc(years* OBJECT_NAME_LEN);
		if (*table_name == NULL || *data_time == NULL)
		{
			return 0;
		}
		memset(*table_name, 0, years * OBJECT_NAME_LEN);
		memset(*data_time, 0, years * OBJECT_NAME_LEN);
		day = 1;
		month = 1;
		while(years-- > 0 )
		{
			if(!CreateName(table_name_ft, *table_name + count*OBJECT_NAME_LEN, year, month, day, 0, 0, 0))
			{
				dcisg_free(*table_name);
				return 0;
			}
			sprintf(*data_time + count*OBJECT_NAME_LEN, "%d-%02d-%02d 00:00:00", year, month, day);
			year ++;
			count++;
		}
	}
	else
	{
		*table_name = (char*)dcisg_malloc(OBJECT_NAME_LEN);
		*data_time = (char*)dcisg_malloc(OBJECT_NAME_LEN);
		if (*table_name == NULL || *data_time == NULL)
		{
			return 0;
		}
		memset(*table_name, 0, OBJECT_NAME_LEN);
		memset(*data_time, 0, OBJECT_NAME_LEN);
		strcpy(*table_name, table_name_ft);
		sprintf(*data_time, "%d-%02d-%02d 00:00:00", year, month, day);
		count = 1;
	}

	return 1;
}

int GetTableType(const char* table_name_ft)
{
	if (strstr(table_name_ft, "@D") != NULL)
	{
		return TABLE_TYPE_DAY;
	}
	else if (strstr(table_name_ft, "@M") != NULL)
	{
		return TABLE_TYPE_MONTH;
	}
	else if (strstr(table_name_ft, "@YY") != NULL)
	{
		return TABLE_TYPE_YEAR;
	}
	else
	{
		return TABLE_TYPE_NONE;
	}
}
static
void GetEndTableTime(int table_type, time_t ltime, time_t &out_end_table_time)
{
	tm tm_start;
	tm_start = *localtime(&ltime);
	switch (table_type)
	{
	case TABLE_TYPE_DAY:
		tm_start.tm_hour = 0;
		tm_start.tm_min = 0;
		tm_start.tm_sec = 0;
		out_end_table_time = mktime(&tm_start) + ONE_DAY_SECOND - 1;
		break;
	case TABLE_TYPE_MONTH:
		tm_start.tm_mday = 1;
		tm_start.tm_hour = 0;
		tm_start.tm_min = 0;
		tm_start.tm_sec = 0;
		out_end_table_time = mktime(&tm_start) + ONE_DAY_SECOND * month_end_day(tm_start.tm_year + 1900, tm_start.tm_mon + 1) - 1;
		break;
	case TABLE_TYPE_YEAR:
		tm_start.tm_mon = 0;
		tm_start.tm_mday = 1;
		tm_start.tm_hour = 0;
		tm_start.tm_min = 0;
		tm_start.tm_sec = 0;
		out_end_table_time = mktime(&tm_start) + ONE_DAY_SECOND * year_end_day(tm_start.tm_year + 1900) - 1;
		break;
	default:
		//memset(&out_end_table_time, 0XEF, sizeof(out_end_table_time));
		out_end_table_time = ltime;//无行跨度更新
		break;
	}
}
//addition_col_name_ft参数格式：表名中@YYYY表示年份，@YY表示后两位年份，@MM表示月份，@M表示月份，但是小于10月的月分前面不加0，@DD表示日号，@D表示日号，但是小于10的日号前面不加0
static
int GetAdditionColName(const char* addition_name_ft, char **addition_name, int &count, tm *tm_start, tm *tm_end)
{
	int year, month, day, hour, minute, second;
	int days = 0, months = 0, years = 0;
	count = 0;
	*addition_name = NULL;
	if (strlen(addition_name_ft) == 0)
	{
		return FALSE;
	}
	year = tm_start->tm_year + 1900;
	month = tm_start->tm_mon + 1;
	day = tm_start->tm_mday;
	hour = tm_start->tm_hour;
	minute = tm_start->tm_min;
	second = tm_start->tm_sec;
	if (strstr(addition_name_ft, "@D") != NULL)
	{
		days = datatime_diff(tm_start, tm_end, FUN_DATE_DAY);
		days ++;
		*addition_name = (char*)dcisg_malloc(days * OBJECT_NAME_LEN);
		if (*addition_name == NULL)
		{
			return 0;
		}
		memset(*addition_name, 0, days * OBJECT_NAME_LEN);
		while(days-- > 0 )
		{
			if(!CreateName(addition_name_ft, *addition_name + count*OBJECT_NAME_LEN, year, month, day, 0, 0, 0))
			{
				dcisg_free(*addition_name);
				return 0;
			}
			date_add(&year, &month, &day, 1);
			count++;
		}
	}
	else if (strstr(addition_name_ft, "@M") != NULL)
	{
		months = datatime_diff(tm_start, tm_end, FUN_DATE_MONTH);
		months ++;
		*addition_name = (char*)dcisg_malloc(months * OBJECT_NAME_LEN);
		if (*addition_name == NULL)
		{
			return 0;
		}
		memset(*addition_name, 0, months * OBJECT_NAME_LEN);
		day = 1;
		while(months-- > 0 )
		{
			if(!CreateName(addition_name_ft, *addition_name + count*OBJECT_NAME_LEN, year, month, day, 0, 0, 0))
			{
				dcisg_free(*addition_name);
				return 0;
			}
			month_add(&year, &month, 1);
			count++;
		}
	}
	else if (strstr(addition_name_ft, "@YY") != NULL)
	{
		years = datatime_diff(tm_start, tm_end, FUN_DATE_YEAR);
		years ++;
		*addition_name = (char*)dcisg_malloc(years * OBJECT_NAME_LEN);
		if (*addition_name == NULL)
		{
			return 0;
		}
		memset(*addition_name, 0, years * OBJECT_NAME_LEN);
		day = 1;
		month = 1;
		while(years-- > 0 )
		{
			if(!CreateName(addition_name_ft, *addition_name + count*OBJECT_NAME_LEN, year, month, day, 0, 0, 0))
			{
				dcisg_free(*addition_name);
				return 0;
			}
			year ++;
			count++;
		}
	}
	else
	{
		*addition_name = (char*)dcisg_malloc(OBJECT_NAME_LEN);
		if (*addition_name == NULL)
		{
			return 0;
		}
		memset(*addition_name, 0, OBJECT_NAME_LEN);
		strcpy(*addition_name, addition_name_ft);
		count = 1;
	}

	return 1;
}
int GetColName(Curveinfo *info, char **col_name, int &count, int row_time_span, int time_pace, tm *tm_time, bool is_update)
{
	int year, month, day, hour, minute, second;
	int cols = 0;
	const char* col_name_ft = NULL;
	count = 0;
	*col_name = NULL;
	year = 1900;
	if (row_time_span == 0 || time_pace == 0)
	{
		cols = 1;
	}
	else
	{
		cols = row_time_span/time_pace;
	}
	*col_name = (char*)dcisg_malloc(cols * MAX_CONF_VALUE_LEN);
	memset(*col_name, 0, cols * MAX_CONF_VALUE_LEN);
	if (*col_name == NULL)
	{
		return FALSE;
	}
	month = 1;
	day = 1;
	hour = 0;
	minute = 0;
	second = 0;
	while(cols-- >0 )
	{
		if (strchr(info->column_name_ft, '%') == NULL)
		{
			if(!CreateName(info->column_name_ft, *col_name + count*MAX_CONF_VALUE_LEN, year, month, day, hour, minute, second))
			{
				dcisg_free(*col_name);
				return FALSE;
			}
		}
		else
		{
			const char *p1, *p2;
			char *p3;
			char temp_col_ft[MAX_CONF_VALUE_LEN];
			p3 = *col_name + count*MAX_CONF_VALUE_LEN;
			col_name_ft = info->column_name_ft;
			p1 = strchr(col_name_ft, '%');
			while(p1 != NULL)//这里是为了处理列名表达式配置了多个%d的情况，例如：col_%d, col_old_%d
			{
				p2 = strchr(p1+1, '%');
				if (p2 == NULL)
				{
					strcpy(temp_col_ft, col_name_ft);
				}
				else
				{
					memcpy(temp_col_ft, col_name_ft, p2-col_name_ft);
					temp_col_ft[p2-col_name_ft] = '\0';
				}
				sprintf(p3 + strlen(p3), temp_col_ft, count + info->base_col_index);
				col_name_ft = p2;
				p1 = col_name_ft;
			}
		}
		if (is_update)
		{
			if (month == tm_time->tm_mon+1 && day == tm_time->tm_mday
				&& hour == tm_time->tm_hour && minute == tm_time->tm_min && second == tm_time->tm_sec)
			{
				if (count != 0)
				{
					memcpy(*col_name, *col_name + count*MAX_CONF_VALUE_LEN, MAX_CONF_VALUE_LEN);
				}
				count = 1;
				return TRUE;
			}
		}
		second_add(&month, &day, &hour, &minute, &second, time_pace);
		count++;
	}
	return TRUE;
}

void StrcatSql(char **sqlpp, const char* sql_srcp)
{
	int len1, len2;
	int l1, l2;
	char *temp = NULL;
	if(*sqlpp != NULL)
	{
		len1 = strlen(*sqlpp);
	}
	else
	{
		*sqlpp = (char*)dcisg_malloc(SQL_MALLOC_LEN);
		memset(*sqlpp, 0, SQL_MALLOC_LEN);
		len1 = 0;
	}
	len2 = strlen(sql_srcp) + 1;
	l1 = len1/(SQL_MALLOC_LEN-1);
	l2 = (len1 + len2)/(SQL_MALLOC_LEN-1);
	if (l2 > l1)
	{
		temp = (char*)dcisg_malloc((l2+1) * SQL_MALLOC_LEN);
		if (temp == NULL)
		{
			dcisg_free(*sqlpp);
			*sqlpp = NULL;
		}
		memset(temp, 0, (l2+1) * SQL_MALLOC_LEN);
		memcpy(temp, *sqlpp, len1);
		memcpy(temp + len1, sql_srcp, len2);
		dcisg_free(*sqlpp);
		*sqlpp = temp;
	}
	else
	{
		memcpy(*sqlpp + len1, sql_srcp, len2);
	}
}
static
int CheckStartAndEndTime(Curveinfo *info, CurvePara &param, ErrorInfo &error)
{
	int needpase = info->time_pace;
	struct tm tm_start;
	struct tm tm_end;

	//修改为线程安全函数 liqipeng 20121116
    if(localtime_r(&param.starttime,&tm_start) == NULL)
	{
	    printf("localtime_r: time = %ld\n",param.starttime);
	    return FALSE;
	}
    if(localtime_r(&param.stoptime,&tm_end) == NULL)
	{
	    printf("localtime_r: time = %ld\n",param.starttime);
	    return FALSE;
	}
	//tm_start = *localtime(&param.starttime);
	//tm_end = *localtime(&param.stoptime);
	memset(&error, 0, sizeof(ErrorInfo));
	error.error_no = -1;
	if(param.starttime > param.stoptime)
	{
		sprintf(error.error_info, "要求采样的起始时间大于终止时间");
		return FALSE;
	}
	if (strlen(info->date_col_name) != 0 && (strlen(info->day_col_name) != 0 || strlen(info->month_col_name) != 0 || strlen(info->year_col_name) != 0))
	{
		strcpy(error.error_info, "配置文件错误，不能同时指定了日期列和年、月、日列中的一种");
		return FALSE;
	}
	if (info->row_time_span != FREE_SECOND && info->row_time_span != ONE_SECOND && info->row_time_span != ONE_MINUTE_SECOND && info->row_time_span != ONE_HOUR_SECOND
		&& info->row_time_span != ONE_DAY_SECOND && info->row_time_span != ONE_MONTH_SECOND
		 && info->row_time_span != ONE_YEAR_SECOND)
	{
		sprintf(error.error_info, "配置的时间跨度不符合规定，该值必须是1分，1小时，1天，1月，1年中的一种", info->row_time_span);
		return FALSE;
	}
	if ((info->time_pace == 0 || info->row_time_span == 0) && (info->row_time_span != info->time_pace))
	{
		sprintf(error.error_info, "只有在不定义采样间隔的情况下，行集和列的采样时间步长才能配置为0");
		return FALSE;
	}
	//如果info->time_pace为0，表示列名为采样的设备ID，而竖向为采样的时间，这个时候列名中不允许指定表达式
	if (info->row_time_span != 0 && (info->row_time_span < info->time_pace
		|| info->row_time_span%needpase != 0))
	{
		sprintf(error.error_info, "配置文件中取得的采样表单行时间跨度不合法，该值必需能被步进整除");
		return FALSE;
	}

	if (strlen(info->hour_col_name) != 0 && info->row_time_span != ONE_HOUR_SECOND && info->row_time_span != FREE_SECOND)
	{
		sprintf(error.error_info, "配置文件错误，在配置该项时指定了小时列，但是配置的行集时间的跨度却不是一天");
		return FALSE;
	}
	if (needpase != 0 && info->row_time_span/needpase > 1440)
	{
		sprintf(error.error_info, "配置文件错误，通过行集的时间跨度和单列的步进算出来的结果集列数已经超过了1440的最大值");
		return FALSE;
	}
	if(param.data_format == 0)	//修正时间取值
	{
		param.data_format = FIRST_VALUE;
	}
	if(param.needpace <= 0)		//修正步长值
	{
		param.needpace = needpase;
	}
	if (param.needpace > ONE_DAY_SECOND)
	{
		if (param.needpace != ONE_MONTH_SECOND && param.needpace != ONE_YEAR_SECOND)
		{
			sprintf(error.error_info, "非法的步进长度(%d)，步长在大于1天时，只允许两种情况（一年和一月）", param.needpace);
			return FALSE;
		}
	}
	if (needpase != 0 && param.needpace % needpase != 0)
	{
		sprintf(error.error_info, "非法的步进长度(%d)，该种表结构步进长度需要能被步进(%d)整除", param.needpace, needpase);
		return FALSE;
	}
	if (info->row_time_span >= ONE_MONTH_SECOND || needpase>=ONE_DAY_SECOND)
	{
		if ((param.stoptime - param.starttime + needpase) % param.needpace != 0)
		{
			param.stoptime -= (param.stoptime - param.starttime + needpase) % param.needpace;
		}
	}
	else
	{
		if (needpase != 0 && param.starttime % needpase != 0)//如果起始时间不能被步进整除，那么修正它为步进的整数倍
		{
			param.starttime -= param.starttime % needpase;
		}
		if(needpase != 0 && param.stoptime % needpase != 0)//如果终止时间不能被步进整除，那么修正它为步进的整数倍
		{
			param.stoptime -= param.stoptime % needpase;
		}
		if (needpase != 0 && (param.stoptime - param.starttime + param.needpace) % param.needpace != 0)
		{
			param.stoptime -= (param.stoptime - param.starttime + param.needpace) % param.needpace;
		}
	}
	if ((param.data_format == AVG_VALUE || param.data_format == LAST_VALUE) && param.stoptime == param.starttime && param.needpace != info->time_pace)
	{
		sprintf(error.error_info, "非法的起始和终止时间，经过调整以后，起始时间等于终止时间，这种情况下不能取平均值或末值操作");
		return FALSE;
	}
	error.error_no = 0;
	return TRUE;
}

int  CDci::MakeSql(Curveinfo *info, CurvePara &param, char** sqlpp, char** sql2pp, int &table_count, int &col_count, ErrorInfo &error)
{
	char *table_name = NULL;
	char *table_name2 = NULL;
	char *col_name = NULL;
	char *data_time = NULL;
	char temp[MAX_DEV_KEY_ID_LIST_LEN + 200];
	char temp_key_id[MAX_DEV_KEY_ID_LIST_LEN];
	int i;
	char year[OBJECT_NAME_LEN];
	char month[OBJECT_NAME_LEN];
	char day[OBJECT_NAME_LEN];
	char hour[OBJECT_NAME_LEN];
	char minute[OBJECT_NAME_LEN];
	char start_time_str[OBJECT_NAME_LEN];
	char end_time_str[OBJECT_NAME_LEN];
	char datetime_col[OBJECT_NAME_LEN*3];
	struct tm tm_start;
	struct tm tm_end;
	char  table_flag[512];
	memset(table_flag, 0, 512);
	memset(&error, 0, sizeof(ErrorInfo));


    if(localtime_r(&param.starttime,&tm_start) == NULL)//修改为线程安全函数 liqipeng 20121116
	{
	    printf("localtime_r: time = %ld\n",param.starttime);
	    return FALSE;
	}
    if(localtime_r(&param.stoptime,&tm_end) == NULL)
	{
	    printf("localtime_r: time = %ld\n",param.starttime);
	    return FALSE;
	}
	//tm_start = *localtime(&param.starttime);
	//tm_end = *localtime(&param.stoptime);

	trim_str_ex(param.dev_key_id_values, param.dev_key_id_values + strlen(param.dev_key_id_values), temp_key_id);
	if( ((strlen(temp_key_id) == 0) || strstr(temp_key_id,",")!=NULL) && (strlen(info->dev_key_col_name)!=0))
	{
		strcpy(error.error_info, "未指定设备或者指定了多个设备id。");
		error.error_no = -1;
		return FALSE;
	}
	if (strlen(info->state_table_name_ft) != 0)
	{
		if(GetTableName(info->state_table_name_ft, &data_time, &table_name2, table_count, &tm_start, &tm_end) == FALSE)
		{
			strcpy(error.error_info, "枚举表名失败，可能因为内存不足导致的");
			error.error_no = -1;
			return FALSE;
		}
		dcisg_free(data_time);//这里必需释放掉，data_time会由下面采样表重新生成
		data_time = NULL;
	}

	if(GetTableName(info->table_name_ft, &data_time, &table_name, table_count, &tm_start, &tm_end) == FALSE)
	{
		strcpy(error.error_info, "枚举表名失败，可能因为内存不足导致的");
		error.error_no = -1;
		dcisg_free(table_name2);
		return FALSE;
	}
	if (table_count > 512)
	{
		strcpy(error.error_info, "枚举表名数量过多");
		error.error_no = -1;
		dcisg_free(table_name);
		if (table_name2)
		{
			dcisg_free(table_name2);
		}
		dcisg_free(data_time);
		return FALSE;
	}
	char tempsql[500];
	for (i=0; i<table_count; i++)
	{
		ErrorInfo errtemp;
		sprintf(tempsql, "select 0 from %s where 0=1", table_name + i*OBJECT_NAME_LEN);
		table_flag[i] = ExecSingle(tempsql, &errtemp);
		if (table_flag[i] == FALSE)
		{
			error = errtemp;
			log_report("表%s在数据库中不存在！！", table_name + i*OBJECT_NAME_LEN); //danath log test 20111017
		}
	}
	if(GetColName(info, &col_name, col_count, info->row_time_span, info->time_pace, &tm_start, FALSE) == FALSE)
	{
		strcpy(error.error_info, "枚举表名失败，可能因为内存不足导致的");
		error.error_no = -1;
		dcisg_free(table_name2);
		dcisg_free(table_name);
		dcisg_free(data_time);
		return FALSE;
	}
	switch (info->row_time_span)//下面是根据行集的跨度来调整查询的起始和终止条件
	{
	case FREE_SECOND:				//行集时间中跨度是不确定的
		sprintf(start_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_start.tm_year + 1900, tm_start.tm_mon + 1, tm_start.tm_mday, tm_start.tm_hour, tm_start.tm_min, tm_start.tm_sec);
		sprintf(end_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_end.tm_year + 1900, tm_end.tm_mon + 1, tm_end.tm_mday, tm_end.tm_hour, tm_end.tm_min, tm_start.tm_sec);
		break;
	case ONE_MINUTE_SECOND:			//行集为一分钟的时间跨度
		sprintf(start_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_start.tm_year + 1900, tm_start.tm_mon + 1, tm_start.tm_mday, tm_start.tm_hour, tm_start.tm_min, 0);
		sprintf(end_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_end.tm_year + 1900, tm_end.tm_mon + 1, tm_end.tm_mday, tm_end.tm_hour, tm_end.tm_min, 59);
		break;
	case ONE_HOUR_SECOND:			//行集为一小时的时间跨度
		sprintf(start_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_start.tm_year + 1900, tm_start.tm_mon + 1, tm_start.tm_mday, tm_start.tm_hour, 0, 0);
		sprintf(end_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_end.tm_year + 1900, tm_end.tm_mon + 1, tm_end.tm_mday, tm_end.tm_hour, 59, 59);
		break;
	case ONE_DAY_SECOND:		//行集为一天的时间跨度
		sprintf(start_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_start.tm_year + 1900, tm_start.tm_mon + 1, tm_start.tm_mday, 0, 0, 0);
		sprintf(end_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_end.tm_year + 1900, tm_end.tm_mon + 1, tm_end.tm_mday, 23, 59, 59);
		break;
	case ONE_MONTH_SECOND:	//行集为一月的时间跨度
		sprintf(start_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_start.tm_year + 1900, tm_start.tm_mon + 1, 1, 0, 0, 0);
		sprintf(end_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_end.tm_year + 1900, tm_end.tm_mon + 1, month_end_day(tm_end.tm_year + 1900, tm_end.tm_mon + 1), 23, 59, 59);
		break;
	case ONE_YEAR_SECOND://行集为一年的时间跨度
		sprintf(start_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_start.tm_year + 1900, 1, 1, 0, 0, 0);
		sprintf(end_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_end.tm_year + 1900, 12, 31, 23, 59, 59);
		break;
	default:
		strcpy(error.error_info, "非法的行集时间跨度");
		error.error_no = -1;
		dcisg_free(table_name2);
		dcisg_free(table_name);
		dcisg_free(data_time);
		dcisg_free(col_name);
		return FALSE;
	}
	int first_flag = 1;
	for(i=0; i<table_count; i++)
	{
		if (table_flag[i] == FALSE)
		{
			continue;//danath 对不存在的表，跳过
		}
		memset(year, 0, OBJECT_NAME_LEN);
		memset(month, 0, OBJECT_NAME_LEN);
		memset(day, 0, OBJECT_NAME_LEN);
		memset(hour, 0, OBJECT_NAME_LEN);
		if (strlen(info->year_col_name) != 0)
		{
			strncpy(year, info->year_col_name, OBJECT_NAME_LEN -1);
		}
		else
		{
			memcpy(year, data_time + i*OBJECT_NAME_LEN, 4);
		}
		if (strlen(info->month_col_name) != 0)
		{
			strncpy(month, info->month_col_name, OBJECT_NAME_LEN -1);
		}
		else
		{
			memcpy(month, data_time + i*OBJECT_NAME_LEN + 5, 2);
		}
		if (strlen(info->day_col_name) != 0)
		{
			strncpy(day, info->day_col_name, OBJECT_NAME_LEN -1);
		}
		else
		{
			memcpy(day, data_time + i*OBJECT_NAME_LEN + 8, 2);
		}
		if (strlen(info->hour_col_name) != 0)
		{
			strncpy(hour, info->hour_col_name, OBJECT_NAME_LEN -1);
		}
		else
		{
			strcpy(hour, "0");
		}
		if (strlen(info->minute_col_name) != 0)
		{
			strncpy(minute, info->minute_col_name, OBJECT_NAME_LEN -1);
		}
		else
		{
			strcpy(minute, "0");
		}
		if (first_flag == 0)
		{
			StrcatSql(sqlpp, " union all ");
			if (table_name2 != NULL)
			{
				StrcatSql(sql2pp, " union all ");
			}
		}
		if (strlen(info->date_col_name) == 0)
		{
			//sprintf(datetime_col, "sysdba.to_datetime(%s, %s, %s, %s, %s)", year, month, day, hour, minute);
			sprintf(datetime_col, "to_datetime(%s, %s, %s, %s, %s)", year, month, day, hour, minute);
		}
		else
		{
			if (strlen(info->hour_col_name) !=0 || strlen(info->minute_col_name) != 0)
			{
				sprintf(datetime_col, "TIMESTAMPADD(SQL_TSI_MINUTE, %s*60 + %s, %s)", hour, minute, info->date_col_name);
			}
			else
			{
				strcpy(datetime_col, info->date_col_name);
			}
		}
		sprintf(temp, " select %s as dm_datetime ", datetime_col);
		StrcatSql(sqlpp, temp);
		if (table_name2 != NULL)
		{
			StrcatSql(sql2pp, temp);
		}
		//danath 20111130 begin 华北曲线效率问题
		int begin_tag = 0;
		int end_tag = col_count;
		if(info->row_time_span > 0
                && col_count > 1
                && table_count == 1
                && (param.stoptime - param.starttime) < info->row_time_span
                && (param.starttime + 28800)% (info ->row_time_span) <= (param.stoptime + 28800) % (info ->row_time_span) )
		{
            log_report("starttime = %ld, stoptime = %ld",param.starttime,param.stoptime);
            begin_tag = ( (param.starttime + 28800)% (info ->row_time_span) ) / (info ->time_pace);
            end_tag = ( (param.stoptime + 28800) % (info ->row_time_span) ) / (info ->time_pace) + 1;
		}
		log_report("row_time_span = %d,time_pace = %d",info ->row_time_span,info ->time_pace);
        log_report("begin_tag = %d, end_tag = %d",begin_tag,end_tag);
        for (int j=begin_tag; j<end_tag; j++)
        {
            sprintf(temp, ", %s", col_name + j*MAX_CONF_VALUE_LEN);//注意，为了支持配置文件配置多个列，在这里列名的长度限定跟表名长度不一样
            StrcatSql(sqlpp, temp);
            if (table_name2 != NULL)
            {
                StrcatSql(sql2pp, temp);
            }
        }
		//danath 20111130 end
		sprintf(temp, " from %s", table_name + i*OBJECT_NAME_LEN);
		StrcatSql(sqlpp, temp);
		if (table_name2 != NULL)
		{
			sprintf(temp, " from %s", table_name2 + i*OBJECT_NAME_LEN);
			StrcatSql(sql2pp, temp);
		}
		if(strlen(info->dev_key_col_name)!=0)
		{
			sprintf(temp, " where %s=%s and %s >= %s and %s <= %s",
			info->dev_key_col_name,
			temp_key_id,
			datetime_col,
			start_time_str,
			datetime_col,
			end_time_str);
		}
		else
		{
			sprintf(temp, " where  %s >= %s and %s <= %s",
			datetime_col,
			start_time_str,
			datetime_col,
			end_time_str);
		}
		if (strlen(info->where_nor) != 0)
		{
			strcat(temp, " and ");
			strcat(temp, info->where_nor);
		}
		StrcatSql(sqlpp, temp);
		if (table_name2 != NULL)
		{
			StrcatSql(sql2pp, temp);
		}
		first_flag = 0;
	}
	if (first_flag == 0)
	{
		StrcatSql(sqlpp, " order by dm_datetime");
		if (table_name2 != NULL)
		{
			StrcatSql(sql2pp, " order by dm_datetime");
		}
	}
	dcisg_free(table_name);
	dcisg_free(table_name2);
	dcisg_free(col_name);
	dcisg_free(data_time);
#ifdef __DEBUG
	printf("%s\r\n", *sqlpp);
	if (table_name2 != NULL)
	{
		printf("%s\r\n", *sql2pp);
	}
#endif
	return 1;
}

static
int  MakeTimeWhereCond(Curveinfo *info,const time_t starttime,const time_t stoptime,char *date_time, char ** where_time_str,ErrorInfo &error)
{
	struct tm tm_start;
	struct tm tm_end;
	tm_start = *localtime(&starttime);
	tm_end = *localtime(&stoptime);
	int where_str_len = 0;
	char year[OBJECT_NAME_LEN];
	char month[OBJECT_NAME_LEN];
	char day[OBJECT_NAME_LEN];
	char hour[OBJECT_NAME_LEN];
	char minute[OBJECT_NAME_LEN];
	char start_time_str[OBJECT_NAME_LEN];
	char end_time_str[OBJECT_NAME_LEN];
	char *temp_where_time_str;
	char datetime_col[100];

	memset(year,0,OBJECT_NAME_LEN);
	memset(month,0,OBJECT_NAME_LEN);
	memset(day,0,OBJECT_NAME_LEN);
	memset(hour,0,OBJECT_NAME_LEN);
	memset(minute,0,OBJECT_NAME_LEN);
    memset(start_time_str,0,OBJECT_NAME_LEN);
	memset(end_time_str,0,OBJECT_NAME_LEN);

	switch (info->row_time_span)//下面是根据行集的跨度来调整查询的起始和终止条件
	{
	case FREE_SECOND:				//行集时间中跨度是不确定的
		sprintf(start_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_start.tm_year + 1900, tm_start.tm_mon + 1, tm_start.tm_mday, tm_start.tm_hour, tm_start.tm_min, tm_start.tm_sec);
		sprintf(end_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_end.tm_year + 1900, tm_end.tm_mon + 1, tm_end.tm_mday, tm_end.tm_hour, tm_end.tm_min, tm_start.tm_sec);
		break;
	case ONE_MINUTE_SECOND:			//行集为一分钟的时间跨度
		sprintf(start_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_start.tm_year + 1900, tm_start.tm_mon + 1, tm_start.tm_mday, tm_start.tm_hour, tm_start.tm_min, 0);
		sprintf(end_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_end.tm_year + 1900, tm_end.tm_mon + 1, tm_end.tm_mday, tm_end.tm_hour, tm_end.tm_min, 59);
		break;
	case ONE_HOUR_SECOND:			//行集为一小时的时间跨度
		sprintf(start_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_start.tm_year + 1900, tm_start.tm_mon + 1, tm_start.tm_mday, tm_start.tm_hour, 0, 0);
		sprintf(end_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_end.tm_year + 1900, tm_end.tm_mon + 1, tm_end.tm_mday, tm_end.tm_hour, 59, 59);
		break;
	case ONE_DAY_SECOND:		//行集为一天的时间跨度
		sprintf(start_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_start.tm_year + 1900, tm_start.tm_mon + 1, tm_start.tm_mday, 0, 0, 0);
		sprintf(end_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_end.tm_year + 1900, tm_end.tm_mon + 1, tm_end.tm_mday, 23, 59, 59);
		break;
	case ONE_MONTH_SECOND:	//行集为一月的时间跨度
		sprintf(start_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_start.tm_year + 1900, tm_start.tm_mon + 1, 1, 0, 0, 0);
		sprintf(end_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_end.tm_year + 1900, tm_end.tm_mon + 1, month_end_day(tm_end.tm_year + 1900, tm_end.tm_mon + 1), 23, 59, 59);
		break;
	case ONE_YEAR_SECOND://行集为一年的时间跨度
		sprintf(start_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_start.tm_year + 1900, 1, 1, 0, 0, 0);
		sprintf(end_time_str, "to_date('%d-%02d-%02d %02d:%02d:%02d', 'YYYY-MM-DD HH:MI:SS')",
			tm_end.tm_year + 1900, 12, 31, 23, 59, 59);
		break;
	default:
		strcpy(error.error_info, "非法的行集时间跨度");
		return FALSE;
	}

	if (strlen(info->year_col_name) != 0)
	{
		strncpy(year, info->year_col_name, OBJECT_NAME_LEN -1);
	}
	else
	{
		memcpy(year, date_time, 4);
	}
	if (strlen(info->month_col_name) != 0)
	{
		strncpy(month, info->month_col_name, OBJECT_NAME_LEN -1);
	}
	else
	{
		memcpy(month, date_time + 5, 2);
	}
	if (strlen(info->day_col_name) != 0)
	{
		strncpy(day, info->day_col_name, OBJECT_NAME_LEN -1);
	}
	else
	{
		memcpy(day, date_time + 8, 2);
	}
	if (strlen(info->hour_col_name) != 0)
	{
		strncpy(hour, info->hour_col_name, OBJECT_NAME_LEN -1);
	}
	else
	{
		strcpy(hour, "0");
	}
	if (strlen(info->minute_col_name) != 0)
	{
		strncpy(minute, info->minute_col_name, OBJECT_NAME_LEN -1);
	}
	else
	{
		strcpy(minute, "0");
	}

	if (strlen(info->date_col_name) == 0)
	{
		//sprintf(datetime_col, "sysdba.to_datetime(%s, %s, %s, %s, %s)", year, month, day, hour, minute);
		sprintf(datetime_col, "to_datetime(%s, %s, %s, %s, %s)", year, month, day, hour, minute);
	}
	else
	{
		if (strlen(info->hour_col_name) !=0 || strlen(info->minute_col_name) != 0)
		{
			sprintf(datetime_col, "TIMESTAMPADD(SQL_TSI_MINUTE, %s*60 + %s, %s)", hour, minute, info->date_col_name);
		}
		else
		{
			strcpy(datetime_col, info->date_col_name);
		}
	}
	where_str_len =400;
	temp_where_time_str = (char *)dcisg_malloc(where_str_len);
	memset(temp_where_time_str,0,where_str_len);
	sprintf(temp_where_time_str, " %s >= %s and %s <=%s ",datetime_col,start_time_str,datetime_col,end_time_str);
	*where_time_str = temp_where_time_str;

	return TRUE;
}

int CDci::MakeSheetSql(Curveinfo *info, CurvePara &param, const char *col_str, const char *where_str, const char* order_by, char** sqlpp, int &table_count, int &col_count, ErrorInfo &error)
{
    int  i;
	char *table_name = NULL;
	char *data_time = NULL;
	char *addition_col_name = NULL;
	char *col_name = NULL;
	char *where_time_str = NULL;  //上一级调用函数进行释放

	struct tm tm_start;
	struct tm tm_end;

	memset(&error, 0, sizeof(ErrorInfo));
	error.error_no = -1;

	// 将localtime修改为线程安全版本localtime_r
	localtime_r(&param.starttime,&tm_start);
	localtime_r(&param.stoptime,&tm_end);

	if(GetTableName(info->table_name_ft, &data_time, &table_name, table_count, &tm_start, &tm_end) == FALSE)
	{
		strcpy(error.error_info, "枚举表名失败，可能因为内存不足导致的");
		return FALSE;
	}
	if (strlen(info->addition_column_name_ft) != 0)
	{
		if (GetAdditionColName(info->addition_column_name_ft, &addition_col_name, i, &tm_start, &tm_end) == FALSE)
		{
			strcpy(error.error_info, "枚举附加列名失败，可能因为内存不足导致的");
			dcisg_free(table_name);
			dcisg_free(data_time);
			return FALSE;
		}
		if (i != table_count)
		{
			strcpy(error.error_info, "附加列名枚举数量和表名的数量不一致");
			dcisg_free(table_name);
			dcisg_free(data_time);
			return FALSE;
		}
	}

	if(GetColName(info, &col_name, col_count, info->row_time_span, info->time_pace, &tm_start, FALSE) == FALSE)
	{
		strcpy(error.error_info, "枚举列名失败，可能因为内存不足导致的");
		dcisg_free(addition_col_name);
		dcisg_free(table_name);
		dcisg_free(data_time);
		return FALSE;
	}

    int nexist = 0;
    char table_nexist[366] = {0};
    char sql_exist[128] = {0};
    struct ErrorInfo err_exist;


	// 检查枚举的表名是否存在
    for(i = 0; i < table_count; i++)
    {
        snprintf(sql_exist,sizeof(sql_exist),"select * from %s where 1 = 2;",table_name + i * OBJECT_NAME_LEN);
        if(!ExecSingle(sql_exist,&err_exist))
        {
            nexist += 1;
            table_nexist[i] = 1;
            printf("ExecSingle: error = %s\n",err_exist.error_info);
        }
    }

    if(nexist == table_count)
    {
        error = err_exist;  // 返回表存在的错误信息
        dcisg_free(table_name);
        dcisg_free(data_time);
        dcisg_free(col_name);
        dcisg_free(addition_col_name);
        return FALSE;
    }

	for(i = 0; i < table_count; i++)
	{
	    if(table_nexist[i] != 0)
	    {
	        continue;   /* sql语句中只拼接商用库中存在的表 */
	    }

		if(*sqlpp != NULL)
		{
			StrcatSql(sqlpp,"union all");
		}

		StrcatSql(sqlpp, " select ");
		if (addition_col_name != NULL)
		{
			StrcatSql(sqlpp, addition_col_name + i*OBJECT_NAME_LEN);
			StrcatSql(sqlpp, ",");
		}
		if (strlen(col_str) != 0)
		{
			StrcatSql(sqlpp, col_str);
		}
		else
		{
			char temp[MAX_CONF_VALUE_LEN + 10];
			for (int j=0; j<col_count; j++)
			{
				if (j==0)
				{
					sprintf(temp, "%s", col_name + j*MAX_CONF_VALUE_LEN);//注意，为了支持配置文件配置多个列，在这里列名的长度限定跟表名长度不一样
				}
				else
				{
					sprintf(temp, ", %s", col_name + j*MAX_CONF_VALUE_LEN);//注意，为了支持配置文件配置多个列，在这里列名的长度限定跟表名长度不一样
				}

				StrcatSql(sqlpp, temp);
			}
		}

		StrcatSql(sqlpp, " from ");
		StrcatSql(sqlpp, table_name + i*OBJECT_NAME_LEN);

		if(where_str != NULL && strlen(where_str) != 0)
		{
			StrcatSql(sqlpp, " where ");
			StrcatSql(sqlpp, where_str);
			if((param.starttime != 0) && (param.stoptime != 0))
			{
				StrcatSql(sqlpp, " and");
				MakeTimeWhereCond(info,param.starttime,param.stoptime,data_time,&where_time_str,error);
				StrcatSql(sqlpp,where_time_str);
			}
		}
		else
		{
			if((param.starttime != 0) && (param.stoptime != 0))
			{
				StrcatSql(sqlpp, " where");
				MakeTimeWhereCond(info,param.starttime,param.stoptime,data_time,&where_time_str,error);
				StrcatSql(sqlpp,where_time_str);
			}
		}
	}

	if(order_by != NULL && strlen(order_by) != 0)
	{
		char temp_orderby[1024];
		trim_str(order_by, order_by + strlen(order_by), temp_orderby);
		if(strnicmp(temp_orderby, "ORDER ", 6) != 0)
		{
			StrcatSql(sqlpp, " ORDER BY ");
			StrcatSql(sqlpp, order_by);
		}
		else
		{
			StrcatSql(sqlpp, " ");
			StrcatSql(sqlpp, order_by);
		}
	}

    error.error_no = 0;

	dcisg_free(table_name);
	dcisg_free(data_time);
	dcisg_free(col_name);
	dcisg_free(where_time_str);
	dcisg_free(addition_col_name);

	return 1;
}

static
int GetOffsetSecond(time_t _time, int offset_second)
{
	tm			tm_time;
	int			days;
	int			months;
	if(offset_second >= ONE_MONTH_SECOND)
	{
		tm_time = *localtime(&_time);
		months = offset_second/ONE_MONTH_SECOND;
		days = total_month_end_day(tm_time.tm_year+1900, tm_time.tm_mon+1, months);
		return days * ONE_DAY_SECOND;
	}

	return offset_second;
}
static
int MergResult(CurvePara &param, char** out_result_datapp, char *result_datap, int result_rows, int rec_size, int start_rec_offset, int offset_second)
{
	int			nrows, len;
	char		*datap = NULL;
	char		*tempp = NULL;
	tm			start_time;
	tm			end_time;
	time_t		current_time;
	time_t		result_time;

	// 将localtime修改为线程安全版本的localtime_r
	if(localtime_r(&param.starttime,&start_time) == NULL)
	{
	    printf("localtime_r: time = %ld\n",param.starttime);
	    return FALSE;
	}

	if(localtime_r(&param.stoptime,&end_time) == NULL)
	{
	    printf("localtime_r: time = %ld\n",param.stoptime);
	    return FALSE;
	}

	start_time.tm_sec = 0;
	*out_result_datapp = NULL;

	switch (offset_second)
	{
	case ONE_MINUTE_SECOND:
		nrows = datatime_diff(&start_time, &end_time, FUN_DATE_MIN);
		break;
	case ONE_HOUR_SECOND:
		nrows = datatime_diff(&start_time, &end_time, FUN_DATE_HOUR);
		start_time.tm_min = 0;
		break;
	case ONE_DAY_SECOND:
		nrows = datatime_diff(&start_time, &end_time, FUN_DATE_DAY);
		start_time.tm_hour = 0;
		start_time.tm_min = 0;
		break;
	case ONE_MONTH_SECOND:
		nrows = datatime_diff(&start_time, &end_time, FUN_DATE_MONTH);
		start_time.tm_mday = 1;
		start_time.tm_hour = 0;
		start_time.tm_min = 0;
		break;
	case ONE_YEAR_SECOND:
		nrows = datatime_diff(&start_time, &end_time, FUN_DATE_YEAR);
		start_time.tm_mon = 1;
		start_time.tm_mday = 1;
		start_time.tm_hour = 0;
		start_time.tm_min = 0;
		break;
	default:
		return FALSE;
	}
	nrows ++;
	len = nrows * (rec_size - start_rec_offset);
	if(len == 0)
	{
		return FALSE;
	}

	*out_result_datapp = (char*)dcisg_malloc(len);
	datap = *out_result_datapp;
	if (datap == NULL)
	{
		return FALSE;
	}
	tempp = (char*)dcisg_malloc(rec_size - start_rec_offset);
	if(tempp == NULL)
	{
		return FALSE;
	}
	memset(tempp, 0, rec_size - start_rec_offset);
	for (int i=0; i*(sizeof(float) + sizeof(sb2)) < rec_size - start_rec_offset; i++)
	{
		*(sb2*)(tempp + i*(sizeof(float) + sizeof(sb2)) + sizeof(float)) = -1;
	}
	memset(datap, 0, len);
	current_time = param.starttime;
	while(current_time<= param.stoptime && result_rows > 0)
	{
		result_time = 0;
		convert_dat_to_time(result_datap, &result_time);
		if (current_time >= result_time)
		{
			memcpy(datap, result_datap + start_rec_offset, rec_size - start_rec_offset);
			result_datap += rec_size;
			result_rows --;
			//如果存在某个行的时间和上一行时间是相等的，那不断的跳过这些相等的行
			while(result_rows > 0 && memcmp(result_datap, result_datap-rec_size, start_rec_offset) == 0)
			{
				result_datap += rec_size;
				result_rows --;
			}
		}
		else
		{
			memcpy(datap, tempp, rec_size - start_rec_offset);
		}
		current_time += GetOffsetSecond(current_time, offset_second);
#ifdef _DEBUG
		start_time = *localtime(&current_time);
#endif // _DEBUG
		datap += (rec_size - start_rec_offset);
	}
	while(current_time<= param.stoptime)
	{
		memcpy(datap, tempp, rec_size - start_rec_offset);
		current_time += offset_second;
		datap += (rec_size - start_rec_offset);
	}

	dcisg_free(tempp);
	return TRUE;
}
#define RETURN_FALSE_MAKEDATA dcisg_free(temp_value);dcisg_free(temp_count);return FALSE
static
int MakeData(Curveinfo *info, CurvePara &param, CurveData* outdatap,
			 char* result_datap, char* result_data2p,
			 int rec_size, int rec_size2,
			 int &itemsEx,
			 int result_cols,	//注意，这里的结果集列数可不是指查询语句里面的列数，而是指GetColName中计算出来的列数
								//如果配置脚本中列表达式配置了多个列，那么，在这里合计为一列
			 int group_col_count, ErrorInfo &error)
{
	double *temp_value = NULL;
	int state_value = 0;
	int *temp_count = NULL;
	tm start_time = *localtime(&param.starttime);
	int	needpace = 0;
	int col = 0, row = 0;
	int i, j = 0;
	time_t temp_time;
	char *tempdatap = result_datap;
	char *tempdata2p = result_data2p;
	sb2 null_flag = 0;
	int items = itemsEx;
	int group_col_count_ex = 0;
	if (outdatap == NULL || result_datap == NULL || items == 0 || result_cols == 0)
	{
		error.error_no = -1;
		strcpy(error.error_info, "调整步长时，发现函数调用使用了非法的参数");
		return FALSE;
	}
	int needpace_ex = info->time_pace;
	switch (info->row_time_span)//下面是根据行集的跨度来调整查询的起始和终止条件
	{
	case ONE_MINUTE_SECOND:			//行集为一分钟的时间跨度
		col = (start_time.tm_sec)/needpace_ex;
		break;
	case ONE_HOUR_SECOND:			//行集为一小时的时间跨度
		col = (start_time.tm_min*60 + start_time.tm_sec)/needpace_ex;
		break;
	case ONE_DAY_SECOND:		//行集为一天的时间跨度
		col = (start_time.tm_hour * 60 * 60 + start_time.tm_min*60 + start_time.tm_sec)/needpace_ex;
		break;
	case ONE_MONTH_SECOND:	//行集为一月的时间跨度
		col = ((start_time.tm_mday-1) * 24 * 60 * 60 + start_time.tm_hour * 60 * 60 + start_time.tm_min*60 + start_time.tm_sec)/needpace_ex;
		break;
	case ONE_YEAR_SECOND://行集为一年的时间跨度
		col = ((start_time.tm_mon-1)* 31 * 24 * 60 * 60 + (start_time.tm_mday-1) * 24 * 60 * 60 + start_time.tm_hour * 60 * 60 + start_time.tm_min*60 + start_time.tm_sec)/needpace_ex;
		break;
	default:
		error.error_no = -1;
		strcpy(error.error_info, "非法的行集时间跨度");
		return FALSE;
	}
	//danath 20111130 begin
	if(info->row_time_span > 0 &&
        (result_cols / group_col_count) > 1 &&
        (param.stoptime - param.starttime) < info->row_time_span
        && (param.starttime + 28800)% (info ->row_time_span) <= (param.stoptime + 28800) % (info ->row_time_span))
	{
        col = 0;
	}
	log_report("col = %d",col);
	//danath 20111130 end
	for (i=0, j=0; i<col; i++)
	{
		tempdatap += 4 * group_col_count;
		tempdatap += sizeof(sb2) * group_col_count;//跳过空指示符空间
		if (tempdata2p != NULL)
		{
			tempdata2p += 4 * group_col_count;
			tempdata2p += sizeof(sb2) * group_col_count;//跳过空指示符空间
		}
		j++;
		if (j >= result_cols)
		{
			error.error_no = -1;
			strcpy(error.error_info, "在调整起始列时偏移位置时产生了错误，列的偏移计数超过了行集的最大列数");
			return FALSE;
		}
	}
	temp_value = (double*)dcisg_malloc(sizeof(double) * group_col_count);
	temp_count = (int*)dcisg_malloc(sizeof(int) * group_col_count);
	if(temp_value == NULL || temp_count == NULL)
	{
		error.error_no = -1;
		strcpy(error.error_info, "内存不足");
		RETURN_FALSE_MAKEDATA;
	}
	memset(temp_value, 0, sizeof(double) * group_col_count);
	memset(temp_count, 0, sizeof(int) * group_col_count);

	col = j;
	temp_time = param.starttime;
	outdatap->is_null = TRUE;
	while (temp_time <= param.stoptime && items>0)
	{
		needpace_ex = GetOffsetSecond(temp_time, info->time_pace);
		needpace += needpace_ex;
		group_col_count_ex = 0;
		while (group_col_count_ex < group_col_count)
		{
			if (needpace == needpace_ex)
			{
				outdatap[group_col_count_ex].x_time = temp_time;
			}
			null_flag = *((sb2*)(tempdatap + 4));
			if (tempdata2p != NULL)
			{
				state_value = *(sb4*)tempdata2p;
			}
			if(null_flag == FALSE)
			{
				temp_count[group_col_count_ex] ++;
				switch (param.data_format)
				{
				case MAX_VALUE://步长中的最大值
					outdatap[group_col_count_ex].is_null = FALSE;
					if (temp_count[group_col_count_ex] == 1)
					{
						outdatap[group_col_count_ex].f_value = *(float*)tempdatap;
						outdatap[group_col_count_ex].state = state_value;
					}
					else
					{
						if (outdatap[group_col_count_ex].f_value < *(float*)tempdatap)
						{
							outdatap[group_col_count_ex].f_value = *(float*)tempdatap;
							outdatap[group_col_count_ex].state = state_value;
						}
					}
					break;
				case MIN_VALUE://步长中的最小值
					outdatap[group_col_count_ex].is_null = FALSE;
					if (temp_count[group_col_count_ex] == 1)
					{
						outdatap[group_col_count_ex].f_value = *(float*)tempdatap;
						outdatap[group_col_count_ex].state = state_value;
					}
					else
					{
						if (outdatap[group_col_count_ex].f_value > *(float*)tempdatap)
						{
							outdatap[group_col_count_ex].f_value = *(float*)tempdatap;
							outdatap[group_col_count_ex].state = state_value;
						}
					}
					break;
				case AVG_VALUE://步长中的平均值
					outdatap[group_col_count_ex].is_null = FALSE;
					if (temp_count[group_col_count_ex] == 1)
					{
						temp_value[group_col_count_ex] = *(float*)tempdatap;
						outdatap[group_col_count_ex].state = state_value;
					}
					else
					{
						temp_value[group_col_count_ex] += *(float*)tempdatap;
					}
					break;
				case FIRST_VALUE://步长中的第一个值
					if (needpace == needpace_ex)
					{
						outdatap[group_col_count_ex].is_null = FALSE;
						memcpy(&outdatap[group_col_count_ex].f_value, tempdatap, 4);
						outdatap[group_col_count_ex].state = state_value;
					}

					break;
				case LAST_VALUE://步长中的最后一个值
					if(needpace == param.needpace)
					{
						outdatap[group_col_count_ex].is_null = FALSE;
						memcpy(&outdatap[group_col_count_ex].f_value, tempdatap, 4);
						outdatap[group_col_count_ex].state = state_value;
					}
					break;
				default:
					error.error_no = -1;
					strcpy(error.error_info, "非法的取值方式，目前仅支持最大值，最小值，平均值，首值和末值");
					RETURN_FALSE_MAKEDATA;
				}
			}
			tempdatap += 4;
			tempdatap += sizeof(sb2);		//跳过空指示符空间
			if (tempdata2p != NULL)
			{
				tempdata2p += 4;
				tempdata2p += sizeof(sb2);	//跳过空指示符空间
			}
			group_col_count_ex++;
		}


		if (needpace == GetOffsetSecond(temp_time, param.needpace))
		{
			group_col_count_ex = 0;
			while (group_col_count_ex < group_col_count)
			{
				if (outdatap[group_col_count_ex].is_null == FALSE)
				{
					if (param.data_format == AVG_VALUE)
					{
						outdatap[group_col_count_ex].f_value = (float)(temp_value[group_col_count_ex]/temp_count[group_col_count_ex]);
					}
				}
				group_col_count_ex++;
				items --;
			}
			outdatap += group_col_count;
			group_col_count_ex = 0;
			while (group_col_count_ex < group_col_count)//预先把下一个列组置空
			{
				outdatap[group_col_count_ex].is_null = TRUE;//在这里，使用了前面申请空间时预留的一份空间
				group_col_count_ex++;
			}

			needpace = 0;
			memset(temp_count, 0, sizeof(int) * group_col_count);
		}

		temp_time += needpace_ex;
		col ++;
		if (info->row_time_span >= ONE_MONTH_SECOND)//如果行集大小是一个月的跨度或一年的跨度
		{
			start_time = *localtime(&temp_time);
			if (start_time.tm_mday == 1 && start_time.tm_hour == 0 && start_time.tm_min == 0 && start_time.tm_sec == 0
				&& (info->row_time_span == ONE_MONTH_SECOND || start_time.tm_mon == 1 && info->row_time_span == ONE_YEAR_SECOND))
			{
				row ++;
				col = 0;
				tempdatap = result_datap + row * rec_size;
				tempdata2p = result_data2p + row * rec_size2;
			}
			if (col >= result_cols)
			{
				error.error_no = -1;
				strcpy(error.error_info, "根据步进抽取采样点时产生了异常情况，偏移列数超过了结果集的列数");
				RETURN_FALSE_MAKEDATA;
			}
		}
		else
		{
			if (col >= result_cols)
			{
				col = 0;
			}
		}

	}
	if (param.data_format == AVG_VALUE || param.data_format == LAST_VALUE)
	{
		itemsEx = itemsEx - items;//调整输出点的个数，正常情况下，items的值只有两种可能，一种为0，一种为1
	}
#ifdef _DEBUG
	if (items != 0 && items != group_col_count)
	{
		error.error_no = -1;
		printf("算法出错了，理论上items的值应该为配置文件中的列数(%s)或是0", group_col_count);
		return FALSE;
	}
#endif
	dcisg_free(temp_value);
	dcisg_free(temp_count);
	return TRUE;
}
static
int GetResultRecSize(ColAttr_t* attr, int result_cols, ErrorInfo &error)
{
	int rec_size;

	//注意，下面的代码很重要，目前只支持4字节的采样，如果想支持8字节的采样值，那MakeData一定要联动的修改
	rec_size = 0;
	for (int i=0; i<result_cols; i++)
	{
		if (i == 0 && attr[i].data_type != DCI_DAT)
		{
			sprintf(error.error_info, "采样查询生成的结果集不合规定，要求采样结果集的第一列必需为日期时间列");
			error.error_no = -1;
			return 0;
		}
		else if (i != 0 && attr[i].data_size != 4)//attr[i].data_type != DCI_FLT && attr[i].data_type != DCI_INT
		{
			strcpy(error.error_info, "非法的采样数据格式，目前仅支持FLOAT，INT数据类型");
			error.error_no = -1;
			return 0;
		}
		rec_size += attr[i].data_size;
		rec_size += sizeof(sb2);
	}
	return rec_size;
}

#define  RETURN_FALSE_MAKETABLE dcisg_free(sql); dcisg_free(attr); dcisg_free(data); dcisg_free(dataEx); \
 dcisg_free(sql2); dcisg_free(attr2); dcisg_free(data2); dcisg_free(dataEx2); dcisg_free(*datapp); *datapp= NULL; items = 0; return FALSE
#define  RETURN_TRUE_MAKETABLE	dcisg_free(attr); dcisg_free(data); dcisg_free(dataEx); dcisg_free(sql);\
	dcisg_free(attr2); dcisg_free(data2); dcisg_free(dataEx2); dcisg_free(sql2); return TRUE;
int CDci::MakeTable(Curveinfo *info, CurvePara &param, CurveData **datapp, int &items, ErrorInfo &error)
{
	int			table_count, col_count, group_col_count;
	char		*sql = NULL;
	char		*sql2 = NULL;
	ColAttr_t	*attr = NULL;
	ColAttr_t	*attr2 = NULL;
	char		*data = NULL;
	char		*data2 = NULL;
	char		*dataEx = NULL;
	char		*dataEx2 = NULL;
	int			rec_size = 0;
	int			rec_size2 = 0;
	int			rec_num, col_num, rec_num2, col_num2;
	ub4			buf_size;
	*datapp = NULL;
	items = 0;
	if (!CheckStartAndEndTime(info, param, error))
	{
		RETURN_FALSE_MAKETABLE;
	}
	if (MakeSql(info, param, &sql, &sql2, table_count, col_count, error) == FALSE)
	{
		RETURN_FALSE_MAKETABLE;
	}
	int start_rec_offest = 0;
	if (sql != NULL)
	{
	//	log_report("采样SQL：%s", sql);
		if(ReadDataEx(sql, -1, &rec_num, &col_num, &attr, &data, &buf_size, &error) == FALSE)
		{
			RETURN_FALSE_MAKETABLE;
		}
		if (rec_num == 0)
		{
			strcpy(error.error_info, "查询空结果集，未找到对应的key记录");
			error.error_no = -1;
			RETURN_FALSE_MAKETABLE;
		}
		log_report("col_num = %d, col_count = %d",col_num,col_count);
		if ((col_num-1) % col_count != 0 && //减一是为了减去日期列
            !(info->row_time_span > 0 &&
                 col_count> 1 &&
                (param.stoptime - param.starttime) < info->row_time_span )
                && (param.starttime + 28800)% (info ->row_time_span) <= (param.stoptime + 28800) % (info ->row_time_span)) //danath 20111130
		{
			strcpy(error.error_info, "查询结果集错误，结果集中的列数并不能被配置文件中的配置的列数整除");
			error.error_no = -1;
			RETURN_FALSE_MAKETABLE;
		}
		group_col_count = (col_num-1)/col_count;//得到配置文件中列名配置项中配置列的个数
		//danath 20111130 begin
		if(group_col_count == 0)
		{
		    group_col_count =1;
		}
		//danath 20111130 end
		if ((rec_size = GetResultRecSize(attr, col_num, error)) == 0)
		{
			RETURN_FALSE_MAKETABLE;
		}
		start_rec_offest = attr[0].data_size + sizeof(sb2);
	}
	else
	{
        //danath 20111027 add begin
//        error.error_no = -9001; // 该错误号屏蔽了数据库连接错误
		strcat(error.error_info, "导致生成SQL语句失败,将返回空采样点.");
		//danath 20111027 add end
		group_col_count = 1;
		const char *pcol1 = NULL;
		pcol1 = info->column_name_ft;
		pcol1 = strstr(pcol1, ",");
		while(pcol1)
		{
			group_col_count ++;
			pcol1 ++;
			pcol1 = strstr(pcol1, ",");
		}
		rec_num = 0;
		col_count *= group_col_count;
		start_rec_offest = 8 + sizeof(sb2);
		rec_size = start_rec_offest + (4 + sizeof(sb2)) * col_count;
	}
	if(!MergResult(param, &dataEx, data, rec_num, rec_size, start_rec_offest, info->row_time_span))
	{
		strcpy(error.error_info, "可能是内存不足或配置文件中的行集时间跨度和采样步进不匹配导致修整查询结果集失败");
		error.error_no = -1;
		RETURN_FALSE_MAKETABLE;
	}
	if (sql2 != NULL)//如果采样表存在状态表查询语句
	{
		//log_report("状态SQL：%s", sql2);
		if(ReadDataEx(sql2, -1, &rec_num2, &col_num2, &attr2, &data2, &buf_size, &error) == FALSE)
		{
			RETURN_FALSE_MAKETABLE;
		}
		if (col_num != col_num2)
		{
			//这里需要注意，如果采样表列表达式配置了多列，那这些列就应该在状态表中同样存在
			strcpy(error.error_info, "存在采样状态表的情况下，状态表中的列数和采样表中的列数不一致");
			RETURN_FALSE_MAKETABLE;
		}
		if ((rec_size2 = GetResultRecSize(attr2, col_num2, error)) == 0)
		{
			RETURN_FALSE_MAKETABLE;
		}
		if(!MergResult(param, &dataEx2, data2, rec_num2, rec_size2, attr2[0].data_size + sizeof(sb2), info->row_time_span))
		{
			strcpy(error.error_info, "可能是内存不足或配置文件中的行集时间跨度和采样步进不匹配导致修整查询结果集失败");
			error.error_no = -1;
			RETURN_FALSE_MAKETABLE;
		}
	}
	items = (param.stoptime - param.starttime)/param.needpace + 1;
	items = items * group_col_count;
	*datapp = (CurveData*)dcisg_malloc((items + group_col_count) * sizeof(CurveData));//注意了，这边多申请了一个group_col_count数，是为了在MakeData里算法在置空时，预置当前节点的下一个节点为空
	if (*datapp == NULL)
	{
		strcpy(error.error_info, "内存不足");
		error.error_no = -1;
		RETURN_FALSE_MAKETABLE;
	}
	memset(*datapp, 0, (items + group_col_count) * sizeof(CurveData));
	if (!MakeData(info, param, *datapp, dataEx, dataEx2,
		rec_size - start_rec_offest,
		rec_size2 == 0 ? rec_size2: (rec_size2 - start_rec_offest),
		items,
		col_count, group_col_count, error))
	{
		RETURN_FALSE_MAKETABLE;
	}
	if (items == 0)
	{
		error.error_no = -1;
		strcpy(error.error_info, "起始和终止时间和要求取的数据格式不正确，可能是因为起始时间和终止时间的差额小于步长所致");
		RETURN_FALSE_MAKETABLE;
	}
	RETURN_TRUE_MAKETABLE;
}

#define  RETURN_FALSE_MAKETABLE_Sheet_Ex dcisg_free(sql); dcisg_free(attrp); dcisg_free(datap); dcisg_free(col_str); attr.clear(); data.clear(); return FALSE
#define  RETURN_TRUE_MAKETABLE_Sheet_Ex  dcisg_free(sql); dcisg_free(attrp); dcisg_free(col_str); return TRUE;
/*
int CDci::MakeSheetTable(Curveinfo *info, CurvePara &param, ColAttr_t **attr, char **data,  const char* col_str, const char* where_str, const char* order_by, ErrorInfo &error)
{
	int			table_count, col_count;
	char		*sql = NULL;
	ColAttr_t	*attrp = NULL;
	char		*datap = NULL;
	char		*tempp = NULL;
	char		*col_str = NULL;
	int			rec_size = 0;
	int			rec_num, col_num;
	ub4			buf_size;
	int			i, j;
	ColDataItem item;

	//char         *sql;
	if (attr.size() !=0 || data.size() != 0)
	{
		strcpy(error.error_info, "attr或data容器不为空，请把它们置空以后再调用本接口");
		RETURN_FALSE_MAKETABLE_Sheet_Ex;
	}
	if (!CheckStartAndEndTime(info, param, error))
	{
		RETURN_FALSE_MAKETABLE_Sheet_Ex;
	}
	if(colname.size() > 0)
	{
		for (i=0; i<colname.size(); i++)
		{
			if (i != 0)
			{
				StrcatSql(&col_str, ",");
			}
			StrcatSql(&col_str, colname[i].c_str());
		}
	}
	//进行通用。
	if (MakeSheetSql(info, param,col_str,where_str, order_by, &sql, table_count, col_count, error) == FALSE)
	{
		RETURN_FALSE_MAKETABLE_Sheet_Ex;
	}

	//感觉是否要求换attr里的东西。主要看数据的合并。
	if(ReadDataTime_t(sql, -1, &rec_num, &col_num, &attrp, &datap, &buf_size, &error) == FALSE)
	{
		RETURN_FALSE_MAKETABLE_Sheet_Ex;
	}
	for (i=0; i<col_num; i++)
	{
		attr.push_back(attrp[i]);
	}
	param.temp_memory1 = datap;
	tempp = datap;
	for (i=0; i<rec_num; i++)
	{
		for (j=0; j<col_num; j++)
		{
			item.col_type = (ub1)attrp[j].data_type;
			switch (attrp[j].data_type)
			{
			case DCI_STR:
				item.str_valuep = tempp;
				break;
			case DCI_INT:
				switch (attrp[j].data_size)
				{
				case 1:
					item.sb1_value = *tempp;
					break;
				case 2:
					item.sb2_value = *(sb2*)tempp;
					break;
				case 4:
					item.sb4_value = *(sb4*)tempp;
					break;
				case 8:
					item.sb8_value = *(sb8*)tempp;
					break;
				default:
					sprintf(error.error_info, "整数列(%s)的数据类型长度(%d)不合法", attrp[j].col_name, attrp[j].data_size);
					RETURN_FALSE_MAKETABLE_Sheet_Ex;
				}
				break;
			case DCI_FLT:
				if (attr[j].data_size == 4)
				{
					item.f_value = *(float*)tempp;
				}
				else
				{
					item.d_value = *(double*)tempp;
				}
				break;
			case DCI_DAT:
				item.t_value = *(time_t*)tempp;
				break;
			case DCI_CLOB:
			case DCI_BLOB:
				item.blob_value.datap = datap + *(ub4*)tempp;
				item.blob_value.data_size = *(ub4*)(tempp + sizeof(ub4));
				break;
			default:
				sprintf(error.error_info, "列(%s)的数据类型(%d)暂未支持", attrp[j].col_name, attrp[j].data_type);
				RETURN_FALSE_MAKETABLE_Sheet_Ex;
				break;
			}
			tempp += attr[j].data_size ;
			if (*(sb2*)tempp == -1)
			{
				item.null_flag = (sb1)1;
			}
			else
			{
				item.null_flag = (sb1)0;
			}
			tempp += sizeof(sb2);
			data.push_back(item);
		}
	}
	if (tempp - datap != buf_size)
	{
		sprintf(error.error_info, "数据在整合到容器中时，过程中产生了非法的偏移");
		RETURN_FALSE_MAKETABLE_Sheet_Ex;
	}
	RETURN_TRUE_MAKETABLE_Sheet_Ex;
}
*/
int CDci::MakeSheetTable(Curveinfo *info, CurvePara &param, vector<ColAttr_t> &attr, vector<ColDataItem> &data, vector<string> &colname, const char* where_str, const char* order_by, ErrorInfo &error)
{
	int			table_count, col_count;
	char		*sql = NULL;
	ColAttr_t	*attrp = NULL;
	char		*datap = NULL;
	char		*tempp = NULL;
	char		*col_str = NULL;
	int			rec_size = 0;
	int			rec_num, col_num;
	ub4			buf_size;
	int			i, j;
	ColDataItem item;

	//char         *sql;
	if (attr.size() !=0 || data.size() != 0)
	{
		strcpy(error.error_info, "attr或data容器不为空，请把它们置空以后再调用本接口");
		RETURN_FALSE_MAKETABLE_Sheet_Ex;
	}
	if (!CheckStartAndEndTime(info, param, error))
	{
		RETURN_FALSE_MAKETABLE_Sheet_Ex;
	}
	if(colname.size() > 0)
	{
		for (i=0; i<colname.size(); i++)
		{
			if (i != 0)
			{
				StrcatSql(&col_str, ",");
			}
			StrcatSql(&col_str, colname[i].c_str());
		}
	}
	//进行通用。
	if (MakeSheetSql(info, param,col_str,where_str, order_by, &sql, table_count, col_count, error) == FALSE)
	{
		RETURN_FALSE_MAKETABLE_Sheet_Ex;
	}

	//感觉是否要求换attr里的东西。主要看数据的合并。
	if(ReadDataTime_t(sql, -1, &rec_num, &col_num, &attrp, &datap, &buf_size, &error) == FALSE)
	{
		RETURN_FALSE_MAKETABLE_Sheet_Ex;
	}
	for (i=0; i<col_num; i++)
	{
		attr.push_back(attrp[i]);
	}
	param.temp_memory1 = datap;
	tempp = datap;
	for (i=0; i<rec_num; i++)
	{
		for (j=0; j<col_num; j++)
		{
			item.col_type = (ub1)attrp[j].data_type;
			switch (attrp[j].data_type)
			{
			case DCI_STR:
				item.str_valuep = tempp;
				break;
			case DCI_INT:
				switch (attrp[j].data_size)
				{
				case 1:
					item.sb1_value = *tempp;
					break;
				case 2:
					item.sb2_value = *(sb2*)tempp;
					break;
				case 4:
					item.sb4_value = *(sb4*)tempp;
					break;
				case 8:
					item.sb8_value = *(sb8*)tempp;
					break;
				default:
					sprintf(error.error_info, "整数列(%s)的数据类型长度(%d)不合法", attrp[j].col_name, attrp[j].data_size);
					RETURN_FALSE_MAKETABLE_Sheet_Ex;
				}
				break;
			case DCI_FLT:
				if (attr[j].data_size == 4)
				{
					item.f_value = *(float*)tempp;
				}
				else
				{
					item.d_value = *(double*)tempp;
				}
				break;
			case DCI_DAT:
				item.t_value = *(time_t*)tempp;
				break;
			case DCI_CLOB:
			case DCI_BLOB:
				item.blob_value.datap = datap + *(ub4*)tempp;
				item.blob_value.data_size = *(ub4*)(tempp + sizeof(ub4));
				break;
			default:
				sprintf(error.error_info, "列(%s)的数据类型(%d)暂未支持", attrp[j].col_name, attrp[j].data_type);
				RETURN_FALSE_MAKETABLE_Sheet_Ex;
				break;
			}
			tempp += attr[j].data_size ;
			if (*(sb2*)tempp == -1)
			{
				item.null_flag = (sb1)1;
			}
			else
			{
				item.null_flag = (sb1)0;
			}
			tempp += sizeof(sb2);
			data.push_back(item);
		}
	}
	if (tempp - datap != buf_size)
	{
		sprintf(error.error_info, "数据在整合到容器中时，过程中产生了非法的偏移");
		RETURN_FALSE_MAKETABLE_Sheet_Ex;
	}
	RETURN_TRUE_MAKETABLE_Sheet_Ex;
}
#define  RETURN_FALSE_MAKETABLE_Sheet_SQL dcisg_free(attrp); dcisg_free(datap); attr.clear(); data.clear(); return FALSE
#define  RETURN_TRUE_MAKETABLE_Sheet_SQL  dcisg_free(attrp); return TRUE;
int CDci::MakeSheetTable(char *sql, int top_number, char** out_buf, vector<ColAttr_t> &attr, vector<ColDataItem> &data, ErrorInfo &error)
{
	ColAttr_t	*attrp = NULL;
	char		*datap = NULL;
	char		*tempp = NULL;
	int			rec_size = 0;
	int			rec_num, col_num;
	ub4			buf_size;
	int			i, j;
	ColDataItem item;
	if (out_buf == NULL)
	{
		strcpy(error.error_info, "非法的out_buf参数");
		RETURN_FALSE_MAKETABLE_Sheet_SQL;
	}
	*out_buf = NULL;
	//char         *sql;
	if (attr.size() !=0 || data.size() != 0)
	{
		strcpy(error.error_info, "attr或data容器不为空，请把它们置空以后再调用本接口");
		RETURN_FALSE_MAKETABLE_Sheet_SQL;
	}

	//感觉是否要求换attr里的东西。主要看数据的合并。
	if(ReadDataTime_t(sql, top_number, &rec_num, &col_num, &attrp, &datap, &buf_size, &error) == FALSE)
	{
		RETURN_FALSE_MAKETABLE_Sheet_SQL;
	}
	for (i=0; i<col_num; i++)
	{
		attr.push_back(attrp[i]);
	}
	*out_buf = datap;
	tempp = datap;
	for (i=0; i<rec_num; i++)
	{
		for (j=0; j<col_num; j++)
		{
			memset(&item, 0, sizeof(item));
			item.col_type = (ub1)attrp[j].data_type;
			switch (attrp[j].data_type)
			{
			case DCI_STR:
				item.str_valuep = tempp;
				break;
			case DCI_INT:
				switch (attrp[j].data_size)
				{
				case 1:
					item.sb1_value = *tempp;
					break;
				case 2:
					item.sb2_value = *(sb2*)tempp;
					break;
				case 4:
					item.sb4_value = *(sb4*)tempp;
					break;
				case 8:
					item.sb8_value = *(sb8*)tempp;
					break;
				default:
					sprintf(error.error_info, "整数列(%s)的数据类型长度(%d)不合法", attrp[j].col_name, attrp[j].data_size);
					RETURN_FALSE_MAKETABLE_Sheet_SQL;
				}
				break;
			case DCI_FLT:
				if (attr[j].data_size == 4)
				{
					item.f_value = *(float*)tempp;
				}
				else
				{
					item.d_value = *(double*)tempp;
				}
				break;
			case DCI_DAT:
				item.t_value = *(time_t*)tempp;
				break;
			case DCI_CLOB:
			case DCI_BLOB:
				item.blob_value.datap = datap + *(ub4*)tempp;
				item.blob_value.data_size = *(ub4*)(tempp + sizeof(ub4));
				break;
			default:
				sprintf(error.error_info, "列(%s)的数据类型(%d)暂未支持", attrp[j].col_name, attrp[j].data_type);
				RETURN_FALSE_MAKETABLE_Sheet_SQL;
				break;
			}
			tempp += attr[j].data_size ;
			if (*(sb2*)tempp == -1)
			{
				item.null_flag = (sb1)1;
			}
			else
			{
				item.null_flag = (sb1)0;
			}
			tempp += sizeof(sb2);
			data.push_back(item);
		}
	}
// 	if (tempp - datap != buf_size)
// 	{
// 		sprintf(error.error_info, "数据在整合到容器中时，过程中产生了非法的偏移");
// 		RETURN_FALSE_MAKETABLE_Sheet_SQL;
// 	}
	RETURN_TRUE_MAKETABLE_Sheet_SQL;
}
#define  RETURN_FALSE_MAKETABLE_Sheet dcisg_free(sql); dcisg_free(attr); dcisg_free(data); *datapp= NULL; items = 0;\
*attrpp=NULL; items = 0; return FALSE
#define  RETURN_TRUE_MAKETABLE_Sheet  dcisg_free(sql); return TRUE;
int CDci::MakeSheetTable(Curveinfo *info, CurvePara &param, char *col_str,char *where_str,char **datapp, ColAttr_t **attrpp, int &items, ErrorInfo &error)
{
	int			table_count, col_count;
	char		*sql = NULL;
	ColAttr_t	*attr = NULL;
	char		*data = NULL;
	char		*dataEx = NULL;
	int			rec_size = 0;
	int			rec_size2 = 0;
	int			rec_num, col_num;
	ub4			buf_size;
	//char         *sql;
	*datapp = NULL;
	items = 0;
	if (!CheckStartAndEndTime(info, param, error))
	{
		RETURN_FALSE_MAKETABLE_Sheet;
	}
	//进行通用。
	if (MakeSheetSql(info, param,col_str,where_str, NULL, &sql, table_count, col_count, error) == FALSE)
	{
		RETURN_FALSE_MAKETABLE_Sheet;
	}
	//感觉是否要求换attr里的东西。主要看数据的合并。
	if(ReadDataTime_t(sql, -1, &rec_num, &col_num, &attr, &data, &buf_size, &error) == FALSE)
	{
		RETURN_FALSE_MAKETABLE_Sheet;
	}

	*datapp = data;
	*attrpp = attr;
	items = rec_num;
	RETURN_TRUE_MAKETABLE_Sheet;
}

static void PrintfCurPara(CurvePara &param)
{
	printf("conf_id: %s\n", param.conf_id);
	printf("dev_key_id_values: %s\n", param.dev_key_id_values);
	printf("needpace: %d\n", param.needpace);
	printf("starttime: %ld\n", param.starttime);
	printf("stoptime: %ld\n", param.stoptime);
}

int CDci::GetCurveData(CurvePara &param, CurveData **datapp, int &items, ErrorInfo &error)
{
	//特别要注意：为了降底算法的复杂度，目前采样值数据类型只限制在了float和int类型，并且如果想要去int类型，数据格式那只支持取首值和末值
	//如果想支持采样值是double类型，那需要重新调整MakeData函数中的偏移
	Curveinfo *info = NULL;
	//PrintfCurPara(param);
	memset(&error, 0, sizeof(ErrorInfo_t));
	info = FindCurveinfo(param.conf_id);
	if (info == NULL)
	{
		strcpy(error.error_info, "配置文件中未发现相应的应用号");
		error.error_no = -1;
		return FALSE;
	}
	if(strcmp(param.dev_key_id_values,"-1")==0)
	{
		param.dev_key_id_values[0] = 0;
	}
	if (strlen(info->dev_key_col_name) == 0)
	{
		if (strlen(param.dev_key_id_values) != 0)
		{
			strcpy(error.error_info, "配置文件中并未配置设备ID列，但是在更新时却指定了设备ID值，请重新确认");
			error.error_no = -1;
			return FALSE;
		}
	}
	else
	{
		if (strlen(param.dev_key_id_values) == 0)
		{
			strcpy(error.error_info, "配置文件中配置设备ID列名，但是在更新时未指定了设备ID值，请重新确认");
			error.error_no = -1;
			return FALSE;
		}
	}
	if (info->row_time_span <= 0)
	{
		return MakeTableEx(info, param, datapp, items, error);
	}
	return MakeTable(info, param, datapp, items, error);
}

int CDci::GetCurveDataBat(int nParam, CurvePara *param, CurveData **datapp, int *items, ErrorInfo &error)
{
	//特别要注意：为了降底算法的复杂度，目前采样值数据类型只限制在了float和int类型，并且如果想要去int类型，数据格式那只支持取首值和末值
	//如果想支持采样值是double类型，那需要重新调整MakeData函数中的偏移
	int rc;
	for(int i = 0; i < nParam; i++)
	{
		Curveinfo *info = NULL;
		//PrintfCurPara(param[i]);  danath 20111115
		memset(&error, 0, sizeof(ErrorInfo_t));
		info = FindCurveinfo(param[i].conf_id);
		if (info == NULL)
		{
			strcpy(error.error_info, "配置文件中未发现相应的应用号");
			error.error_no = -1;
			return FALSE;
		}
		if(strcmp(param[i].dev_key_id_values,"-1")==0)
		{
			param[i].dev_key_id_values[0] = 0;
		}
		if (strlen(info->dev_key_col_name) == 0)
		{
			if (strlen(param[i].dev_key_id_values) != 0)
			{
				strcpy(error.error_info, "配置文件中并未配置设备ID列，但是在更新时却指定了设备ID值，请重新确认");
				error.error_no = -1;
				return FALSE;
			}
		}
		else
		{
			if (strlen(param[i].dev_key_id_values) == 0)
			{
				strcpy(error.error_info, "配置文件中配置设备ID列名，但是在更新时未指定了设备ID值，请重新确认");
				error.error_no = -1;
				return FALSE;
			}
		}
		if (info->row_time_span <= 0)
			rc =  MakeTableEx(info, param[i], &datapp[i], items[i], error);
		else
			rc =  MakeTable(info, param[i], &datapp[i], items[i], error);
        /* danath 20111109 华北core， 此处跳出了循环，会导致之后的datapp[i]、items[i]无法初始化
		if(rc == FALSE)
			return rc;
        */
	}
	return rc;
}

int CDci::SetCurveData(CurvePara &param, CurveData &curvedata, ErrorInfo &error)
{
	return SetCurveData(param, &curvedata, 1, error);
}
void PrintfCurveData(CurveData *curvedatap, ub2 items)
{
	tm tm_time;
	if (g_log_flag == LOG_FLAG_NO_LOG)
	{
		return;
	}
	for (int i=0; i<items; i++)
	{
		tm_time = *localtime(&curvedatap[i].x_time);
		log_report("%d: %ld(%04d-%02d-%02d %02d:%02d:%02d), %f", i, curvedatap[i].x_time,
			tm_time.tm_year+1900,
			tm_time.tm_mon+1,
			tm_time.tm_mday,
			tm_time.tm_hour,
			tm_time.tm_min,
			tm_time.tm_sec,
			curvedatap[i].f_value);
	}
}
static
int compare_curvedata( const void *arg1, const void *arg2 )
{
	//两个参数指针向的是要比较的值在数据中的索引值的地址
	if(((CurveData*)arg1)->x_time == ((CurveData*)arg2)->x_time)
	{
		return 0;
	}
	else if(((CurveData*)arg1)->x_time > ((CurveData*)arg2)->x_time)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}
int CDci::SetCurveDataToDatabase(Curveinfo *info, CurvePara &param, CurveData *curvedatap, ub2 items, ErrorInfo &error)
{
	int ret;
	char *updatesql = NULL;
	char table_name[200];
	char *updatesql2 = NULL;
	char table_name2[200];
	memset(table_name, 0, 200);
	memset(table_name2, 0, 200);
	if(MakeUpdateSql(info, param, curvedatap, items, &updatesql, table_name, &updatesql2, table_name2, error))
	{
		ub4 row_count = ExecSqlReturnAffectRowCount(updatesql, &error);
		if(error.error_no == 942)
		{
			char sqlstr[1000];
			char index_str[1000];
			time_t temp_time;
			if (strlen(info->table_name_model) == 0)
			{
				dcisg_free(updatesql);
				dcisg_free(updatesql2);
				return FALSE;
			}
			sprintf(sqlstr,"create table %s as select * from %s ",table_name,info->table_name_model);
			//初始化后有index_str:
			if (strlen(info->index_str)>0)
			{
				sprintf(index_str,"create unique index index_%s%ld on %s(%s)",table_name,time(&temp_time), table_name,info->index_str);
			}
			else
			{
				sprintf(index_str,"");
			}
			if(!ExecSingle(sqlstr,&error))
			{
				dcisg_free(updatesql);
				dcisg_free(updatesql2);
				return FALSE;
			}
			if (strlen(index_str)>0)
			{
				if(!ExecSingle(index_str,&error))
				{
					dcisg_free(updatesql);
					dcisg_free(updatesql2);
					return FALSE;
				}
			}
		}
		else if(error.error_no != 0)
		{
			log_report("SetCurveDataToDatabase:%s -->%s", updatesql, error.error_info);
			dcisg_free(updatesql);
			dcisg_free(updatesql2);
			return FALSE;
		}
		dcisg_free(updatesql);
		updatesql = NULL;
		if (row_count == 0)
		{
			if (MakeInsertSql(info, param, curvedatap, items, &updatesql, error))
			{
				if(!ExecSingle(updatesql, &error))
				{
					dcisg_free(updatesql);
					dcisg_free(updatesql2);
					return FALSE;
				}
			}
			else
			{
				dcisg_free(updatesql);
				dcisg_free(updatesql2);
				return FALSE;
			}
		}
		if (updatesql2 == NULL)
		{
			return TRUE;
		}
		row_count = ExecSqlReturnAffectRowCount(updatesql2, &error);
		dcisg_free(updatesql2);
		updatesql2 = NULL;
		if (row_count == 0)
		{
			if (error.error_no == 0)
			{
				sprintf(error.error_info, "成功更新采样值，但是未找到对应设备(%s)的质量位", param.dev_key_id_values);
			}
			return TRUE;
		}
		return TRUE;

	}
	return FALSE;
}
int CDci::SetCurveData(CurvePara &param, CurveData *curvedatap, ub2 items, ErrorInfo &error)
{
	char *updatesql = NULL;
	Curveinfo *info = NULL;
	CurveData *curvedata_tempp = NULL;
	CurveData *curvedata_tempp_ex = NULL;
	ub2	temp_items = 0;
	int table_type;
	time_t last_table_time, end_table_time;
	CurvePara param_ex;
	memcpy(&param_ex, &param, sizeof(CurvePara));
	log_report("SetCurveData in, conf_id:%s, key:%s starttime:%ld, stoptime:%ld, items:%d",
		param.conf_id, param.dev_key_id_values, param.starttime, param.stoptime, items);
	PrintfCurveData(curvedatap, items);
	memset(&error, 0, sizeof(ErrorInfo_t));
	if (curvedatap == NULL || items == 0)
	{
		strcpy(error.error_info, "采样值指针为NULL或更新的采样点个数为0");
		error.error_no = -1;
		log_report("SetCurveData error: %s", error.error_info);
		return FALSE;
	}
	info = FindCurveinfo(param.conf_id);
	if (info == NULL)
	{
		strcpy(error.error_info, "配置文件中未发现相应的应用号");
		error.error_no = -1;
		log_report("SetCurveData error: %s", error.error_info);
		return FALSE;
	}
	if(strcmp(param.dev_key_id_values,"-1")==0)
	{
		param.dev_key_id_values[0] = 0;
	}
	/*************
	if (param.needpace <= 0)
	{
		param.needpace = info->time_pace;
	}
	if (param.needpace != 0 && info->time_pace != param.needpace)
	{
		strcpy(error.error_info, "只有在存取步长跟历史表结构列步长一致的情况下，才能使用更新");
		error.error_no = -1;
		return FALSE;
	}
	**************/
	if (param.data_format != 0 && param.data_format != FIRST_VALUE)
	{
		strcpy(error.error_info, "只有存取的值为首值和末值的情况下，才允许更新");
		error.error_no = -1;
		log_report("SetCurveData error: %s", error.error_info);
		return FALSE;
	}
	if (strlen(info->dev_key_col_name) == 0)
	{
		if (strlen(param.dev_key_id_values) != 0)
		{
			strcpy(error.error_info, "配置文件中并未配置设备ID列，但是在更新时却指定了设备ID值，请重新确认");
			error.error_no = -1;
			log_report("SetCurveData error: %s", error.error_info);
			return FALSE;
		}
	}
	else
	{
		if (strlen(param.dev_key_id_values) == 0)
		{
			strcpy(error.error_info, "配置文件中配置设备ID列名，但是在更新时未指定了设备ID值，请重新确认");
			error.error_no = -1;
			log_report("SetCurveData error: %s", error.error_info);
			return FALSE;
		}
	}
	curvedata_tempp = (CurveData*)dcisg_malloc(sizeof(CurveData) * items);
	if (curvedata_tempp == NULL)
	{
		strcpy(error.error_info, "内存不足");
		error.error_no = -1;
		log_report("SetCurveData error: %s", error.error_info);
		return FALSE;
	}
	memcpy(curvedata_tempp, curvedatap, sizeof(CurveData) * items);
	qsort(curvedata_tempp, items, sizeof(CurveData), compare_curvedata);//用快速排序法排序
	table_type = GetTableType(info->table_name_ft);
	GetEndTableTime(table_type, curvedata_tempp->x_time, last_table_time);
	temp_items = 0;
	curvedata_tempp_ex = curvedata_tempp;
	for (ub2 i=0; i<items; i++)
	{
		GetEndTableTime(table_type, curvedata_tempp[i].x_time, end_table_time);
		if (last_table_time != end_table_time || (info->row_time_span <= 0 && temp_items>0))//无行跨度更新
		{
			param_ex.starttime = last_table_time;
			param_ex.stoptime = last_table_time;
			if(!SetCurveDataToDatabase(info, param_ex, curvedata_tempp_ex, temp_items, error))
			{
				dcisg_free(curvedata_tempp);
				log_report("SetCurveData error: %s", error.error_info);
				return FALSE;
			}
			temp_items = 0;
			last_table_time = end_table_time;
			curvedata_tempp_ex = curvedata_tempp + i;
		}
		temp_items ++;
	}
	param_ex.starttime = end_table_time;
	param_ex.stoptime = end_table_time;
	if(!SetCurveDataToDatabase(info, param_ex, curvedata_tempp_ex, temp_items, error))
	{
		dcisg_free(curvedata_tempp);
		log_report("SetCurveData error: %s", error.error_info);
		return FALSE;
	}
	dcisg_free(curvedata_tempp);
	log_report("SetCurveData out, 执行成功...");
	return TRUE;;
}
int CDci::GetSheetData(CurvePara &param, vector<ColAttr_t> &attr, vector<ColDataItem> &data, vector<string> &colname, const char* where_str, const char* order_by, ErrorInfo &error)
{
	Curveinfo *info_sheet = NULL;
	memset(&error, 0, sizeof(ErrorInfo_t));
	if (param.temp_memory1 != NULL)
	{
		strcpy(error.error_info, "param参数中的temp_memory1指针值不为NULL，请把它置NULL以后再调用本接口，成功调用本接口以后，需要释放temp_memory1中的临时内存");
		error.error_no = -1;
		return FALSE;
	}
#ifdef DEBUG
	printf("警告：当调用GetSheetData接口成功并使用完容器中数据后，需要释放param参数中的temp_memory1指针所指的临时内存\r\n");
#endif // DEBUG
	info_sheet = FindCurveinfo(param.conf_id);
	if (info_sheet == NULL)
	{
		strcpy(error.error_info, "配置文件中未发现相应的应用号");
		error.error_no = -1;
		return FALSE;
	}
	return MakeSheetTable(info_sheet, param, attr, data, colname, where_str, order_by, error);
}
int CDci::GetSheetData(char *sql, int top_number, char** out_buf, vector<ColAttr_t> &attr, vector<ColDataItem> &data, ErrorInfo &error)
{
	return MakeSheetTable(sql, top_number, out_buf, attr, data, error);
}
int CDci::GetSheetData(CurvePara &param, vector<ColAttr_t> &attr, vector<ColDataItem> &data, vector<string> &colname, const char* where_str, ErrorInfo &error)
{
	memset(&error, 0, sizeof(ErrorInfo_t));
	return GetSheetData(param, attr, data, colname, where_str, NULL, error);
}
int CDci::GetSheetData(CurvePara &param,char **datapp,ColAttr_t **attrpp,char *col_str,char *where_str,int &items, ErrorInfo &error)
{
	Curveinfo *info_sheet = NULL;
	memset(&error, 0, sizeof(ErrorInfo_t));
// 	if (param.temp_memory1 != NULL)
// 	{
// 		strcpy(error.error_info, "param参数中的temp_memory1指针值不为NULL，请把它置NULL以后再调用本接口");
// 		error.error_no = -1;
// 		return FALSE;
// 	}
	info_sheet = FindCurveinfo(param.conf_id);
	if (info_sheet == NULL)
	{
		strcpy(error.error_info, "配置文件中未发现相应的应用号");
		error.error_no = -1;
		return FALSE;
	}
	return MakeSheetTable(info_sheet, param, col_str,where_str,datapp, attrpp,items, error);
}

int CDci::MakeUpdateSql(Curveinfo *info, CurvePara &param, CurveData *curvedatap, ub2 items, char **updatesql, char* out_table_name, char **updatesql2, char* out_table_name2, ErrorInfo &error)
{
	char *table_name = NULL;
	char *table_name2 = NULL;
	char *col_name = NULL;
	char *date_time = NULL;
	char *date_time2 = NULL;
	char temp_key_id[MAX_DEV_KEY_ID_LIST_LEN];
	int count;
	char year[OBJECT_NAME_LEN];
	char month[OBJECT_NAME_LEN];
	char day[OBJECT_NAME_LEN];
	char hour[OBJECT_NAME_LEN];
	char minute[OBJECT_NAME_LEN];
	char datetime_col[OBJECT_NAME_LEN*3];
	struct tm tm_start, tm_stop;
	*updatesql = NULL;
	out_table_name[0] = '\0';
	*updatesql2 = NULL;
	out_table_name2[0] = '\0';
	memset(&error, 0, sizeof(ErrorInfo));

	error.error_no = -1;
	//if (items == 1)
	//{
		tm_start = *localtime(&curvedatap->x_time);
		tm_stop = *localtime(&curvedatap->x_time);
	//}
	//else
	//{
	//	tm_start = *localtime(&param.starttime);
	//	tm_stop = *localtime(&param.stoptime);
	//}

	trim_str_ex(param.dev_key_id_values, param.dev_key_id_values + strlen(param.dev_key_id_values), temp_key_id);
	if(strstr(temp_key_id,",")!=NULL)
	{
		strcpy(error.error_info, "未指定设备或者指定了多个设备id。");
		return FALSE;
	}
	if (strlen(info->state_table_name_ft) > 0)
	{
		if(GetTableName(info->state_table_name_ft, &date_time, &table_name, count, &tm_start, &tm_stop) == FALSE)
		{
			strcpy(error.error_info, "枚举表名失败，可能因为内存不足导致的");
			return FALSE;
		}
		strncpy(out_table_name2, table_name, 128);
		dcisg_free(table_name);
		dcisg_free(date_time);
	}
	if(GetTableName(info->table_name_ft, &date_time, &table_name, count, &tm_start, &tm_stop) == FALSE)
	{
		strcpy(error.error_info, "枚举表名失败，可能因为内存不足导致的");
		return FALSE;
	}
	if (count != 1)
	{
		sprintf(error.error_info, "程序更新采样跨表了，理论上不应该有这种情况，请联系开发人员起始时间(%ld)和终止时间(%ld)",
			param.starttime, param.stoptime);
		return FALSE;
	}
	strcpy(out_table_name, table_name);
	switch (info->row_time_span)//下面是根据行集的跨度来调整更新的where条件
	{
	case FREE_SECOND:
		sprintf(date_time,"%d-%02d-%02d %02d:%02d:%02d", tm_start.tm_year+1900, tm_start.tm_mon+1,
			tm_start.tm_mday, tm_start.tm_hour, tm_start.tm_min, tm_start.tm_sec);
		break;
	case ONE_MINUTE_SECOND:			//行集为一分钟的时间跨度
		sprintf(date_time,"%d-%02d-%02d %02d:%02d:%02d", tm_start.tm_year+1900, tm_start.tm_mon+1,
			tm_start.tm_mday, tm_start.tm_hour, tm_start.tm_min, 0);
		break;
	case ONE_HOUR_SECOND:			//行集为一小时的时间跨度
		sprintf(date_time,"%d-%02d-%02d %02d:%02d:%02d", tm_start.tm_year+1900, tm_start.tm_mon+1,
			tm_start.tm_mday, tm_start.tm_hour, 0, 0);
		break;
	case ONE_DAY_SECOND:		//行集为一天的时间跨度
		sprintf(date_time,"%d-%02d-%02d %02d:%02d:%02d", tm_start.tm_year+1900, tm_start.tm_mon+1,
			tm_start.tm_mday, 0, 0, 0);
		break;
	case ONE_MONTH_SECOND:	//行集为一月的时间跨度
		sprintf(date_time,"%d-%02d-%02d %02d:%02d:%02d", tm_start.tm_year+1900, tm_start.tm_mon+1,
			1, 0, 0, 0);
		break;
	case ONE_YEAR_SECOND://行集为一年的时间跨度
		sprintf(date_time,"%d-%02d-%02d %02d:%02d:%02d", tm_start.tm_year+1900, 1,
			1, 0, 0, 0);
		break;
	default:
		strcpy(error.error_info, "非法的行集时间跨度");
		dcisg_free(table_name);
		dcisg_free(date_time);
		return FALSE;
	}
	memset(year, 0, OBJECT_NAME_LEN);
	memset(month, 0, OBJECT_NAME_LEN);
	memset(day, 0, OBJECT_NAME_LEN);
	memset(hour, 0, OBJECT_NAME_LEN);
	memset(minute, 0, OBJECT_NAME_LEN);
	if (strlen(info->year_col_name) != 0)
	{
		strncpy(year, info->year_col_name, OBJECT_NAME_LEN -1);
	}
	else
	{
		memcpy(year, date_time, 4);
	}
	if (strlen(info->month_col_name) != 0)
	{
		strncpy(month, info->month_col_name, OBJECT_NAME_LEN -1);
	}
	else
	{
		memcpy(month, date_time + 5, 2);
	}
	if (strlen(info->day_col_name) != 0)
	{
		strncpy(day, info->day_col_name, OBJECT_NAME_LEN -1);
	}
	else
	{
		memcpy(day, date_time + 8, 2);
	}
	if (strlen(info->hour_col_name) != 0)
	{
		strncpy(hour, info->hour_col_name, OBJECT_NAME_LEN -1);
	}
	else
	{
		strcpy(hour, "0");
	}
	if (strlen(info->minute_col_name) != 0)
	{
		strncpy(minute, info->minute_col_name, OBJECT_NAME_LEN -1);
	}
	else
	{
		strcpy(minute, "0");
	}

	if (strlen(info->date_col_name) == 0)
	{
		//sprintf(datetime_col, "sysdba.to_datetime(%s, %s, %s, %s, %s)", year, month, day, hour, minute);
		sprintf(datetime_col, "to_datetime(%s, %s, %s, %s, %s)", year, month, day, hour, minute);
	}
	else
	{
		if (strlen(info->hour_col_name) !=0 || strlen(info->minute_col_name) != 0)
		{
			sprintf(datetime_col, "TIMESTAMPADD(SQL_TSI_MINUTE, %s*60 + %s, %s)", hour, minute, info->date_col_name);
		}
		else
		{
			strcpy(datetime_col, info->date_col_name);
		}
	}
	char tempsql[1000];
	sprintf(tempsql, "update %s set ", table_name);
	StrcatSql(updatesql, tempsql);
	if (out_table_name2[0] != '\0')
	{
		sprintf(tempsql, "update %s set ", out_table_name2);
		StrcatSql(updatesql2, tempsql);
	}
	for (int i=0; i<items; i++)
	{
		tm_start = *localtime(&curvedatap[i].x_time);
		switch (info->row_time_span)//下面是根据行集的跨度来调整更新的where条件
		{
		case FREE_SECOND:
			tm_start.tm_min = 0;
			tm_start.tm_hour = 0;
			tm_start.tm_mday = 1;
			tm_start.tm_mon = 0;
			tm_start.tm_year = 109;
			break;
		case ONE_MINUTE_SECOND:			//行集为一分钟的时间跨度
			tm_start.tm_min = 0;
			tm_start.tm_hour = 0;
			tm_start.tm_mday = 1;
			tm_start.tm_mon = 0;
			tm_start.tm_year = 109;
			break;
		case ONE_HOUR_SECOND:			//行集为一小时的时间跨度
			tm_start.tm_hour = 0;
			tm_start.tm_mday = 1;
			tm_start.tm_mon = 0;
			tm_start.tm_year = 109;
			break;
		case ONE_DAY_SECOND:		//行集为一天的时间跨度
			tm_start.tm_mday = 1;
			tm_start.tm_mon = 0;
			tm_start.tm_year = 109;
			break;
		case ONE_MONTH_SECOND:	//行集为一月的时间跨度
			tm_start.tm_mon = 0;
			tm_start.tm_year = 109;
			break;
		case ONE_YEAR_SECOND://行集为一年的时间跨度
			tm_start.tm_year = 109;
			break;
		}
		//下面必需放在switch (info->row_time_span)后面，因为要修整tm_start的值
		if(GetColName(info, &col_name, count, info->row_time_span, info->time_pace, &tm_start, TRUE) == FALSE)
		{
			strcpy(error.error_info, "枚举表名失败，可能因为内存不足导致的");
			dcisg_free(table_name);
			dcisg_free(date_time);
			return FALSE;
		}
		if (i==0)
		{
			sprintf(tempsql, "%s=%f", col_name, curvedatap[i].f_value);
		}
		else
		{
			sprintf(tempsql, ",%s=%f", col_name, curvedatap[i].f_value);
		}
		StrcatSql(updatesql, tempsql);
		if (out_table_name2[0] != '\0')
		{
			if (i==0)
			{
				sprintf(tempsql, "%s=%d", col_name, curvedatap[i].state);
			}
			else
			{
				sprintf(tempsql, ",%s=%d", col_name, curvedatap[i].state);
			}
			StrcatSql(updatesql2, tempsql);
		}
		dcisg_free(col_name);
		//col_name = NULL;
	}
	if (strlen(info->dev_key_col_name) != 0)
	{
		sprintf(tempsql, " where %s = %s and %s = to_date('%s','YYYY-MM-DD HH24:MI:SS')",
			info->dev_key_col_name, temp_key_id, datetime_col, date_time);
	}
	else
	{
		sprintf(tempsql, " where %s = to_date('%s','YYYY-MM-DD HH24:MI:SS')",
			datetime_col, date_time);
	}
	StrcatSql(updatesql, tempsql);
	if (out_table_name2[0] != '\0')
	{
		if (strlen(info->dev_key_col_name) != 0)
		{
			sprintf(tempsql, " where %s = %s and %s = to_date('%s','YYYY-MM-DD HH24:MI:SS')",
				info->dev_key_col_name, temp_key_id, datetime_col, date_time);
		}
		else
		{
			sprintf(tempsql, " where %s = to_date('%s','YYYY-MM-DD HH24:MI:SS')",
				datetime_col, date_time);
		}
		StrcatSql(updatesql2, tempsql);
	}
	if (strlen(info->where_nor) != 0)
	{
		StrcatSql(updatesql, " and ");
		StrcatSql(updatesql, info->where_nor);
		if (out_table_name2[0] != '\0')
		{
			StrcatSql(updatesql2, " and ");
			StrcatSql(updatesql2, info->where_nor);
		}
	}
	//printf(" the sql is %s \n",*updatesql);
	dcisg_free(table_name);
	dcisg_free(date_time);
	error.error_no = 0;
#ifdef _DEBUG
//	printf("%s\r\n", *updatesql);
#endif
	return TRUE;
}
int CDci::MakeInsertSql(Curveinfo *info, CurvePara &param, CurveData *curvedatap, ub2 items, char **insertsql, ErrorInfo &error)
{
	char *table_name = NULL;
	char *col_name = NULL;
	char *date_time = NULL;
	char temp_key_id[MAX_DEV_KEY_ID_LIST_LEN];
	int count;
	char year[OBJECT_NAME_LEN];
	char month[OBJECT_NAME_LEN];
	char day[OBJECT_NAME_LEN];
	char hour[OBJECT_NAME_LEN];
	char minute[OBJECT_NAME_LEN];
	char *sql1=NULL;
	char *sql2=NULL;
	struct tm tm_start, tm_stop;
	*insertsql = NULL;

	memset(&error, 0, sizeof(ErrorInfo));
	error.error_no = -1;
	//if (items == 1)
	//{
		tm_start = *localtime(&curvedatap->x_time);
		tm_stop = *localtime(&curvedatap->x_time);
	//}
	//else
	//{
	//	tm_start = *localtime(&param.starttime);
	//	tm_stop = *localtime(&param.stoptime);
	//}

	trim_str_ex(param.dev_key_id_values, param.dev_key_id_values + strlen(param.dev_key_id_values), temp_key_id);
	if(strstr(temp_key_id,",")!=NULL)
	{
		strcpy(error.error_info, "指定了多个设备id。");
		return FALSE;
	}
	if(GetTableName(info->table_name_ft, &date_time, &table_name, count, &tm_start, &tm_stop) == FALSE)
	{
		strcpy(error.error_info, "枚举表名失败，可能因为内存不足导致的");
		return FALSE;
	}
	if (count != 1)
	{
		strcpy(error.error_info, "单次更新采样跨表了，理论上不会存在这种情况，请联系开发人员");
		return FALSE;
	}
	switch (info->row_time_span)//下面是根据行集的跨度来调整更新的where条件
	{
	case FREE_SECOND:
		sprintf(date_time,"%d-%02d-%02d %02d:%02d:%02d", tm_start.tm_year+1900, tm_start.tm_mon+1,
			tm_start.tm_mday, tm_start.tm_hour, tm_start.tm_min, tm_start.tm_sec);
		break;
	case ONE_MINUTE_SECOND:			//行集为一分钟的时间跨度
		sprintf(date_time,"%d-%02d-%02d %02d:%02d:%02d", tm_start.tm_year+1900, tm_start.tm_mon+1,
			tm_start.tm_mday, tm_start.tm_hour, tm_start.tm_min, 0);
		break;
	case ONE_HOUR_SECOND:			//行集为一小时的时间跨度
		sprintf(date_time,"%d-%02d-%02d %02d:%02d:%02d", tm_start.tm_year+1900, tm_start.tm_mon+1,
			tm_start.tm_mday, tm_start.tm_hour, 0, 0);
		break;
	case ONE_DAY_SECOND:		//行集为一天的时间跨度
		sprintf(date_time,"%d-%02d-%02d %02d:%02d:%02d", tm_start.tm_year+1900, tm_start.tm_mon+1,
			tm_start.tm_mday, 0, 0, 0);
		break;
	case ONE_MONTH_SECOND:	//行集为一月的时间跨度
		sprintf(date_time,"%d-%02d-%02d %02d:%02d:%02d", tm_start.tm_year+1900, tm_start.tm_mon+1,
			1, 0, 0, 0);
		break;
	case ONE_YEAR_SECOND://行集为一年的时间跨度
		sprintf(date_time,"%d-%02d-%02d %02d:%02d:%02d", tm_start.tm_year+1900, 1,
			1, 0, 0, 0);
		break;
	default:
		strcpy(error.error_info, "非法的行集时间跨度");
		dcisg_free(table_name);
		dcisg_free(date_time);
		return FALSE;
	}

	memset(year, 0, OBJECT_NAME_LEN);
	memset(month, 0, OBJECT_NAME_LEN);
	memset(day, 0, OBJECT_NAME_LEN);
	memset(hour, 0, OBJECT_NAME_LEN);
	memset(minute, 0, OBJECT_NAME_LEN);
	if (strlen(info->year_col_name) != 0)
	{
		strncpy(year, info->year_col_name, OBJECT_NAME_LEN -1);
	}
	else
	{
		memcpy(year, date_time, 4);
	}
	if (strlen(info->month_col_name) != 0)
	{
		strncpy(month, info->month_col_name, OBJECT_NAME_LEN -1);
	}
	else
	{
		memcpy(month, date_time + 5, 2);
	}
	if (strlen(info->day_col_name) != 0)
	{
		strncpy(day, info->day_col_name, OBJECT_NAME_LEN -1);
	}
	else
	{
		memcpy(day, date_time + 8, 2);
	}
	if (strlen(info->hour_col_name) != 0)
	{
		strncpy(hour, info->hour_col_name, OBJECT_NAME_LEN -1);
	}
	else
	{
		strcpy(hour, "0");
	}
	if (strlen(info->minute_col_name) != 0)
	{
		strncpy(minute, info->minute_col_name, OBJECT_NAME_LEN -1);
	}
	else
	{
		strcpy(minute, "0");
	}
	char tempsql[1000];
	StrcatSql(&sql1, "insert into ");
	StrcatSql(&sql1, table_name);
	StrcatSql(&sql1, "(");
	StrcatSql(&sql2, " values(");
	if (strlen(info->dev_key_col_name) != 0)
	{
		StrcatSql(&sql1, info->dev_key_col_name);
		StrcatSql(&sql1, ",");
		StrcatSql(&sql2, param.dev_key_id_values);
		StrcatSql(&sql2, ",");
	}
	if(strlen(info->date_col_name) != 0)
	{
		StrcatSql(&sql1, info->date_col_name);
		StrcatSql(&sql1, ",");
		StrcatSql(&sql2, "'");
		StrcatSql(&sql2, date_time);
		StrcatSql(&sql2, "',");
	}
	else
	{
		if (strlen(info->year_col_name) != 0)
		{
			StrcatSql(&sql1, info->year_col_name);
			StrcatSql(&sql1, ",");
			sprintf(year, "%d", tm_start.tm_year+1900);
			StrcatSql(&sql2 , year);
			StrcatSql(&sql2 , ",");
		}
		if (strlen(info->month_col_name) != 0)
		{
			StrcatSql(&sql1, info->month_col_name);
			StrcatSql(&sql1, ",");
			sprintf(month, "%d", tm_start.tm_mon+1);
			StrcatSql(&sql2 , month);
			StrcatSql(&sql2 , ",");
		}
		if (strlen(info->day_col_name) != 0)
		{
			StrcatSql(&sql1, info->day_col_name);
			StrcatSql(&sql1, ",");
			sprintf(day, "%d", tm_start.tm_mday);
			StrcatSql(&sql2 , day);
			StrcatSql(&sql2 , ",");
		}
	}
	if (strlen(info->hour_col_name) != 0)
	{
		StrcatSql(&sql1, info->hour_col_name);
		StrcatSql(&sql1, ",");
		sprintf(hour, "%d", tm_start.tm_hour);
		StrcatSql(&sql2 , hour);
		StrcatSql(&sql2 , ",");
	}
	if (strlen(info->minute_col_name) != 0)
	{
		StrcatSql(&sql1, info->minute_col_name);
		StrcatSql(&sql1, ",");
		sprintf(minute, "%d", tm_start.tm_min);
		StrcatSql(&sql2 , minute);
		StrcatSql(&sql2 , ",");
	}
	for (int i=0; i<items; i++)
	{
		tm_start = *localtime(&curvedatap[i].x_time);
		switch (info->row_time_span)//下面是根据行集的跨度来调整更新的where条件
		{
		case FREE_SECOND:
			tm_start.tm_min = 0;
			tm_start.tm_hour = 0;
			tm_start.tm_mday = 1;
			tm_start.tm_mon = 0;
			tm_start.tm_year = 109;
			break;
		case ONE_MINUTE_SECOND:			//行集为一分钟的时间跨度
			tm_start.tm_min = 0;
			tm_start.tm_hour = 0;
			tm_start.tm_mday = 1;
			tm_start.tm_mon = 0;
			tm_start.tm_year = 109;
			break;
		case ONE_HOUR_SECOND:			//行集为一小时的时间跨度
			tm_start.tm_hour = 0;
			tm_start.tm_mday = 1;
			tm_start.tm_mon = 0;
			tm_start.tm_year = 109;
			break;
		case ONE_DAY_SECOND:		//行集为一天的时间跨度
			tm_start.tm_mday = 1;
			tm_start.tm_mon = 0;
			tm_start.tm_year = 109;
			break;
		case ONE_MONTH_SECOND:	//行集为一月的时间跨度
			tm_start.tm_mon = 0;
			tm_start.tm_year = 109;
			break;
		case ONE_YEAR_SECOND://行集为一年的时间跨度
			tm_start.tm_year = 109;
			break;
		}
		//下面必需放在switch (info->row_time_span)后面，因为要修整tm_start的值
		if(GetColName(info, &col_name, count, info->row_time_span, info->time_pace, &tm_start, TRUE) == FALSE)
		{
			strcpy(error.error_info, "枚举表名失败，可能因为内存不足导致的");
			dcisg_free(table_name);
			dcisg_free(date_time);
			return FALSE;
		}
		if (i==0)
		{
			sprintf(tempsql, "%s", col_name);
		}
		else
		{
			sprintf(tempsql, ",%s", col_name);
		}
		StrcatSql(&sql1, tempsql);
		if (i==0)
		{
			sprintf(tempsql, "%f", curvedatap[i].f_value);
		}
		else
		{
			sprintf(tempsql, ",%f", curvedatap[i].f_value);
		}
		StrcatSql(&sql2, tempsql);
		dcisg_free(col_name);
	}
	dcisg_free(table_name);
//	dcisg_free(col_name);
	dcisg_free(date_time);
	*insertsql = (char*)dcisg_malloc(strlen(sql1) + strlen(sql2) + strlen(info->insert_exp) + 100);
	if (*insertsql == NULL)
	{
		dcisg_free(sql1);
		dcisg_free(sql2);
		strcpy(error.error_info, "内存不足");
		return FALSE;
	}
	sprintf(*insertsql, info->insert_exp, sql1, sql2);//"%s, xxx) %s, xxx)"
	dcisg_free(sql1);
	dcisg_free(sql2);
	error.error_no = 0;
#ifdef _DEBUG
//	printf("%s\r\n", *insertsql);
#endif
	return TRUE;
}
Curveinfo* CDci::FindCurveinfo(char *conf_id)
{
	char temp_id[OBJECT_NAME_LEN];
	Curveinfo *temp_info = NULL;
	trim_str(conf_id, conf_id+strlen(conf_id), temp_id);
 	if (g_curveinfop == NULL)
 	{
 		return NULL;
 	}
	temp_info = g_curveinfop;
	while(temp_info)
	{
		if(stricmp(temp_id, temp_info->conf_id) == 0)
		{
			return temp_info;
		}
		temp_info = temp_info->next;
	}
	return NULL;
}
void aaaaa()
{
	Curveinfo *temp_info = g_curveinfop;
	char buf[5000];
	CDci dci;
	ErrorInfo error;
	dci.Connect("40.10.1.103", "HISDB", "HISDB", &error);
	sprintf(buf, "truncate table CURVE_CONFIG");
	dci.ExecSingle(buf, &error);
	while(temp_info)
	{
		sprintf(buf, "insert into CURVE_CONFIG(CONF_ID,CURVE_ID,TABLE_NAME_FT,STATE_TABLE_NAME_FT,COLUMN_NAME_FT,ADDITION_COLUMN_NAME_FT,DEV_KEY_COL_NAME,\
			MINUTE_COL_NAME,HOUR_COL_NAME,DAY_COL_NAME,MONTH_COL_NAME,YEAR_COL_NAME,DATE_COL_NAME,BASE_COL_INDEX,ROW_TIME_SPAN,TIME_PACE,WHERE_NOR,INSERT_EXP,INDEX_STR) values(\
			'%s',  '%s',     '%s',        '%s',                '%s',          '%s',                   '%s',            '%s',            '%s',             '%s',   '%s',          '%s',         '%s',          %d,         %d,           %d,    '%s',    '%s',       '%s' )",
			temp_info->conf_id,
			temp_info->curve_id,
			temp_info->table_name_ft,
			temp_info->state_table_name_ft,
			temp_info->column_name_ft,
			temp_info->addition_column_name_ft,
			temp_info->dev_key_col_name,
			temp_info->minute_col_name,
			temp_info->hour_col_name,
			temp_info->day_col_name,
			temp_info->month_col_name,
			temp_info->year_col_name,
			temp_info->date_col_name,
			temp_info->base_col_index,
			temp_info->row_time_span,
			temp_info->time_pace,
			temp_info->where_nor,
			temp_info->insert_exp,
			temp_info->index_str);
		dci.ExecSingle(buf, &error);
		temp_info = temp_info->next;
	}
}
// Curveinfo* CDci::FindSheetinfo(char *sheetid)
// {
// 	char temp_id[OBJECT_NAME_LEN];
// 	Curveinfo *temp_info = NULL;
// 	trim_str(sheetid, sheetid+strlen(sheetid), temp_id);
// 	temp_info = g_sheetinfop;
// 	while(temp_info)
// 	{
// 		if(stricmp(temp_id, temp_info->conf_id) == 0)
// 		{
// 			return temp_info;
// 		}
// 		temp_info = temp_info->next;
// 	}
// 	return NULL;
// }
/*
#if 0	//下面这段函数是支持8字节长的DOUBLE类型的采样值，目前用不到
static
int MakeData(Curveinfo *info, CurvePara &param, CurveData* outdatap,
			 char* result_datap, char* result_data2p,
			 int rec_size, int rec_size2,
			 int items,
			 ColAttr_t *attr, ColAttr_t *attr2,
			 int result_cols, ErrorInfo &error)
{
	double temp_value = 0;
	double temp_value2 = 0;
	int state_value = 0, state_value2 = 0;
	tm start_time = *localtime(&param.starttime);
	int	needpace = 0;
	int col = 0, row = 0;
	int i, j = 0;
	time_t temp_time;
	char *tempdatap = result_datap;
	char *tempdata2p = result_data2p;
	sb2 null_flag = 0;
	int temp_count = 0;
	if (outdatap == NULL || result_datap == NULL || attr == NULL || items == 0 || result_cols == 0)
	{
		strcpy(error.error_info, "调整步长时，发现函数调用使用了非法的参数");
		return FALSE;
	}
	int needpace_ex = info->time_pace != 0 ? info->time_pace : info->row_time_span; //如果列名就是采样设备名的情况，那么取它的行集时间跨度为每两个采样点的步行;
	switch (info->row_time_span)//下面是根据行集的跨度来调整查询的起始和终止条件
	{
	case ONE_MINUTE_SECOND:			//行集为一分钟的时间跨度
		col = (start_time.tm_sec)/needpace_ex;
		break;
	case ONE_HOUR_SECOND:			//行集为一小时的时间跨度
		col = (start_time.tm_min*60 + start_time.tm_sec)/needpace_ex;
		break;
	case ONE_DAY_SECOND:		//行集为一天的时间跨度
		col = (start_time.tm_hour * 60 * 60 + start_time.tm_min*60 + start_time.tm_sec)/needpace_ex;
		break;
	case ONE_MONTH_SECOND:	//行集为一月的时间跨度
		col = ((start_time.tm_mday-1) * 24 * 60 * 60 + start_time.tm_hour * 60 * 60 + start_time.tm_min*60 + start_time.tm_sec)/needpace_ex;
		break;
	case ONE_YEAR_SECOND://行集为一年的时间跨度
		col = ((start_time.tm_mon-1)* 31 * 24 * 60 * 60 + (start_time.tm_mday-1) * 24 * 60 * 60 + start_time.tm_hour * 60 * 60 + start_time.tm_min*60 + start_time.tm_sec)/needpace_ex;
		break;
	default:
		strcpy(error.error_info, "非法的行集时间跨度");
		return FALSE;
	}
	for (i=0, j=0; i<col; i++)
	{
		if (attr[j].data_size == 8)//是double类型值的采样值
		{
			tempdatap += 8;
		}
		else						//是float类型的采样值
		{
			tempdatap += 4;
		}
		tempdatap += sizeof(sb2);
		if (tempdata2p != NULL)
		{
			if (attr2[j].data_size == 8)//是BIGINT类型值的状态值
			{
				tempdata2p += 8;
			}
			else						//是INT类型值的状态值
			{
				tempdata2p += 4;
			}
			tempdata2p += sizeof(sb2);
		}
		j++;
		if (j >= result_cols)
		{
			strcpy(error.error_info, "在调整起始列时偏移位置时产生了错误，列的偏移计数超过了行集的最大列数");
			return FALSE;
		}
	}
	col = j;
	temp_time = param.starttime;
	outdatap->is_null = TRUE;
	while (temp_time <= param.stoptime && items>0)
	{
		needpace += needpace_ex;
		if (attr[col].data_size == 8)
		{
			temp_value2 = *(double*)tempdatap;
			tempdatap += 8;
		}
		else
		{
			temp_value2 = *(float*)tempdatap;
			tempdatap += 4;
		}
		null_flag = *((sb2*)tempdatap);
		tempdatap += sizeof(sb2);
		if (tempdata2p != NULL)
		{
			if (attr2[col].data_size == 8)
			{
				state_value2 = (sb4)(*(sb8*)tempdata2p);
				tempdata2p += 8;
			}
			else
			{
				state_value2 = *(sb4*)tempdata2p;
				tempdata2p += 4;
			}
			tempdata2p += sizeof(sb2);	//跳过空指示符空间
		}
		if(null_flag == FALSE)
		{
			temp_count ++;
			switch (param.data_format)
			{
			case MAX_VALUE://步长中的最大值
				outdatap->is_null = FALSE;
				if (temp_count == 1)
				{
					temp_value = temp_value2;
					state_value = state_value2;
				}
				else
				{
					if (temp_value < temp_value2)
					{
						temp_value = temp_value2;
						state_value = state_value2;
					}
				}
				break;
			case MIN_VALUE://步长中的最小值
				outdatap->is_null = FALSE;
				if (temp_count == 1)
				{
					temp_value = temp_value2;
					state_value = state_value2;
				}
				else
				{
					if (temp_value > temp_value2)
					{
						temp_value = temp_value2;
						state_value = state_value2;
					}
				}
				break;
			case AVG_VALUE://步长中的平均值
				outdatap->is_null = FALSE;
				if (temp_count == 1)
				{
					temp_value = temp_value2;
					state_value = state_value2;
				}
				else
				{
					temp_value += temp_value2;
				}
				break;
			case FIRST_VALUE://步长中的第一个值
				if (needpace == needpace_ex)
				{
					outdatap->is_null = FALSE;
					temp_value = temp_value2;
					state_value = state_value2;
				}

				break;
			case LAST_VALUE://步长中的最后一个值
				if(needpace == param.needpace)
				{
					outdatap->is_null = FALSE;
					temp_value = temp_value2;
					state_value = state_value2;
				}
				break;
			default:
				strcpy(error.error_info, "非法的取值方式，目前仅支持最大值，最小值，平均值，首值和末值");
				return FALSE;
			}
		}
		if (needpace == param.needpace)
		{
			if (outdatap->is_null == FALSE)
			{
				if (param.data_format != AVG_VALUE)
				{
					outdatap->values = (float)temp_value;
				}
				else
				{
					outdatap->values = (float)(temp_value/temp_count);
				}
				outdatap->state = state_value;
			}
			outdatap->x_time = temp_time;
			outdatap++;
			needpace = 0;
			temp_count = 0;
			outdatap->is_null = TRUE;//在这里，使用了前面申请空间时预留的一份空间
			items --;
		}
		temp_time += needpace_ex;
		col ++;
		if (info->row_time_span >= 31*24*60*60)//如果行集大小是一个月的跨度或一年的跨度
		{
			start_time = *localtime(&temp_time);
			if (start_time.tm_mday == 1 && start_time.tm_hour == 0 && start_time.tm_min == 0 && start_time.tm_sec == 0
				&& (info->row_time_span == 31*24*60*60 || start_time.tm_mon == 1 && info->row_time_span == 12*31*24*60*60))
			{
				tempdatap = result_datap + row * rec_size;
				tempdata2p = result_data2p + row * rec_size2;
				row ++;
				col = 0;
			}
			if (col >= result_cols)
			{
				strcpy(error.error_info, "根据步进抽取采样点时产生了异常情况，偏移列数超过了结果集的列数");
				return FALSE;
			}
		}
		else
		{
			if (col >= result_cols)
			{
				col = 0;
			}
		}

	}
#ifdef _DEBUG
	if (items != 0 && items != 1)
	{
		printf("算法出错了，理论上items的值应该为0");
		return FALSE;
	}
#endif
	return TRUE;
}
#endif
*/




int CDci::ExecSqlReturnAffectRowCount(const char *sqlstr, ErrorInfo_t *error)
{
	int retcode = 0;
	DCIStmt          *stmthp = NULL;
	DCIError           *errhp = NULL;
	ub4			row_count = 0;
	memset(error, 0, sizeof(ErrorInfo_t));
	if (is_connect == FALSE)
	{
		SET_ERROR(error, 3113, "服务器已经断开");
		return FALSE;
	}
	log_report("sql:%s", sqlstr);
	memset(error, 0, sizeof(ErrorInfo_t));
	DCI_CHECK1(error, DCIHandleAlloc(envhp, (void**)&stmthp, DCI_HTYPE_STMT, 0, 0));
	DCI_CHECK1(error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));
	DCI_CHECK(error, DCIStmtPrepare(stmthp, errhp, (DciText *)sqlstr, (ub4)strlen(sqlstr), (ub4)DCI_NTV_SYNTAX, (ub4)DCI_DEFAULT));
	DCI_CHECK(error, StmtExecute(svchp, stmthp, errhp, 1, DCI_COMMIT_ON_SUCCESS));
	DCI_CHECK(error, DCIAttrGet(stmthp, DCI_HTYPE_STMT, &row_count, 0, DCI_ATTR_ROW_COUNT, errhp));
	DCIHandleFree((dvoid *)errhp, DCI_HTYPE_ERROR);
	DCIHandleFree((dvoid *)stmthp, DCI_HTYPE_STMT);
	return row_count;
}

static int MakeDataEx(CurveData* outdatap, char* result_datap, char* result_data2p,
					  int rec_size, int rec_size2, int datetime_size, int rows, int cols, ErrorInfo &error)
{
	int col = 0, row = 0;
	time_t temp_time;
	char *tempdatap = NULL;
	char *tempdata2p = NULL;
	if (outdatap == NULL || result_datap == NULL || rows == 0 || cols == 0 || datetime_size < 7)
	{
		error.error_no = -1;
		strcpy(error.error_info, "输出采样值时，发现函数调用使用了非法的参数");
		return FALSE;
	}

	for (row = 0; row < rows; row ++)
	{
		tempdatap = result_datap + row * rec_size;
		tempdata2p = result_data2p + row * rec_size2;
		convert_dat_to_time(tempdatap, &temp_time);
		tempdatap += datetime_size + sizeof(sb2);
		if (result_data2p)
		{
			result_data2p += datetime_size + sizeof(sb2);
		}
		for (col = 0; col < cols; col ++)
		{
			outdatap->x_time = temp_time;
			outdatap->f_value = *(float*)tempdatap;
			tempdatap += 4;
			outdatap->is_null = (*(sb2*)tempdatap == -1);
			tempdatap += sizeof(sb2);		//跳过空指示符空间
			if (tempdata2p != NULL)
			{
				outdatap->state = *(sb4*)tempdata2p;
				tempdata2p += 4;
				tempdata2p += sizeof(sb2);	//跳过空指示符空间
			}
			outdatap ++;
		}
	}
	return TRUE;
}
int CDci::MakeTableEx(Curveinfo *info, CurvePara &param, CurveData **datapp, int &items, ErrorInfo &error)
{
	int			table_count, col_count;
	char		*sql = NULL;
	char		*sql2 = NULL;
	ColAttr_t	*attr = NULL;
	ColAttr_t	*attr2 = NULL;
	char		*data = NULL;
	char		*data2 = NULL;
	char		*dataEx = NULL;
	char		*dataEx2 = NULL;
	int			rec_size = 0;
	int			rec_size2 = 0;
	int			rec_num, col_num, rec_num2, col_num2;
	ub4			buf_size;
	*datapp = NULL;
	items = 0;
	if (!CheckStartAndEndTime(info, param, error))
	{
		RETURN_FALSE_MAKETABLE;
	}
	if (MakeSql(info, param, &sql, &sql2, table_count, col_count, error) == FALSE)
	{
		RETURN_FALSE_MAKETABLE;
	}
	if (sql == NULL)
	{
		error.error_no = -1;
		strcpy(error.error_info, "未找到采样表");
		RETURN_FALSE_MAKETABLE;
	}
	if(ReadDataEx(sql, -1, &rec_num, &col_num, &attr, &data, &buf_size, &error) == FALSE)
	{
		RETURN_FALSE_MAKETABLE;
	}
	if ((rec_size = GetResultRecSize(attr, col_num, error)) == 0)
	{
		RETURN_FALSE_MAKETABLE;
	}
	if (sql2 != NULL)//如果采样表存在状态表查询语句
	{
		if(ReadDataEx(sql2, -1, &rec_num2, &col_num2, &attr2, &data2, &buf_size, &error) == FALSE)
		{
			RETURN_FALSE_MAKETABLE;
		}
		if ((rec_size2 = GetResultRecSize(attr2, col_num2, error)) == 0)
		{
			RETURN_FALSE_MAKETABLE;
		}
		if (col_num != col_num2)
		{
			//这里需要注意，如果采样表列表达式配置了多列，那这些列就应该在状态表中同样存在
			error.error_no = -1;
			strcpy(error.error_info, "存在采样状态表的情况下，状态表中的列数和采样表中的列数不一致");
			RETURN_FALSE_MAKETABLE;
		}
		if (rec_num != rec_num2)
		{
			error.error_no = -1;
			strcpy(error.error_info, "存在采样状态表的情况下，状态表中的行数和采样表中的行数不一致");
			RETURN_FALSE_MAKETABLE;
		}
	}
	items = (col_num - 1) * rec_num;
	if(items == 0)
	{
		error.error_no = -1;
		strcpy(error.error_info, "未取到任何采样值");
		RETURN_FALSE_MAKETABLE;
	}
	*datapp = (CurveData*)dcisg_malloc(items * sizeof(CurveData));
	if (*datapp == NULL)
	{
		error.error_no = -1;
		strcpy(error.error_info, "内存不足");
		RETURN_FALSE_MAKETABLE;
	}
	memset(*datapp, 0, items * sizeof(CurveData));
	if (!MakeDataEx(*datapp, data, data2, rec_size, rec_size2, attr[0].data_size, rec_num, col_num-1, error))
	{
		RETURN_FALSE_MAKETABLE;
	}
	RETURN_TRUE_MAKETABLE;
}

void CDci::InitSchemaTableColAttr()
{
	char sql[2048];
	int attr_num;
	ColAttr_t *attrp;
	Curveinfo *ptemp = g_curveinfop;
	ErrorInfo error;
	//danath 南瑞要求异步图表提交接口 20120223
	//放开此处检查条件是为了防止midhs运行时g_curveinfop中col_attr初始化不完整
	g_has_curveinfo_init = 0;
	if (g_has_curveinfo_init == 1 || is_connect == FALSE)
	{
		return;
	}
	g_has_curveinfo_init = 1;
	while(ptemp)
	{
		if (strlen(ptemp->table_name_model) != 0)
		{
			attr_num = 0;
			attrp = NULL;
			sprintf(sql, "select * from %s", ptemp->table_name_model);
			if (GetAttrs(sql, &attr_num, &attrp, &error))
			{
				for (int i=0; i<attr_num; i++)
				{
					ColType ct;
					ct.col_len = attrp[i].data_size;
					ct.col_type = attrp[i].data_type;
					ptemp->col_attr.insert(std::pair<string, ColType>(attrp[i].col_name, ct));
				}
				dcisg_free(attrp);
				attrp = NULL;
			}
			else
			{
				printf("描述模板表(%s)时产生错误：%s\r\n", ptemp->table_name_model, error.error_info);
			}
		}
		ptemp = ptemp->next;
	}
}

int CDci::SetSheetData(char *model_name, time_t record_time, const SheetColData *indexp, ub2 ixsize, const SheetColData *valuep, ub2 vesize, ErrorInfo &error)
{
	Curveinfo	*info_sheet = NULL;
	DCIBind		*ppbnd[2048];
	char		sql[2048];
	int			retcode;
	DCIStmt          *stmthp = NULL;
	DCIError           *errhp = NULL;
	int row_count;
	SheetColData *tempp = NULL;
	memset(&error, 0, sizeof(ErrorInfo));
	info_sheet = FindCurveinfo(model_name);
	if (info_sheet == NULL)
	{
		strcpy(error.error_info, "配置文件中未发现相应的模板名");
		error.error_no = -1;
		RETURN_FALSE;
	}
	if (indexp == NULL && ixsize != 0 || valuep == NULL || vesize ==0)
	{
		strcpy(error.error_info, "输入参数错误，更新数据或索引数据指针描述不正确");
		error.error_no = -1;
		RETURN_FALSE;
	}


	DCI_CHECK1(&error, DCIHandleAlloc(envhp, (void**)&stmthp, DCI_HTYPE_STMT, 0, 0));
	DCI_CHECK1(&error, DCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, DCI_HTYPE_ERROR, (size_t)0, (dvoid **)0));
	if (ixsize == 0)
	{
		if (!MakeSheetUpdateSql(info_sheet, valuep, vesize, vesize, TRUE, record_time, sql, error))
		{
			RETURN_FALSE;
		}
		DCI_CHECK(&error, DCIStmtPrepare(stmthp, errhp, (DciText *)sql, (ub4)strlen(sql), (ub4)DCI_NTV_SYNTAX, (ub4)DCI_DEFAULT));
		if (!BindSheetCol(info_sheet, stmthp, errhp, ppbnd, valuep, vesize, error))
		{
			RETURN_FALSE;
		}
		DCI_CHECK(&error, StmtExecute(svchp, stmthp, errhp, 1, DCI_DEFAULT));
		RETURN_TRUE;
	}
	else
	{
		if (vesize == 0)
		{
			strcpy(error.error_info, "未找到需要更新的列数据");
			error.error_no = -1;
			RETURN_FALSE;
		}
		tempp = (SheetColData*)dcisg_malloc(sizeof(SheetColData) * (ixsize + vesize));
		if (tempp == NULL)
		{
			strcpy(error.error_info, "内存不足");
			error.error_no = -1;
			RETURN_FALSE;
		}
		memcpy(tempp, valuep, sizeof(SheetColData) * vesize);
		memcpy(tempp + vesize, indexp, sizeof(SheetColData) * ixsize);
		if (!MakeSheetUpdateSql(info_sheet, tempp, vesize, ixsize + vesize, FALSE, record_time, sql, error))
		{
			dcisg_free(tempp);
			RETURN_FALSE;
		}
		DCI_CHECK_EX(&error, DCIStmtPrepare(stmthp, errhp, (DciText *)sql, (ub4)strlen(sql), (ub4)DCI_NTV_SYNTAX, (ub4)DCI_DEFAULT));
		if (retcode == DCI_ERROR)
		{
			dcisg_free(tempp);
			RETURN_FALSE;
		}
		if(!BindSheetCol(info_sheet, stmthp, errhp, ppbnd, tempp, ixsize + vesize, error))
		{
			dcisg_free(tempp);
			RETURN_FALSE;
		}
		DCI_CHECK_EX(&error, StmtExecute(svchp, stmthp, errhp, 1, DCI_DEFAULT));
		if (retcode == DCI_ERROR)
		{
			dcisg_free(tempp);
			RETURN_FALSE;
		}
		row_count = 0;
		DCIAttrGet(stmthp, DCI_HTYPE_STMT, &row_count, 0, DCI_ATTR_ROW_COUNT, errhp);
		if(row_count == 0)
		{
			int k,m,n,find_flag;
			n = vesize;
			for (k=0; k<ixsize; k++)
			{
				find_flag = 0;
				for (m=0; m<vesize; m++)
				{
					if (strcmp(indexp[k].col_name, valuep[m].col_name) == 0)
					{
						find_flag = 1;
						break;
					}
				}
				if (find_flag == 0)
				{
					memcpy(tempp + n, indexp + k, sizeof(SheetColData));
					n++;
				}
			}
			if(!SetSheetData(model_name, record_time, NULL, 0, tempp, n, error))
			{
				dcisg_free(tempp);
				RETURN_FALSE;
			}
		}
		dcisg_free(tempp);
		RETURN_TRUE;
	}
	RETURN_TRUE;
}

int CDci::SetSheetData(char *model_name, vector<SheetRecData> &rec, ErrorInfo &error)
{
	SheetColData *indexp;
	SheetColData *valuep;
	int i, j;
	memset(&error, 0, sizeof(ErrorInfo_t));
	error.error_no = -1;
	for (i=0; i<rec.size(); i++)
	{
		indexp = (SheetColData*)dcisg_malloc(rec[i].index.size() * sizeof(SheetColData));
		if (indexp == NULL)
		{
			strcpy(error.error_info, "内存不足");
			return FALSE;
		}
		valuep = (SheetColData*)dcisg_malloc(rec[i].values.size() * sizeof(SheetColData));
		if (valuep == NULL)
		{
			dcisg_free(indexp);
			strcpy(error.error_info, "内存不足");
			return FALSE;
		}
		for (j=0; j<rec[i].index.size(); j++)
		{
			indexp[j] = rec[i].index[j];
		}
		for (j=0; j<rec[i].values.size(); j++)
		{
			valuep[j] = rec[i].values[j];
		}
		if (!SetSheetData(model_name, rec[i].rec_time, indexp, rec[i].index.size(), valuep, rec[i].values.size(), error))
		{
			dcisg_free(indexp);
			dcisg_free(valuep);
			ExecRollback(NULL);
			return FALSE;
		}
		dcisg_free(indexp);
		dcisg_free(valuep);
		indexp = NULL;
		valuep = NULL;
	}
	error.error_no = 0;
	return ExecCommit(&error);
}


int CDci::BindSheetCol(Curveinfo *info_sheet, DCIStmt *stmthp, DCIError *errhp, DCIBind **ppbnd, const SheetColData *sheet_col_datap, ub2 col_size, ErrorInfo &error)
{
	ub2 i=0;
	for (i=0; i<col_size; i++)
	{
		map<string, ColType>::iterator iter;
		iter = info_sheet->col_attr.find(sheet_col_datap[i].col_name);
		if(iter == info_sheet->col_attr.end())
		{
			error.error_no = -1;
			sprintf(error.error_info, "未在模板表的列中发现列(%s)的属性(注：一般列中的英文应该设置为大写格式)", sheet_col_datap[i].col_name);
			return FALSE;
		}
		ColType		&ct = iter->second;
		if (ct.col_type == DCI_DAT)
		{
			struct tm tm_time;
			DCIDate *col_date_buf= (DCIDate*)(sheet_col_datap[i].col_name + OBJECT_NAME_LEN);
			tm_time = *localtime((time_t*)&sheet_col_datap[i].t_value);
			col_date_buf->DCIDateYYYY = tm_time.tm_year + 1900;
			col_date_buf->DCIDateMM = tm_time.tm_mon + 1;
			col_date_buf->DCIDateDD = tm_time.tm_mday;
			col_date_buf->DCIDateTime.DCITimeHH = tm_time.tm_hour;
			col_date_buf->DCIDateTime.DCITimeMI = tm_time.tm_min;
			col_date_buf->DCIDateTime.DCITimeSS = tm_time.tm_sec;
			DCIBindByPos(stmthp, &ppbnd[i], errhp, i+1, col_date_buf, ct.col_len,
				DCI_ODT, NULL, NULL, NULL, 0, NULL, DCI_DEFAULT);
		}
		else
		{
			DCIBindByPos(stmthp, &ppbnd[i], errhp, i+1, (void *)&sheet_col_datap[i].str_value, ct.col_len,
				ct.col_type, NULL, NULL, NULL, 0, NULL, DCI_DEFAULT);
		}
	}
	return TRUE;
}

int GetIntBySize(const SheetColData *sheet_col_datap, ColType ct)
{
	if (ct.col_type != DCI_INT)
	{
		return 1;
	}
	switch (ct.col_len)
	{
	case 1:
		return (int)sheet_col_datap->sb1_value;
	case 2:
		return (int)sheet_col_datap->sb2_value;
	case 4:
		return (int)sheet_col_datap->sb4_value;
	case 8:
		return (int)sheet_col_datap->sb8_value;
	default:
		break;
	}
	return 1;
}

int CDci::MakeSheetUpdateSql(Curveinfo *info_sheet, const SheetColData *sheet_col_datap, ub2 col_value_size, ub2 col_size, int is_insert, time_t record_time, char *sql, ErrorInfo &error)
{
	map<string, ColType>::iterator iter;
	char *table_name = NULL;
	char *data_time = NULL;
//	char start_time_str[OBJECT_NAME_LEN];
	char temp_str[200];
	const SheetColData* sheet_datetime_col = NULL;
	int table_count;
//	ColType		ct;
	time_t		current_time = record_time;
	struct tm tm_start;
	struct tm tm_end;
	ub2 i;
	error.error_no = -1;
//	time(&current_time);
	tm_start = *localtime(&current_time);
// 	if (strlen(info_sheet->date_col_name) != 0)
// 	{
// 		sheet_datetime_col = FindSheetColData(info_sheet->date_col_name, sheet_col_datap, col_size);
// 		if (sheet_datetime_col)
// 		{
// 			iter = info_sheet->col_attr.find(info_sheet->date_col_name);
// 			if(iter == info_sheet->col_attr.end())
// 			{
// 				error.error_no = -1;
// 				sprintf(error.error_info, "未在模板表的列中发现配置项中配置的日期列(%s)的属性", info_sheet->date_col_name);
// 				return FALSE;
// 			}
// 			ct = iter->second;
// 			if (ct.col_type != DCI_DAT)
// 			{
// 				strcpy(error.error_info, "配置文件中配置的时间列名的数据类型不是日期时间类型");
// 				return FALSE;
// 			}
// 			tm_start = *localtime((time_t*)&sheet_datetime_col->t_value);
// 		}
// 	}
// 	else
// 	{
// 		sheet_datetime_col = FindSheetColData(info_sheet->year_col_name, sheet_col_datap, col_size);
// 		if (sheet_datetime_col)
// 		{
// 			iter = info_sheet->col_attr.find(info_sheet->year_col_name);
// 			if(iter == info_sheet->col_attr.end())
// 			{
// 				error.error_no = -1;
// 				sprintf(error.error_info, "未在模板表的列中发现配置项中配置的年份列(%s)的属性", info_sheet->year_col_name);
// 				return FALSE;
// 			}
// 			ct = iter->second;
// 			tm_start.tm_year = GetIntBySize(sheet_datetime_col, ct) - 1900;
// 		}
// 		sheet_datetime_col = FindSheetColData(info_sheet->month_col_name, sheet_col_datap, col_size);
// 		if (sheet_datetime_col)
// 		{
// 			iter = info_sheet->col_attr.find(info_sheet->month_col_name);
// 			if(iter == info_sheet->col_attr.end())
// 			{
// 				error.error_no = -1;
// 				sprintf(error.error_info, "未在模板表的列中发现配置项中配置的月份列(%s)的属性", info_sheet->month_col_name);
// 				return FALSE;
// 			}
// 			ct = iter->second;
// 			tm_start.tm_mon = GetIntBySize(sheet_datetime_col, ct) -1;
// 		}
// 		sheet_datetime_col = FindSheetColData(info_sheet->day_col_name, sheet_col_datap, col_size);
// 		if (sheet_datetime_col)
// 		{
// 			iter = info_sheet->col_attr.find(info_sheet->day_col_name);
// 			if(iter == info_sheet->col_attr.end())
// 			{
// 				error.error_no = -1;
// 				sprintf(error.error_info, "未在模板表的列中发现配置项中配置的日份列(%s)的属性", info_sheet->day_col_name);
// 				return FALSE;
// 			}
// 			ct = iter->second;
// 			tm_start.tm_mday = GetIntBySize(sheet_datetime_col, ct);
// 		}
// 	}


	tm_end = tm_start;
	if(GetTableName(info_sheet->table_name_ft, &data_time, &table_name, table_count, &tm_start, &tm_end) == FALSE)
	{
		strcpy(error.error_info, "枚举表名失败，可能因为内存不足导致的");
		return FALSE;
	}
	dcisg_free(data_time);
	data_time = NULL;
	if (is_insert)
	{
		sprintf(sql, "insert into %s(", table_name);
		for (i=0; i<col_size; i++)
		{
			if (i == col_size - 1)
			{
				sprintf(temp_str, "%s) values(", sheet_col_datap[i].col_name);
			}
			else
			{
				sprintf(temp_str, "%s,", sheet_col_datap[i].col_name);
			}
			strcat(sql, temp_str);
		}
		for (i=0; i<col_size; i++)
		{
			if (i == col_size - 1)
			{
				sprintf(temp_str, ":%d)", i);
			}
			else
			{
				sprintf(temp_str, ":%d,", i);
			}
			strcat(sql, temp_str);
		}
	}
	else
	{
		sprintf(sql, "update %s set ", table_name);
		for (i=0; i<col_value_size; i++)
		{
			if (i == col_value_size - 1)
			{
				sprintf(temp_str, "%s=:%d ", sheet_col_datap[i].col_name, i);
			}
			else
			{
				sprintf(temp_str, "%s=:%d,", sheet_col_datap[i].col_name, i);
			}
			strcat(sql, temp_str);
		}
		for (i=col_value_size; i<col_size; i++)
		{
			if (i == col_value_size)
			{
				strcat(sql, " where ");
			}

			if (i == col_size - 1)
			{
				sprintf(temp_str, "%s=:%d", sheet_col_datap[i].col_name, i);
			}
			else
			{
				sprintf(temp_str, "%s=:%d and ", sheet_col_datap[i].col_name, i);

			}
			strcat(sql, temp_str);
		}
	}
	error.error_no = 0;
	dcisg_free(table_name);
	return TRUE;
}

const SheetColData* CDci::FindSheetColData(const char *col_name, const SheetColData *sheet_col_datap, ub2 col_size)
{
	if (col_name[0] == '\0')
	{
		return NULL;
	}
	for (ub2 i=0; i<col_size; i++)
	{
		if(stricmp(sheet_col_datap[i].col_name, col_name) == 0)
		{
			return sheet_col_datap + i;
		}
	}
	return NULL;
}

bool CDci::Connect(const char *server, const char *user_name, const char *pasword, ErrorInfo &error)
{
	return Connect(server, user_name, pasword, &error);
}

void CDci::ConvertDciType(ub2 &dtype, ub2 &col_width, ub2 dprecision, ub1 dscale)
{
	if (dtype == DCI_NUM)
	{
		if ((dprecision == 0) && (dscale == 0))
		{
			col_width = sizeof(float);
			dtype = DCI_FLT;
		}
		else if ((dprecision == 1) && (dscale == 0))
		{
			col_width = sizeof(char);
			dtype = DCI_INT;
		}
		else if ((dprecision == 3) && (dscale == 0))
		{
			col_width = sizeof(short);
			dtype = DCI_INT;
		}
		else if ((dprecision == 5) && (dscale == 0))
		{
			col_width = sizeof(short);
			dtype = DCI_INT;
		}
		else if ((dprecision == 10) && (dscale == 0))
		{
			col_width = 4;
			dtype = DCI_INT;
		}
		//<XM_20110630  BEGIN>
		else if ((dprecision == 19) && (dscale == 0))
		{
		    col_width = 8;
			dtype = DCI_INT;
		}
		else if((dprecision <= 19 && dprecision > 10) && (dscale == 0))
		{
		    col_width = 8;
			dtype = DCI_INT;
		}
		//<XM_20110630 END>
		else if (col_width == 8)
		{
			col_width = sizeof(sb8);
			dtype = DCI_INT;
		}
		else if ((dprecision == 20) && (dscale == 0))
		{
			col_width = sizeof(sb8);
			dtype = DCI_INT;
		}
		else if ((dprecision == 7) && (dscale == 0))
		{
			col_width = sizeof(float);//4
			dtype = DCI_FLT;
		}
		else if ((dprecision == 15) && (dscale == 0))
		{
			col_width = sizeof(double);//8
			dtype = DCI_FLT;
		}
		else if (((dscale == 0)) || (dtype == DCI_INT))
		{
			col_width = sizeof(long);
			dtype = DCI_INT;
		}
		else if ((dprecision == 22) && (dscale == 6))
		{
			col_width = sizeof(float);//4
			dtype = DCI_FLT;
		}
		else if ((dprecision == 38) && (dscale == 12))
		{
			col_width = sizeof(double);//8
			dtype = DCI_FLT;
		}
		else if (((dscale > 0)) || (dtype == DCI_FLT))
		{
			col_width = sizeof(float);
			dtype = DCI_FLT;
		}
	}
	else if(dtype == DCI_CHR || dtype == DCI_AFC)
	{
		dtype = DCI_STR;
	}
}

int CDci::ReadDataTime_t(IN const char *query,IN int top_number, OUT int *rec_num, OUT int *attr_num,OUT struct ColAttr ** attrs,OUT char **buf, OUT ub4 *buf_size, ErrorInfo_t* error)
{
	if (ReadDataEx(query, top_number, rec_num, attr_num, attrs, buf, buf_size, error))
	{
		ub4 reclen = 0;
		ColAttr *attr = *attrs;
		for (int col=0; col<*attr_num; col++)
		{
			reclen += attr[col].data_size + sizeof(ub2);
		}
		ConvertDatToTime_t(*attrs, *attr_num, *buf, *rec_num, reclen);
		return TRUE;
	}
	return FALSE;
}

void CDci::ConvertDatToTime_t(ColAttr *attr, ub2 cols, char *data, ub4 rows, ub4 reclen)
{

	ub2 col;
	ub4 row;
	ub4 offset = 0;
	char *temp = NULL;
	for (col=0; col<cols; col++)
	{
		if (attr[col].data_type == DCI_DAT)
		{
			for (row=0; row<rows; row++)
			{
				temp = data + (row*reclen) + offset;
				convert_dat_to_time(temp, (time_t *)temp);
			}
		}
		offset += attr[col].data_size + sizeof(sb2);
	}
}

/*******************************************************************************
**
** Routine: InsertFutureData(PutHIsData)
**
** Author: Xiemei
**
** DATE :2009/10/20
**
** Description: insert or update the future data to table of plan
**
*******************************************************************************/
bool CDci::PutHisData(char *model_name, vector<SheetRecData> &rec, ErrorInfo &error)
{
	Curveinfo	*info_sheet = NULL;
	int retcode;
	int count;
	SheetColData *indexp;
	SheetColData *valuep;
	int i,j;
	char index_str[1000];
	memset(&error, 0, sizeof(ErrorInfo_t));
	for (i=0; i<rec.size(); i++)
	{
		indexp = (SheetColData*)dcisg_malloc(rec[i].index.size() * sizeof(SheetColData));
		if (indexp == NULL)
		{
			strcpy(error.error_info, "内存不足");
			return FALSE;
		}
		valuep = (SheetColData*)dcisg_malloc(rec[i].values.size() * sizeof(SheetColData));
		if (valuep == NULL)
		{
			dcisg_free(indexp);
			strcpy(error.error_info, "内存不足");
			return FALSE;
		}

		for (j=0; j<rec[i].index.size(); j++)
		{
			indexp[j] = rec[i].index[j];
		}
		for (j=0; j<rec[i].values.size(); j++)
		{
			valuep[j] = rec[i].values[j];
		}

		if(!SetSheetData(model_name, rec[i].rec_time, indexp, rec[i].index.size(), valuep, rec[i].values.size(), error))
		{
			if(error.error_no == 942) //第一次插入不成功并且是缺表的情况。
			{
				info_sheet = FindCurveinfo(model_name);
				if(info_sheet !=NULL)
				{
					char *tablename;
					char *datatime;
					char sqlstr[1000];
					GetTableName(info_sheet->table_name_ft, &datatime, &tablename, count, localtime(&rec[i].rec_time),localtime(&rec[i].rec_time));
					dcisg_free(datatime);
					datatime = NULL;
					sprintf(sqlstr,"create table %s as select * from %s ",tablename,info_sheet->table_name_model);
					//初始化后有index_str:
					if (strlen(info_sheet->index_str)>0)
					{
						sprintf(index_str,"create unique index index_%s on %s(%s)",tablename, tablename,info_sheet->index_str);
					}
					else
					{
						sprintf(index_str,"");
					}
					dcisg_free(tablename);
					tablename = NULL;

					if(!ExecSingle(sqlstr,&error))
					{
						dcisg_free(indexp);
						dcisg_free(valuep);
						ExecRollback(NULL);
						return false;
					}
					if (strlen(index_str)>0)
					{
						if(!ExecSingle(index_str,&error))
						{
							dcisg_free(indexp);
							dcisg_free(valuep);
							return FALSE;
						}
					}
					//重新进行更新，发现更新了零行，就进行 vector的整理工作，发现已有，就丢弃掉。
					if(!SetSheetData(model_name, rec[i].rec_time, indexp, rec[i].index.size(), valuep, rec[i].values.size(), error))
					{
						dcisg_free(indexp);
						dcisg_free(valuep);
						ExecRollback(NULL);
						return false;
					}
				}
			}
			else
			{
				printf("error = %s\n",error.error_info);
				dcisg_free(indexp);
				dcisg_free(valuep);
				ExecRollback(NULL);
				return false;
			}
		}
		dcisg_free(indexp);
		dcisg_free(valuep);
		indexp = NULL;
		valuep = NULL;
	}
	ExecCommit(&error);
	return true;
}
#define EXEC_TRY_COUNT	5
sword CDci::StmtExecute(DCISvcCtx *svchp, DCIStmt *stmthp, DCIError *errhp, ub4 iters, ub4 commit_flag)
{
	int try_count = EXEC_TRY_COUNT;
	int retcode = 0;
	ErrorInfo error;
	while(try_count --)
	{
		memset(&error, 0, sizeof(ErrorInfo));
		DCI_CHECK_EX(&error, DCIStmtExecute(svchp, stmthp, errhp, iters, 0, (CONST DCISnapshot *)NULL, (DCISnapshot *)NULL, commit_flag));
		if (retcode == DCI_SUCCESS)
		{
			return DCI_SUCCESS;
		}
		else if (error.error_no == 54)//超时
		{
#ifndef WIN32
			sleep(1);
#else
			Sleep(1000);
#endif
			continue;
		}
		else
		{
			return retcode;
		}
	}
	return retcode;
}

char*** CDci::ParseResultsForReadData(int rec_num, int attr_num, ColAttr *attrs, char *buf)
{
	char *x;
	int i,j;
	int rowLen = 0;
	char *tmp = NULL;
	char ***dataFormat = NULL;

	if(rec_num <= 0 || attr_num <= 0)
	{
		printf("error\n");
		return NULL;
	}

	dataFormat = (char ***)malloc(sizeof(char ***) * rec_num);

	for(i = 0;i < rec_num; i++)
	{
		dataFormat[i] = (char **)malloc(sizeof(char **) * attr_num);
		memset(dataFormat[i], 0, sizeof(char **) * attr_num);
	}

	tmp = buf;

	for(i = 0 ; i < rec_num; i++)
		for(j = 0; j < attr_num; j++)
		{
			dataFormat[i][j] = tmp;
			tmp = tmp + attrs[j].data_size;
		}

	return dataFormat;
}
int CDci::ParseResultsForReadData(int rec_num, int attr_num, ColAttr *attrs, char *buf, char *** &data_tab)
{
	char *x;
	int i,j;
	int rowLen = 0;
	char *tmp = NULL;
	char ***dataFormat = NULL;

	if(rec_num <= 0 || attr_num <= 0)
	{
		printf("error\n");
		return FALSE;
	}

	dataFormat = (char ***)malloc(sizeof(char ***) * rec_num);

	for(i = 0;i < rec_num; i++)
	{
		dataFormat[i] = (char **)malloc(sizeof(char **) * attr_num);
		memset(dataFormat[i], 0, sizeof(char **) * attr_num);
	}

	tmp = buf;

	for(i = 0 ; i < rec_num; i++)
		for(j = 0; j < attr_num; j++)
		{
			dataFormat[i][j] = tmp;
			tmp = tmp + attrs[j].data_size;
		}
	data_tab = dataFormat;
	return TRUE;
}

void CDci::FreeSpaceForReadData(int row, char ***data)
{
	for(int i = 0;i < row; i++)
	{
		free(data[i]);
	}
	free(data);
	data = NULL;
}

/*/<XM_20110718> ===> BEGIN
bool CDci::CloseConnectSocket()
{
	if (is_connect)
	{
		DCICloseSvcSocket(svchp);
	}
}
//<XM_20110718> ===> END */

#if 0
//query_result_log danath logtest 20111010
void query_result_log(int rec_num,int col_num,ColAttr_t* attr,void* data)
{
    int rec,col;
    char* buf = (char*)malloc(256*col_num);
    memset(buf,0,256*col_num);
    char* temp = buf;
    for(col = 0; col < col_num; col++)
    {
        sprintf(temp,"%s\t",attr[col].col_name);
        temp += strlen(attr[col].col_name) + 1;
    }
    log_report("%s",buf);

    for (rec = 0; rec < rec_num; rec++)
    {
        temp = buf;
        memset(buf,0,256*col_num);
        for(col = 0; col < col_num; col++)
        {
            switch(attr[col].data_type)
            {
                case DCI_INT:   if(attr[col].data_size == 2)
                                {
                                        sprintf(temp," %d \t",*(short*)data);
                                        temp += sizeof(short)+3;
                                }
                                if(attr[col].data_size == 4)
                                {
                                        sprintf(temp," %d \t",*(int*)data);
                                        temp += sizeof(int)+3;
                                }
                                if(attr[col].data_size == 8)
                                {
                                        sprintf(temp," %ld \t",*(long*)data);
                                        temp += sizeof(long)+3;
                                }
                                break;
                case DCI_FLT:   if(attr[col].data_size == 4)
                                {
                                        sprintf(temp," %f \t",*(float*)data);
                                        temp += sizeof(float)+3;
                                }
                                if(attr[col].data_size == 8)
                                {
                                        sprintf(temp," %lf \t",*(double*)data);
                                        temp += sizeof(double)+3;
                                }
                                break;
                case DCI_STR: strcat(temp,(char*)data);
                                temp += strlen((char*)data) + 1;
                                break;
                case DCI_DAT:  {
                                struct tm tt;
                                tt.tm_year=(*(char*)data - 100)*100 + (unsigned char)*(char*)(data + 1) - 100 - 1900;
                                tt.tm_mon=*(char*)(data + 2) - 1;
                                tt.tm_mday=*(char*)(data + 3);
                                tt.tm_hour=*(char*)(data + 4) - 1;
                                tt.tm_min=*(char*)(data + 5) - 1;
                                tt.tm_sec=*(char*)(data + 6) - 1;
                                char time_c[30]={0};
                                strftime(time_c,30,"%Y-%m-%d %H:%M:%S",&tt);
                                strcat(temp,time_c);
                                temp += strlen(time_c);
                                break;
                                }
                        default:      sprintf(temp,"nullity\t");
                                temp += 8;
                                break;
            }
            data += attr[col].data_size +sizeof(sb2);
        }
        log_report("%s",buf);
    }
    free(buf);
}
#endif


