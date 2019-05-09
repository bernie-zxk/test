
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// [基本信息]
// 文件名      :
// 创建日期             :
// 设计者           :
// 编码者               :

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
// [描述] FTP客户端类。实现FTP功能；
// [备注]

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
// [修改记录]
//=================================================================
//  序号          修改人             修改原因
//------------------+--------------------------+-------------------------


///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////



#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <iostream>

#if defined(__unix__)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#elif defined(VMS)
#include <types.h>
#include <socket.h>
#include <in.h>
#include <netdb.h>
#include <inet.h>
#endif

#include "ftp_client.h"

using namespace std;

#define SETSOCKOPT_OPTVAL_TYPE (void *)

#define FTPLIB_BUFSIZ 8192
#define ACCEPT_TIMEOUT 30

#define FTPLIB_CONTROL 0
#define FTPLIB_READ 1
#define FTPLIB_WRITE 2

#define net_read read
#define net_write write
#define net_close close

#if defined(VMS)
/*
 * VAX C does not supply a memccpy routine so I provide my own
 */
void *memccpy(void *dest, const void *src, int c, size_t n)
{
    int i=0;
    const unsigned char *ip=src;
    unsigned char *op=dest;

    while (i < n)
    {
        if ((*op++ = *ip++) == c)
            break;
        i++;
    }
    if (i == n)
        return NULL;
    return op;
}
#endif

/*
 * Constructor
 */
AFC_FTP::AFC_FTP()
{
    ftplib_debug = 0;
    InitNetConnect(&net_control);
}
/*
 * Destructor
 */
AFC_FTP::~AFC_FTP()
{
    FtpQuit();
}
/*
 * read a line of text
 *
 * return -1 on error or bytecount
 */
int AFC_FTP::ReadLine(char *buf,int max,NetBuf *ctl)
{
    int x,retval = 0;
    char *end,*bp=buf;
    int eof = 0;

    if ((ctl->dir != FTPLIB_CONTROL) && (ctl->dir != FTPLIB_READ))
        return -1;
    if (max == 0)
        return 0;
    do
    {
        if (ctl->cavail > 0)
        {
            x = (max >= ctl->cavail) ? ctl->cavail : max-1;
            end = (char*)memccpy(bp,ctl->cget,'\n',x);
            if (end != NULL)
                x = end - bp;
            retval += x;
            bp += x;
            *bp = '\0';
            max -= x;
            ctl->cget += x;
            ctl->cavail -= x;
            if (end != NULL)
            {
                bp -= 2;
                if (strcmp(bp,"\r\n") == 0)
                {
                    *bp++ = '\n';
                    *bp++ = '\0';
                    --retval;
                }
                break;
            }
        }
        if (max == 1)
        {
            *buf = '\0';
            break;
        }
        if (ctl->cput == ctl->cget)
        {
            ctl->cput = ctl->cget = ctl->buf;
            ctl->cavail = 0;
            ctl->cleft = FTPLIB_BUFSIZ;
        }
        if (eof)
        {
            if (retval == 0)
                retval = -1;
            break;
        }
        if ((x = net_read(ctl->handle,ctl->cput,ctl->cleft)) == -1)
        {
            perror("read");
            retval = -1;
            break;
        }
        if (x == 0)
            eof = 1;
        ctl->cleft -= x;
        ctl->cavail += x;
        ctl->cput += x;
    }
    while (1);
    return retval;
}

/*
 * write lines of text
 *
 * return -1 on error or bytecount
 */
int AFC_FTP::WriteLine(char *buf, int len, NetBuf *nData)
{
    int x, nb=0, w;
    char *ubp = buf, *nbp;
    char lc=0;

    if (nData->dir != FTPLIB_WRITE)
        return -1;
    nbp = nData->buf;
    for (x=0; x < len; x++)
    {
        if ((*ubp == '\n') && (lc != '\r'))
        {
            if (nb == FTPLIB_BUFSIZ)
            {
                w = net_write(nData->handle, nbp, FTPLIB_BUFSIZ);
                if (w != FTPLIB_BUFSIZ)
                {
                    printf("net_write(1) returned %d, errno = %d\n", w, errno);
                    return(-1);
                }
                nb = 0;
            }
            nbp[nb++] = '\r';
        }
        if (nb == FTPLIB_BUFSIZ)
        {
            w = net_write(nData->handle, nbp, FTPLIB_BUFSIZ);
            if (w != FTPLIB_BUFSIZ)
            {
                printf("net_write(2) returned %d, errno = %d\n", w, errno);
                return(-1);
            }
            nb = 0;
        }
        nbp[nb++] = lc = *ubp++;
    }
    if (nb)
    {
        w = net_write(nData->handle, nbp, nb);
        if (w != nb)
        {
            printf("net_write(3) returned %d, errno = %d\n", w, errno);
            return(-1);
        }
    }
    return len;
}

/*
 * read a response from the server
 *
 * return 0 if first char doesn't match
 * return 1 if first char matches
 */
int AFC_FTP::ReadResp(char c, NetBuf *nControl)
{
    char match[5];
    if (ReadLine(nControl->response,256,nControl) == -1)
    {
        perror("Control socket read failed");
        return 0;
    }
    if (ftplib_debug > 1)
        fprintf(stderr,"%s",nControl->response);
    if (nControl->response[3] == '-')
    {
        strncpy(match,nControl->response,3);
        match[3] = ' ';
        match[4] = '\0';
        do
        {
            if (ReadLine(nControl->response,256,nControl) == -1)
            {
                perror("Control socket read failed");
                return 0;
            }
            if (ftplib_debug > 1)
                fprintf(stderr,"%s",nControl->response);
        }
        while (strncmp(nControl->response,match,4));
    }
    if (nControl->response[0] == c)
        return 1;
    return 0;
}
/*
 * InitNetConnect - init net_control
 *
 *
 */

void AFC_FTP::InitNetConnect(NetBuf* p)
{
    p->cput = NULL;
    p->cget = NULL;
    p->handle = 0;
    p->cavail = 0;
    p->cleft = 0;
    p->dir = 0;
    p->buf[0] = '\0';
    p->response[0] = '\0';
    return;
}
/*
 * FtpConnect - connect to remote server
 *
 * return 1 if connected, 0 if not
 */

int AFC_FTP::FtpConnect(const char *host,const char *user, const char *
                        pass)
{
    if(host == NULL || user == NULL || pass == NULL)
    {
        return 0;
    }
    if(!FtpConnectHost(host))
    {
        return FTP_HOST_NAME_ERROR;
    }
    if(!FtpLoginHost(user, pass))
    {
        return FTP_USER_PWS_ERROR;
    }
    return FTP_CONNECT_SUCCESS;
}
/*
 * FtpConnect - connect to remote server using host name or IP address
 *
 * return 1 if connected, 0 if not
 */
int AFC_FTP::FtpConnectHost(const char *host)
{
    int sControl = 0;
    struct sockaddr_in sin;
    struct hostent *phe = NULL;
    struct servent *pse = NULL;
    int on=1;
    //   NetBuf *ctrl = NULL;
    char *lhost = NULL;
    char *pnum = NULL;

    InitNetConnect(&net_control);

    memset(&sin,0,sizeof(sin));
    sin.sin_family = AF_INET;
    lhost = strdup(host);
    pnum = strchr(lhost,':');
    if (pnum == NULL)
    {
        /*
        #if defined(VMS)
                sin.sin_port = htons(21);
        #else
        */
        if ((pse = getservbyname("ftp","tcp")) == NULL)
        {
            perror("getservbyname");
            return 0;
        }
        sin.sin_port = pse->s_port;
        //#endif
    }
    else
    {
        *pnum++ = '\0';
        if (isdigit(*pnum))
            sin.sin_port = htons(atoi(pnum));
        else
        {
            pse = getservbyname(pnum,"tcp");
            sin.sin_port = pse->s_port;
        }
    }
    if ((sin.sin_addr.s_addr = inet_addr(lhost)) == -1)
    {
        if ((phe = gethostbyname(lhost)) == NULL)
        {
            perror("gethostbyname");
            return 0;
        }
        memcpy((char *)&sin.sin_addr, phe->h_addr, phe->h_length);
    }
    free(lhost);
    sControl = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sControl == -1)
    {
        perror("socket");
        return 0;
    }
    if (setsockopt(sControl,SOL_SOCKET,SO_REUSEADDR,
                   SETSOCKOPT_OPTVAL_TYPE &on, sizeof(on)) == -1)
    {
        perror("setsockopt");
        net_close(sControl);
        return 0;
    }
    if (connect(sControl, (struct sockaddr *)&sin, sizeof(sin)) == -1)
    {
        perror("connect");
        net_close(sControl);
        return 0;
    }
    /*   ctrl = (netbuf*)calloc(1,sizeof(netbuf));
       if (ctrl == NULL)
       {
       perror("calloc");
       net_close(sControl);
       return 0;
       }
       ctrl->buf = (char*)malloc(FTPLIB_BUFSIZ);
       if (ctrl->buf == NULL)
       {
       perror("calloc");
       net_close(sControl);
       free(ctrl);
       return 0;
       }*/
    net_control.handle = sControl;
    net_control.dir = FTPLIB_CONTROL;

    net_control.cput = NULL;
    net_control.cget = NULL;
    net_control.cavail = 0;
    net_control.cleft = 0;
    memset(net_control.buf,'\0',FTPLIB_BUFSIZ);
    memset(net_control.response,'\0',FTPLIB_RESPONSE);

    if (ReadResp('2', &net_control) == 0)
    {
        net_close(sControl);
        return 0;
    }
    return 1;
}

/*
 * FtpSendCmd - send a command and wait for expected response
 *
 * return 1 if proper response received, 0 otherwise
 */
int AFC_FTP::FtpSendCmd(const char *cmd, char expresp, NetBuf *nControl)
{
    char buf[256];
    if (nControl->dir != FTPLIB_CONTROL)
        return 0;
    if (ftplib_debug > 2)
        fprintf(stderr,"%s\n",cmd);
    sprintf(buf,"%s\r\n",cmd);
    if (net_write(nControl->handle,buf,strlen(buf)) <= 0)
    {
        perror("write");
        return 0;
    }
    return ReadResp(expresp, nControl);
}

/*
 * FtpLogin - log in to remote server
 *
 * return 1 if logged in, 0 otherwise
 */
int AFC_FTP::FtpLoginHost(const char *user, const char *pass)
{
    char tempbuf[64];

    sprintf(tempbuf,"USER %s",user);
    if (!FtpSendCmd(tempbuf,'3',&net_control))
    {
        if (net_control.response[0] == '2')
            return 1;
        return 0;
    }
    sprintf(tempbuf,"PASS %s",pass);
    return FtpSendCmd(tempbuf,'2',&net_control);
}

/*
 * FtpOpenPort - set up data connection
 *
 * return 1 if successful, 0 otherwise
 */
int AFC_FTP::FtpOpenPort(NetBuf *nControl, NetBuf *nData, int mode, int
                         dir)
{
    int sData;
    union
    {
        struct sockaddr sa;
        struct sockaddr_in in;
    } sin;
    struct linger lng = { 0, 0 };
    int l;
    int on=1;
    char *cp;
    unsigned int v[6];
    NetBuf *ctrl;

    if (nControl->dir != FTPLIB_CONTROL)
        return -1;
    if ((dir != FTPLIB_READ) && (dir != FTPLIB_WRITE))
    {
        sprintf(nControl->response, "Invalid direction %d\n", dir);
        return -1;
    }
    if ((mode != FTPLIB_ASCII) && (mode != FTPLIB_IMAGE))
    {
        sprintf(nControl->response, "Invalid mode %c\n", mode);
        return -1;
    }
    l = sizeof(sin);
    memset(&sin, 0, l);
    sin.in.sin_family = AF_INET;
    if (!FtpSendCmd("PASV",'2',nControl))
        return -1;
    cp = strchr(nControl->response,'(');
    if (cp == NULL)
        return -1;
    cp++;
    sscanf(cp,"%u,%u,%u,%u,%u,%u",&v[2],&v[3],&v[4],&v[5],&v[0],&v[1]);
    sin.sa.sa_data[2] = v[2];
    sin.sa.sa_data[3] = v[3];
    sin.sa.sa_data[4] = v[4];
    sin.sa.sa_data[5] = v[5];
    sin.sa.sa_data[0] = v[0];
    sin.sa.sa_data[1] = v[1];
    sData = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (sData == -1)
    {
        perror("socket");
        return -1;
    }
    if (setsockopt(sData,SOL_SOCKET,SO_REUSEADDR,
                   SETSOCKOPT_OPTVAL_TYPE &on,sizeof(on)) == -1)
    {
        perror("setsockopt");
        net_close(sData);
        return -1;
    }
    if (setsockopt(sData,SOL_SOCKET,SO_LINGER,
                   SETSOCKOPT_OPTVAL_TYPE &lng,sizeof(lng)) == -1)
    {
        perror("setsockopt");
        net_close(sData);
        return -1;
    }
    if (connect(sData, &sin.sa, sizeof(sin.sa)) == -1)
    {
        perror("connect");
        net_close(sData);
        return -1;
    }
    /*
        ctrl = (netbuf*)calloc(1,sizeof(netbuf));
        if (ctrl == NULL)
        {
        perror("calloc");
        net_close(sData);
        return -1;
        }

        if ((mode == 'A') && ((ctrl->buf = (char*)malloc(FTPLIB_BUFSIZ)) ==
    NULL))
        {
        perror("calloc");
        net_close(sData);
        free(ctrl);
        return -1;
        }
    */
    nData->handle = sData;
    nData->dir = dir;

    nData->cput = NULL;
    nData->cget = NULL;
    nData->cavail = 0;
    nData->cleft = 0;
    memset(nData->buf,'\0',FTPLIB_BUFSIZ);
    memset(nData->response,'\0',FTPLIB_RESPONSE);


    //  *nData = ctrl;
    return 1;
}

/*
 * FtpAccess - return a handle for a data stream
 *
 * return 1 if successful, 0 otherwise
 */
int AFC_FTP::FtpAccess(const char *path, int typ, int mode, NetBuf *
                       nControl,
                       NetBuf *nData)
{

    char buf[256];
    int dir;
    if ((path == NULL) && ((typ == FTPLIB_FILE_WRITE) || (typ ==
                           FTPLIB_FILE_READ)))
    {
        sprintf(nControl->response, "Missing path argument for file transfer\n");
        return 0;
    }
    sprintf(buf, "TYPE %c", mode);
    if (!FtpSendCmd(buf, '2', nControl))
        return 0;
    switch (typ)
    {
        case FTPLIB_DIR:
            strcpy(buf,"NLST");
            dir = FTPLIB_READ;
            break;
        case FTPLIB_DIR_VERBOSE:
            strcpy(buf,"LIST");
            dir = FTPLIB_READ;
            break;
        case FTPLIB_FILE_READ:
            strcpy(buf,"RETR");
            dir = FTPLIB_READ;
            break;
        case FTPLIB_FILE_WRITE:
            strcpy(buf,"STOR");
            dir = FTPLIB_WRITE;
            break;
        default:
            sprintf(nControl->response, "Invalid open type %d\n", typ);
            return 0;
    }
    if (path != NULL)
        sprintf(buf+strlen(buf)," %s",path);
    if (FtpOpenPort(nControl, nData, mode, dir) == -1)
        return 0;
    if (!FtpSendCmd(buf, '1', nControl))
    {
        FtpClose(nData);

        return 0;
    }
    return 1;
}

/*
 * FtpRead - read from a data connection
 */
int AFC_FTP::FtpRead(void *buf, int max, NetBuf *nData)
{
    if (nData->dir != FTPLIB_READ)
        return 0;
    if (nData->buf[0] != '\0')
        return ReadLine((char*)buf, max, nData);
    return net_read(nData->handle, (char*)buf, max);
}

/*
 * FtpWrite - write to a data connection
 */
int AFC_FTP::FtpWrite(void *buf, int len, NetBuf *nData)
{
    if (nData->dir != FTPLIB_WRITE)
        return 0;
    if (nData->buf[0] != '\0')
        return WriteLine((char*)buf, len, nData);
    return net_write(nData->handle, (char*)buf, len);
}

/*
 * FtpClose - close a data connection
 */
int AFC_FTP::FtpClose(NetBuf *nData)
{
    if (nData->dir == FTPLIB_WRITE)
    {
        if (nData->buf[0] != '\0')
            WriteLine(NULL, 0, nData);
    }
    else if (nData->dir != FTPLIB_READ)
        return 0;
    shutdown(nData->handle,2);
    net_close(nData->handle);
    return 1;
}

/*
 * FtpXfer - issue a command and transfer data
 *
 * return 1 if successful, 0 otherwise
 */
int AFC_FTP::FtpXfer(const char *localfile, const char *path,
                     NetBuf *nControl, int typ, int mode)
{
    //cout << "ftpxfer!!"<< localfile << endl;
    int l,c;
    //    char *dbuf;
    FILE *local = NULL;
    NetBuf net_data;

    InitNetConnect(&net_data);
    //  net_buf[0] = '\0';
    memset(net_buf,'\0',FTPLIB_BUFSIZ);

    if (localfile != NULL)
    {
        local = fopen(localfile, (typ == FTPLIB_FILE_WRITE) ? "r" : "w");
        if (local == NULL)
        {
            cout << "open failed!! localfile::"<< localfile << endl;
            strcpy(nControl->response, strerror(errno));
            return 0;
        }
    }
    if (local == NULL)
    {
        local = (typ == FTPLIB_FILE_WRITE) ? stdin : stdout;
    }
    if (!FtpAccess(path, typ, mode, nControl, &net_data))
        return 0;
    //    dbuf = (char*)malloc(FTPLIB_BUFSIZ);
    if (typ == FTPLIB_FILE_WRITE)
    {
        while ((l = fread(net_buf, 1, FTPLIB_BUFSIZ, local)) > 0)
            if ((c = FtpWrite(net_buf, l, &net_data)) < l)
                printf("short write: passed %d, wrote %d\n", l, c);
    }
    else
    {
        while ((l = FtpRead(net_buf, FTPLIB_BUFSIZ, &net_data)) > 0)
            if (fwrite(net_buf, 1, l, local) <= 0)
            {
                perror("localfile write");
                break;
            }
    }
    fflush(local);
    if (localfile != NULL)
        fclose(local);
    FtpClose(&net_data);

    return ReadResp('2', nControl);
}


/*
 * FtpGet - issue a GET command and write received data to output
 *
 * return 1 if successful, 0 otherwise
 */
int AFC_FTP::FtpGet(const char *outputfile, const char *path,char mode)
{
    //cout << "FtpGet!!" << endl;
    if(outputfile == NULL || path == NULL)
    {
        return 0;
    }
    return FtpXfer(outputfile, path, &net_control, FTPLIB_FILE_READ, mode)
           ;
}

/*
 * FtpPut - issue a PUT command and send data from input
 *
 * return 1 if successful, 0 otherwise
 */
int AFC_FTP::FtpPut(const char *inputfile, const char *path, char mode)
{
   // cout << "ftpput!!" << endl;
    //cout << inputfile << endl;
    if(inputfile == NULL || path == NULL)
    {
        return 0;
    }
    return FtpXfer(inputfile, path, &net_control, FTPLIB_FILE_WRITE, mode)
           ;
}

/*
 * FtpQuit - disconnect from remote
 *
 * return 1 if successful, 0 otherwise
 */
void AFC_FTP::FtpQuit()
{
    if (net_control.dir != FTPLIB_CONTROL)
    {
        return;
    }
    FtpSendCmd("QUIT",'2',&net_control);
    net_close(net_control.handle);
    return;
}

