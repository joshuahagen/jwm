#ifndef WINDOW_H
#define WINDOW_H

void apply_rules(client_t *c);
int apply_size_hints(client_t *c, int *x, int *y, int *w, int *h, int interact);
void arrange(monitor_t *m);
void attach(client_t *c);
void attach_stack(client_t *c);
void configure(client_t *c);
void detach(client_t *c);
void detach_stack(client_t *c);
void focus(client_t *c);
void resize(client_t *c, int x, int y, int w, int h, int interact);
void resize_client(client_t *c, int x, int y, int w, int h);
void window_resize_mouse(const arg_t *arg);
void restack(monitor_t *m);
Atom get_atom_prop(client_t *c, Atom prop);
long get_state(Window w);
void kill_client(const arg_t *arg);
void manage(Window w, XWindowAttributes *wa);
void window_move_mouse(const arg_t *arg);
client_t *next_tiled(client_t *c);
void set_client_state(client_t *c, long state);
void set_focus(client_t *c);
void set_full_screen(client_t *c, int fullscreen);
void set_urgent(client_t *c, int urg);
void show_hide(client_t *c);
void toggle_floating(const arg_t *arg);
void unfocus(client_t *c, int setfocus);
void unmanage(client_t *c, int destroyed);
void update_client_list(void);
void update_size_hints(client_t *c);
void update_window_type(client_t *c);
client_t *win_to_client(Window w);

#endif