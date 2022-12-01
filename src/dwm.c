/* See LICENSE file for copyright and license details.
 *
 * dynamic window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance. Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * The event handlers of dwm are organized in an array which is accessed
 * whenever a new event has been fetched. This allows event dispatching
 * in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag. Clients are organized in a linked client
 * list on each monitor, the focus history is remembered through a stack list
 * on each monitor. Each client contains a bit array to indicate the tags of a
 * client.
 *
 * Keys and tagging rules are organized as arrays and defined in config.h.
 *
 * To understand everything else, start reading main().
 */
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */
#include <X11/Xft/Xft.h>

#include "types.h"
#include "bar.h"
#include "events.h"
#include "helpers.h"
#include "keyboard.h"
#include "monitor.h"
#include "mouse.h"
#include "settings.h"
#include "window.h"
#include "dwm.h"

/* function declarations */
static void check_other_wm(void);
static void cleanup(void);
static void focus_stack(const arg_t *arg);
static void inc_n_master(const arg_t *arg);
static void pop(client_t *c);
static void quit(const arg_t *arg);
static void run(void);
static void scan(void);
static void set_m_fact(const arg_t *arg);
static void setup(void);
static void sig_chld(int unused);
static void spawn(const arg_t *arg);
static void tag(const arg_t *arg);
static void toggle_tag(const arg_t *arg);
static void toggle_view(const arg_t *arg);
static void view(const arg_t *arg);
static int xerror_start(Display *dpy, XErrorEvent *ee);
static void xinit_visual(void);
static void zoom(const arg_t *arg);

/* variables */
const char broken[] = "broken";
int screen;
int sw, sh;           /* X display screen geometry width, height */
int bh;               /* bar height */
int lrpad;            /* sum of left and right padding for text */
int (*xerrorxlib)(Display *, XErrorEvent *);
void (*handler[LASTEvent]) (XEvent *) = {
	[ButtonPress] = button_press,
	[ClientMessage] = client_message,
	[ConfigureRequest] = configure_request,
	[ConfigureNotify] = configure_notify,
	[DestroyNotify] = destroy_notify,
	[EnterNotify] = enter_notify,
	[Expose] = expose,
	[FocusIn] = focus_in,
	[KeyPress] = key_press,
	[MappingNotify] = mapping_notify,
	[MapRequest] = map_request,
	[MotionNotify] = motion_notify,
	[PropertyNotify] = property_notify,
	[UnmapNotify] = unmap_notify
};
Atom wm_atom[WMLast], net_atom[NetLast], xatom[XLast];
int running = 1;
cur_t *cursor[CurLast];
clr_t **scheme;
int scm_len = 0;
Display *dpy;
drw_t *drw;
layout_t *last_layout;
monitor_t *mons, *selmon;
Window root, wm_check_win;
int useargb = 0;
Visual *visual;
int depth;
Colormap cmap;

#define OPAQUE 0xffU

/* configuration, allows nested code to access above variables */
#include "../config.h"

/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags { char limitexceeded[LENGTH(tags) > 31 ? -1 : 1]; };

/* function implementations */
void check_other_wm(void)
{
	xerrorxlib = XSetErrorHandler(xerror_start);
	/* this causes an error if some other window manager is running */
	XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XSync(dpy, False);
}

void cleanup(void)
{
	arg_t a = {.ui = ~0};
	layout_t foo = { "", NULL };
	monitor_t *m;
	size_t i;

	view(&a);
	selmon->lt[selmon->sellt] = &foo;
	for (m = mons; m; m = m->next)
		while (m->stack)
			unmanage(m->stack, 0);

	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	while (mons)
		cleanup_mon(mons);

        for (i = 0; i < CurLast; i++)
		drw_cur_free(drw, cursor[i]);

	for (i = 0; i < scm_len + 1; i++)
		free(scheme[i]);

	free(scheme);
	XDestroyWindow(dpy, wm_check_win);
	drw_free(drw);
	XSync(dpy, False);
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	XDeleteProperty(dpy, root, net_atom[NetActiveWindow]);
}

void focus_stack(const arg_t *arg)
{
	client_t *c = NULL, *i;

	if (!selmon->sel || (selmon->sel->isfullscreen && lock_full_screen))
		return;

	if (arg->i > 0) {
		for (c = selmon->sel->next; c && !IS_VISIBLE(c); c = c->next);
		if (!c)
			for (c = selmon->clients; c && !IS_VISIBLE(c); c = c->next);
	} else {
		for (i = selmon->clients; i != selmon->sel; i = i->next)
			if (IS_VISIBLE(i))
				c = i;
		if (!c)
			for (; i; i = i->next)
				if (IS_VISIBLE(i))
					c = i;
	}

	if (c) {
		focus(c);
		restack(selmon);
	}
}

uint32_t pre_alpha(uint32_t p) 
{
	uint8_t a = p >> 24u;
	uint32_t rb = (a * (p & 0xFF00FFu)) >> 8u;
	uint32_t g = (a * (p & 0x00FF00u)) >> 8u;
	return (rb & 0xFF00FFu) | (g & 0x00FF00u) | (a << 24u);
}

int get_text_prop(Window w, Atom atom, char *text, unsigned int size)
{
	char **list = NULL;
	int n;
	XTextProperty name;

	if (!text || size == 0)
		return 0;

	text[0] = '\0';
	if (!XGetTextProperty(dpy, w, &name, atom) || !name.nitems)
		return 0;

	if (name.encoding == XA_STRING) {
		strncpy(text, (char *)name.value, size - 1);
	} else if (XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success && n > 0 && *list) {
		strncpy(text, *list, size - 1);
		XFreeStringList(list);
	}

	text[size - 1] = '\0';
	XFree(name.value);
	return 1;
}

void inc_n_master(const arg_t *arg)
{
	selmon->nmaster = MAX(selmon->nmaster + arg->i, 0);
	arrange(selmon);
}

#ifdef XINERAMA
static int is_unique_geom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info)
{
	while (n--)
		if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org
		&& unique[n].width == info->width && unique[n].height == info->height)
			return 0;
	return 1;
}
#endif /* XINERAMA */

void pop(client_t *c)
{
	detach(c);
	attach(c);
	focus(c);
	arrange(c->mon);
}

void quit(const arg_t *arg)
{
	running = 0;
}

void run(void)
{
	XEvent ev;
	/* main event loop */
	XSync(dpy, False);
	while (running && !XNextEvent(dpy, &ev))
		if (handler[ev.type])
			handler[ev.type](&ev); /* call handler */
}

void scan(void)
{
	unsigned int i, num;
	Window d1, d2, *wins = NULL;
	XWindowAttributes wa;

	if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
		for (i = 0; i < num; i++) {
			if (!XGetWindowAttributes(dpy, wins[i], &wa)
			|| wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
				continue;
			if (wa.map_state == IsViewable || get_state(wins[i]) == IconicState)
				manage(wins[i], &wa);
		}

		for (i = 0; i < num; i++) { /* now the transients */
			if (!XGetWindowAttributes(dpy, wins[i], &wa))
				continue;
			if (XGetTransientForHint(dpy, wins[i], &d1)
			&& (wa.map_state == IsViewable || get_state(wins[i]) == IconicState))
				manage(wins[i], &wa);
		}

		if (wins)
			XFree(wins);
	}
}

int send_event(Window w, Atom proto, int mask, long d0, long d1, long d2, long d3, long d4)
{
	int n;
	Atom *protocols, mt;
	int exists = 0;
	XEvent ev;

	if (proto == wm_atom[WMTakeFocus] || proto == wm_atom[WMDelete]) {
		mt = wm_atom[WMProtocols];
		if (XGetWMProtocols(dpy, w, &protocols, &n)) {
			while (!exists && n--)
				exists = protocols[n] == proto;
			
			XFree(protocols);
		}
	} else {
		exists = True;
		mt = proto;
    	}

	if (exists) {
		ev.type = ClientMessage;
		ev.xclient.window = w;
		ev.xclient.message_type = mt;
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = d0;
		ev.xclient.data.l[1] = d1;
		ev.xclient.data.l[2] = d2;
		ev.xclient.data.l[3] = d3;
		ev.xclient.data.l[4] = d4;
		XSendEvent(dpy, w, False, mask, &ev);
	}

	return exists;
}

/* arg > 1.0 will set mfact absolutely */
void set_m_fact(const arg_t *arg)
{
	float f;

	if (!arg || !selmon->lt[selmon->sellt]->arrange)
		return;

	f = arg->f < 1.0 ? arg->f + selmon->mfact : arg->f - 1.0;
	if (f < 0.05 || f > 0.95)
		return;

	selmon->mfact = f;
	arrange(selmon);
}

void setup(void)
{
	int i;
	XSetWindowAttributes wa;
	Atom utf8string;

	/* clean up any zombies immediately */
	sig_chld(0);

	printf("Setting up...");

	/* init screen */
	screen = DefaultScreen(dpy);
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);
	root = RootWindow(dpy, screen);

	xinit_visual();
	drw = drw_create(dpy, screen, root, sw, sh, visual, depth, cmap);

	if (!drw_fontset_create(drw, fonts, LENGTH(fonts)))
		die("no fonts could be loaded.");
	
	lrpad = drw->fonts->h;
	bh = drw->fonts->h + topbar_padding;
	update_geom();
	sp = side_pad;
	vp = (topbar == 1) ? vert_pad : - vert_pad;
	
	/* init atoms */
	utf8string = XInternAtom(dpy, "UTF8_STRING", False);
	wm_atom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wm_atom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wm_atom[WMState] = XInternAtom(dpy, "WM_STATE", False);
	wm_atom[WMTakeFocus] = XInternAtom(dpy, "WM_TAKE_FOCUS", False);
	net_atom[NetActiveWindow] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
        net_atom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
        net_atom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
        net_atom[NetWMIcon] = XInternAtom(dpy, "_NET_WM_ICON", False);
	net_atom[NetWMState] = XInternAtom(dpy, "_NET_WM_STATE", False);
	net_atom[NetWMCheck] = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
	net_atom[NetWMFullscreen] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
	net_atom[NetWMWindowType] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
	net_atom[NetWMWindowTypeDock] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
	net_atom[NetWMWindowTypeDialog] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	net_atom[NetClientList] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
	xatom[Manager] = XInternAtom(dpy, "MANAGER", False);
	xatom[Xembed] = XInternAtom(dpy, "_XEMBED", False);
	xatom[XembedInfo] = XInternAtom(dpy, "_XEMBED_INFO", False);
        
	/* init cursors */
	cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
	cursor[CurResize] = drw_cur_create(drw, XC_sizing);
	cursor[CurMove] = drw_cur_create(drw, XC_fleur);

	/* init appearance */
	/* loop through length of colors array until we hit (null) */
	int exit = 0;
	for (i = 0; i < LENGTH(colors); ++i) {
		for (int j = 0; j < 3; ++j) {
			if (colors[i][j] == NULL) {
				exit = 1;
				scm_len = i;
				break;
			}
		}

		if (exit)
			break;
	}

	scheme = ecalloc(scm_len + 1, sizeof(clr_t *));
	scheme[scm_len] = drw_scm_create(drw, colors[0], alphas[0], 3);
	for (i = 0; i < scm_len; i++)
		scheme[i] = drw_scm_create(drw, colors[i], alphas[i], 3);
	
	/* init bars */
	update_bars();
	update_status();
	update_bar_pos(selmon);
	
	/* supporting window for NetWMCheck */
	wm_check_win = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
	XChangeProperty(dpy, wm_check_win, net_atom[NetWMCheck], XA_WINDOW, 32,
		PropModeReplace, (unsigned char *) &wm_check_win, 1);
	XChangeProperty(dpy, wm_check_win, net_atom[NetWMName], utf8string, 8,
		PropModeReplace, (unsigned char *) "dwm", 3);
	XChangeProperty(dpy, root, net_atom[NetWMCheck], XA_WINDOW, 32,
		PropModeReplace, (unsigned char *) &wm_check_win, 1);
	
	/* EWMH support per view */
	XChangeProperty(dpy, root, net_atom[NetSupported], XA_ATOM, 32,
		PropModeReplace, (unsigned char *) net_atom, NetLast);
	XDeleteProperty(dpy, root, net_atom[NetClientList]);
	
	/* select events */
	wa.cursor = cursor[CurNormal]->cursor;
	wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask
		|ButtonPressMask|PointerMotionMask|EnterWindowMask
		|LeaveWindowMask|StructureNotifyMask|PropertyChangeMask;

	XChangeWindowAttributes(dpy, root, CWEventMask|CWCursor, &wa);
	XSelectInput(dpy, root, wa.event_mask);
	grab_keys();
	focus(NULL);
}

void sig_chld(int unused)
{
	if (signal(SIGCHLD, sig_chld) == SIG_ERR)
		die("can't install sig_chld handler:");

	while (0 < waitpid(-1, NULL, WNOHANG));
}

void spawn(const arg_t *arg)
{
	if (arg->v == dmenucmd)
		dmenumon[0] = '0' + selmon->num;

	if (fork() == 0) {
		if (dpy)
			close(ConnectionNumber(dpy));
		setsid();
		execvp(((char **)arg->v)[0], (char **)arg->v);
		die("dwm: execvp '%s' failed:", ((char **)arg->v)[0]);
	}
}

void tag(const arg_t *arg)
{
	if (selmon->sel && arg->ui & TAG_MASK) {
		selmon->sel->tags = arg->ui & TAG_MASK;
		focus(NULL);
		arrange(selmon);
	}
}

void toggle_tag(const arg_t *arg)
{
	unsigned int newtags;

	if (!selmon->sel)
		return;

	newtags = selmon->sel->tags ^ (arg->ui & TAG_MASK);
	if (newtags) {
		selmon->sel->tags = newtags;
		focus(NULL);
		arrange(selmon);
	}
}

void toggle_view(const arg_t *arg)
{
	unsigned int newtagset = selmon->tagset[selmon->seltags] ^ (arg->ui & TAG_MASK);

	if (newtagset) {
		selmon->tagset[selmon->seltags] = newtagset;
		focus(NULL);
		arrange(selmon);
	}
}

int update_geom(void)
{
	int dirty = 0;

#ifdef XINERAMA
	if (XineramaIsActive(dpy)) {
		int i, j, n, nn;
		client_t *c;
		monitor_t *m;
		XineramaScreenInfo *info = XineramaQueryScreens(dpy, &nn);
		XineramaScreenInfo *unique = NULL;

		for (n = 0, m = mons; m; m = m->next, n++);
		/* only consider unique geometries as separate screens */
		unique = ecalloc(nn, sizeof(XineramaScreenInfo));
		for (i = 0, j = 0; i < nn; i++)
			if (is_unique_geom(unique, j, &info[i]))
				memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));

		XFree(info);
		nn = j;

		/* new monitors if nn > n */
		for (i = n; i < nn; i++) {
			for (m = mons; m && m->next; m = m->next);
			if (m)
				m->next = create_mon();
			else
				mons = create_mon();
		}

		for (i = 0, m = mons; i < nn && m; m = m->next, i++)
			if (i >= n
			|| unique[i].x_org != m->mx || unique[i].y_org != m->my
			|| unique[i].width != m->mw || unique[i].height != m->mh) 
			{
				dirty = 1;
				m->num = i;
				m->mx = m->wx = unique[i].x_org;
				m->my = m->wy = unique[i].y_org;
				m->mw = m->ww = unique[i].width;
				m->mh = m->wh = unique[i].height;
				update_bar_pos(m);
			}

		/* removed monitors if n > nn */
		for (i = nn; i < n; i++) {
			for (m = mons; m && m->next; m = m->next);

			while ((c = m->clients)) {
				dirty = 1;
				m->clients = c->next;
				detach_stack(c);
				c->mon = mons;
				attach(c);
				attach_stack(c);
			}

			if (m == selmon)
				selmon = mons;
			
			cleanup_mon(m);
		}

		free(unique);
	} else
#endif /* XINERAMA */
	{ /* default monitor setup */
		if (!mons)
			mons = create_mon();

		if (mons->mw != sw || mons->mh != sh) {
			dirty = 1;
			mons->mw = mons->ww = sw;
			mons->mh = mons->wh = sh;
			update_bar_pos(mons);
		}
	}

	if (dirty) {
		selmon = mons;
		selmon = win_to_mon(root);
	}

	return dirty;
}

void update_wm_hints(client_t *c)
{
	XWMHints *wmh;

	if ((wmh = XGetWMHints(dpy, c->win))) {
		if (c == selmon->sel && wmh->flags & XUrgencyHint) {
			wmh->flags &= ~XUrgencyHint;
			XSetWMHints(dpy, c->win, wmh);
		} else
			c->isurgent = (wmh->flags & XUrgencyHint) ? 1 : 0;

		if (wmh->flags & InputHint)
			c->neverfocus = !wmh->input;
		else
			c->neverfocus = 0;
		
		XFree(wmh);
	}
}

void view(const arg_t *arg)
{
	if ((arg->ui & TAG_MASK) == selmon->tagset[selmon->seltags])
		return;

	selmon->seltags ^= 1; /* toggle sel tagset */
	if (arg->ui & TAG_MASK)
		selmon->tagset[selmon->seltags] = arg->ui & TAG_MASK;
	
	focus(NULL);
	arrange(selmon);
}

/* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (especially on UnmapNotify's). Other types of errors call Xlibs
 * default error handler, which may call exit. */
int xerror(Display *dpy, XErrorEvent *ee)
{
	if (ee->error_code == BadWindow
	|| (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
	|| (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
	|| (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
	|| (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
	|| (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
	|| (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
		return 0;
	
	fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n",
		ee->request_code, ee->error_code);
	
	return xerrorxlib(dpy, ee); /* may call exit */
}

int xerror_dummy(Display *dpy, XErrorEvent *ee)
{
	return 0;
}

/* Startup Error handler to check if another window manager
 * is already running. */
int xerror_start(Display *dpy, XErrorEvent *ee)
{
	die("dwm: another window manager is already running");
	return -1;
}

void xinit_visual(void)
{
	XVisualInfo *infos;
	XRenderPictFormat *fmt;
	int nitems;
	int i;

	XVisualInfo tpl = {
		.screen = screen,
		.depth = 32,
		.class = TrueColor
	};
	
	long masks = VisualScreenMask | VisualDepthMask | VisualClassMask;

	infos = XGetVisualInfo(dpy, masks, &tpl, &nitems);
	visual = NULL;
	for (i = 0; i < nitems; i ++) {
		fmt = XRenderFindVisualFormat(dpy, infos[i].visual);
		if (fmt->type == PictTypeDirect && fmt->direct.alphaMask) {
			visual = infos[i].visual;
			depth = infos[i].depth;
			cmap = XCreateColormap(dpy, root, visual, AllocNone);
			useargb = 1;
			break;
		}
	}

	XFree(infos);

	if (!visual) {
		visual = DefaultVisual(dpy, screen);
		depth = DefaultDepth(dpy, screen);
		cmap = DefaultColormap(dpy, screen);
	}
}

void zoom(const arg_t *arg)
{
	client_t *c = selmon->sel;

	if (!selmon->lt[selmon->sellt]->arrange || !c || c->isfloating)
		return;

	if (c == next_tiled(selmon->clients) && !(c = next_tiled(c->next)))
		return;
	
	pop(c);
}

int main(int argc, char *argv[])
{
	if (argc == 2 && !strcmp("-v", argv[1]))
		die("dwm-"VERSION);
	else if (argc != 1)
		die("usage: dwm [-v]");

	if (!setlocale(LC_CTYPE, "") || !XSupportsLocale())
		fputs("warning: no locale support\n", stderr);
	
	if (!(dpy = XOpenDisplay(NULL)))
		die("dwm: cannot open display");
	
	check_other_wm();
	setup();
#ifdef __OpenBSD__
	if (pledge("stdio rpath proc exec", NULL) == -1)
		die("pledge");
#endif /* __OpenBSD__ */
	scan();
	run();
	cleanup();
	XCloseDisplay(dpy);
	return EXIT_SUCCESS;
}
