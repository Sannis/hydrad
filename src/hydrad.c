/* Hydra (c) 2013 Oleg Efimov */

#include <stdio.h>

#include "util.h"
#include "uv.h"

int main()
{
  uv_loop_t *loop = uv_loop_new();

  hlog_info("Now quitting");
  uv_run(loop, UV_RUN_DEFAULT);

  return 0;
}
