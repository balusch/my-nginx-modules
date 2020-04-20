
/*
 * Copyright (C) Jianyong Chen
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <sys/inotify.h>


typedef struct {
    ngx_flag_t                   only_dir;
    ngx_int_t                    buffer_size;
    ngx_int_t                    max_buffer_size;
    ngx_array_t                 *paths;          /* ngx_str_t */
} ngx_http_inotify_main_conf_t ;



static ngx_int_t ngx_http_inotify_init_process(ngx_cycle_t *cycle);
static void ngx_http_inotify_read_handler(ngx_event_t *e);
static void ngx_http_inotify_dump_event(ngx_log_t *log, struct inotify_event *ie);

static ngx_int_t ngx_http_inotify_postconfiguration(ngx_conf_t *cf);
static char *ngx_http_inotify(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void *ngx_http_inotify_create_main_conf(ngx_conf_t *cf);


ngx_command_t  ngx_http_inotify_commands[] = {

        { ngx_string("inotify"),
          NGX_HTTP_MAIN_CONF|NGX_CONF_NOARGS,
          ngx_http_inotify,
          NGX_HTTP_MAIN_CONF_OFFSET,
          0,
          NULL },

        { ngx_string("inotify_add_watch"),
          NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
          ngx_conf_set_str_array_slot,
          NGX_HTTP_MAIN_CONF_OFFSET,
          offsetof(ngx_http_inotify_main_conf_t, paths),
          NULL },

        { ngx_string("inotify_buffer_size"),
          NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
          ngx_conf_set_size_slot,
          NGX_HTTP_MAIN_CONF_OFFSET,
          offsetof(ngx_http_inotify_main_conf_t, buffer_size),
          NULL },

        { ngx_string("inotify_max_buffer_size"),
          NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
          ngx_conf_set_size_slot,
          NGX_HTTP_MAIN_CONF_OFFSET,
          offsetof(ngx_http_inotify_main_conf_t, max_buffer_size),
          NULL },

        { ngx_string("inotify_onlydir"),
          NGX_HTTP_MAIN_CONF|NGX_CONF_FLAG,
          ngx_conf_set_flag_slot,
          NGX_HTTP_MAIN_CONF_OFFSET,
          offsetof(ngx_http_inotify_main_conf_t, only_dir),
          NULL },

        ngx_null_command
};

ngx_http_module_t  ngx_http_inotify_module_ctx = {
        NULL,                               /* preconfiguration */
        ngx_http_inotify_postconfiguration, /* postconfiguration */

        ngx_http_inotify_create_main_conf,  /* create main configuration */
        NULL,                               /* init main configuration */

        NULL,                               /* create server configuration */
        NULL,                               /* merge server configuration */

        NULL,                               /* create location configuration */
        NULL,                               /* merge location configuration */
};


ngx_module_t ngx_http_inotify_module = {
        NGX_MODULE_V1,
        &ngx_http_inotify_module_ctx,       /* module context */
        ngx_http_inotify_commands,          /* module directives */
        NGX_HTTP_MODULE,                    /* module type */
        NULL,                               /* init master */
        NULL,                               /* init module */
        ngx_http_inotify_init_process,      /* init process */
        NULL,                               /* init thread */
        NULL,                               /* exit thread */
        NULL,                               /* exit process */
        NULL,                               /* exit master */
        NGX_MODULE_V1_PADDING,
};


static ngx_int_t
ngx_http_inotify_handler(ngx_http_request_t *r)
{
    ngx_http_inotify_main_conf_t  *imcf;

    imcf = ngx_http_get_module_main_conf(r, ngx_http_inotify_module);
    if (imcf == NULL) {
        return NGX_ERROR;
    }

    if (imcf->paths == NGX_CONF_UNSET_PTR) {
        return NGX_OK;
    }

    // TODO
    return NGX_OK;
}

static char *
ngx_http_inotify(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_inotify_handler;

    return NGX_CONF_OK;
}

static void *
ngx_http_inotify_create_main_conf(ngx_conf_t *cf)
{
    ngx_http_inotify_main_conf_t  *imcf;

    imcf = ngx_palloc(cf->pool, sizeof(ngx_http_inotify_main_conf_t));
    if (imcf == NULL) {
        return NULL;
    }

    imcf->only_dir = NGX_CONF_UNSET;
    imcf->buffer_size = NGX_CONF_UNSET_SIZE;
    imcf->max_buffer_size = NGX_CONF_UNSET_SIZE;
    imcf->paths = NGX_CONF_UNSET_PTR;

    return imcf;
}

static ngx_int_t
ngx_http_inotify_postconfiguration(ngx_conf_t *cf)
{
    printf("call inotfiy postconfiguration\n");

    ngx_str_t *paths;
    ngx_uint_t                     i;
    ngx_http_inotify_main_conf_t  *imcf;

    imcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_inotify_module);
    if (imcf == NULL) {
        printf("no inotify main conf\n");
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, cf->log, 0, "no inotify main conf");
        return NGX_ERROR;
    }

    if (imcf->paths == NULL) {
        printf("no file to watch\n");
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, cf->log, 0, "no file to watch");
        return NGX_OK;
    }

    paths = imcf->paths->elts;

    for (i = 0; i < imcf->paths->nelts; i++) {
        printf("watch file: %.*s\n", (int) paths[i].len, paths[i].data);
        ngx_log_debug1(NGX_LOG_DEBUG_HTTP, cf->log, 0, "watch file: %V",
                       paths + i);
    }

    return NGX_OK;
}


static ngx_int_t
ngx_http_inotify_init_process(ngx_cycle_t *cycle)
{

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, cycle->log, 0, "call inotify init process");

    int flag;
    ngx_str_t                    *paths;
    ngx_uint_t                     i;
    ngx_event_t                   *rev;
    ngx_socket_t                   s, wd;
    ngx_connection_t              *c;
    ngx_http_inotify_main_conf_t  *imcf;

    if (ngx_worker != 0) {
        return NGX_OK;
    }

    s = inotify_init1(0);
    if (s < 0) {
        ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "inotify_init1(0) failed");
        return NGX_ERROR;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, cycle->log, 0, "inotify fd: %d", s);

    imcf = ngx_http_cycle_get_module_main_conf(cycle,
                                               ngx_http_inotify_module);
    if (imcf == NULL) {
        return NGX_ERROR;
    }

    if (imcf->paths == NGX_CONF_UNSET_PTR) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, cycle->log, 0,
                       "inotify no file to watch");
        return NGX_OK;
    }

    paths = imcf->paths->elts;

    flag = IN_CREATE | IN_MOVED_FROM | IN_MOVED_TO | IN_DELETE_SELF | IN_ONLYDIR;

    for (i = 0; i < imcf->paths->nelts; i++) {

        wd = inotify_add_watch(s, (char *)(paths[i].data), flag);

        ngx_log_debug2(NGX_LOG_DEBUG_HTTP, cycle->log, 0,
                       "inotify watch: %V, wd: %d", paths + i, wd);

        if (wd < 0) {
            return NGX_ERROR;
        }
    }

    c = ngx_get_connection(s, cycle->log);
    if (c == NULL) {
        return NGX_ERROR;
    }

    c->data = imcf;

    rev = c->read;
    rev->handler = ngx_http_inotify_read_handler;
    rev->log = c->log;

    if (ngx_add_event(rev, NGX_READ_EVENT, NGX_LEVEL_EVENT) == NGX_ERROR) {
        return NGX_ERROR;
    }

    return NGX_OK;
}


static void
ngx_http_inotify_read_handler(ngx_event_t *ev)
{
    static const int INOTIFY_READ_BUF_LEN =
            (100 * (sizeof(struct inotify_event) + NAME_MAX + 1));

    char                          *p;
    ssize_t                        n;
    ngx_connection_t              *c;
    struct inotify_event          *ie;

    char buf[INOTIFY_READ_BUF_LEN]
            __attribute__((aligned(__alignof__(struct inotify_event))));

    c = ev->data;

    n = read(c->fd, buf, sizeof(buf));

    if (n == -1) {
        ngx_log_error(NGX_LOG_ERR, ev->log, 0, "read inotify event fail: %d", n);
        return;
    }

    if (n == 0) {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, ev->log, 0,
                      "read inotify event size 0");
        return;
    }

    for (p = buf;
         p + sizeof(struct inotify_event) <= buf + n;
         p += sizeof(struct inotify_event))
    {
        ie = (struct inotify_event *) p;
        ngx_http_inotify_dump_event(ev->log, ie);
    }
}


static void
ngx_http_inotify_dump_event(ngx_log_t *log, struct inotify_event *ie)
{
    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, log, 0, "dump event, wd: %d", ie->wd);

    if (ie->cookie > 0) {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, log, 0, "cookie: %4d", ie->cookie);
    }

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, log, 0, "dump event, mask: ");

    if (ie->mask & IN_ISDIR) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, log, 0, "IN_ISDIR");
    }

    if (ie->mask & IN_CREATE) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, log, 0, "IN_CREATE");
    }

    if (ie->mask & IN_DELETE_SELF) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, log, 0, "IN_DELETE_SELF");
    }

    if (ie->mask & IN_MOVE_SELF) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, log, 0, "IN_MOVE_SELF");
    }

    if (ie->mask & IN_MOVED_FROM) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, log, 0, "IN_MOVE_FROM");
    }

    if (ie->mask & IN_MOVED_TO) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, log, 0, "IN_MOVE_TO");
    }

    if (ie->mask & IN_IGNORED) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, log, 0, "IN_IGNORED");
    }

    if (ie->mask & IN_Q_OVERFLOW) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, log, 0, "IN_Q_OVERFLOW");
    }
    if (ie->mask & IN_UNMOUNT) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, log, 0, "IN_UNMOUNT");
    }

    if (ie->len > 0) {
        ngx_log_debug1(NGX_LOG_DEBUG_HTTP, log, 0, "dump event, name: %s",
                       ie->name);
    }
}
