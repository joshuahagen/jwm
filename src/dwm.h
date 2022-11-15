#ifndef DWM_H
#define DWM_H

#include <stdint.h>
#include <X11/Xproto.h>
#include "types.h"
#include "drw.h"
#include "util.h"

/* variables */
extern int lrpad;
extern sys_tray_t *sys_tray;
extern const char broken[];
extern int screen;
extern int sw, sh;             
extern unsigned int num_lock_mask;
extern cur_t *cursor[CurLast];
extern Display *dpy;
extern drw_t *drw;
extern monitor_t *mons, *selmon;
extern Window root, wm_check_win;
extern clr_t **scheme;
extern int scm_len;
extern Atom wm_atom[WMLast], net_atom[NetLast], xatom[XLast];
extern void (*handler[LASTEvent]) (XEvent *);

int get_root_ptr(int *x, int *y);
int get_text_prop(Window w, Atom atom, char *text, unsigned int size);
uint32_t pre_alpha(uint32_t p);
int update_geom(void);
void update_wm_hints(client_t *c);
int xerror(Display *dpy, XErrorEvent *ee);
int xerror_dummy(Display *dpy, XErrorEvent *ee);

#endif