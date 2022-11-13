#ifndef MONITOR_H
#define MONITOR_H

void arrange_mon(monitor_t *m);
void cleanup_mon(monitor_t *mon);
monitor_t *create_mon(void);
monitor_t *dir_to_mon(int dir);
void focus_mon(const arg_t *arg);
monitor_t *rect_to_mon(int x, int y, int w, int h);
void send_mon(client_t *c, monitor_t *m);
monitor_t *sys_tray_to_mon(monitor_t *m);
void tag_mon(const arg_t *arg);
monitor_t *win_to_mon(Window w);

#endif