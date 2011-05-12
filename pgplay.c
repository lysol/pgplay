#include <libpq-fe.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>

struct myconn {
    char *conninfo;
    PGconn *conn;
} myconn;

typedef struct myconn Conn;

int main(int argc, char **argv) {
    char c;
    char *filename = NULL;
    Conn *conns = malloc(sizeof(Conn));
    int conn_count = 0;
    char *inputfile = "-";
    char *payload = malloc(1024);

    static struct option long_options[] = {
        {"inputfile", required_argument, 0, 'i'},
        {"configfile", required_argument, 0, 'f'},
        {0, 0, 0, 0}
    };

    int option_index = 0;

    while (1) {
        c = getopt_long(argc, argv, "f:i:", long_options, &option_index);
        if (c == -1)
            break;
        switch (c) {
            case 'f':
                filename = optarg;
                break;
            case 'i':
                inputfile = optarg;
                break;
            default:
                abort();
        }
    }
    if (!filename) {
        fputs("No filename provided.\n", stderr);
        return 1;
    }

    


    int n = 0;
    int bc = 0;
    char *buf = malloc(512);

    FILE *config = fopen(filename, "r");
    if (config == NULL) {
        printf("Error: %s\n", strerror(errno));
        return 1;
    }
    
    int x;
    do {
        x = fgetc(config);
        
        if (bc > 512)
            buf = realloc(buf, bc);

        if (x == '\n' || x == EOF) {
            conn_count++;
            conns = realloc(conns, sizeof(Conn) * conn_count);
            buf[bc] = '\0';
            //memset(conns + n, 0, sizeof(Conn));
            int l = strlen(buf);
            conns[n].conninfo = malloc(l + 1);
            char *des = conns[n].conninfo;
            strcpy(des, buf);
            bc = 0;
            n++;
            if (x == EOF)
                break;
        } else {
            buf[bc] = x;
            bc++;
        }
    } while (1);
    fclose(config);

    int i;
    FILE *input = (strcmp(inputfile, "-") == 0) ? stdin : fopen(inputfile, "r");
    int pos = 0;
    do {
        x = fgetc(input);
        if (x == EOF)
            break;
        if (pos > sizeof(*payload))
            payload = realloc(payload, pos + 2);
        if (pos > 0)
            sprintf(payload, "%s%c", payload, x);
        else
            sprintf(payload, "%c", x);
        //payload[pos] = x;
        pos++;
    } while (1);
    fclose(input);
    
    for (i=0; i < conn_count; i++) {
        conns[i].conn = PQconnectdb(conns[i].conninfo);
        if (!conns[i].conn) {
            fprintf(stderr, "libpq error: PQconnectdb returned NULL for connection: %s.\n\n", conns[i].conninfo);
            exit(0);
        }

        if (PQstatus(conns[i].conn) != CONNECTION_OK) {
            fprintf(stderr, "libpq error: PQstatus(psql) != CONNECTION_OK for connection: %s\n\n", conns[i].conninfo);
            exit(0);
        }
        //printf("Connection OK: %s\n", conns[i].conninfo);
    }

    char **textresults = malloc(sizeof(char *) * conn_count);
    memset(textresults, 0, sizeof(char *) * conn_count);
    PGresult *result;

    for (i=0;i < conn_count; i++) {
        Conn c = conns[i];
        result = PQexec(c.conn, payload);
        char fub[1024];
        if (PQresultStatus(result) == PGRES_TUPLES_OK) {
            int x;
            int y;
            for (x = 0; x < PQntuples(result); x++) {
                for (y = 0; y < PQnfields(result); y++) {
                    char *tupt = (PQgetisnull(result, x, y)) ? "NULL" : PQgetvalue(result, x, y);
                    
                    if (y > 0)
                        sprintf(fub, "%s,%s", fub, tupt);
                    else
                        sprintf(fub, "%s", tupt);
                }
                sprintf(fub, "%s\n", fub);
            }
            textresults[i] = fub;
        } else {
            textresults[i] = PQerrorMessage(c.conn);
        }
        PQclear(result);
        PQfinish(c.conn);
    }

    for (i=0; i < conn_count; i++) {
        if (textresults[i])
            printf("%s: %s", conns[i].conninfo, textresults[i]);
        else
            printf("%s: ERROR\n", conns[i].conninfo);
        free(conns[i].conninfo);
    }
    free(conns);
    return 0;
}
