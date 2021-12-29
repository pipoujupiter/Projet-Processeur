#include <stdint.h>
#include <stdbool.h>

#define ASM_LINE_MAXLEN 4096
#define ASM_SEPS " \t"
#define ASM_COMMENT_CAR '*'

/* *****************************************************************************
 * Function:  decode_asm
 * --------------------
 * decode une ligne de code assembleur et renvoi le code machine
 * interrompt le programme en cas d'erreur de syntaxe ou de label
 *
 *  line: chaine à analyser
 *  line_number: numéro de la ligne en cours d'analyse
 *  verbose: verbose
 *
 *  returns: codage binaire de l'adresse du label
 * ****************************************************************************/
uint32_t decode_asm(char *line, uint16_t line_number, bool verbose);
