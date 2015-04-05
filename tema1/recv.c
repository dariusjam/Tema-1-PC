#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

/* Functie pentru valoare in binar pt checksum */
int* binar(unsigned char c) {
	int* a;
	int i;
	
	a =(int *)calloc(8, sizeof(int));
	
	for(i = 7; i >= 0; i--) {
		if(c % 2 == 0) {
			a[i] = 0;
		} else {
			a[i] = 1;
		}
		c = c / 2;
	}
	
	return a;
}

int main(int argc,char** argv){
  msg t;
  msg *p;
  init(HOST,PORT);
  receiver_msg my_msg;
  sender_msg sent;
  char filename[1400], buffer[1400];
  int fd, i, j;
  unsigned char frame_expected = 1;
  unsigned char xor;
  time_t ltime;
  struct tm *Tm;
  
  FILE *log;
  
  memset(t.payload, 0, sizeof(t.payload));
  
  memcpy(filename, "date.out", 8);

  fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  
  while(1) {
  
  	/* Primesc mesajul in cazul in are nu s-a depasit timeoutul*/
  	memset(t.payload, 0, sizeof(t.payload));

    p = receive_message_timeout(100);
    if (p == NULL) {
      continue;
  	}
  	t = *p;
	
	sent = *((sender_msg*) t.payload);
	
	/* Scriu in log.txt */
    log = fopen("log.txt", "a");
    ltime = time(NULL);
 	Tm = localtime(&ltime);
  	fprintf(log, "[%d %d %d %d:%d:%d] [RECEIVER] Am primit urmatorul pachet:\n",
			Tm->tm_mday,
			Tm->tm_mon,
			Tm->tm_year + 1900,
			Tm->tm_hour,
			Tm->tm_min,
			Tm->tm_sec);
			
	fprintf(log, "Seq_no: %d\n", sent.seq_no);
	fprintf(log, "Payload: %s\n", sent.payload);

	/* Daca pachetul are numarul de secventa asteptat sau daca
	este ultimul pachet cu numarul de secventa 0, se verifica in continuare,
	altfel se trimite ack pt a cere acelasi pachet */
	if(sent.seq_no == frame_expected || sent.seq_no == 0) {
		/* Se calculeaza checksum pt pachetul primit*/
		xor = sent.seq_no;
   		for(i = 0; i < sizeof(sent.payload); i++) {
    		xor ^= sent.payload[i];
   		}
   		
   		fprintf(log, "Checksum: ");
   		int* a = binar(xor);
	
		for (j = 0; j < 8; j++) {
			fprintf(log, "%d", a[j]);
		}
		
		/* Daca checksumul din pachet este acelasi cu cel calculat acum, 
		se incrementeaza numarul pachetului ce va fi asteptat iar payloadul din
		acest pachet e pus in fisierul de output, alftel se trimite ack pt acelasi
		pachet.*/
   		if(sent.checksum == xor) {
   			fprintf(log, "\nAm calculat checksum si pachetul s-a trimis fara erori, scriu in fisierul de iesire. ");
   			fprintf(log, "Trimit ACK cu Seq no: %d\n", frame_expected + 1);
   			fprintf(log, "---------------------------------------------------------\n");
   			fclose(log);
   			memcpy(buffer, sent.payload, t.len - 2);
    		write(fd, sent.payload, t.len - 2);
    		frame_expected++;
    		if(sent.seq_no == 0) {
    			frame_expected = 0;
    		}
    	} else {
    		fprintf(log, "\nAm calculat checksum si s-a detectat eroare. Trimit ACK cu Seq no : %d\n", frame_expected);
   			fprintf(log, "---------------------------------------------------------\n");
   			fclose(log);
    	}
    }
    		
   
   
   /* Scriu in log*/
   log = fopen("log.txt", "a");
   ltime = time(NULL);
   Tm = localtime(&ltime);
   fprintf(log, "[%d %d %d %d:%d:%d] [RECEIVER] Trimit ACK: \n",
      	        Tm->tm_mday,
				Tm->tm_mon,
				Tm->tm_year + 1900,
				Tm->tm_hour,
				Tm->tm_min,
				Tm->tm_sec);
	fprintf(log, "Seq no : %d\n", frame_expected);
	fprintf(log, "Checksum : ");
	int* a = binar(frame_expected);
	
		for (j = 0; j < 8; j++) {
			fprintf(log, "%d", a[j]);
		}
  	fprintf(log, "\n---------------------------------------------------------\n");
  	fclose(log);	
	
	/* Setez campurile pt pachetul ack si il trimit*/
   	memset(t.payload, 0, sizeof(t.payload));

    my_msg.seq_no = frame_expected;
    my_msg.checksum = frame_expected;
    memcpy(t.payload, &my_msg, 2);
    send_message(&t);
   
   /* Daca numarul de secventa este 0 si daca pachetul a fost primit cu succes inseamna
   ca s-a terminat transferul si se inchide receiverul.*/
    if(sent.seq_no == 0 && xor == sent.checksum) {
		/* Scriu in log*/
  		log = fopen("log.txt", "a");
  		ltime = time(NULL);
  		Tm = localtime(&ltime);
  		fprintf(log, "[%d %d %d %d:%d:%d] [RECEIVER] FILE TRANSFER ENDED. S-A IESIT DIN RECEIVER\n",
      			Tm->tm_mday,
				Tm->tm_mon,
				Tm->tm_year + 1900,
				Tm->tm_hour,
				Tm->tm_min,
				Tm->tm_sec);
  		fprintf(log, "---------------------------------------------------------\n");
  		fclose(log);	
		break;
	}
	
  }

  close(fd);
  return 0;
}


