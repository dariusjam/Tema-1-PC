#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

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
  init(HOST,PORT);
  msg t;
  msg *p;
  sender_msg my_msg;
  receiver_msg resp;
  int fd, c;
  int random;
  int seq = 1, i, j;
  struct stat f_status;
  int sent_so_far = 0, filesize;
  unsigned char xor;
  time_t ltime;
  struct tm *Tm;
  
  
  FILE *log;		
			
  srand(time(NULL));
  random = (rand() % 60) + 1;
  
  fd = open(argv[1], O_RDONLY);
  char buffer[random];
  
  fstat(fd, &f_status);
  filesize = (int) f_status.st_size;

  c = read(fd, buffer, random);
  seq = 1;
  
  while(1) {
	
	/* Pun ce am in buffer intr-un packet de tip sender_msg, impreuna cu celelalte
	informatii, pe care il pun intr-un pachet msg, pe care apoi il trimit */
  	memset(t.payload, 0, sizeof(t.payload));
    memset(my_msg.payload, 0, sizeof(my_msg.payload));
    
    memcpy(my_msg.payload, buffer, c);
    my_msg.seq_no = seq;
    
    /* Calculeaza checksumul */
 	xor = seq;
    for(i = 0; i < random; i++) {
    	xor ^= my_msg.payload[i];
    }
    my_msg.checksum = xor;
    
    t.len = 2 * sizeof(char) + c;
    memcpy(t.payload, &my_msg, t.len);
 
    /* Trimit mesajul */
    send_message(&t);
    
    /* Scriu in log.txt */
    log = fopen("log.txt", "a");
    ltime = time(NULL);
 	Tm = localtime(&ltime);
  	fprintf(log, "[%d %d %d %d:%d:%d] [SENDER] Am trimis pachet:\n",
			Tm->tm_mday,
			Tm->tm_mon,
			Tm->tm_year + 1900,
			Tm->tm_hour,
			Tm->tm_min,
			Tm->tm_sec);
			
	fprintf(log, "Seq_no: %d\n", my_msg.seq_no);
	fprintf(log, "Payload: %s\n", my_msg.payload);
	fprintf(log, "Checksum: ");
	
	int* a = binar(my_msg.checksum);
	
	for (j = 0; j < 8; j++) {
	  fprintf(log, "%d", a[j]);
	}
	fprintf(log, "\n---------------------------------------------------------\n");
	fclose(log);
    
    
    
    /* Scriu in log.txt */
    log = fopen("log.txt", "a");
    ltime = time(NULL);
 	Tm = localtime(&ltime);
  	fprintf(log, "[%d %d %d %d:%d:%d] [SENDER] Am pornit cronometrul (timeout 100ms)\n",
			Tm->tm_mday,
			Tm->tm_mon,
			Tm->tm_year + 1900,
			Tm->tm_hour,
			Tm->tm_min,
			Tm->tm_sec);
	fclose(log);		
	
	/* Pornesc cronometrul (timeout 100ms)*/
	p = receive_message_timeout(100);
			
	
	/* Daca a trecut de timeout sau s-a primit un mesaj eronat se da continue, 
	adica se intra in while si se retrimite acelasi pachet */
    if (p == NULL) {
      /* Scriu in log*/
      log = fopen("log.txt", "a");
      ltime = time(NULL);
 	  Tm = localtime(&ltime);
      fprintf(log, "[%d %d %d %d:%d:%d] [SENDER] Timeout depasit. Se retrimite pachetul cu Seq no: %d\n",
      		Tm->tm_mday,
			Tm->tm_mon,
			Tm->tm_year + 1900,
			Tm->tm_hour,
			Tm->tm_min,
			Tm->tm_sec,
			my_msg.seq_no);
      fprintf(log, "---------------------------------------------------------\n");
      fclose(log);	
      continue;
  	}
  	
  	t = *p;
  	

  	resp = *((receiver_msg *) t.payload);
  	
  	/* Ca sa stiu cand sa ma opresc, ultimul pachet il trimit cu seq_no = 0
  	(eu incep cu seq_no de la 1 la inceput) */
  	if(resp.seq_no == 0) {
  	  break;
  	}
  	
  	/* Daca cadrul este corect, se avanseaza seq cu o unitate sau se pune 0
  	daca trebuie sa trimit ultimul pachet */
  	if(resp.seq_no == seq + 1) {
      sent_so_far += c;
      c = read(fd, buffer, random);
      if (sent_so_far + c == filesize) {
      	seq = 0;
      } else {
      	seq = resp.seq_no;
      }
    }
   
  } 
  

  /* Scriu in log ca s-a terminat transferul pt ca am iesit din while */
  log = fopen("log.txt", "a");
  ltime = time(NULL);
  Tm = localtime(&ltime);
  fprintf(log, "[%d %d %d %d:%d:%d] [SENDER] FILE TRANSFER ENDED. S-A IESIT DIN SENDER\n",
      	Tm->tm_mday,
		Tm->tm_mon,
		Tm->tm_year + 1900,
		Tm->tm_hour,
		Tm->tm_min,
		Tm->tm_sec);
  fprintf(log, "---------------------------------------------------------\n");
  fclose(log);	
  
  close(fd);

  return 0;
}
	

