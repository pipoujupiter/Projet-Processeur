// IR
#include "asm2bin.h"
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

extern char *optarg;
extern int opterr, optind;

// Affiche l'aide pour l'utilisation du programme
void usage(FILE *output, char *progname)
{
  fprintf(output, "%s " USAGE_STR "\n", progname ? basename(progname) : DEFAULT_PROGNAME);
}

// Lit les options de la ligne de commande et
// ouvre les descripteurs de fichiers dans le bon mode si nécessaire.
// Interrompt le programme en cas d'erreur.
void get_options(options_t *options, int argc, char *argv[])
{
  int opt;
  opterr = 0;

  while ((opt = getopt(argc, argv, OPTSTR)) != EOF)
  {
    switch (opt)
    {
    case 'i':
      if (!((*options).input = fopen(optarg, "r")))
      {
        perror("fopen(input, r)");
        exit(EXIT_FAILURE);
        // Not reached
      }
      break;

    case 'o':
      if (!((*options).output = fopen(optarg, "w")))
      {
        perror("fopen(output, r)");
        exit(EXIT_FAILURE);
        // Not reached
      }
      break;

    case 'v':
      (*options).verbose = true;
      break;

    case 'q':
      (*options).quiet = true;
      break;

    case 'l':
      (*options).logisim = true;
      break;

    case 'h':
      usage(stdout, argv[0]);
      exit(EXIT_SUCCESS);
      // Not reached

    default:
      usage(stderr, argv[0]);
      exit(EXIT_FAILURE);
      // Not reached
    }
  }

  if (NULL == (*options).input || NULL == (*options).output)
  {
    usage(stderr, argv[0]);
    exit(EXIT_FAILURE);
    // Not reached
  }

}
