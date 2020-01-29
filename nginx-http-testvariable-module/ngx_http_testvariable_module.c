
/*
 * Copyright (C) Jianyong Chen
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


static char *ngx_http_allow_in(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_allow_in_init(ngx_conf_t *cf);
static void *ngx_http_allow_in_create_loc_conf(ngx_conf_t *cf);
static ngx_int_t ngx_http_allow_in_handler(ngx_http_request_t *r);


typedef struct {
    int        variable_index;
    ngx_str_t  variable;
    ngx_str_t  target;
} ngx_http_allow_in_loc_conf_t;


static ngx_command_t  ngx_http_testvariable_commands[] = {

        { ngx_string("allow_in"),
          NGX_HTTP_LOC_CONF|NGX_CONF_TAKE2,
          ngx_http_allow_in,
          NGX_HTTP_LOC_CONF_OFFSET,
          0,
          NULL,
        },

        ngx_null_command,
};


static ngx_http_module_t  ngx_http_testvariable_module_ctx = {
        NULL,
        ngx_http_allow_in_init,
        NULL,
        NULL,
        NULL,
        NULL,
        ngx_http_allow_in_create_loc_conf,
        NULL
};


ngx_module_t  ngx_http_testvariable_module = {
        NGX_MODULE_V1,                          /* */
        &ngx_http_testvariable_module_ctx,         /* */
        ngx_http_testvariable_commands,
        NGX_HTTP_MODULE,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NGX_MODULE_V1_PADDING
};

static void *
ngx_http_allow_in_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_allow_in_loc_conf_t  *aicf;

    aicf = ngx_palloc(cf->pool, sizeof(ngx_http_allow_in_loc_conf_t));
    if (aicf == NULL) {
        return NULL;
    }

    aicf->variable_index = -1;

    return aicf;
}

static char *
ngx_http_allow_in(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t                     *value;
    ngx_http_allow_in_loc_conf_t  *aicf;

    aicf = conf;
    value = cf->args->elts;

    if (cf->args->nelts != 3) {
        return NGX_CONF_ERROR;
    }

    if (value[1].data[0] != '$') {
        return NGX_CONF_ERROR;
    }

    value[1].len--;
    value[1].data++;
    aicf->variable_index = ngx_http_get_variable_index(cf, &value[1]);
    if (aicf->variable_index == NGX_ERROR) {
        return NGX_CONF_ERROR;
    }
    aicf->variable = value[1];
    aicf->target = value[2];

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_allow_in_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_ACCESS_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_allow_in_handler;

    return NGX_OK;
}

static ngx_int_t
ngx_http_allow_in_handler(ngx_http_request_t *r)
{
    ngx_http_variable_value_t     *vv;
    ngx_http_allow_in_loc_conf_t  *aicf;

    aicf = ngx_http_get_module_loc_conf(r, ngx_http_testvariable_module);
    if (aicf == NULL) {
        return NGX_ERROR;
    }

    if (aicf->variable_index == -1) {
        return NGX_DECLINED;
    }

    vv = ngx_http_get_indexed_variable(r, aicf->variable_index);
    if (vv == NULL || vv->not_found) {
        return NGX_HTTP_FORBIDDEN;
    }

    if (vv->len == aicf->target.len
        && ngx_strncmp(aicf->target.data, vv->data, vv->len) == 0)
    {
        return NGX_DECLINED;
    }

    return NGX_HTTP_FORBIDDEN;
}