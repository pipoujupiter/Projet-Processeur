#include "options.h"
#include "asm.h"
#include "labels.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

// Constante pour sortie non loigisim
#define OUT_DEFAULT_FMT "%u\n"

// Constantes pour sortie logisim
#define OUT_LOGISIM_FMT "%08X\n"
#define OUT_LOGISIM_HEADER "v2.0 raw\n"

/* *****************************************************************************
 * Function:  print_error
 * --------------------
 * termine les écriture de la sortie standard puis affiche le message sur
 * la sortie d'erreur
 *
 *   fmt: format d'affichage
 *   ...: arguments
 * ****************************************************************************/
void print_error(const char *fmt, ...)
{
  va_list aq;
  // force un retour chariot sur la sortie standard
  printf("\n");
  // force l'affichage du buffer de la sortie standard
  // avant d'afficher le message d'erreur
  fflush(stdout);
  // affichage sur stderr du message
  va_start(aq, fmt);
  vfprintf(stderr, fmt, aq);
  va_end(aq);
}

/* *****************************************************************************
 * Corps du programme
 * ****************************************************************************/
int main(int argc, char *argv[])
{
  // ***************************************************************************
  // variables
  // ***************************************************************************

  options_t options = OPT_DEFAULTS;
  uint16_t line_number = 0; // compteur de lignes
  char buffer[ASM_LINE_MAXLEN + 1];
  char *out_fmt = OUT_DEFAULT_FMT; // format de sortie des instructions

  // ***************************************************************************
  // traitement des options
  // ***************************************************************************
  get_options(&options, argc, argv);

  // si options.logisim ecrit l'entête et change le format de sortie
  if (options.logisim)
  {
    out_fmt = OUT_LOGISIM_FMT;
    fprintf(options.output, OUT_LOGISIM_HEADER);
  }

  // ***************************************************************************
  // première passe pour construire la table des labels
  // ***************************************************************************
  if (options.verbose)
    printf("-- First pass: get labels\n");

  // iteration sur les lignes du fichier d'entree
  while (NULL != fgets(buffer, ASM_LINE_MAXLEN + 1, options.input))
  {
    // Teste si le premier mot est un label
    if (NULL != strtok(buffer, ASM_SEPS) && is_label(buffer))
      add_label(buffer, line_number, options.verbose);

    line_number++;
  }

  // erreur de lecture
  if (!feof(options.input))
  {
    perror("read input file\n");
    return EXIT_FAILURE;
    // Not reached
  }

  // ***************************************************************************
  // deuxieme passe pour les instructions
  // ***************************************************************************

  // retourne au début du fichier pour la deuxième passe
  if (0 != fseek(options.input, 0, SEEK_SET))
  {
    perror("rewind input file");
    return EXIT_FAILURE;
  }

  line_number = 0;

  if (options.verbose)
    printf("-- Second pass: decode instructions\n");

  // iteration sur les lignes du fichier d'entree
  while (NULL != fgets(buffer, ASM_LINE_MAXLEN + 1, options.input))
  {
    // suppression des caractères de fin de ligne
    buffer[strcspn(buffer, "\r\n")] = '\0';

    // écrit l'instruction décodée dans le fichier résultat
    if (0 > fprintf(options.output, out_fmt,
                    decode_asm(buffer, line_number, options.verbose)))
    {
      perror("write instruction: ");
      return EXIT_FAILURE;
      // Not reached
    }

    line_number++;
  }

  // erreur de lecture
  if (!feof(options.input))
  {
    perror("read input file\n");
    return EXIT_FAILURE;
    // Not reached
  }

  // ***************************************************************************
  // Nettoyage et fin
  // ***************************************************************************

  free_labels();
  fclose(options.input);
  fclose(options.output);

  return EXIT_SUCCESS;
}
