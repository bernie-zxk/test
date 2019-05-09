#ifndef   __COMMON_H__
#define   __COMMON_H__

const short C_DATATYPE_STRING		= 1;
const short C_DATATYPE_UCHAR		= 2;
const short C_DATATYPE_SHORT		= 3;
const short C_DATATYPE_INT		= 4;
const short C_DATATYPE_DATETIME = 5;
const short C_DATATYPE_FLOAT		= 6;
const short C_DATATYPE_DOUBLE	= 7;
const short C_DATATYPE_KEYID		= 8;
const short C_DATATYPE_BINARY	= 9;
const short C_DATATYPE_TEXT 	= 10;
const short C_DATATYPE_IMAGE		= 11;

const short 	C_DATATYPE_APPKEYID 	= 12;
const short 	C_DATATYPE_APPID		= 13;
const short   C_DATATYPE_UINT		= 14;
const short   C_DATATYPE_LONG		= 15;

typedef long TKeyID;
struct TAppKeyID
{
	int	app_id;
	TKeyID	key_id;

};
typedef TAppKeyID TAppID;



// 定义统一数据库数据类型
#define UNI_DATATYPE_CHAR               1
#define UNI_DATATYPE_STRING             2
#define UNI_DATATYPE_UCHAR              3
#define UNI_DATATYPE_SHORT              4
#define UNI_DATATYPE_INT                5
#define UNI_DATATYPE_FLOAT              6
#define UNI_DATATYPE_DOUBLE             7
#define UNI_DATATYPE_DATETIME           8
#define UNI_DATATYPE_KEYID              9
#define UNI_DATATYPE_BINARY             10
#define UNI_DATATYPE_TEXT               11
#define UNI_DATATYPE_IMAGE              12
#define UNI_DATATYPE_APPKEYID           13
#define UNI_DATATYPE_APPID              14
#define UNI_DATATYPE_LONG             	15


#endif

