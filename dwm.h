#ifndef DWM_H
#define DWM_H

#include <stdint.h>
#include <X11/Xproto.h>
#include "types.h"
#include "drw.h"
#include "util.h"

/* variables */
extern char stext[1024];
extern int lrpad;
extern Systray *sys_tray;
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
extern Atom wm_atom[WMLast], net_atom[NetLast], xatom[XLast];
extern void (*handler[LASTEvent]) (XEvent *);

/* config stuff */
extern const unsigned int border_px;
extern const unsigned int gap_px;
extern const unsigned int snap;     
extern const int showbar;              
extern const int topbar;                
extern int bh;
extern const int center_title;
extern const unsigned int sys_tray_pinning;
extern const unsigned int sys_tray_on_left;
extern const unsigned int sys_tray_spacing;
extern const int sys_tray_pinning_fail_first;
extern const int show_sys_tray;
extern const unsigned int icon_size;
extern const unsigned int icon_spacing;
extern const float mfact;           
extern const int nmaster;   
extern const int resize_hints;

extern const char *colors[2][3];
extern const char* tags[9];
extern const rule_t rules[12];
extern const layout_t layouts[12];
extern const _key_t keys[100];
extern const button_t buttons[12];

/* macros */
#define BUTTON_MASK              (ButtonPressMask|ButtonReleaseMask)
#define IS_VISIBLE(C)            ((C->tags & C->mon->tagset[C->mon->seltags]))
#define INTERSECT(x,y,w,h,m)    (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) \
                               * MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))
#define CLEAN_MASK(mask)         (mask & ~(num_lock_mask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define TEXTW(X)                (drw_fontset_getwidth(drw, (X)) + lrpad)
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define MOUSE_MASK               (BUTTON_MASK|PointerMotionMask)
#define WIDTH(X)                ((X)->w + 2 * (X)->bw + gap_px)
#define HEIGHT(X)               ((X)->h + 2 * (X)->bw + gap_px)
#define TAG_MASK                 ((1 << LENGTH(tags)) - 1)
#define SYSTEM_TRAY_REQUEST_DOCK    0

/* XEMBED messages */
#define XEMBED_EMBEDDED_NOTIFY      0
#define XEMBED_WINDOW_ACTIVATE      1
#define XEMBED_FOCUS_IN             4
#define XEMBED_MODALITY_ON          10
#define XEMBED_MAPPED               (1 << 0)
#define XEMBED_WINDOW_ACTIVATE      1
#define XEMBED_WINDOW_DEACTIVATE    2
#define VERSION_MAJOR               0
#define VERSION_MINOR               0
#define XEMBED_EMBEDDED_VERSION (VERSION_MAJOR << 16) | VERSION_MINOR

int get_root_ptr(int *x, int *y);
int get_text_prop(Window w, Atom atom, char *text, unsigned int size);
uint32_t pre_alpha(uint32_t p);
int update_geom(void);
void update_wm_hints(client_t *c);

int xerror(Display *dpy, XErrorEvent *ee);
int xerror_dummy(Display *dpy, XErrorEvent *ee);

#endif