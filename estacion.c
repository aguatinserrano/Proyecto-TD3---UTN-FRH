/**
*********************************************************************************************************
*                               Headers
*********************************************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

/**
*********************************************************************************************************
*                               Definiciones
*********************************************************************************************************
*/

#define STR_IP_LEN 40         /* Tamaño del string que contiene la direccion IP */
#define PORT_LEN 10           /* Tamaño del string que contiene el numero de puerto */
#define STAT_NUM_LEN 5        /* Tamaño del string que contiene el numero de estaciones */
#define ACK "ACK"             /* Palabra de control */
#define NACK "NACK"           /* Palabra de control */
#define DAT_NUM 8             /* Cantidad de datos de la estacion meteorologica */
#define TAMDAT 10             /* Tamaño del string que contiene los datos */
#define TAM_STR 10
#define EXIT "N"              /* Letra para salir del programa */
#define ID_EST "1"            /* ID de la estacion meteorologica */


/**
*********************************************************************************************************
*                               Prototipos
*********************************************************************************************************
*/

int ConexionTCP(char*, char*);

/**
*********************************************************************************************************
*                               Main
*********************************************************************************************************
*/

int main()
{
  char str[TAM_STR];

  /* Variables de las estaciones meteorologicas */
  char station_id[STAT_NUM_LEN];
  char station_status[STAT_NUM_LEN];
  char datos[DAT_NUM][TAMDAT];
  int i;
  char nombres[DAT_NUM][15]= {"ID", "Temperatura", "Humedad", "Presion", "Velocidad", "Direccion", "Dia", "Hora" };
  char unidades [DAT_NUM][5]= {" ", "°C", "%", "Pa", "km/h", "°", " ", "hs"};

  /* Variables para la conexion TCP */
  int numbytes;
  int sockfd1, sockfd2;
  char str_ip[STR_IP_LEN] = "172.16.9.217";
  char str_port[PORT_LEN] = "10000";

  /* Inicializacion de los datos */
  for (i = 0; i < DAT_NUM; i++)
    strcpy(datos[i], "15");

  /* Ingreso de direccion IP y puerto */
  /*printf("Ingrese la direccion IP del servidor\n");
  scanf("%s", str_ip);
  printf("Ingrese el puerto del servidor\n");
  scanf("%s", str_port);*/

  /* Establezco la conexion TCP */
  if((sockfd1 = ConexionTCP(str_ip, str_port)) == -1)
  {
    perror("ConexionTCP");
    exit(1);
  }

  printf("Conectado al servidor satisfactoriamente\n");

  if ((numbytes = recv(sockfd1, str_port, PORT_LEN, 0)) == -1)
  {
    perror("recv");
    exit(1);
  }
  str_port[numbytes] = '\0';
  printf("Numero de puerto designado: %s\n", str_port);
  close(sockfd1);

  /* Establezco la conexion TCP */
  if((sockfd2 = ConexionTCP(str_ip, str_port)) == -1)
  {
    perror("ConexionTCP");
    exit(2);
  }

  printf("Conectado al servidor satisfactoriamente\n\n");

  while (1)
  {
    /* Envio la ID al servidor */
    while (send(sockfd2, ID_EST, strlen(ID_EST), 0) == -1)
      perror("send");
    if ((numbytes = recv(sockfd2, str, sizeof(str), 0)) == -1)
    {
      perror("recv");
      exit(1);
    }
    str[numbytes] = '\0';
    printf("Posicion: %d, ACK: %s\n", i, str);

    /* Envio los datos */
    for (i = 0; i < DAT_NUM; i++)
    {
      if (send(sockfd2, datos[i], strlen(datos[i]), 0) == -1)
      {
        perror("send");
        continue;
      }
      printf("Posicion: %d, Dato: %s\n", i, datos[i]);

      if ((numbytes = recv(sockfd2, str, sizeof(str), 0)) == -1)
      {
        perror("recv");
        continue;
      }
      str[numbytes] = '\0';
      printf("Posicion: %d, ACK: %s\n", i, str);
    }
    printf("Presione una tecla para continuar\n");
    scanf("%s", str);
  }

  close(sockfd2);
  return 0;
}

/**
*********************************************************************************************************
*                               Funciones
*********************************************************************************************************
*/

int ConexionTCP(char* str_ip, char* str_port)
{
  /* Variables para la conexion TCP */
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;

  /* Configuracion para la conexion TCP */
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;          /* IPv4 o IPv6 */
  hints.ai_socktype = SOCK_STREAM;      /* TCP */

  /* Obtengo la informacion de la IP y el puerto */
  if ((rv = getaddrinfo(str_ip, str_port, &hints, &servinfo)) != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return -1;
  }

  /* Busco por todos los resultados y me conecto al primero que puedo */
  for (p = servinfo; p != NULL; p = p->ai_next)
  {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
    {
      perror("socket");
      continue;
    }
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
    {
      perror("connect");
      continue;
    }
    break;
  }

  /* Se termino de loopear la lista sin conexiones */
  if (p == NULL)
  {
    fprintf(stderr, "Fallo la conexion\n");
    return -1;
  }

  /* Libero la estructura porque no la necesito mas */

  freeaddrinfo(servinfo);

  return sockfd;
}
