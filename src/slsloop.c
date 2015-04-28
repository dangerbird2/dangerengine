//
// Created by Steven on 4/27/15.
//

#include "slsloop.h"
#include "slsutils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

slsMainLoop *sls_mainloop_init(slsMainLoop *self, slsContext *ctx);

void sls_mainloop_dtor(slsMainLoop *self);

void sls_mainloop_run(slsMainLoop *self);

void sls_mainloop_handle_events(slsMainLoop *self, double dt);

void sls_mainloop_update(slsMainLoop *self, double dt);

void sls_mainloop_display(slsMainLoop *self, double dt);

struct slsMainLoop_p {
    slsContext *ctx;
};

static const slsMainLoop sls_mainloop_proto = {
    .init = sls_mainloop_init,
    .dtor = sls_mainloop_dtor,
    .run = sls_mainloop_run,
    .update = sls_mainloop_update,
    .display = sls_mainloop_display,
    .interval = 20,
    .priv = NULL
};

const slsMainLoop *sls_mainloop_class() {
    return &sls_mainloop_proto;
}

slsMainLoop *sls_mainloop_new(slsContext *ctx) {
    slsMainLoop *self = sls_objalloc(sls_mainloop_class(), sizeof(slsMainLoop));
    return sls_msg(self, init, ctx);
}

void sls_mainloop_dtor(slsMainLoop *self)
{
    if (!self) {return;}
    if (self->priv && self->priv->ctx) {
        sls_msg(self->priv->ctx, dtor);
        free(self->priv);
    }
    free(self);
}

slsMainLoop *sls_mainloop_init(slsMainLoop *self, slsContext *ctx) {
    sls_checkmem(self);
    sls_checkmem(ctx);
    return self;

error:
    if (self && self->dtor) {
        sls_msg(self, dtor);
    }
    return NULL;
}


void sls_mainloop_run(slsMainLoop *self) {
    sleep(1);
}

void sls_mainloop_update(slsMainLoop *self, double dt) {

}

void sls_mainloop_display(slsMainLoop *self, double dt) {

}