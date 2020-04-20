
/*
 * Copyright (C) Jianyong Chen
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


static void *ngx_http_testslab_create_loc_conf(ngx_conf_t *cf, void *);

ngx_command_t ngx_http_testslab_commands[] = {

    { ngx_string("test_slab"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE2,
      NULL,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL,},

    ngx_null_command
};


ngx_http_module_t ngx_http_testslab_module_ctx = {
        NULL,                           /* preconfiguration */
        NULL,                           /* postconfiguration */

        NULL,                           /* create main configuration */
        NULL,                           /* init main configuration */

        NULL,                           /* create server configuration */
        NULL,                           /* merge server configuration */

        NULL,                           /* create location configuration */
        NULL,                           /* merge location configuration */
};

ngx_module_t ngx_http_testslab_module = {
        NGX_MODULE_V1,
        &ngx_http_testslab_module_ctx,  /* module context */
        ngx_http_testslab_commands,     /* module directives */
        NGX_HTTP_MODULE,                /* module type */
        NULL,                           /* init master */
        NULL,                           /* init module */
        NULL,                           /* init process */
        NULL,                           /* init thread */
        NULL,                           /* exit thread */
        NULL,                           /* exit process */
        NULL,                           /* exit master */
        NGX_MODULE_V1_PADDING,
};

static ngx_http_variable_t ngx_http_testslab_vars[] = {
        ngx_http_null_variable,
};
