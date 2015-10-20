/*******************************************************
Protocolos de Transporte
Grado en Ingeniería Telemática
Dpto. Ingeníería de Telecomunicación
Univerisdad de Jaén

Fichero: servidor.c
Versión: 1.0
Fecha: 23/09/2012
Descripción:
	Servidor de eco sencillo TCP.

Autor: Juan Carlos Cuevas Martínez

*******************************************************/
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <Winsock2.h>

#include "protocol.h"




main()
{

	WORD wVersionRequested;
	WSADATA wsaData;
	SOCKET sockfd,nuevosockfd;
	struct sockaddr_in  local_addr,remote_addr;
	char buffer_out[1024],buffer_in[1024], cmd[10], usr[10], pas[10], SUM[10]; //Se añade un array para el comando sum. 
	int err,tamanio;
	int fin=0, fin_conexion=0;
	int recibidos=0,enviados=0;
	int estado=0;
	int NUM1,NUM2,SUMA; //Variables para los números enteros introducidos y el resultado de la suma. 

	/** INICIALIZACION DE BIBLIOTECA WINSOCK2 **
	 ** OJO!: SOLO WINDOWS                    **/
	wVersionRequested=MAKEWORD(1,1);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0){
		return(-1);
	}
	if(LOBYTE(wsaData.wVersion)!=1||HIBYTE(wsaData.wVersion)!=1){
		WSACleanup() ;
		return(-2);
	}
	/** FIN INICIALIZACION DE BIBLIOTECA WINSOCK2 **/


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
	//En caso de que la función socket = sockfd me devuelva un error (INVALID_SOCKET), imprimo error al 
	//usuario a la hora de crear el Socket. 
	if(sockfd==INVALID_SOCKET)	{
		return(-3);
	}
	else { //En caso de que no haya error.
		//Estructuras de direcciones de IPv4.
		local_addr.sin_family		=AF_INET;			        //Campo de la estructura sockaddr_in para el Protocolo de Internet. 
		local_addr.sin_port			=htons(TCP_SERVICE_PORT);	//Campo de la estructura sockaddr_in para introducir en el que escucha el servidor, el puerto que está ya definido como #define TCP_SERVICE_PORT 6000.
		local_addr.sin_addr.s_addr	=htonl(INADDR_ANY);	        // Direccion IP del servidor Any cualquier disponible
													            // Cambiar para que conincida con la del host
	}
	
	// Enlace el socket a la direccion local (IP y puerto)
	if(bind(sockfd,(struct sockaddr*)&local_addr,sizeof(local_addr))<0)
		return(-4);
/*********************************************************************************************
int bind(int socket, struct sockaddr *localAddress, unsigned int addressLength)

Parámetros:

socket        -> Descriptor del socket que fue creado con la función socket(). 
localAddress  -> El parámetro de la dirección es declarado como un puntero a sockaddr, pero para aplicaciones TCP/IP, 
                 será realmente un puntero a sockaddr_in conteniendo la dirección de Internet de la interfaz local y 
				 el puerto en el que escuchar.
addressLength -> Loongitud de la estructura de dirección.

bind() devuelve 0 para un éxito y -1 para un fallo. Si es un éxito, el socket identificado por el descriptor dado (y no otro)
es asociado con la dirección IP y el puerto dados. La dirección de Internet puede ser colocada al valor especial INADDR_ANY,
lo que significa que las conexiones al puerto especificado serán dirigidas a este socket, sin importar a que direcciones de 
internet son enviadas;  
*********************************************************************************************/

/*********************************************************************************************
int listen(int socket, int queueLimit)

Parámetros:

socket      -> Descriptor del socket que fue creado con la función socket(). 
queueLimit  -> Especifica un máximo del número de conexiones entrantes que puede esperar en cualquier momento. 

Las peticiones entrantes de conexión TCP serán manejadas y entonces colocadas en cola para ser aceptadas por el programa.  

listen() devuelve 0 para el éxito y -1 para un fallo. 
*********************************************************************************************/
	if(listen(sockfd,5)!=0)
		return (-6);
	
	tamanio=sizeof(remote_addr);

	do
	{
		printf ("SERVIDOR> ESPERANDO NUEVA CONEXION DE TRANSPORTE\r\n");
/*********************************************************************************************
Ahora el socket que ha sido atado a un puerto y marcado para "escuchar" no será usado realmente
para enviar y recibir. En su lugar, es usado como una forma de obtener nuevos sockets, para 
cada conexión de cliente; el servidor envía y recibe en estos nuevos sockets. El servidor 
obtener un socket para una conexión entrante de cliente al llamar a la función accept(). 

int accept(int socket, struct sockaddr *clientAddress, unsigned int *addressLength)

Parámetros:

socket        -> Descriptor del socket que fue creado con la función socket(). 
clientAddress -> Llena la estructura sockaddr apuntando por clientAddress, con la dirección del cliente.
addressLength -> Especifica el tamaño máximo de la dirección clientAddress. 

En un éxito, accept() devuelve un descriptor para un nuevo socket que está conectado al cliente. Si 
es un fallo, devuelve -1. El servidor se comunica con el cliente usando send() y recv(); Cuando la 
comunicación se completa, esta se termina con la función close(). 
*********************************************************************************************/
		nuevosockfd=accept(sockfd,(struct sockaddr*)&remote_addr,&tamanio);

		if(nuevosockfd==INVALID_SOCKET) {
			
			return(-5);
		}

		printf ("SERVIDOR> CLIENTE CONECTADO\r\nSERVIDOR [IP CLIENTE]> %s\r\nSERVIDOR [CLIENTE PUERTO TCP]>%d\r\n",
					inet_ntoa(remote_addr.sin_addr),ntohs(remote_addr.sin_port));

		//Mensaje de Bienvenida
		sprintf_s (buffer_out, sizeof(buffer_out), "%s Bienvenindo al servidor de ECO%s",OK,CRLF);
		
		enviados=send(nuevosockfd,buffer_out,(int)strlen(buffer_out),0);
		//TODO Comprobar error de envío
				if(enviados<=0)                     //En caso de que la función send me devuelva un valor entero negativo o cero, informo al cliente del error. 
					{
						DWORD error=GetLastError(); //Tomo el error que se ha producido. 
						if(enviados<0)              //En caso de que me devuelva un valor entero negativo, aviso al cliente del error. 
						{
							printf("CLIENTE> Error %d en el envio de datos\r\n",error);
							estado=S_USER;          //Vuelvo al estado S_USER. 
							fin_conexion = 1;
						}
						else //En otro caso. 
						{
							printf("CLIENTE> Conexión con el cliente cerrada\r\n"); //Se cierra la conexión con el usuario. 
							estado=S_USER;                                          //Volvemos al estado S_USER. 
							fin_conexion = 1;
						}											
		}
        //Se reestablece el estado inicial
		estado = S_USER;
		fin_conexion = 0;

		printf ("SERVIDOR> Esperando conexion de aplicacion\r\n");
		do
		{
			//Se espera un comando del cliente
			recibidos = recv(nuevosockfd,buffer_in,1023,0);
			//TODO Comprobar posible error de recepción
			if(recibidos<=0)                           //En caso de que la función recv me devuelva un valor entero negativo o cero, informo al cliente del error. 
					{
						DWORD error=GetLastError();    //Tomo el error que se ha producido. 
						if(recibidos<0)                //En caso de que me devuelva un valor entero negativo, aviso al cliente del error. 
						{
							printf("CLIENTE> Error %d en la recepcion de datos\r\n",error);
							
							fin_conexion = 1;
							continue;
						}
						else //En otro caso, se cierra la conexión y se vuelve al estado S_USER. 
						{
							printf("CLIENTE> Conexión con el cliente cerrada\r\n");
							
							fin_conexion = 1;
							continue;
						}											
			}
			
			buffer_in[recibidos] = 0x00; //Tomo el array de los datos recibidos por el servidor y en C los arrays finalizan con el byte 0000 0000. 
			printf ("SERVIDOR [bytes recibidos]> %d\r\nSERVIDOR [datos recibidos]>%s", recibidos, buffer_in); //Imprimo el array con los datos recibidos por el servidor.
			//Comienzo la máquina de estados finita. 
			switch (estado)
			{
				case S_USER:    /*****************************************/
					strncpy_s ( cmd, sizeof(cmd),buffer_in, 4);//Copia caracteres de una cadena en otra.
					cmd[4]=0x00; // en C los arrays finalizan con el byte 0000 0000

					if ( strcmp(cmd,SC)==0 ) // si recibido es solicitud de conexion de aplicacion
					{
						sscanf_s (buffer_in,"USER %s\r\n",usr,sizeof(usr));
						
						// envia OK acepta todos los usuarios hasta que tenga la clave
						sprintf_s (buffer_out, sizeof(buffer_out), "%s%s", OK,CRLF);
						
						estado = S_PASS;
						printf ("SERVIDOR> Esperando clave\r\n");
					} else
					if ( strcmp(cmd,SD)==0 )
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Fin de la conexión%s", OK,CRLF);
						fin_conexion=1;
					}
					else
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Comando incorrecto%s",ER,CRLF);
						
					}
				break;

				case S_PASS: /******************************************************/

					
					strncpy_s ( cmd, sizeof(cmd), buffer_in, 4);
					cmd[4]=0x00; // en C los arrays finalizan con el byte 0000 0000

					if ( strcmp(cmd,PW)==0 ) // si comando recibido es password
					{
						sscanf_s (buffer_in,"PASS %s\r\n",pas,sizeof(usr));

						if ( (strcmp(usr,USER)==0) && (strcmp(pas,PASSWORD)==0) ) // si password recibido es correcto
						{
							// envia aceptacion de la conexion de aplicacion, nombre de usuario y
							// la direccion IP desde donde se ha conectado
							sprintf_s (buffer_out, sizeof(buffer_out), "%s %s IP(%s)%s", OK, usr, inet_ntoa(remote_addr.sin_addr),CRLF);
							estado = S_DATA;
							printf ("SERVIDOR> Esperando comando\r\n");
						}
						else
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s Autenticación erronea%s",ER,CRLF);
						}
					} else
					if ( strcmp(cmd,SD)==0 )
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Fin de la conexion%s", OK,CRLF);
						fin_conexion=1;
					}
					else
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Comando incorrecto%s",ER,CRLF);
					}
				break;

				case S_DATA: /***********************************************************/
					
					buffer_in[recibidos] = 0x00;
					
					strncpy_s(cmd,sizeof(cmd), buffer_in, 4);
					sscanf(buffer_in,"%s%d%d",&SUM,&NUM1,&NUM2);//Extraigo del mensaje el comando y los dos números enteros.

					printf ("SERVIDOR [Comando]>%s\r\n",SUM);
					if(NUM1==0 && NUM2==0)
						sprintf_s(buffer_out, sizeof(buffer_out), "Formato incorrecto.\r\n");

			else{
					if ( strcmp(cmd,SD)==0 )
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Fin de la conexion%s", OK,CRLF);
						fin_conexion=1;
					}
					else if (strcmp(cmd,SD2)==0)
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Finalizando servidor%s", OK,CRLF);
						fin_conexion=1;
						fin=1;
					}
					else
			        {   
					if(strcmp(SUM,"sum")==0 && NUM1<9999 && NUM2<9999){ //Compruebo que el comando es sum, y que los números son de 4 cifras máximo. 
							SUMA=NUM1+NUM2; //Se realiza la suma. 
							printf("ok NUM1:%d  NUM2:%d SUM:%d %s",NUM1,NUM2,SUMA,CRLF);//Imrpimo por pantalla el resultado de la suma y los dos números introducidos. 
							sprintf_s(buffer_out, sizeof(buffer_out), "ok %d %s",SUMA,CRLF);//Envío al cliente el resultado de la suma y se lo imprimo por pantalla de forma segura. 
						    NUM1=0;
							NUM2=0;
					}else
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Comando incorrecto%s",ER,CRLF);
					} }
					break;

				default:
					    break;
					
			} // switch

			enviados=send(nuevosockfd,buffer_out,(int)strlen(buffer_out),0);
			//TODO 
			if(enviados<=0)
					{
						DWORD error=GetLastError();
						if(enviados<0)
						{
							printf("CLIENTE> Error %d en el envio de datos\r\n",error);
							estado=S_USER;
							fin_conexion = 1;
						}
						else
						{
							printf("CLIENTE> Conexión con el cliente cerrada\r\n");
							estado=S_USER;
							fin_conexion = 1;
						   }
						}

		} while (!fin_conexion);
		printf ("SERVIDOR> CERRANDO CONEXION DE TRANSPORTE\r\n");
		shutdown(nuevosockfd,SD_SEND);
		closesocket(nuevosockfd); //Cierro el socket una vez se termina la comunicación. 

	}while(!fin);

	printf ("SERVIDOR> CERRANDO SERVIDOR\r\n");

	return(0);
} 
