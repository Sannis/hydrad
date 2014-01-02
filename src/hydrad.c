/* Hydra (c) 2013 Oleg Efimov */

#include <stdio.h>
#include <unistd.h>

#include "util.h"
#include "uv.h"

#define HYDRAD_PORT 8888
#define HYDRAD_BACKLOG 128

#define HYDRAD_EXIT_SUCCESS 0
#define HYDRAD_EXIT_ADDRESS 1
#define HYDRAD_EXIT_BIND    2
#define HYDRAD_EXIT_LISTEN  3

void signal_handler(uv_signal_t *handle, int signum)
{
  hlog_info("Signal %d received, exiting", signum);
  uv_signal_stop(handle);
  exit(HYDRAD_EXIT_SUCCESS);
}

void on_connection_close(uv_handle_t* handle)
{
  hlog_debug("Connection closed");
}

void on_read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf)
{
  hlog_debug("Read bytes: %d", nread);
  if (nread >= 0) {
    //
  } else {
    if (nread != UV_EOF) {
      //UVERR(nread, "read");
    }
    uv_close((uv_handle_t*)tcp, on_connection_close);
  }
  free(buf->base);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t* buf)
{
    *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
}

void on_new_connection(uv_stream_t *server, int status)
{
  if (status != 0) {
    hlog_error("Cannot handle new connection: %s", uv_err_name(status));
    return;
  }

  uv_loop_t *loop = uv_default_loop();
  int err;

  hlog_debug("New connection");

  uv_tcp_t *client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, client);
  if ((err = uv_accept(server, (uv_stream_t*) client)) != 0) {
    hlog_error("Cannot accept connection: %s", uv_err_name(err));
    uv_close((uv_handle_t*)client, NULL);
    return;
  }

  if ((err = uv_read_start((uv_stream_t*)client, alloc_buffer, on_read)) != 0) {
    hlog_error("Cannot start reading data from client: %s", uv_err_name(err));
    uv_close((uv_handle_t*)client, NULL);
    return;
  }
}

int main()
{
  hlog_info("Starting...");

  uv_loop_t *loop = uv_default_loop();
  int err;

  hlog_info("Init signal handler...");
  uv_signal_t signal;
  uv_signal_init(loop, &signal);
  uv_signal_start(&signal, signal_handler, SIGINT);

  hlog_info("Init server...");
  uv_tcp_t server;
  uv_tcp_init(loop, &server);

  struct sockaddr_in address;
  if ((err = uv_ip4_addr("0.0.0.0", HYDRAD_PORT, &address)) != 0) {
    hlog_error("Wrong socket address: %s", uv_err_name(err));
    return HYDRAD_EXIT_ADDRESS;
  }
  if ((err = uv_tcp_bind(&server, (struct sockaddr*)&address)) != 0) {
    hlog_error("Cannot bind server: %s", uv_err_name(err));
    return HYDRAD_EXIT_BIND;
  }
  if ((err = uv_listen((uv_stream_t*)&server, HYDRAD_BACKLOG, on_new_connection)) != 0) {
    hlog_error("Cannot start listening: %s", uv_err_name(err));
    return HYDRAD_EXIT_LISTEN;
  }

  hlog_info("Enter event loop");
  uv_run(loop, UV_RUN_DEFAULT);

  return HYDRAD_EXIT_SUCCESS;
}
