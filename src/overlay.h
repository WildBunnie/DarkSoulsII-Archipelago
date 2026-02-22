#pragma once

typedef void (*overlay_render_fn)();

int init_overlay(overlay_render_fn fn);
void shutdown_overlay();