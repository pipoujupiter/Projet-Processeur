#include <stdint.h>
#include <stdbool.h>

/* *****************************************************************************
 * Function:  is_label
 * --------------------
 * indique si une chaine de caractère correspond à la syntaxe d'un label
 *
 *   buffer: chaine à analyser
 * 
 * returns: true si buffer est un label sinon false
 * ****************************************************************************/
bool is_label(const char *buffer);

/* *****************************************************************************
 * Function:  get_label_addr
 * --------------------
 * renvoi l'adresse d'un label s'il existe, UINT16_MAX sinon
 *
 *   label: chaine de caractère correspondant au label
 * 
 * returns: renvoi l'adresse d'un label s'il existe, UINT16_MAX sinon
 * ****************************************************************************/
uint16_t get_label_addr(const char *label);

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
void add_label(const char *label, uint16_t addr, bool verbose);

/* *****************************************************************************
 * Function:  free_labels
 * --------------------
 * vide la liste des labels et libère la mémoire
 * ****************************************************************************/
void free_labels();
