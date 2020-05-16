#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <wctype.h>

typedef struct {
    long * CurrentStatus;
    long InitialValue;
    long TerminationValue; 
} PROGRESS_STATUS;

long wordcount(char* filename);
void *progress_monitor(void *status);
void printProgress(long currProgress);

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("no file specified\n");
	exit(0);
    }

    long wc = wordcount(argv[1]);
    if (wc == 0) {
        printf("could not open file=%s\n", argv[1]);
        exit(0);
    }
    printf("There are %ld words in %s.\n", wc, argv[1]);
}

/*
 * open the text to be examined.
 * create the progress_monitor thread.
 * count and return the word count; return 0 if error.
 * wait for the progress_monitor thread to exit before return.
 */
long wordcount(char* filename)
{
    FILE *fp = fopen(filename, "r");

    if (fp == NULL)
        return 0;
   
    PROGRESS_STATUS ps;
    ps.InitialValue = 0;
    ps.CurrentStatus = malloc(sizeof(long));

    fseek(fp, 0L, SEEK_END); // to get file size
    ps.TerminationValue = ftell(fp);
    fseek(fp, 0L, SEEK_SET); // back to starting point to read

    pthread_t tid;
    pthread_create(&tid, NULL, progress_monitor, (void *)&ps); 

    // now read, calcurate the word count, and update progress

    long wordCount = 0;
    long bytesProgressed = 0;
    int prevNoneWhiteSpace = 0;

    while (1) {
        char c = fgetc(fp);
        if (c == EOF) {
            if (prevNoneWhiteSpace)
                wordCount++;
            break;
        }
    
        // check if there are multiple consecutive white spaces
        if (iswspace(c)) {
            if (prevNoneWhiteSpace)
                wordCount++;
            prevNoneWhiteSpace = 0;
        }
        else prevNoneWhiteSpace = 1;

        // update byte number
        *(ps.CurrentStatus) = ++bytesProgressed;
    }

    pthread_join(tid, NULL); // wait, block until tid is done
    free(ps.CurrentStatus);

    return wordCount;
}

/*
 * read progress status which is written by the main thread.
 * display progress bar; one '-' represents 1/40th of progress
 */
void *progress_monitor(void *status)
{
    // cast local variables as PROGRESS_STATUS Struct's variables 
    long initValue = ((PROGRESS_STATUS *)status)->InitialValue;
    long totalValue =  ((PROGRESS_STATUS *)status)->TerminationValue;
    long *currValue = ((PROGRESS_STATUS *)status)->CurrentStatus;
    long unitValue = (totalValue - initValue) / 40; //the size of each hyphen

    long myCurrValue = *currValue; // currValue keeps changing, take a snapshot

    int progressLast = 0;
    int progress = (int)((myCurrValue - initValue) / unitValue);

    if (progress) {
        printProgress(progress);
        progressLast = progress; 
    }

    while (myCurrValue < totalValue) {
        myCurrValue = *currValue;
        progress = (int)((myCurrValue - initValue) / unitValue);
        if (progress > progressLast) {
            printProgress(progress - progressLast);
            progressLast = progress;
        }
    }
    printf("\n");
}

/*
 * print the number of hyphens based on the difference
 * of the last progress
 */
void printProgress(long progress)
{
    int i;
    for (i = 0; i < progress; i++) {
        putchar('-');
        fflush(stdout);
    }
}
