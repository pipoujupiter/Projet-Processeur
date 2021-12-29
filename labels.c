#include "labels.h"
#include "asm.h"
#include "asm2bin.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

// Constantes
#define ASM_LABEL_ENDCAR ':'

// Type pour la définition d'un label
typedef struct label_t
{
  const char *label;
  uint16_t addr;
} label_t;

// Variables globales
uint16_t label_count = 0;
label_t *labels = NULL;

/* *****************************************************************************
 * Function:  is_label
 * --------------------
 * indique si une chaine de caractère correspond à la syntaxe d'un label
 *
 *   buffer: chaine à analyser
 * 
 * returns: true si buffer est un label sinon false
 * ****************************************************************************/
bool is_label(const char *buffer)
{
  uint16_t len = strlen(buffer);
  return (len > 0 && ASM_LABEL_ENDCAR == buffer[len - 1]);
}

/* *****************************************************************************
 * Function:  get_label_addr
 * --------------------
 * renvoi l'adresse d'un label s'il existe, UINT16_MAX sinon
 *
 *   label: chaine de caractère correspondant au label
 * 
 * returns: renvoi l'adresse d'un label s'il existe, UINT16_MAX sinon
 * ****************************************************************************/
uint16_t get_label_addr(const char *label)
{
  uint16_t i = 0;
  for (; label_count > i; i++)
  {
    if (0 == strcmp(label, labels[i].label))
    {
      return labels[i].addr;
      // Not reached
    }
  }
  return UINT16_MAX;
}

/* *****************************************************************************
 * Function:  add_label
 * --------------------
 * ajoute un label à la liste des labels connus
 * interrompt le programme en cas de label dupliqué
 *
 *   label: chaine de caractère correspondant au label
 *   addr: adresse du label
 *   verbose: verbose
 * ****************************************************************************/
void add_label(const char *label, uint16_t addr, bool verbose)
{
  // vérification que le label n'existe pas déja
  if (UINT16_MAX != get_label_addr(label))
  {
    print_error("duplicate label %s at line %u\n", label, addr);
    exit(EXIT_FAILURE);
    // Not reached
  }

  // extension de la taille du tableau
  labels = (label_t *)realloc((void *)labels,
                              sizeof(label_t) * (++label_count));
  if (NULL == labels)
  {
    perror("extend label array");
    exit(EXIT_FAILURE);
    // Not Reached
  }

  labels[label_count - 1].label = strdup(label);
  labels[label_count - 1].addr = addr;

  if (verbose)
    printf("%s\t%04u=0x%04x\n", label, addr, addr);

}

/* *****************************************************************************
 * Function:  free_labels
 * --------------------
 * vide la liste des labels et libère la mémoire
 * ****************************************************************************/
void free_labels()
{
  for (; label_count > 0; label_count--)
  {
    free((void *)labels[label_count - 1].label);
  }
  free((void *)labels);
  labels = NULL;
}
