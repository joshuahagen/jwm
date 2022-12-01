#ifndef BAR_H
#define BAR_H

extern int sp, vp;
extern char stext[1024];
extern int tag_len;

void draw_bar(monitor_t *m);
void draw_bars(void);
int draw_status_bar(monitor_t *m, int bh, char* text);
void free_icon(client_t *c);
Picture get_icon_prop(Window w, unsigned int *icw, unsigned int *ich);
void resize_bar_win(monitor_t *m);
void toggle_bar(const arg_t *arg);
void update_bars(void);
void update_bar_pos(monitor_t *m);
void update_icon(client_t *c);
void update_status(void);
void update_title(client_t *c);

#endif