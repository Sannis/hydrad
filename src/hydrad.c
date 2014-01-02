/* Hydra (c) 2013 Oleg Efimov */

#include <stdio.h>
#include <unistd.h>

#include "util.h"
#include "uv.h"
#include "buffer.h"

#define HYDRAD_VERSION "0.0.1"

#define HYDRAD_PORT 8888
#define HYDRAD_BACKLOG 128

#define HYDRAD_EXIT_SUCCESS 0
#define HYDRAD_EXIT_ADDRESS 1
#define HYDRAD_EXIT_BIND    2
#define HYDRAD_EXIT_LISTEN  3

#define HYDRAD_ERROR_LOW_MEMORY     1
#define HYDRAD_ERROR_BAD_REQUEST    2
#define HYDRAD_ERROR_UNKNOWN_METHOD 3

static uv_loop_t *uv_loop;

static unsigned int request_num = 1;

typedef struct {
  unsigned int request_num;
  uv_tcp_t client;

  buffer_t* request_buffer;
  buffer_t* request_method_buffer;
  buffer_t* request_params_buffer;
} req_res_t;

void signal_handler(uv_signal_t *handle, int signum);
void on_new_connection(uv_stream_t *server, int status);
void on_read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf);
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t* buf);
void process_request(req_res_t* req_res);
void process_request_version(req_res_t* req_res);
void send_error_response(req_res_t* req_res, unsigned int error_code, const char* error_message);
void on_connection_close(uv_handle_t* handle);

int main()
{
  hlog_info("Starting...");

  uv_loop = uv_default_loop();

  int err;

  hlog_info("Init signal handler...");
  uv_signal_t signal;
  uv_signal_init(uv_loop, &signal);
  uv_signal_start(&signal, signal_handler, SIGINT);

  hlog_info("Init server...");
  uv_tcp_t server;
  uv_tcp_init(uv_loop, &server);

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
  uv_run(uv_loop, UV_RUN_DEFAULT);

  return HYDRAD_EXIT_SUCCESS;
}

void signal_handler(uv_signal_t *handle, int signum)
{
  hlog_info("Signal %d received, exiting", signum);
  uv_signal_stop(handle);
  exit(HYDRAD_EXIT_SUCCESS);
}

void on_new_connection(uv_stream_t *server, int status)
{
  if (status != 0) {
    hlog_error("Cannot handle new connection: %s", uv_err_name(status));
    return;
  }

  int err;

  req_res_t* req_res = (req_res_t*)malloc(sizeof(req_res_t));
  req_res->request_num = request_num;
  request_num++;

  hlog_debug("[req_res=%u] New connection", req_res->request_num);

  uv_tcp_init(uv_loop, &req_res->client);
  req_res->client.data = req_res;

  // TODO: Causes segfault, possible libuv bug
  //if (!(req_res->request_buffer = 0)) {
  if (!(req_res->request_buffer = buffer_new())) {
    hlog_error("[req_res=%u] Cannot create request_buffer with buffer_new()", req_res->request_num);
    uv_close((uv_handle_t*)&req_res->client, NULL);
    free(req_res);
    return;
  }

  if ((err = uv_accept(server, (uv_stream_t*)&req_res->client)) != 0) {
    hlog_error("[req_res=%u] Cannot accept connection: %s", req_res->request_num, uv_err_name(err));
    uv_close((uv_handle_t*)&req_res->client, NULL);
    buffer_free(req_res->request_buffer);
    free(req_res);
    return;
  }

  if ((err = uv_read_start((uv_stream_t*)&req_res->client, alloc_buffer, on_read)) != 0) {
    hlog_error("[req_res=%u] Cannot start reading data from client: %s", req_res->request_num, uv_err_name(err));
    uv_close((uv_handle_t*)&req_res->client, NULL);
    buffer_free(req_res->request_buffer);
    free(req_res);
    return;
  }
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t* buf)
{
    *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
}

void on_read(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf)
{
  req_res_t* req_res = (req_res_t*)client->data;

  if (nread >= 0) {
    hlog_debug("[req_res=%u] Read %d bytes: %s", req_res->request_num, nread, buf->base);

    buffer_append(req_res->request_buffer, buf->base);
  } else {
    if (nread != UV_EOF) {
      hlog_error("[req_res=%u] Not EOF, read error: %s", req_res->request_num, uv_err_name(nread));
    } else {
      hlog_debug("[req_res=%u] EOF", req_res->request_num);
    }

    hlog_debug("[req_res=%u] Full request (%d bytes): %s", req_res->request_num, buffer_length(req_res->request_buffer), buffer_string(req_res->request_buffer));

    process_request(req_res);
  }
  free(buf->base);
}

void process_request(req_res_t* req_res)
{
  // Split request into method and params
  ssize_t space_position = buffer_indexof(req_res->request_buffer, " ");
  if (space_position <= 0) {
    send_error_response(req_res, HYDRAD_ERROR_BAD_REQUEST, "Wrong request: no space separator");
    return;
  }

  req_res->request_method_buffer = buffer_slice(req_res->request_buffer, 0, space_position);
  if (!req_res->request_method_buffer) {
    send_error_response(req_res, HYDRAD_ERROR_LOW_MEMORY, "No memory");
    return;
  }

  req_res->request_params_buffer = buffer_slice(req_res->request_buffer, space_position + 1, buffer_length(req_res->request_buffer));
  if (!req_res->request_params_buffer) {
    send_error_response(req_res, HYDRAD_ERROR_LOW_MEMORY, "No memory");
    return;
  }

  hlog_debug("[req_res=%u] Request method (%d bytes): %s", req_res->request_num, buffer_length(req_res->request_method_buffer), buffer_string(req_res->request_method_buffer));
  hlog_debug("[req_res=%u] Request params (%d bytes): %s", req_res->request_num, buffer_length(req_res->request_params_buffer), buffer_string(req_res->request_params_buffer));

  // Handle request method
  if (0 == strcmp(buffer_string(req_res->request_method_buffer), "version")) {
    process_request_version(req_res);
  } else {
    send_error_response(req_res, HYDRAD_ERROR_UNKNOWN_METHOD, "Unknown method");
  }
}

void process_request_version(req_res_t* req_res)
{
  hlog_info("[req_res=%u] Version %s", req_res->request_num, HYDRAD_VERSION);

  uv_close((uv_handle_t*)&req_res->client, on_connection_close);
}

void send_error_response(req_res_t* req_res, unsigned int error_code, const char* error_message)
{
  hlog_error("[req_res=%u] Error response %u: %s", req_res->request_num, error_code, error_message);
  uv_close((uv_handle_t*)&req_res->client, on_connection_close);
}

void on_connection_close(uv_handle_t* client)
{
  req_res_t* req_res = (req_res_t*)client->data;

  hlog_debug("[req_res=%u] Connection closed", req_res->request_num);

  buffer_free(req_res->request_buffer);
  if (req_res->request_method_buffer) {
    buffer_free(req_res->request_method_buffer);
  }
  if (req_res->request_params_buffer) {
    buffer_free(req_res->request_params_buffer);
  }
  free(req_res);
}
