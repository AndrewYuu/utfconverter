#include "debug.h"
#include "utf.h"
#include "wrappers.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int opterr;
int optopt;
int optind;
char *optarg;

const char *STR_UTF16BE  = "UTF16BE";
const char *STR_UTF16LE  = "UTF16LE";
const char *STR_UTF8  = "UTF8";

state_t *program_state;

void
parse_args(int argc, char *argv[])
{
  int i;
  char option;
  char *joined_argv;

  joined_argv = join_string_array(argc, argv);
  info("argc: %d argv: %s", argc, joined_argv);
  free(joined_argv);

  //EXTRA ARGS, RETURN ERROR
  if(argc > 5){
    exit(EXIT_FAILURE);
  }

  program_state = Calloc(1, sizeof(state_t));
  for (i = 0; optind < argc; ++i) {
    debug("%d opterr: %d", i, opterr);
    debug("%d optind: %d", i, optind);
    debug("%d optopt: %d", i, optopt);
    debug("%d argv[optind]: %s", i, argv[optind]);
    if ((option = getopt(argc, argv, "+e:h")) != -1) {
      switch (option) {
        case 'e': {
            info("Encoding Argument: %s", optarg);
            if ((program_state->encoding_to = determine_format(optarg)) == 0){ //IF (program_state.encoding_to = determine_format) == 0
                print_state();
            }
            break;
        }
        case '?': {
            if (optopt != 'h'){
                fprintf(stderr, KRED "-%c is not a supported argument\n" KNRM, optopt);
            }
            break;
        }
        case 'h': {
            USAGE(argv[0]);
            exit(0);
            break;
        }
        default: {
            break;
        }
      }
    }
    elsif(argv[optind] != NULL)
    {
      if (program_state->in_file == NULL) {
        program_state->in_file = argv[optind];
      }
      elsif(program_state->out_file == NULL)
      {
        program_state->out_file = argv[optind];
      }
      optind++;
    }
  }
  // free(joined_argv);
}

format_t
determine_format(char *argument)
{
  if (strcmp(argument, STR_UTF16LE) == 0) // IF ARGUMENT IS EQUAL TO THE STRING "UTF16LE", return UTF16LE for the encoding_to field of program_state.
    return UTF16LE;
  if (strcmp(argument, STR_UTF16BE) == 0)// IF ARGUMENT IS EQUAL TO THE STRING "UTF16BE", return UTF16BE for the encoding_to field of program_state.
    return UTF16BE;
  if (strcmp(argument, STR_UTF8) == 0)// IF ARGUMENT IS EQUAL TO THE STRING "UTF8", return UTF8 for the encoding_to field of program_state.
    return UTF8;
  return 0;
}

char*
bom_to_string(format_t bom){
    switch(bom){
        case UTF8:
            return (char *)STR_UTF8;
        case UTF16BE:
            return (char *)STR_UTF16BE;
        case UTF16LE:
            return (char *)STR_UTF16LE;
    }
  return "UNKNOWN";
}

char*
join_string_array(int count, char *array[]) //JOINS/CONCATENATES STRINGS IN THE ARRAY INTO ONE ENTIRE ARRAY WITH SPACES BETWEEN EACH STRING ELEMENT.
{
  char *ret;
  // char *charArray;
  int i;
  int len = 0,  str_len , cur_str_len;

  str_len = array_size(count, array);
  ret = malloc(str_len);

  // ret = charArray;

  for (i = 0; i < count; ++i) {

    char *arrayPointer = *(array+i);

    cur_str_len = strlen(array[i]);
    memecpy(ret + len, arrayPointer, cur_str_len);

    len += cur_str_len;
    memecpy(ret + len, " ", 1);
    len += 1;
  }

  return ret;
}

int
array_size(int count, char *array[]) //RETURN LENGTH OF ENTIRE ARRAY SIZE IN BYTES BY FINDING THE LENGTH OF EACH ELEMENT AND ADDING TO THE SUM.
{
  int i, sum = 1; /* NULL terminator */
  for (i = 0; i < count; ++i) {
    sum += strlen(array[i]);
    ++sum; /* For the spaces */
  }
  return sum+1;
}

void
print_state()
{
  if (program_state == NULL) {
    error("program_state is %p", (void*)program_state);
    exit(EXIT_FAILURE);
  }
  info("program_state {\n"
         "  format_t encoding_to = 0x%X;\n"
         "  format_t encoding_from = 0x%X;\n"
         "  char *in_file = '%s';\n"
         "  char *out_file = '%s';\n"
         "};\n",
         program_state->encoding_to, program_state->encoding_from,
         program_state->in_file, program_state->out_file);
}
