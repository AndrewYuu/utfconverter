#include "debug.h"
#include "utf.h"
#include "wrappers.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int
main(int argc, char *argv[])
{
  int infile, outfile, in_flags, out_flags;
  parse_args(argc, argv);
  check_bom();
  print_state();
  in_flags = O_RDONLY;
  out_flags = O_WRONLY | O_CREAT;
  infile = Open(program_state->in_file, in_flags);
  outfile = Open(program_state->out_file, out_flags);

  if(strcmp(program_state->in_file, program_state->out_file) == 0){
    return EXIT_FAILURE;
  }

  lseek(SEEK_SET, program_state->bom_length, infile); /* Discard BOM */
  get_encoding_function()(infile, outfile);
  if(program_state != NULL) {
    close((uintptr_t)program_state);
  }

  free((void*)program_state);
  // free((void*)infile);
  return EXIT_SUCCESS;
}
