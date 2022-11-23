#ifndef LAYOUT_H
#define LAYOUT_H

void dwindle(monitor_t *mon);
void fullscreen(const arg_t *arg);
void grid(monitor_t *mon);
void monocle(monitor_t *mon);
void set_layout(const arg_t *arg);
void spiral(monitor_t *mon);
void tile(monitor_t *mon);

#endif