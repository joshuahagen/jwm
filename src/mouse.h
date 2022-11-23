#ifndef MOUSE_H
#define MOUSE_H

int get_root_ptr(int *x, int *y);
void grab_buttons(client_t *c, int focused);

#endif