/*******************************************************
Protocolos de Transporte
Grado en Ingeniería Telemática
Dpto. Ingeníería de Telecomunicación
Univerisdad de Jaén

Fichero: cliente.c
Versión: 1.0
Fecha: 23/09/2012
Descripción:
	Cliente de eco sencillo TCP.

Autor: Juan Carlos Cuevas Martínez

*******************************************************/
#include <stdio.h>
#include <winsock.h>
#include <time.h>
#include <conio.h>

#include "protocol.h"





int main(int *argc, char *argv[])
{
	SOCKET sockfd;
	struct sockaddr_in server_in;
	char buffer_in[1024], buffer_out[1024],input[1024];
	int recibidos=0,enviados=0;
	int estado=S_HELO;
	char option;
	int com = 0; 

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

    char ipdest[16];
	char default_ip[16]="127.0.0.1";
	
	//Inicialización Windows sockets
	wVersionRequested=MAKEWORD(1,1);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0)
		return(0);

	if(LOBYTE(wsaData.wVersion)!=1||HIBYTE(wsaData.wVersion)!=1)
	{
		WSACleanup();
		return(0);
	}
	//Fin: Inicialización Windows sockets

	/*Creación del Socket TCP del servidor.*/
	do{
		sockfd=socket(AF_INET,SOCK_STREAM,0); 
	/***************************************************************************************************************************
	Aquí se crea el socket al utilizar la función socket, la cual es la primitiva que nos permite crear un descriptor de socket.
	La función socket se iguala a la variable sockfd, la cual recibirá el valor de retorno de la función socket, si no ocurre 
	ningún error, la función devuelve un descriptor que referencia un nuevo socket (valor int). En otro caso devuelve el valor -1
	(macro INVALID_SOCKET), el código de error específico se puede obtener llamando a WSAGetLastError (o GetLastError). 

	En caso de no tener un fallo, este valor entero no negativo, será el que llamaremos un descriptor socket, y es pasado a otras
	funciones API para identificar la abstración socket sobre la cual se va a llevar a cabo la operación.

	int socket(int af, int type, int protocol); 
	af: Familia de direcciones, en este caso AF_INET -> IPv4.
	type: Especificación del tipo del nuevo socket, en este caso SOCK_STREAM -> usar conexiones TCP. 
	protocol: Indica el procolo dentro de la familia elegida -> Por defecto usamos 0, protocolo por defecto de la familia.
	****************************************************************************************************************************/
		if(sockfd==INVALID_SOCKET) //En caso de que la función socket = sockfd me devuelva un error (INVALID_SOCKET), imprimo error al 
		{                          //usuario a la hora de crear el Socket. 
			printf("CLIENTE> ERROR AL CREAR SOCKET\r\n");
			exit(-1);
		}
		else //En caso de que no haya error, se imprime al cliente que se ha creado el SOCKET correctamente. 
		{
			printf("CLIENTE> SOCKET CREADO CORRECTAMENTE\r\n");

		    //Pasamos a pedirle al dirección IP de destino al usuario. 
			printf("CLIENTE> Introduzca la IP destino (pulsar enter para IP por defecto): ");
			gets(ipdest); //gets me toma lo que imprimo en la ventana CMD. 
			/*strcmp, me compara los dos string que se le introducen como argumentos, y en caso de ser iguales, diría que ipdest es 0 
			y por tanto el usuario quiere usar la IP por defecto.*/
			if(strcmp(ipdest,"")==0)
				strcpy(ipdest,default_ip); //En caso de cumplirse el if, me copia en ipdest la dirección default_ip. 

			//Estructuras de direcciones de IPv4.
			server_in.sin_family=AF_INET;                //Campo de la estructura sockaddr_in para el Protocolo de Internet. 
			server_in.sin_port=htons(TCP_SERVICE_PORT);  //Campo de la estructura sockaddr_in para introducir en el que escucha el servidor, el puerto que está ya definido como #define TCP_SERVICE_PORT 6000.
			server_in.sin_addr.s_addr=inet_addr(ipdest); //Campo de la estructura sockaddr_in para introducir la dirección de Internet (32 bits).
			//inet_addr: function converts a string containing an IPv4 dotted-decimal address into a proper address.
			//htons: function makes sure that numbers are stored in memory in network byte order
			estado=S_HELO;//Inicializo el estado en S_HELO, estado 0 definido en la cabecera protocol.h. 
		
			/***************************************************************************************************************************
			Un socket TCP debe ser conectado a otro socket antes de que cualquier dato pueda ser enviado a través de él. El cliente 
			debe iniciar la conexión mientras el servidor espera pasívamente por clientes que se conecten a él. Para establecer la 
			conexión con el servidor, utilizamos la función connect(). 

            int connect(int socket, struct sockaddr *foreignAddress, unsigned int addressLength)

			socket -> Es el descriptor creado para un determinado socket.

			foreignAddress -> Declarado como un puntero hacia un sockaddr será siempre un puntero a un sockaddr_in conteniendo la 
                              dirección de internet y el puerto del servidor. 

			addressLength -> especifica la longitud de la dirección de la estructura
           
		    Cuando connect() vuelve exitósamente, el socket es conectado y la comunicación puede proceder con llamadas a send() y recv().
			****************************************************************************************************************************/
			if(connect(sockfd,(struct sockaddr*)&server_in,sizeof(server_in))==0)
			{
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);
			
		
				//Inicio de la máquina de estados
				do{
					switch(estado)
					{
					case S_HELO:
						// Se recibe el mensaje de bienvenida
						break;
					case S_USER:
						// establece la conexion de aplicacion 
						printf("CLIENTE> Introduzca el usuario (enter para salir): ");
						gets(input);//Tomo lo que introduzca el usuario. 
						if(strlen(input)==0)//La misión de la función strlen es contar el nº de carácteres de una cadena.
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);//Imprimir de forma segura. 
							estado=S_QUIT; //Si pulso enter, se acaba la conexión al pasarme al estado S_QUIT. 
						}
						else

						sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",SC,input,CRLF);
						break;
						//Autentificación del usuario. 
					case S_PASS:
						printf("CLIENTE> Introduzca la clave (enter para salir): ");
						gets(input);//Tomo la cadena que introduzca el usuario. 
						if(strlen(input)==0)//En caso de ser enter, salgo de la conexión por estado S_QUIT.
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
						}
						else//De otra forma se imprime la contraseña y se transmite. 
							sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",PW,input,CRLF);
						break;
						//Una vez establecida la conexión, y autentificado el usuario se pasa a transmitir los datos.
					case S_DATA:
						printf("CLIENTE> Introduzca dos numeros enteros de maximo 4 digitos precedidos de 'sum' (enter o QUIT para salir): ");
						gets(input);
						if(strlen(input)==0)
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;//En caso de introducir enter, cierro la conexión al estar en el estado S_QUIT:
						}
						else//En otro caso se imprime lo que el usuario escriba por teclado. 
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",input,CRLF);
						break;
				 
				
					}
            /***************************************************************************************************************************
		    Envio de mensajes TCP.
			int send(int socket, const void *msg, unsigned int msg_len, int flags);

			socket -> Es el descriptor creado para un determinado socket.

			msg    -> Apunta al mensaje para enviar. 

			msgLength -> Es la longitud (bytes) del mensaje.
			
			send me devuelve el número de bytes si no hay error, si me envía un -1 es error y un 0 es que se libera la conexión de forma 
			acordada o por algún problema entre servidor y cliente. 
			****************************************************************************************************************************/
					if(estado!=S_HELO){//En todo momento que el estado no sea S_HELO, se ejecutan las funciones recv y send. Las cuales
						              //son las que nos permitirán transmitir y recibir información del servidor. 
					enviados=send(sockfd,buffer_out,(int)strlen(buffer_out),0);
					 if(com==0){//Con el entero 'com' compruebo si estoy en la primera iteración o en el resto.
						com=1;
						if(enviados<0) {//En el caso de que la función send devuelva un valor negativo informo al usuario del error. 
							DWORD error=GetLastError();//Tomo el tipo de error producido. 
							printf("CLIENTE> Error %d en el envio de datos\r\n",error);//Imprimo el error al usuario. 
							estado=S_QUIT;//Cierro la conexión. 
						}
					}else{
							if(enviados<=0)//En el caso de que send me devuelva un valor negativo/cero.
					{
						DWORD error=GetLastError();//Tomo el error que ha tenido lugar. 
						if(enviados<0)//En caso de que se termine la conexión aviso al usuario.
						{
							printf("CLIENTE> Error %d en envio de datos\r\n",error);
							estado=S_QUIT;
						}
						else
						{
							printf("CLIENTE> Conexion con el servidor cerrada\r\n");
							estado=S_QUIT;
						 }
						}
					 }}
            /********************************************************************************************************************************
		    Envio de mensajes TCP.
			int recv(int socket, void *rcvBuffer, unsigned int bufferLength, int flags);

			socket       -> Es el descriptor creado para un determinado socket.

			rcvBuffer    -> Apunta al buffer --que es, un área en memoria como un carácter array-- donde los datos recibidos serán colocados.

			bufferLength -> Nos da la longitud del buffer, esta longitud es el número máximo de bytes que pueden ser recibidos de una vez.
			
			recv me devuelve el número de bytes si no hay error, si me envía un -1 es error y un 0 es que se libera la conexión de forma 
			acordada o por algún problema entre cliente y servidor. 
			*******************************************************************************************************************************/
					recibidos=recv(sockfd,buffer_in,512,0);
					if(recibidos<=0)//Compruebo lo que me devuelve la función recv, y en caso de devolver valores de error se lo hago saber al usuario y libero la conexión.
					{
						DWORD error=GetLastError();
						if(recibidos<0)
						{
							printf("CLIENTE> Error %d en la recepcion de datos\r\n",error);
							estado=S_QUIT;
						}
						else
						{
							printf("CLIENTE> Conexion con el servidor cerrada\r\n");
							estado=S_QUIT;											
						}
					}else //En caso de que recv no me devuelva un valor de error, imprimo los datos que reciba el cliente del servidor y son guardados en el buffer_in.
					{
						buffer_in[recibidos]=0x00;//Los arrays en C deben acabar en 0x00.
						printf(buffer_in);//Imprimo lo que recibo. 
						if(estado!=S_DATA && strncmp(buffer_in,OK,2)==0) 
							estado++;  
					}

				}while(estado!=S_QUIT);
				
	
		
			}
			else
			{
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);//Error al tratar de conectar con el servidor. 
			}		
			// fin de la conexion de transporte
			closesocket(sockfd);//Comando para cerrar la conexión int closesocket(int socket);
			
		}	
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option=_getche();//lee un solo carácter de la consola con repetición, es decir, el carácter se muestra en la consola.
		
	}while(option!='n' && option!='N');

	
	
	return(0);

}
