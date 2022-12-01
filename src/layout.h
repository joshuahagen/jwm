#ifndef LAYOUT_H
#define LAYOUT_H

void centered_master(monitor_t *mon);
void centered_floating_master(monitor_t *mon);
void dwindle(monitor_t *mon);
void fullscreen(const arg_t *arg);
void grid(monitor_t *mon);
void monocle(monitor_t *mon);
void set_layout(const arg_t *arg);
void spiral(monitor_t *mon);
void tile(monitor_t *mon);

#endif