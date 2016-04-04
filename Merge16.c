/* Filename: Merge16.c
 * Created by: Jason Chu
 * Merge16 is a sort program that reads lines from the files specified in the argument flags [filename]*, removes the trailing newlines,
 * and writes out the sorted lines to standard output. A -POS, LEN flag argument can be specified as well to sort by the substring starting
 * from index POS of length LEN in each line. If POS lies beyond the end of the line, the key is the empty string.  If there are fewer than
 * LEN characters starting with line[POS], the key consists of only those characters.
 * Merge16 is stable in the sense that two lines with equal keys must be printed out so that their relative order in the input is maintained.
 *
 * Merge16 uses two queues to do the merge sort, and the implementation of the queue is written in Queue.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "Queue.h"
#include <limits.h>
#include <string.h>
#include "getLine.h"


// Print message to stderr and exit.
#define DIE(msg)    exit (fprintf (stderr, "%s\n", msg))

/* addRemove adds a node from the head of one queue to the tail of another (potentially the same queue).
 * Achieves this by calling addQ and removeQ.
 * Also, addRemove has a boolean "print" that will determine whether the string "line" should be outputted (print will only be true when
 * we want to output the final sorted strings)*/
static void addRemove(Queue *dest, Queue *from, char *line, bool print)
{
    if (!print)
    {
        if (!addQ(dest, line))
        {
            DIE("addQ() failed");
        }
    }
    if (!removeQ(from, &line))
    {
        DIE("removeQ() failed");
    }
    if (print)
    {
        printf("%s\n", line);
        if (line != NULL && line[0] != '\0')
        {
            free(line);
        }
    }
}

/* compareLines returns the strncmp value comparing strings line1 and line2. First, if the passed in int pos lies beyond the length of either line1 or line2,
 * that string becomes the empty string for comparison. Then, return the strncmp value comparing line1 and line2.
 */
static int compareLines(char *line1, char *line2, int pos, int len)
{
    int value;
    if (line1[0] == '\0' && line2[0] != '\0')
    {
        value = strncmp("", (line2) + pos, len);
    }
    else if (line1[0] != '\0' && line2[0] == '\0')
    {
        value = strncmp((line1) + pos, "", len);
    }
    else if (line1[0] == '\0' && line2[0] == '\0')
    {
        value = 0;
    }
    else
    {
        value = strncmp((line1) + pos, (line2) + pos, len);
    }
    return value;
}

/*
 * trim takes in a string and tries to remove the trailing new line character. If a string does not have a new line character at the end,
 * then nothing is trimmed. Also, if a string is just the newline, then return the empty string.
 */
static char *trim (char *s)
{
    if (s[0] == '\n')
    {
        free(s);
        s = "";
    }
    else
    {
        long i = strlen(s)-1;
        if ((i > 0) && (s[i] == '\n'))
            s[i] = '\0';
    }
    return s;
}
int main(int argc, char* argv[])
{
    // pos and len are longs representing the pos and len argument flags
    long pos;
    long len;
    bool hasPos = false;
    // if there are only two arguments and the second argument starts with a -, then throw an error
    if (argc == 2 && argv[1][0] == '-')
    {
        fprintf(stderr, "%s\n", "Invalid -POS,[LEN]");
        exit(EXIT_FAILURE);
    }
    // handling of pos and len
    if (argv[1][0] == '-')
    {
        hasPos = true;
        bool hasComma = false;
        int index = 1;
        int comma = -1;
        while (argv[1][index] != '\0')
        {
            // if seen a character that is not a comma or digit then invalid pos, len
            if (argv[1][index] != ',' && (argv[1][index] < '0' || argv[1][index] > '9'))
            {
                fprintf(stderr, "%s\n", "Invalid -POS,[LEN]");
                exit(EXIT_FAILURE);
            }
            // if seen a comma and no comma has been seen before
            else if (argv[1][index] == ',' && hasComma == false)
            {
                hasComma = true;
                comma = index;
            }
            // if seen a comma but a comma has already been seen before, then invalid pos, len
            else if (argv[1][index] == ',' && hasComma == true)
            {
                fprintf(stderr, "%s\n", "Invalid -POS,[LEN]");
                exit(EXIT_FAILURE);
            }
            index++;
        }
        char *end = argv[1]+strlen(argv[1]);
        // if comma at the end or beginning, then invalid pos, len
        if ((comma == index-1) || comma == 1)
        {
            fprintf(stderr, "%s\n", "Invalid -POS,[LEN]");
            exit(EXIT_FAILURE);
        }
        // if only pos is given and no len arguments
        else if (comma == -1)
        {
            pos = strtol(argv[1]+1, &end, 10);
            len = INT_MAX;
        }
        // if both pos and len arguments are given
        else
        {
            char *commaPtr = argv[1]+comma;
            pos = strtol(argv[1]+1, &commaPtr, 10);
            len = strtol(argv[1]+comma+1, &end,10);
        }
    }
    // if no pos and len are specified, then pos is 0 and len is INT_MAX
    else
    {
        pos = 0;
        len = INT_MAX;
    }
    //printf("POS: %ld\n", pos);
    //printf("LEN: %ld\n", len);
    
    int index;
    if (hasPos)
        index = 2;
    else
        index = 1;
    
    Queue q1, q2;
    if (!createQ(&q1))
    {
        DIE("createQ() failed");
    }
    if (!createQ(&q2))
    {
        DIE("createQ() failed");
    }
    
    // total number of lines
    int totalLines = 0;
    // number of elements in q1
    int q1Size = 0;
    // number of elements in q2
    int q2Size = 0;
    // addQueue1 is true when add to q1; false when add to q2
    bool addQueue1 = true;
    // continuation is true when there is another file after the currently opened file
    bool continuation = false;
    char *line;
    /* Retrieve all the lines from all the files and do the first merge step with group size of 1
     * This step also merges across files; that is, if there is one unprocessed line from one file, that line is compared
     * to the first line of the next valid file and then merged together
     * */
    while (index < argc)
    {
        FILE *file;
        file = fopen(argv[index], "r");
        
        // invalid file
        if (file == NULL)
        {
            fprintf(stderr, "%s\n", "Nonexistent file name");
            exit(EXIT_FAILURE);
        }
        
        // if not continued from a previous file, then override line with the first line of the new file; otherwise, line remains as the last line of the previuos file
        if (!continuation)
        {
            line = getLine(file);
        }
        continuation = false;
        char *nextLine = getLine(file);
        while (line != NULL && nextLine != NULL)
        {
            // remove the new line character using function trim
            line = trim(line);
            nextLine = trim(nextLine);
            
            int value = compareLines(line, nextLine, pos, len);
            // if line should be added before nextLine
            if (value < 0 || value == 0)
            {
                if (addQueue1)
                {
                    if (!addQ(&q1, line))
                    {
                        DIE("addQ() failed");
                    }
                    if (!addQ(&q1, nextLine))
                    {
                        DIE("addQ() failed");
                    }
                    q1Size+=2;
                }
                else
                {
                    if (!addQ(&q2, line))
                    {
                        DIE("addQ() failed");
                    }
                    if (!addQ(&q2, nextLine))
                    {
                        DIE("addQ() failed");
                    }
                    q2Size+=2;
                }
            }
            // if nextLine should be added before line
            else if (value > 0)
            {
                if (addQueue1)
                {
                    if (!addQ(&q1, nextLine))
                    {
                        DIE("addQ() failed");
                    }
                    if (!addQ(&q1, line))
                    {
                        DIE("addQ() failed");
                    }
                    q1Size+=2;
                }
                else
                {
                    if (!addQ(&q2, nextLine))
                    {
                        DIE("addQ() failed");
                    }
                    if (!addQ(&q2, line))
                    {
                        DIE("addQ() failed");
                    }
                    q2Size+=2;
                }
            }
            totalLines+=2;
            addQueue1 = !addQueue1;
            line = getLine(file);
            nextLine = getLine(file);
        }
        // if there are an odd number of lines in the file
        if (line != NULL && nextLine == NULL)
        {
            // if last file
            if (index == argc - 1)
            {
                line = trim(line);
                if (addQueue1)
                {
                    if (!addQ(&q1, line))
                    {
                        DIE("addQ() failed");
                    }
                    q1Size++;
                }
                else
                {
                    if (!addQ(&q2, line))
                    {
                        DIE("addQ() failed");
                    }
                    q2Size++;
                }
                totalLines++;
            }
            // if there are more files to be processed, then continuation is true
            else
            {
                continuation = true;
            }
        }
        if (nextLine != NULL)
            free(nextLine);
        
        //free storage created by fopen
        fclose(file);
        index++;
    }
    
    int groupsize = 2;
    bool print = false;
    // if number of lines is either 1 or 2, then the lines are already sorted at this point
    if (totalLines == 1 || totalLines == 2)
    {
        char *line = "x";
        while (!isEmptyQ(&q1))
        {
            removeQ(&q1, &line);
            printf("%s\n", line);
            if (line[0] != '\0')
                free(line);
        }
    }
    // Merge process with all groupsizes greater than 1; merges groupsize number of elements from each queue together, keeping track of how many elements are left
    // in each queue and the size of each queue.
    else
    {
        // end condition when groupsize is more than the total number of lines
        while (groupsize < totalLines)
        {
            if (groupsize >= (double)totalLines/2)
                print = true;
            
            int linesAdded = 0;
            
            addQueue1 = true;
            
            // how many elements in each queue left to be processed
            int q1Left = q1Size;
            int q2Left = q2Size;
            
            int fromQueue1 = 0;
            int fromQueue2 = 0;
            
            while (q1Left > 0 && q2Left > 0)
            {
                fromQueue1 = 0;
                fromQueue2 = 0;
                // when there are still elements to be processed and have not reached the groupsize number from either queue
                while (q1Left > 0 && q2Left > 0 && fromQueue1 < groupsize \
                       && fromQueue2 < groupsize && !isEmptyQ(&q1) && !isEmptyQ(&q2))
                {
                    
                    char *line1;
                    char *line2;
                    headQ(&q1, &line1);
                    headQ(&q2, &line2);
                    
                    int value = compareLines(line1, line2, pos, len);
                    
                    // if line
                    if (value < 0 || value == 0)
                    {
                        if (addQueue1)
                        {
                            addRemove(&q1, &q1, line1, print);
                            q1Left--;
                        }
                        else
                        {
                            addRemove(&q2, &q1, line1, print);
                            q2Size++;
                            q1Size--;
                            q1Left--;
                        }
                        fromQueue1++;
                    }
                    else if (value > 0)
                    {
                        if (addQueue1)
                        {
                            addRemove(&q1, &q2, line2, print);
                            q1Size++;
                            q2Size--;
                            q2Left--;
                        }
                        else
                        {
                            addRemove(&q2, &q2, line2, print);
                            q2Left--;
                        }
                        fromQueue2++;
                    }
                }
                
                // when number of elements processed from queue1 has not reached the required groupsize
                while (fromQueue1 < groupsize && !isEmptyQ(&q1) && q1Left > 0)
                {
                    char *line1;
                    headQ(&q1, &line1);
                    if (addQueue1)
                    {
                        addRemove(&q1, &q1, line1, print);
                        q1Left--;
                    }
                    else
                    {
                        addRemove(&q2, &q1, line1, print);
                        q2Size++;
                        q1Size--;
                        q1Left--;
                    }
                    fromQueue1++;
                    linesAdded++;
                }
                // when number of elements processed from queue2 has not reached the required groupsize
                while (fromQueue2 < groupsize && !isEmptyQ(&q2) && q2Left > 0)
                {
                    char *line2;
                    headQ(&q2, &line2);
                    if (addQueue1)
                    {
                        addRemove(&q1, &q2, line2, print);
                        q1Size++;
                        q2Size--;
                        q2Left--;
                    }
                    else
                    {
                        addRemove(&q2, &q2, line2, print);
                        q2Left--;
                    }
                    fromQueue2++;
                    linesAdded++;
                }
                // change which queue to add to
                if (addQueue1)
                {
                    addQueue1 = false;
                }
                else
                {
                    addQueue1 = true;
                }
            }
            // when there are still unprocessed elements left in queue1 when queue2 is completely finished
            while (q1Left > 0)
            {
                char *line;
                headQ(&q1, &line);
                if (addQueue1)
                {
                    addRemove(&q1, &q1, line, print);
                }
                else
                {
                    addRemove(&q2, &q1, line, print);
                    q2Size++;
                    q1Size--;
                }
                q1Left--;
            }
            // when there are still unprocessed elements left in queue2 when queue1 is completely finished
            while (q2Left > 0)
            {
                char *line;
                headQ(&q2, &line);
                if (addQueue1)
                {
                    addRemove(&q1, &q2, line, print);
                    q1Size++;
                    q2Size--;
                }
                else
                {
                    addRemove(&q2, &q2, line, print);
                }
                q2Left--;
            }
            
            groupsize *= 2;
        }
    }
    
    if (!destroyQ(&q1))
        DIE ("destroyQ failed");
    if (!destroyQ(&q2))
        DIE ("destroyQ failed");
    
    return 0;
}
