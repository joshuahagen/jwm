#ifndef TYPES_H
#define TYPES_H

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

enum { ColFg, ColBg, ColBorder }; /* Clr scheme index */
typedef XftColor clr_t;

/* enums */
enum { CurNormal, CurResize, CurMove, CurLast }; /* cursor */
enum { SchemeNorm, SchemeSel, SchemeSelAlt }; /* color schemes */
enum { NetSupported, NetWMName, NetWMIcon, NetWMState, NetWMCheck,
       NetSystemTray, NetSystemTrayOP, NetSystemTrayOrientation, NetSystemTrayOrientationHorz,
       NetWMFullscreen, NetActiveWindow, NetWMWindowType,
       NetWMWindowTypeDialog, NetClientList, NetLast }; /* EWMH atoms */
enum { Manager, Xembed, XembedInfo, XLast }; /* Xembed atoms */
enum { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast }; /* default atoms */
enum { ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle,
       ClkClientWin, ClkRootWin, ClkLast }; /* clicks */

typedef union {
	int i;
	unsigned int ui;
	float f;
	const void *v;
} arg_t;

typedef struct {
	unsigned int click;
	unsigned int mask;
	unsigned int button;
	void (*func)(const arg_t *arg);
	const arg_t arg;
} button_t;

typedef struct monitor monitor_t;
typedef struct client client_t;
struct client {
	char name[256];
	float mina, maxa;
	int x, y, w, h;
	int oldx, oldy, oldw, oldh;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh, hintsvalid;
	int bw, oldbw;
	unsigned int tags;
	int isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen;
	unsigned int icw, ich; 
	Picture icon;
	client_t *next;
	client_t *snext;
	monitor_t *mon;
	Window win;
};

typedef struct {
	Cursor cursor;
} cur_t;

typedef struct fnt_t {
	Display *dpy;
	unsigned int h;
	XftFont *xfont;
	FcPattern *pattern;
	struct fnt_t *next;
} fnt_t;

typedef struct {
	unsigned int w, h;
	Display *dpy;
	int screen;
	Window root;
	Drawable drawable;
	Picture picture;
	GC gc;
	clr_t *scheme;
	fnt_t *fonts;
} drw_t;

typedef struct {
	unsigned int mod;
	KeySym keysym;
	void (*func)(const arg_t *);
	const arg_t arg;
} _key_t;

typedef struct {
	const char *symbol;
	void (*arrange)(monitor_t *);
} layout_t;

struct monitor {
	char ltsymbol[16];
	float mfact;
	int nmaster;
	int num;
	int by;               /* bar geometry */
	int mx, my, mw, mh;   /* screen size */
	int wx, wy, ww, wh;   /* window area  */
	unsigned int seltags;
	unsigned int sellt;
	unsigned int tagset[2];
	int showbar;
	int topbar;
	client_t *clients;
	client_t *sel;
	client_t *stack;
	monitor_t *next;
	Window barwin;
	const layout_t *lt[2];
};

typedef struct {
	const char *class;
	const char *instance;
	const char *title;
	unsigned int tags;
	int isfloating;
	int monitor;
} rule_t;

typedef struct sys_tray sys_tray_t;
struct sys_tray {
	Window win;
	client_t *icons;
};

#endif
