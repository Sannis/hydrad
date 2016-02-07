/*
 * Copyright (c) 2013-2016 Oleg Efimov <efimovov@gmail.com>
 *
 * hydrad is free software; you can redistribute it
 * and/or modify it under the terms of the MIT license.
 * See LICENSE for details.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <uv.h>
#include <protobuf2json.h>

#include "util.h"
#include "buffer.h"

#include "hydrad.pb-c.h"

#define HYDRAD_VERSION "0.0.1-dev"

#define HYDRAD_PORT 8888
#define HYDRAD_BACKLOG 128

#define HYDRAD_EXIT_SUCCESS 0
#define HYDRAD_EXIT_ADDRESS 1
#define HYDRAD_EXIT_BIND    2
#define HYDRAD_EXIT_LISTEN  3

#define HYDRAD_ERROR_LOW_MEMORY     1
#define HYDRAD_ERROR_BAD_REQUEST    2
#define HYDRAD_ERROR_UNKNOWN_METHOD 3
#define HYDRAD_ERROR_SYSTEM         4

static uv_loop_t *uv_loop;

typedef struct {
  double uptime_start;
  struct {
    unsigned int total_count;
    unsigned int failed_count;
    unsigned int error_count;
  } requests;
} hydra_daemon_t;

static hydra_daemon_t H;

typedef struct {
  unsigned int request_num;

  uv_tcp_t client;

  buffer_t* request_buffer;
  buffer_t* request_method_buffer;
  buffer_t* request_params_buffer;

  uv_write_t write_req;

  buffer_t* response_buffer;
} req_res_t;

void signal_handler(uv_signal_t *handle, int signum);
void on_new_connection(uv_stream_t *server, int status);
void on_read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf);
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t* buf);

void process_request(req_res_t* req_res);
void process_request_version(req_res_t* req_res);
void process_request_stats(req_res_t* req_res);

void send_error_response(req_res_t* req_res, unsigned int error_code, char* error_message);
void send_response(req_res_t* req_res);
void after_response_sent(uv_write_t* req, int status);
void on_connection_close(uv_handle_t* handle);

int main()
{
  // Populate default loop global variable
  uv_loop = uv_default_loop();

  hlog_info("Starting...");
  struct timeval uptime_start_timeval;
  gettimeofday(&uptime_start_timeval, NULL);
  H.uptime_start = (double)uptime_start_timeval.tv_sec + (double)uptime_start_timeval.tv_usec / 1e6;

  hlog_info("Init signal handler...");
  uv_signal_t signal;
  uv_signal_init(uv_loop, &signal);
  uv_signal_start(&signal, signal_handler, SIGINT);

  hlog_info("Init server...");
  uv_tcp_t server;
  uv_tcp_init(uv_loop, &server);

  int err;

  struct sockaddr_in address;
  if ((err = uv_ip4_addr("0.0.0.0", HYDRAD_PORT, &address)) != 0) {
    hlog_error("Wrong socket address: %s", uv_err_name(err));
    return HYDRAD_EXIT_ADDRESS;
  }
  if ((err = uv_tcp_bind(&server, (struct sockaddr*)&address, 0)) != 0) {
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
  unsigned int request_num = __sync_add_and_fetch(&H.requests.total_count, 1);

  if (status != 0) {
    __sync_add_and_fetch(&H.requests.failed_count, 1);
    hlog_error("[req_res=%u] Cannot handle new connection: %s", request_num, uv_err_name(status));
    return;
  }

  int err;

  req_res_t* req_res = (req_res_t*)malloc(sizeof(req_res_t));
  req_res->request_num = request_num;

  hlog_debug("[req_res=%u] Init new connection", req_res->request_num);

  uv_tcp_init(uv_loop, &req_res->client);
  req_res->client.data = req_res;

  // TODO: Causes segfault, possible libuv bug
  //if (!(req_res->request_buffer = 0)) {
  if (!(req_res->request_buffer = buffer_new())) {
    __sync_add_and_fetch(&H.requests.failed_count, 1);
    hlog_error("[req_res=%u] Cannot create request_buffer with buffer_new()", req_res->request_num);
    uv_close((uv_handle_t*)&req_res->client, NULL);
    free(req_res);
    return;
  }

  if ((err = uv_accept(server, (uv_stream_t*)&req_res->client)) != 0) {
    __sync_add_and_fetch(&H.requests.failed_count, 1);
    hlog_error("[req_res=%u] Cannot accept connection: %s", req_res->request_num, uv_err_name(err));
    uv_close((uv_handle_t*)&req_res->client, NULL);
    buffer_free(req_res->request_buffer);
    free(req_res);
    return;
  }

  if ((err = uv_read_start((uv_stream_t*)&req_res->client, alloc_buffer, on_read)) != 0) {
    __sync_add_and_fetch(&H.requests.failed_count, 1);
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
     // TODO: How to handle this case?
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
    send_error_response(req_res, HYDRAD_ERROR_BAD_REQUEST, "Bad request");
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
  } else if (0 == strcmp(buffer_string(req_res->request_method_buffer), "stats")) {
    process_request_stats(req_res);
  } else {
    send_error_response(req_res, HYDRAD_ERROR_UNKNOWN_METHOD, "Unknown method");
  }
}

void process_request_version(req_res_t* req_res)
{
  const char* uv_version = uv_version_string();

  Hydrad__ResponseVersion response_version = HYDRAD__RESPONSE_VERSION__INIT;

  // Hydrad version
  response_version.version = HYDRAD_VERSION;

  // Dependencies version
  Hydrad__ResponseVersion__DepsT deps = HYDRAD__RESPONSE_VERSION__DEPS_T__INIT;

  deps.uv_version = (char *)calloc(sizeof(char), strlen(uv_version) + 1);
  strcpy(deps.uv_version, uv_version);

  response_version.deps = &deps;

  // Response
  char *json_string;
  int result = protobuf2json_string(&response_version.base, 0, &json_string, NULL, 0);
  if (result != 0) {
    send_error_response(req_res, HYDRAD_ERROR_SYSTEM, "Failed to serialize JSON");
    return;
  }

  req_res->response_buffer = buffer_new_with_copy(json_string/*, len*/);
  free(json_string);

  send_response(req_res);
}

void process_request_stats(req_res_t* req_res)
{
  // Get uptime
  struct timeval uptime_current_timeval;
  gettimeofday(&uptime_current_timeval, NULL);
  double uptime = (double)uptime_current_timeval.tv_sec + (double)uptime_current_timeval.tv_usec / 1e6 - H.uptime_start;

  // TODO: Code for Windows, pull-request to libuv
  // http://stackoverflow.com/questions/5272470/c-get-cpu-usage-on-linux-and-windows
  // http://stackoverflow.com/questions/669438/how-to-get-memory-usage-at-run-time-in-c

  int err;

  // Get RSS
  size_t rss;
  err = uv_resident_set_memory(&rss);
  if (err != 0) {
    send_error_response(req_res, HYDRAD_ERROR_SYSTEM, "Failed to get RSS");
    return;
  }

  // Get rusage
  struct rusage usage;
  err = getrusage(RUSAGE_SELF, &usage);
  if (err != 0) {
    send_error_response(req_res, HYDRAD_ERROR_SYSTEM, "Failed to get rusage");
    return;
  }

  Hydrad__ResponseStats response_stats = HYDRAD__RESPONSE_STATS__INIT;

  // Uptime
  response_stats.uptime = (int)uptime;

  // Resources usage stats
  Hydrad__ResponseStats__RusageT rusage = HYDRAD__RESPONSE_STATS__RUSAGE_T__INIT;

  rusage.rss = rss;
  rusage.ru_maxrss = usage.ru_maxrss;
  rusage.ru_stime = (double)usage.ru_stime.tv_sec + (double)usage.ru_stime.tv_usec / 1e6;
  rusage.ru_utime = (double)usage.ru_utime.tv_sec + (double)usage.ru_utime.tv_usec / 1e6;

  response_stats.rusage = &rusage;

  // Requests stats
  Hydrad__ResponseStats__RequestsT requests = HYDRAD__RESPONSE_STATS__REQUESTS_T__INIT;

  requests.total_count = H.requests.total_count;
  requests.failed_count = H.requests.failed_count;
  requests.error_count = H.requests.error_count;

  response_stats.requests = &requests;

  // Response
  char *json_string;
  int result = protobuf2json_string(&response_stats.base, 0, &json_string, NULL, 0);
  if (result != 0) {
    send_error_response(req_res, HYDRAD_ERROR_SYSTEM, "Failed to serialize JSON");
    return;
  }

  req_res->response_buffer = buffer_new_with_copy(json_string/*, len*/);
  free(json_string);

  send_response(req_res);
}

void send_error_response(req_res_t* req_res, unsigned int error_code, char* error_message)
{
  __sync_add_and_fetch(&H.requests.error_count, 1);

  Hydrad__ResponseError response_error = HYDRAD__RESPONSE_ERROR__INIT;

  response_error.error_code = error_code;
  response_error.error_message = error_message;

  char *json_string;
  int result = protobuf2json_string(&response_error.base, 0, &json_string, NULL, 0);
  if (result != 0) {
    // TODO: what to do?
  }

  req_res->response_buffer = buffer_new_with_copy(json_string/*, len*/);
  free(json_string);

  send_response(req_res);
}

void send_response(req_res_t* req_res)
{
  hlog_debug("[req_res=%u] Response (%d bytes): %s", req_res->request_num, buffer_length(req_res->response_buffer), buffer_string(req_res->response_buffer));

  uv_buf_t buf = uv_buf_init(buffer_string(req_res->response_buffer), buffer_length(req_res->response_buffer));

  req_res->write_req.data = req_res;

  int err = uv_write(&req_res->write_req, (uv_stream_t*)&req_res->client, &buf, 1, after_response_sent);
  if (err != 0) {
    hlog_error("[req_res=%u] Cannot start writing response", req_res->request_num);
    uv_close((uv_handle_t*)&req_res->client, on_connection_close);
  }
}

void after_response_sent(uv_write_t* req, int status)
{
  req_res_t* req_res = (req_res_t*)req->data;

  if (status != 0) {
    hlog_error("[req_res=%u] Cannot write response chunk", req_res->request_num, uv_err_name(status));
  }

  if (!uv_is_closing((uv_handle_t*)&req_res->client)) {
      uv_close((uv_handle_t*)&req_res->client, on_connection_close);
  }
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
  if (req_res->response_buffer) {
    buffer_free(req_res->response_buffer);
  }
  free(req_res);
}
