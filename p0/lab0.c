/*
Name: Keiana Snell
Email: keianarei@g.ucla.edu
ID: 504804776
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> //for strerror
#include <getopt.h> //for getopt_long
#include <fcntl.h> //for open()
#include <unistd.h> //for close(), read(), write(), creat(), dup()


int main(int argc, char* argv[]){

int seg_flag = 0;
int catch_flag = 0;


static struct option options[] = {
          {"catch",     no_argument, NULL, 'c'},
          {"segfault",  no_argument, NULL, 's'}
          {"input",  required_argument, NULL, 'i'},
          {"output",  required_argument, NULL, 'o'}
}

val = getopt_long(argc, argv, "", options, NULL);
while(val != -1){
	switch(val){
		case 'i':
			//may want to do this outside of switch to follow steps
			input_file =  open(optarg, O_RDONLY);
			if(){}
		
			else {
				fprintf(stderr, "");
				exit(2);
			}
	
			break;

		case 'o':
			//may want to do this outside of switch to follow steps
			output_file = creat(optarg, 0666);
			if(){}
                                
                        else {
                                fprintf(stderr, "");
                                exit(3);
                        }
			break;

		case 's':
			seg_flag = 1;
			break;
		
		case 'c':
			catch_flag = 1;
			break;

		default:
			fprintf(stderr, 'Incorrect command line argument used. Only --input=filename --output=filename --segfault --catch are allowed');
			exit(1);

	}
}



if(catch_flag){
	//handle catch of seg fault here
}

if(seg_flag && !catch_flag){
	//handly seg fault here
}


exit(0);
}



