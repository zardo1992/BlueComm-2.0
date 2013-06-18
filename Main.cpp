/*******************************************************************************************
Titolo:		 BlueComm 
Versione:	 1.0
Autore:		 Matteo Zardoni
Descrizione: Programma che simula una comunicazione Server/Client fra due computer, collega-
			 ti tramite tecnologia wireless Bluetooth
Ultima Rev.: 06 giugno 2011		 
*******************************************************************************************/

/*******************************************************************************************	 
=> Da inserire nella cartella
C:\Programmi\DevStudio\VC\include
	=>  bthdef.h
	=>  ws2bth.h
=> Da inserire nella cartella del progetto
	=> 	bthsdpdef.h
*******************************************************************************************/
#include<winsock2.h>
#include<Ws2bth.h>
#include<stdio.h>
#include<conio.h>
#include<string.h>
//*****************************************************************************************/	
//=> Da inserire nelle librerie del linker	 
  #pragma comment(lib, "wsock32")
  #pragma comment(lib, "ws2_32")
//*****************************************************************************************/

//Prototipi funzioni
int server(char *messaggio);
int client (char *messaggio);
int cerca_dispositivo (BTH_ADDR *indirizzo_dispositivo);
int intestazione();

//Variabili globali
char stringa_server[500]={"<Da: SERVER> Richiesta accettata; connessione effettuata!\n"};
char stringa_client[500]={"<Da: CLIENT> Ho ricevuto la risposta correttamente! \n"};

void main ()
{
	int scelta;
	scelta=intestazione();
	switch (scelta)
	{
		case 1:
			printf ("\n\n*** Lato Server *** \n\n");
			server(stringa_server);
		break;

		case 2:
			printf ("\n\n*** Lato Client *** \n\n");
			client (stringa_client);
		break;
		
		default:
			printf ("Scelta errata!\n");
		break;
	}
	printf ("\nPremere un tasto per continuare...\n");
	getch();
	WSACleanup();
}


int server(char *messaggio)
{
	SOCKADDR_BTH SockAddr1;
	SOCKET SockBthServer;
	SOCKADDR_BTH SockAddr2;
	SOCKET SockDest;
	char stringa_ricevuta[500];
	int esito=-1;

	int ilen = sizeof(SockAddr2);
	int len = sizeof(SockAddr1);

	ZeroMemory (&SockAddr1, sizeof(SockAddr1));
	SockAddr1.addressFamily = AF_BTH;
	SockAddr1.port = 1;
	
	WORD wVersionRequested = MAKEWORD(2,2);
	WSADATA m_wsaData;

	//Inizializzazione WSA
	if (WSAStartup(wVersionRequested, &m_wsaData) != 0)
	{
		printf ("Errore StartUp!\n");
		return -1;
	}

	//Creazione del SOCKET
	SockBthServer=socket(AF_BTH,SOCK_STREAM,BTHPROTO_RFCOMM);
	if (INVALID_SOCKET == SockBthServer)
	{
		printf ("Errore creazione Socket!!");
		return -2;
	}
	//Operazione Bind
	if (0 != bind (SockBthServer, (SOCKADDR *) &SockAddr1, sizeof(SockAddr1))) 
	{
		printf ("Errore Bind!\n");
		closesocket (SockBthServer);
		return -3;
	}

	//Operazione trova indirizzo e porta socket locale
	if (0 != getsockname(SockBthServer, (SOCKADDR *) &SockAddr1, &len))
	{
		printf ("Errore GetSockName!\n");
		closesocket (SockBthServer);
		return -4;
	}

	//Operazione Listen
	if (0 != listen (SockBthServer, 1))
	{
		printf ("Errore listen!\n");
		closesocket (SockBthServer);
		return -5;
	}

	printf ("In attesa di connessione al Client ... ...\n\n");

   //Connessione con il Client/Destinatario
	SockDest = accept (SockBthServer, (SOCKADDR *)&SockAddr2, &ilen);
	if (SockDest == INVALID_SOCKET) 
	{
		printf ("Errore creazione Socket Destinatario!\n");
		return -6;
	}

	//Invio notifica di connessione
	send (SockDest,messaggio,500,0);

	//Ricezione conferma di connessione
	recv(SockDest,stringa_ricevuta,sizeof(stringa_ricevuta),0);
	
	if (0==strcmp(stringa_ricevuta,"<Da: CLIENT> Ho ricevuto la risposta correttamente! \n"))
	{
		printf ("%s",stringa_ricevuta);
		printf ("Connessione al Client avvenuta con successo!\n");
	}

	closesocket(SockBthServer);
	closesocket(SockDest);

	return 0;
}

int client (char *messaggio)																					  
{ 
	SOCKET SockBthClient;
	SOCKADDR_BTH SockAddr3;
	char stringa_ricevuta[500];
	BTH_ADDR server_add;
	int esito=-1;

	int len=sizeof(SockAddr3);

	ZeroMemory(&SockAddr3, sizeof(SockAddr3));
	SockAddr3.addressFamily = AF_BTH;

	/*Necessità di porta e indirizzo del server*/
	SockAddr3.port = 1;

	WORD wVersionRequested = MAKEWORD(2,2);
	WSADATA m_wsaData;

	//Inizializzazione WSA
	if (WSAStartup(wVersionRequested, &m_wsaData) != 0)
	{
		printf ("Errore StartUp!\n");
		return -1;
	}

	printf ("In attesa di connessione al Server ... ... \n\n");
	//Creazione del SOCKET
	SockBthClient=socket(AF_BTH,SOCK_STREAM,BTHPROTO_RFCOMM);
	if (INVALID_SOCKET == SockBthClient)
	{
		printf ("Errore creazione Socket!!");
		return -2;
	}

	//Cerca l'indirizzo del Server/Destinatario
	cerca_dispositivo (&server_add);
	SockAddr3.btAddr=server_add;

	//Operazione Connect
	if (0 != connect (SockBthClient, (SOCKADDR *) &SockAddr3, sizeof(SockAddr3))) 
	{
		printf ("Errore connect!\n");
		closesocket (SockBthClient);
		return -3;
	}

	//Ricezione del messaggio
	recv(SockBthClient,stringa_ricevuta,sizeof(stringa_ricevuta),0);
	if (0==strcmp(stringa_ricevuta,"<Da: SERVER> Richiesta accettata; connessione effettuata!\n"))
	{
		printf ("%s",stringa_ricevuta);
		printf ("Connessione al Server avvenuta con successo!\n");
		send (SockBthClient,messaggio,500,0);
	}
	
	closesocket(SockBthClient);

	return 0;
}

int cerca_dispositivo (BTH_ADDR *indirizzo_dispositivo)
{
	CHAR dispositivi[5000];
	LPWSAQUERYSET pwsaResults = (LPWSAQUERYSET) dispositivi;
	WSAQUERYSET parametri_ricerca;
	HANDLE hRicerca;
	DWORD dwSize  = sizeof(dispositivi);

	ZeroMemory(&parametri_ricerca, sizeof(parametri_ricerca));
	parametri_ricerca.dwSize = sizeof(parametri_ricerca);
	parametri_ricerca.dwNameSpace = NS_BTH;
	parametri_ricerca.lpcsaBuffer = NULL;

	if (0 != WSALookupServiceBegin (&parametri_ricerca, LUP_CONTAINERS| LUP_FLUSHCACHE , &hRicerca))
	{
		printf ("Errore LookupBegin!!\n");
		int err=WSAGetLastError();
		return -1;
	}  

	ZeroMemory(pwsaResults, sizeof(WSAQUERYSET));
	pwsaResults->dwSize = sizeof(WSAQUERYSET);
	pwsaResults->dwNameSpace = NS_BTH;
	pwsaResults->lpBlob = NULL;

	if (0 != WSALookupServiceNext (hRicerca, LUP_RETURN_NAME | LUP_RETURN_ADDR, &dwSize, pwsaResults))
	{
		printf ("Errore LookupNext!\n");
		return -2;
	}

	ZeroMemory(indirizzo_dispositivo, sizeof(*indirizzo_dispositivo));
	*indirizzo_dispositivo = ((SOCKADDR_BTH *)pwsaResults->lpcsaBuffer->RemoteAddr.lpSockaddr)->btAddr;

	WSALookupServiceEnd(hRicerca);

	return 0;
}
int intestazione()
{
	int dat;
	for(int i1=0; i1<80;i1++)
	{
		printf("*");
	}
	printf ("\n");	
	printf ("                                    **\n");
	printf ("                                    * *\n");
	printf ("                                    *  *\n");
	printf ("                                 *  *   *\n");
	printf ("                                  * *  *\n");
	printf ("                                   ** *\n");
	printf ("                                    ** BlueComm v.1.0\n");
	printf ("                                   ** *\n");
	printf ("                                  * *  *\n");
	printf ("                                 *  *   *\n");
	printf ("                                    *  *\n");
	printf ("                                    * *\n");
	printf ("                                    **\n");
	printf ("\n");
	printf ("Tesi per la maturita' - Autore: Matteo Zardoni - Classe: 5^A Inf - AS 2010/2011\n");
	for(int i2=0; i2<80;i2++)
	{
		printf("*");
	}
	printf ("\n");
	printf ("\n");
	printf ("Scegliere:\n");
	printf ("1) Applicazione lato server\n");
	printf ("2) Applicazione lato client\n");
	scanf ("%d",&dat);
	return dat;

}