/*
 * Name: Keiana Snell
 * Email: keianarei@g.ucla.edu
 * ID: 504804776
 * SLIPDAYS: 0
 * */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h> //for strerror
#include <signal.h> // for SIGSEV
#include <getopt.h> //for getopt_long
#include <fcntl.h> //for open()
#include <unistd.h> //for close(), read(), write(), creat(), dup()


void signal_handler() {
    	fprintf(stderr, "Error: segmentation fault caught \n");
	exit(4);
}

int main(int argc, char * argv[]) {
	
    	int seg_fault_flag = 0;
    	int catch_flag = 0;
	char* input_file = NULL;
    	char* output_file = NULL;

    	const struct option long_options[] = {
        	{"input", required_argument, NULL, 'i'},
        	{"output", required_argument, NULL, 'o'},
       		{"catch", no_argument, &catch_flag, 1},
        	{"segfault", no_argument, &seg_fault_flag, 1},
		{0, 0, 0, 0}
	    };

    	int c;
    	while ((c = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
    		switch (c) {
			case 'i':
                		input_file = optarg;
                		break;
            		case 'o':
                		output_file = optarg;
                		break;
			case 0:
				break;
			default:
        			fprintf(stderr, "Error: incorrect command line arguments used. Only --input=filename --output=filename --segfault --catch are allowed. \n");
				exit(1);
		}
    	}

    if (optind < argc) {
        fprintf(stderr, "Error: illegal argument number used. \n");
	exit(1);
    }


    if (catch_flag == 1) {
        signal(SIGSEGV, signal_handler);
    }

    if (seg_fault_flag == 1) {
        char* seg_fault = NULL;
        *seg_fault = 0;
}

if (input_file != NULL) {
        int input_file_d = open(input_file, O_RDONLY);
        if (input_file_d < 0) {
            fprintf(stderr, "Error: cannot create file %s \n", input_file);
            exit(2);
        } else {
            close(0);
            dup(input_file_d);
            close(input_file_d);
        }
    }

    if (output_file != NULL) {
        int output_file_d = creat(output_file, 0666);
        if (output_file_d < 0) {
            fprintf(stderr, "Error: cannot output with file %s \n", output_file);
            exit(3);
        } else {
            close(1);
            dup(output_file_d);
            close(output_file_d);
        }
    }


    char* buffer = (char*) malloc(sizeof(char));
    ssize_t i = read(0, buffer, 1);
    while (i > 0) {
        write(1, buffer, 1);
        i = read(0, buffer, 1);
    }

    exit(0);


    return 0;
}

