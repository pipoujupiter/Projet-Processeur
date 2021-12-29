#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define OPTSTR "i:o:hvql"
#define USAGE_STR "-i inputfile [-o outputfile] [-h] [-v] [-q]"
#define DEFAULT_PROGNAME "asm2bin"
#define OPT_DEFAULTS { NULL, stdout, false, false, false }

// structure des options
typedef struct
{
  FILE *input;
  FILE *output;
  bool quiet;
  bool verbose;
  bool logisim;
} options_t;

// affiche un message d'erreur avec la ligne en cours de lecture
void get_options(options_t *options, int argc, char *argv[]);
void usage(FILE *output, char *progname);
