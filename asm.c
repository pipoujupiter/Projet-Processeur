#include "asm2bin.h"
#include "asm.h"
#include "labels.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

/* **************************************************************
 * Types et constantes
 * **************************************************************/

// Constantes de décalage
#define BIN_SHIFT_TYP 0
#define BIN_SHIFT_OP 2
#define BIN_SHIFT_IMM 5
#define BIN_SHIFT_DR 6
#define BIN_SHIFT_R1 9
#define BIN_SHIFT_R2 12
#define BIN_SHIFT_CST 16

// Constantes de type d'instruction
#define BIN_TYP_ALU ((uint32_t)0 << BIN_SHIFT_TYP)
#define BIN_TYP_MEM ((uint32_t)1 << BIN_SHIFT_TYP)
#define BIN_TYP_IO ((uint32_t)2 << BIN_SHIFT_TYP)
#define BIN_TYP_CTL ((uint32_t)3 << BIN_SHIFT_TYP)

// Constantes de bits d'instruction
#define BIN_IMM ((uint32_t)1 << BIN_SHIFT_IMM)

// Enumération des types d'arguments
typedef enum
{
  ASM_ARG_DR_R1_R2oCST,
  ASM_ARG_DR_R1,
  ASM_ARG_R1_DR,
  ASM_ARG_LBL,
  ASM_ARG_R1_R2_LBL,
  ASM_ARG_NONE
} ASM_ARG_TYPE;

// Type pour la définition d'une instruction
typedef struct asm_instruction_t
{
  const char *operation; /* instruction assembleur */
  ASM_ARG_TYPE args;     /* arguments assembleur */
  uint32_t bits;         /* bits processeur */
} asm_instruction_t;

// tableau des instructions
static const asm_instruction_t asm_instructions[] = {
    {"ADD", ASM_ARG_DR_R1_R2oCST, BIN_TYP_ALU | (0 << BIN_SHIFT_OP)},
    {"SUB", ASM_ARG_DR_R1_R2oCST, BIN_TYP_ALU | (1 << BIN_SHIFT_OP)},
    {"AND", ASM_ARG_DR_R1_R2oCST, BIN_TYP_ALU | (2 << BIN_SHIFT_OP)},
    {"OR", ASM_ARG_DR_R1_R2oCST, BIN_TYP_ALU | (3 << BIN_SHIFT_OP)},
    {"XOR", ASM_ARG_DR_R1_R2oCST, BIN_TYP_ALU | (4 << BIN_SHIFT_OP)},
    {"SL", ASM_ARG_DR_R1_R2oCST, BIN_TYP_ALU | (5 << BIN_SHIFT_OP)},
    {"SR", ASM_ARG_DR_R1_R2oCST, BIN_TYP_ALU | (6 << BIN_SHIFT_OP)},
    {"MUL", ASM_ARG_DR_R1_R2oCST, BIN_TYP_ALU | (7 << BIN_SHIFT_OP)},
    {"LD", ASM_ARG_DR_R1, BIN_TYP_MEM | (0 << BIN_SHIFT_OP)},
    {"STR", ASM_ARG_R1_DR, BIN_TYP_MEM | (1 << BIN_SHIFT_OP)},
    {"JMP", ASM_ARG_LBL, BIN_TYP_CTL | (0 << BIN_SHIFT_OP)},
    {"JEQU", ASM_ARG_R1_R2_LBL, BIN_TYP_CTL | (1 << BIN_SHIFT_OP)},
    {"JNEQ", ASM_ARG_R1_R2_LBL, BIN_TYP_CTL | (2 << BIN_SHIFT_OP)},
    {"JSUP", ASM_ARG_R1_R2_LBL, BIN_TYP_CTL | (3 << BIN_SHIFT_OP)},
    {"JINF", ASM_ARG_R1_R2_LBL, BIN_TYP_CTL | (4 << BIN_SHIFT_OP)},
    {"CALL", ASM_ARG_LBL, BIN_TYP_CTL | (5 << BIN_SHIFT_OP)},
    {"RET", ASM_ARG_NONE, BIN_TYP_CTL | (6 << BIN_SHIFT_OP)},
    {"END", ASM_ARG_NONE, BIN_TYP_CTL | (7 << BIN_SHIFT_OP)}};
// taille du tableau des instructions
static const size_t asm_instructions_count = sizeof(asm_instructions)/sizeof(asm_instruction_t);

// Type pour la définition d'un registre
typedef struct asm_register_t
{
  const char *name;
  uint8_t bin;
} asm_register_t;

// tableau des registress
static const asm_register_t asm_registers[] = {
    {"R0", (uint32_t)0},
    {"R1", (uint32_t)1},
    {"R2", (uint32_t)2},
    {"R3", (uint32_t)3},
    {"R4", (uint32_t)4},
    {"R5", (uint32_t)5},
    {"R6", (uint32_t)6},
    {"R7", (uint32_t)7}};
// taille du tableau des registres
static const size_t asm_registers_count = sizeof(asm_registers)/sizeof(asm_register_t);

/* *****************************************************************************
 * Function:  get_arg_R 
 * --------------------
 * decode un argument de type registre
 * interrompt le programme en cas d'erreur de syntaxe ou de registre inconnu
 *
 *  buffer: chaine à analyser
 *  verbose: verbose
 *
 *  returns: codage binaire de l'argument registre
 * ****************************************************************************/
uint32_t get_arg_R(char *buffer, bool verbose)
{
  uint8_t i = 0;

  if (NULL == buffer)
  {
    print_error("expexted register argument not present\n");
    exit(EXIT_FAILURE);
    // Not reached
  }

  for (; i < asm_registers_count ; i++)
  {
    if (0 == strcmp(buffer, asm_registers[i].name))
    {
      if (verbose)
        printf("\t%s:0x%X", buffer, asm_registers[i].bin);

      return asm_registers[i].bin;
      // Not reached
    }
  }

  print_error("argument '%s' is not a valid register\n", buffer);
  exit(EXIT_FAILURE);
}

/* *****************************************************************************
 * Function:  get_arg_C
 * --------------------
 * decode un argument de type constante
 * la valeur doit être un entier signé sur 16 bits 
 * interrompt le programme en cas d'erreur de syntaxe ou de valuer
 *
 *  buffer: chaine à analyser
 *  verbose: verbose
 *
 *  returns: codage binaire de l'argument constante
 * ****************************************************************************/
uint16_t get_arg_C(char *buffer, bool verbose)
{
  long value = 0;
  char *end;

  if (NULL == buffer)
  {
    print_error("expexted constant argument not present\n");
    exit(EXIT_FAILURE);
    // Not reached
  }

  value = strtol(buffer, &end, 10);
  if (end == buffer || '\0' != *end)
  {
    print_error("'%s' is not a valid constant\n", buffer);
    exit(EXIT_FAILURE);
    // Not reached
  }

  if (INT16_MAX < value || INT16_MIN > value)
  {
    print_error("'%s' constant is out ouf bound\n", buffer);
    exit(EXIT_FAILURE);
    // Not reached
  }

  if (verbose)
    printf("\t%ld:0x%04X", value, (uint16_t)value);

  return (uint16_t)value;
}

/* *****************************************************************************
 * Function:  get_arg_L
 * --------------------
 * decode un argument de type label
 * la valeur doit être connue du tableau des labels
 * interrompt le programme en cas d'erreur de syntaxe ou de label
 *
 *  buffer: chaine à analyser
 *  verbose: verbose
 *
 *  returns: codage binaire de l'adresse du label
 * ****************************************************************************/
uint16_t get_arg_L(char *buffer, bool verbose)
{
  uint16_t addr = 0;

  if (NULL == buffer)
  {
    print_error("expexted label argument not present\n");
    exit(EXIT_FAILURE);
    // Not reached
  }

  addr = get_label_addr(buffer);
  if (UINT16_MAX == addr)
  {
    print_error("'%s' is not a valid label\n", buffer);
    exit(EXIT_FAILURE);
    // Not reached
  }

  if (verbose)
    printf("\t%s%04u", buffer, addr);

  return addr;
}

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
uint32_t decode_asm(char *line, uint16_t line_number, bool verbose)
{
  uint32_t i;
  uint32_t bin = 0;
  char *buffer;

  if (verbose)
    printf("%04u:", line_number);

  line = strtok(line, ASM_SEPS);
  if (NULL == line)
  {
    print_error("no instruction\n");
    exit(EXIT_FAILURE);
    // Not reached
  }

  // ignore les labels en début de ligne
  if (is_label(line) &&
      NULL == (line = strtok(NULL, ASM_SEPS)))
  {
    print_error("no instruction after label\n");
    exit(EXIT_FAILURE);
    // Not reached
  }

  // Traitement du type d'operation
  for (i = 0; i < asm_instructions_count; i++)
  {
    if (0 == strcmp(line, asm_instructions[i].operation))
    {
      bin = asm_instructions[i].bits;

      if (verbose)
        printf("\t%s:0x%02X", line, bin);
      
      break;  
      // Not reached
    }
  }

  if (asm_instructions_count == i)
  {
    print_error("invalid instruction '%s'\n", line);
    exit(EXIT_FAILURE);
    // Not reached
  }

  // Traitement des arguments
  switch (asm_instructions[i].args)
  {
  case ASM_ARG_DR_R1_R2oCST:
    bin |= get_arg_R(strtok(NULL, ASM_SEPS), verbose) << BIN_SHIFT_DR |
           get_arg_R(strtok(NULL, ASM_SEPS), verbose) << BIN_SHIFT_R1;

    buffer = strtok(NULL, ASM_SEPS);
    if (NULL == buffer) {
      print_error("expexted register or constant argument not present\n");
      exit(EXIT_FAILURE);
      // Not reached
    }
    
    if ('-' == *buffer || isdigit(*buffer)) {
      bin |= BIN_IMM | (get_arg_C(buffer, verbose) << BIN_SHIFT_CST);
    }
    else{
      bin |= get_arg_R(buffer, verbose) << BIN_SHIFT_R2;
    }
    
    break;
    // Not reached
  case ASM_ARG_DR_R1:
    bin |= get_arg_R(strtok(NULL, ASM_SEPS), verbose) << BIN_SHIFT_DR |
           get_arg_R(strtok(NULL, ASM_SEPS), verbose) << BIN_SHIFT_R1;
    break;
    // Not reached
  case ASM_ARG_R1_DR:
    bin |= get_arg_R(strtok(NULL, ASM_SEPS), verbose) << BIN_SHIFT_R1 |
           get_arg_R(strtok(NULL, ASM_SEPS), verbose) << BIN_SHIFT_DR;
    break;
    // Not reached
  case ASM_ARG_LBL:
    bin |= get_arg_L(strtok(NULL, ASM_SEPS), verbose) << BIN_SHIFT_CST;
    break;
    // Not reached
  case ASM_ARG_R1_R2_LBL:
    bin |= get_arg_R(strtok(NULL, ASM_SEPS), verbose) << BIN_SHIFT_R1 |
           get_arg_R(strtok(NULL, ASM_SEPS), verbose) << BIN_SHIFT_R2 |
           get_arg_L(strtok(NULL, ASM_SEPS), verbose) << BIN_SHIFT_CST;
    break;
    // Not reached
  case ASM_ARG_NONE:
    break;
    // Not reached
  default:
    // Should not reach
    print_error("!! invalid instruction arg_type value: this is a bug !!\n");
    exit(EXIT_FAILURE);
    // Not reached
  }

  // ignore uniquement les commentaires en fin de ligne
  if (NULL != (line = strtok(NULL, ASM_SEPS)) && ASM_COMMENT_CAR != *line)
  {
    print_error("too many args");
    exit(EXIT_FAILURE);
    // Not reached
  }

  if (verbose)
    printf("\n  0x%08u\n", bin);

  return bin;
}
