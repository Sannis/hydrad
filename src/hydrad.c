/* Hydra (c) 2013 Oleg Efimov */

#include <stdio.h>
#include <unistd.h>

#include "util.h"
#include "uv.h"

#define HYDRAD_PORT 88888

void signal_handler(uv_signal_t *handle, int signum)
{
  hlog_info("Signal %d received, exiting", signum);
  uv_signal_stop(handle);
  exit(0);
}

void on_new_connection(uv_stream_t *server, int status) {
  if (status == -1) {
      return;
  }

  uv_loop_t *loop = uv_default_loop();

  uv_tcp_t *client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, client);
  //if (uv_accept(server, (uv_stream_t*) client) == 0) {
  //    uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
  //} else {
      uv_close((uv_handle_t*)client, NULL);
  //}
}

int main()
{
  hlog_info("Starting...");

  uv_loop_t *loop = uv_default_loop();

  hlog_info("Init signal handler...");
  uv_signal_t signal;
  uv_signal_init(loop, &signal);
  uv_signal_start(&signal, signal_handler, SIGINT);

  hlog_info("Init server...");
  uv_tcp_t server;
  uv_tcp_init(loop, &server);

  int err;

  struct sockaddr_in address;
  if ((err = uv_ip4_addr("0.0.0.0", HYDRAD_PORT, &address)) != 0) {
    hlog_error("Wrong socket address: %s", uv_err_name(err));
    return 1;
  }
  if ((err = uv_tcp_bind(&server, (struct sockaddr*)&address)) != 0) {
    hlog_error("Cannot bind server: %s", uv_err_name(err));
    return 2;
  }
  if ((err = uv_listen((uv_stream_t*) &server, 128, on_new_connection)) != 0) {
    hlog_error("Cannot start listening: %s", uv_err_name(err));
    return 3;
  }

  hlog_info("Enter event loop");
  uv_run(loop, UV_RUN_DEFAULT);

  return 0;
}
