#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#define		MSG_ID		(0xb007)

// Init IPC messaging
int InitMsg( )
{
	int err;
	int msgid = MSG_ID;
	
	err = msgget( msgid , IPC_CREAT|IPC_EXCL);
	return err;
}


struct msgdata {
	long mtype;    /* message type */
	char mtext[128];
};

// send a message to listening GUI client...
int SendMesg( char *msg )
{
static	long	sendcount=0;
	int len,err;
	struct msgdata ipcdata;
	
	if ( sendcount == 0 )  InitMsg();
	len = strlen( msg );
	strcpy( ipcdata.mtext, msg );
	err = msgsnd( MSG_ID, (void*)&ipcdata, len, 0);
	sendcount++;
	return err;
}


/*
     int msgrcv(int msqid, void *msgp,
          size_t msgsz, long msgtyp, int msgflg);

*/

int GetMesg( char *txt )
{
	int len,err;
	struct msgdata ipcdata;

	len = 128;
	err = msgrcv( MSG_ID, (void*)&ipcdata, len, 0, 0);
	strcpy( txt, ipcdata.mtext );
	return err;
}



int main ( char argc, char **argv )
{
	char	txt[128];  int err;

	while( err = GetMesg( txt ) ){
		printf( "Msg: er=%d, msg=%s\n", err,txt );
	}

}
  
















