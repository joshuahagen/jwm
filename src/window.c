#include <X11/Xatom.h>
#include "dwm.h"
#include "bar.h"
#include "button.h"
#include "helpers.h"
#include "layout.h"
#include "monitor.h"
#include "settings.h"
#include "window.h"

void apply_rules(client_t *c)
{
	const char *class, *instance;
	unsigned int i;
	const rule_t *r;
	monitor_t *m;
	XClassHint ch = { NULL, NULL };

	/* rule matching */
	c->isfloating = 0;
	c->tags = 0;
	XGetClassHint(dpy, c->win, &ch);
	class    = ch.res_class ? ch.res_class : broken;
	instance = ch.res_name  ? ch.res_name  : broken;

	for (i = 0; i < LENGTH(rules); i++) {
		/* don't handle any non existent rules */
		if (rules[i].class == NULL)
			break;

		r = &rules[i];
		if ((!r->title || strstr(c->name, r->title))
		&& (!r->class || strstr(class, r->class))
		&& (!r->instance || strstr(instance, r->instance)))
		{
			c->isfloating = r->isfloating;
			c->tags |= r->tags;
			for (m = mons; m && m->num != r->monitor; m = m->next);
			if (m)
				c->mon = m;
		}
	}

	if (ch.res_class)
		XFree(ch.res_class);

	if (ch.res_name)
		XFree(ch.res_name);

	c->tags = c->tags & TAG_MASK ? c->tags & TAG_MASK : c->mon->tagset[c->mon->seltags];
}

int apply_size_hints(client_t *c, int *x, int *y, int *w, int *h, int interact)
{
	int baseismin;
	monitor_t *m = c->mon;

	/* set minimum possible */
	*w = MAX(1, *w);
	*h = MAX(1, *h);
	if (interact) {
		if (*x > sw)
			*x = sw - WIDTH(c);
		if (*y > sh)
			*y = sh - HEIGHT(c);
		if (*x + *w + 2 * c->bw < 0)
			*x = 0;
		if (*y + *h + 2 * c->bw < 0)
			*y = 0;
	} else {
		if (*x >= m->wx + m->ww)
			*x = m->wx + m->ww - WIDTH(c);
		if (*y >= m->wy + m->wh)
			*y = m->wy + m->wh - HEIGHT(c);
		if (*x + *w + 2 * c->bw <= m->wx)
			*x = m->wx;
		if (*y + *h + 2 * c->bw <= m->wy)
			*y = m->wy;
	}

	if (*h < bh)
		*h = bh;

	if (*w < bh)
		*w = bh;

	if (resize_hints || c->isfloating || !c->mon->lt[c->mon->sellt]->arrange) {
		if (!c->hintsvalid)
			update_size_hints(c);

		/* see last two sentences in ICCCM 4.1.2.3 */
		baseismin = c->basew == c->minw && c->baseh == c->minh;
		if (!baseismin) { /* temporarily remove base dimensions */
			*w -= c->basew;
			*h -= c->baseh;
		}

		/* adjust for aspect limits */
		if (c->mina > 0 && c->maxa > 0) {
			if (c->maxa < (float)*w / *h)
				*w = *h * c->maxa + 0.5;
			else if (c->mina < (float)*h / *w)
				*h = *w * c->mina + 0.5;
		}

		if (baseismin) { /* increment calculation requires this */
			*w -= c->basew;
			*h -= c->baseh;
		}

		/* adjust for increment value */
		if (c->incw)
			*w -= *w % c->incw;

		if (c->inch)
			*h -= *h % c->inch;

		/* restore base dimensions */
		*w = MAX(*w + c->basew, c->minw);
		*h = MAX(*h + c->baseh, c->minh);
		if (c->maxw)
			*w = MIN(*w, c->maxw);

		if (c->maxh)
			*h = MIN(*h, c->maxh);
	}

	return *x != c->x || *y != c->y || *w != c->w || *h != c->h;
}

void arrange(monitor_t *m)
{
	if (m)
		show_hide(m->stack);
	else for (m = mons; m; m = m->next)
		show_hide(m->stack);

	if (m) {
		arrange_mon(m);
		restack(m);
	} else for (m = mons; m; m = m->next)
		arrange_mon(m);
}

void attach(client_t *c)
{
	c->next = c->mon->clients;
	c->mon->clients = c;
}

void attach_stack(client_t *c)
{
	c->snext = c->mon->stack;
	c->mon->stack = c;
}

void configure(client_t *c)
{
	XConfigureEvent ce;

	ce.type = ConfigureNotify;
	ce.display = dpy;
	ce.event = c->win;
	ce.window = c->win;
	ce.x = c->x;
	ce.y = c->y;
	ce.width = c->w;
	ce.height = c->h;
	ce.border_width = c->bw;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

void detach(client_t *c)
{
	client_t **tc;

	for (tc = &c->mon->clients; *tc && *tc != c; tc = &(*tc)->next);
	*tc = c->next;
}

void detach_stack(client_t *c)
{
	client_t **tc, *t;

	for (tc = &c->mon->stack; *tc && *tc != c; tc = &(*tc)->snext);
	*tc = c->snext;

	if (c == c->mon->sel) {
		for (t = c->mon->stack; t && !IS_VISIBLE(t); t = t->snext);
		c->mon->sel = t;
	}
}

void focus(client_t *c)
{
	if (!c || !IS_VISIBLE(c))
		for (c = selmon->stack; c && !IS_VISIBLE(c); c = c->snext);

	if (selmon->sel && selmon->sel != c)
		unfocus(selmon->sel, 0);

	if (c) {
		if (c->mon != selmon)
			selmon = c->mon;

		if (c->isurgent)
			set_urgent(c, 0);

		detach_stack(c);
		attach_stack(c);
		grab_buttons(c, 1);
		XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColBorder].pixel);
		set_focus(c);
	} else {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, net_atom[NetActiveWindow]);
	}

	selmon->sel = c;
	draw_bars();
}

Atom get_atom_prop(client_t *c, Atom prop)
{
	int di;
	unsigned long dl;
	unsigned char *p = NULL;
	Atom da, atom = None;

	/* FIXME get_atom_prop should return the number of items and a pointer to
	 * the stored data instead of this workaround */
	Atom req = XA_ATOM;
	if (prop == xatom[XembedInfo])
		req = xatom[XembedInfo];

	if (XGetWindowProperty(dpy, c->win, prop, 0L, sizeof atom, False, req,
		&da, &di, &dl, &dl, &p) == Success && p) {
		atom = *(Atom *)p;
		if (da == xatom[XembedInfo] && dl == 2)
			atom = ((Atom *)p)[1];

		XFree(p);
	}

	return atom;
}

long get_state(Window w)
{
	int format;
	long result = -1;
	unsigned char *p = NULL;
	unsigned long n, extra;
	Atom real;

	if (XGetWindowProperty(dpy, w, wm_atom[WMState], 0L, 2L, False, wm_atom[WMState],
		&real, &format, &n, &extra, (unsigned char **)&p) != Success)
		return -1;

	if (n != 0)
		result = *p;

	XFree(p);

	return result;
}

void kill_client(const arg_t *arg)
{
	if (!selmon->sel)
		return;

	if (!send_event(selmon->sel->win, wm_atom[WMDelete], NoEventMask, wm_atom[WMDelete], CurrentTime, 0 , 0, 0)) {
		XGrabServer(dpy);
		XSetErrorHandler(xerror_dummy);
		XSetCloseDownMode(dpy, DestroyAll);
		XKillClient(dpy, selmon->sel->win);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}
}

void manage(Window w, XWindowAttributes *wa)
{
	client_t *c, *t = NULL;
	Window trans = None;
	XWindowChanges wc;

	c = ecalloc(1, sizeof(client_t));
	c->win = w;
	/* geometry */
	c->x = c->oldx = wa->x;
	c->y = c->oldy = wa->y;
	c->w = c->oldw = wa->width;
	c->h = c->oldh = wa->height;
	c->oldbw = wa->border_width;

	update_icon(c);
	update_title(c);
	if (XGetTransientForHint(dpy, w, &trans) && (t = win_to_client(trans))) {
		c->mon = t->mon;
		c->tags = t->tags;
	} else {
		c->mon = selmon;
		apply_rules(c);
	}

	if (c->x + WIDTH(c) > c->mon->wx + c->mon->ww)
		c->x = c->mon->wx + c->mon->ww - WIDTH(c);

	if (c->y + HEIGHT(c) > c->mon->wy + c->mon->wh)
		c->y = c->mon->wy + c->mon->wh - HEIGHT(c);

	c->x = MAX(c->x, c->mon->wx);
	c->y = MAX(c->y, c->mon->wy);
	c->bw = border_px;

	wc.border_width = c->bw;
	XConfigureWindow(dpy, w, CWBorderWidth, &wc);
	XSetWindowBorder(dpy, w, scheme[SchemeNorm][ColBorder].pixel);
	configure(c); /* propagates border_width, if size doesn't change */
	update_window_type(c);
	update_size_hints(c);
	update_wm_hints(c);
	XSelectInput(dpy, w, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
	grab_buttons(c, 0);

	if (!c->isfloating)
		c->isfloating = c->oldstate = trans != None || c->isfixed;

	if (c->isfloating)
		XRaiseWindow(dpy, c->win);

	attach(c);
	attach_stack(c);
	XChangeProperty(dpy, root, net_atom[NetClientList], XA_WINDOW, 32, PropModeAppend,
		(unsigned char *) &(c->win), 1);
	XMoveResizeWindow(dpy, c->win, c->x + 2 * sw, c->y, c->w, c->h); /* some windows require this */
	set_client_state(c, NormalState);

	if (c->mon == selmon)
		unfocus(selmon->sel, 0);

	c->mon->sel = c;
	arrange(c->mon);
	XMapWindow(dpy, c->win);
	focus(NULL);
}

void window_move_mouse(const arg_t *arg)
{
	int x, y, ocx, ocy, nx, ny;
	client_t *c;
	monitor_t *m;
	XEvent ev;
	Time lasttime = 0;

	if (!(c = selmon->sel))
		return;

	if (c->isfullscreen) /* no support moving fullscreen windows by mouse */
		return;

	restack(selmon);
	ocx = c->x;
	ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSE_MASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;

	if (!get_root_ptr(&x, &y))
		return;

	do {
		XMaskEvent(dpy, MOUSE_MASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
			case ConfigureRequest:
			case Expose:
			case MapRequest:
				handler[ev.type](&ev);
				break;
			case MotionNotify:
				if ((ev.xmotion.time - lasttime) <= (1000 / 60))
					continue;

				lasttime = ev.xmotion.time;

				nx = ocx + (ev.xmotion.x - x);
				ny = ocy + (ev.xmotion.y - y);

				if (abs(selmon->wx - nx) < snap)
					nx = selmon->wx;
				else if (abs((selmon->wx + selmon->ww) - (nx + WIDTH(c))) < snap)
					nx = selmon->wx + selmon->ww - WIDTH(c);

				if (abs(selmon->wy - ny) < snap)
					ny = selmon->wy;
				else if (abs((selmon->wy + selmon->wh) - (ny + HEIGHT(c))) < snap)
					ny = selmon->wy + selmon->wh - HEIGHT(c);

				if (!c->isfloating && selmon->lt[selmon->sellt]->arrange
				&& (abs(nx - c->x) > snap || abs(ny - c->y) > snap))
					toggle_floating(NULL);

				if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
					resize(c, nx, ny, c->w, c->h, 1);

				break;
		}
	} while (ev.type != ButtonRelease);

	XUngrabPointer(dpy, CurrentTime);
	if ((m = rect_to_mon(c->x, c->y, c->w, c->h)) != selmon) {
		send_mon(c, m);
		selmon = m;
		focus(NULL);
	}
}

client_t *next_tiled(client_t *c)
{
	for (; c && (c->isfloating || !IS_VISIBLE(c)); c = c->next);
	return c;
}

void resize(client_t *c, int x, int y, int w, int h, int interact)
{
	if (apply_size_hints(c, &x, &y, &w, &h, interact))
		resize_client(c, x, y, w, h);
}

void resize_client(client_t *c, int x, int y, int w, int h)
{
	XWindowChanges wc;
	unsigned int n;
	unsigned int gapoffset;
	unsigned int gapincr;
	client_t *nbc;

	wc.border_width = c->bw;
	
	/* Get number of clients for the client's monitor_t */
	for (n = 0, nbc = next_tiled(c->mon->clients); nbc; nbc = next_tiled(nbc->next), n++);

	/* Do nothing if layout is floating */
	if (c->isfloating || c->mon->lt[c->mon->sellt]->arrange == NULL) {
		gapincr = gapoffset = 0;
	} else {
		gapoffset = gap_px + vp;
		gapincr = 2 * (gap_px + vp);
	}

	c->oldx = c->x; c->x = wc.x = x + gapoffset;
	c->oldy = c->y; c->y = wc.y = y + gapoffset;
	c->oldw = c->w; c->w = wc.width = w - gapincr;
	c->oldh = c->h; c->h = wc.height = h - gapincr;
	
	XConfigureWindow(dpy, c->win, CWX|CWY|CWWidth|CWHeight|CWBorderWidth, &wc);
	configure(c);
	XSync(dpy, False);
}

void window_resize_mouse(const arg_t *arg)
{
	int ocx, ocy, nw, nh;
	client_t *c;
	monitor_t *m;
	XEvent ev;
	Time lasttime = 0;

	if (!(c = selmon->sel))
		return;

	if (c->isfullscreen) /* no support resizing fullscreen windows by mouse */
		return;

	restack(selmon);
	ocx = c->x;
	ocy = c->y;

	if (XGrabPointer(dpy, root, False, MOUSE_MASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurResize]->cursor, CurrentTime) != GrabSuccess)
		return;

	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	do {
		XMaskEvent(dpy, MOUSE_MASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
			case ConfigureRequest:
			case Expose:
			case MapRequest:
				handler[ev.type](&ev);
				break;
			case MotionNotify:
				if ((ev.xmotion.time - lasttime) <= (1000 / 60))
					continue;

				lasttime = ev.xmotion.time;

				nw = MAX(ev.xmotion.x - ocx - 2 * c->bw + 1, 1);
				nh = MAX(ev.xmotion.y - ocy - 2 * c->bw + 1, 1);
				if (c->mon->wx + nw >= selmon->wx && c->mon->wx + nw <= selmon->wx + selmon->ww
				&& c->mon->wy + nh >= selmon->wy && c->mon->wy + nh <= selmon->wy + selmon->wh)
				{
					if (!c->isfloating && selmon->lt[selmon->sellt]->arrange
					&& (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
						toggle_floating(NULL);
				}

				if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
					resize(c, c->x, c->y, nw, nh, 1);

				break;
		}
	} while (ev.type != ButtonRelease);

	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	XUngrabPointer(dpy, CurrentTime);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));

	if ((m = rect_to_mon(c->x, c->y, c->w, c->h)) != selmon) {
		send_mon(c, m);
		selmon = m;
		focus(NULL);
	}
}

void restack(monitor_t *m)
{
	client_t *c;
	XEvent ev;
	XWindowChanges wc;

	draw_bar(m);
	if (!m->sel)
		return;

	if (m->sel->isfloating || !m->lt[m->sellt]->arrange)
		XRaiseWindow(dpy, m->sel->win);

	if (m->lt[m->sellt]->arrange) {
		wc.stack_mode = Below;
		wc.sibling = m->barwin;
		for (c = m->stack; c; c = c->snext)
			if (!c->isfloating && IS_VISIBLE(c)) {
				XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);
				wc.sibling = c->win;
			}
	}

	XSync(dpy, False);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void set_client_state(client_t *c, long state)
{
	long data[] = { state, None };

	XChangeProperty(dpy, c->win, wm_atom[WMState], wm_atom[WMState], 32,
		PropModeReplace, (unsigned char *)data, 2);
}

void set_focus(client_t *c)
{
	if (!c->neverfocus) {
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
		XChangeProperty(dpy, root, net_atom[NetActiveWindow],
			XA_WINDOW, 32, PropModeReplace,
			(unsigned char *) &(c->win), 1);
	}

	send_event(c->win, wm_atom[WMTakeFocus], NoEventMask, wm_atom[WMTakeFocus], CurrentTime, 0, 0, 0);
}

void set_full_screen(client_t *c, int fullscreen)
{
	if (fullscreen && !c->isfullscreen) {
		XChangeProperty(dpy, c->win, net_atom[NetWMState], XA_ATOM, 32,
			PropModeReplace, (unsigned char*)&net_atom[NetWMFullscreen], 1);
		c->isfullscreen = 1;
		c->oldstate = c->isfloating;
		c->oldbw = c->bw;
		c->bw = 0;
		c->isfloating = 1;
		resize_client(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
		XRaiseWindow(dpy, c->win);
	} else if (!fullscreen && c->isfullscreen) {
		XChangeProperty(dpy, c->win, net_atom[NetWMState], XA_ATOM, 32,
			PropModeReplace, (unsigned char*)0, 0);
		c->isfullscreen = 0;
		c->isfloating = c->oldstate;
		c->bw = c->oldbw;
		c->x = c->oldx;
		c->y = c->oldy;
		c->w = c->oldw;
		c->h = c->oldh;
		resize_client(c, c->x, c->y, c->w, c->h);
		arrange(c->mon);
	}
}

void set_urgent(client_t *c, int urg)
{
	XWMHints *wmh;

	c->isurgent = urg;
	if (!(wmh = XGetWMHints(dpy, c->win)))
		return;

	wmh->flags = urg ? (wmh->flags | XUrgencyHint) : (wmh->flags & ~XUrgencyHint);
	XSetWMHints(dpy, c->win, wmh);
	XFree(wmh);
}

void show_hide(client_t *c)
{
	if (!c)
		return;

	if (IS_VISIBLE(c)) {
		/* show clients top down */
		XMoveWindow(dpy, c->win, c->x, c->y);
		if ((!c->mon->lt[c->mon->sellt]->arrange || c->isfloating) && !c->isfullscreen)
			resize(c, c->x, c->y, c->w, c->h, 0);
		show_hide(c->snext);
	} else {
		/* hide clients bottom up */
		show_hide(c->snext);
		XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
	}
}

void toggle_floating(const arg_t *arg)
{
	if (!selmon->sel)
		return;

	if (selmon->sel->isfullscreen) /* no support for fullscreen windows */
		return;

	selmon->sel->isfloating = !selmon->sel->isfloating || selmon->sel->isfixed;
	if (selmon->sel->isfloating)
		resize(selmon->sel, selmon->sel->x, selmon->sel->y,
			selmon->sel->w, selmon->sel->h, 0);

	arrange(selmon);
}

void unfocus(client_t *c, int set_focus)
{
	if (!c)
		return;

	grab_buttons(c, 0);
	XSetWindowBorder(dpy, c->win, scheme[SchemeNorm][ColBorder].pixel);

	if (set_focus) {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, net_atom[NetActiveWindow]);
	}
}

void unmanage(client_t *c, int destroyed)
{
	monitor_t *m = c->mon;
	XWindowChanges wc;

	detach(c);
	detach_stack(c);
	free_icon(c);

	if (!destroyed) {
		wc.border_width = c->oldbw;
		XGrabServer(dpy); /* avoid race conditions */
		XSetErrorHandler(xerror_dummy);
		XSelectInput(dpy, c->win, NoEventMask);
		XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		set_client_state(c, WithdrawnState);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}

	free(c);
	focus(NULL);
	update_client_list();
	arrange(m);
}

void update_client_list()
{
	client_t *c;
	monitor_t *m;

	XDeleteProperty(dpy, root, net_atom[NetClientList]);
	for (m = mons; m; m = m->next)
		for (c = m->clients; c; c = c->next)
			XChangeProperty(dpy, root, net_atom[NetClientList],
				XA_WINDOW, 32, PropModeAppend,
				(unsigned char *) &(c->win), 1);
}

void update_size_hints(client_t *c)
{
	long msize;
	XSizeHints size;

	if (!XGetWMNormalHints(dpy, c->win, &size, &msize))
		/* size is uninitialized, ensure that size.flags aren't used */
		size.flags = PSize;

	if (size.flags & PBaseSize) {
		c->basew = size.base_width;
		c->baseh = size.base_height;
	} else if (size.flags & PMinSize) {
		c->basew = size.min_width;
		c->baseh = size.min_height;
	} else
		c->basew = c->baseh = 0;

	if (size.flags & PResizeInc) {
		c->incw = size.width_inc;
		c->inch = size.height_inc;
	} else
		c->incw = c->inch = 0;

	if (size.flags & PMaxSize) {
		c->maxw = size.max_width;
		c->maxh = size.max_height;
	} else
		c->maxw = c->maxh = 0;

	if (size.flags & PMinSize) {
		c->minw = size.min_width;
		c->minh = size.min_height;
	} else if (size.flags & PBaseSize) {
		c->minw = size.base_width;
		c->minh = size.base_height;
	} else
		c->minw = c->minh = 0;

	if (size.flags & PAspect) {
		c->mina = (float)size.min_aspect.y / size.min_aspect.x;
		c->maxa = (float)size.max_aspect.x / size.max_aspect.y;
	} else
		c->maxa = c->mina = 0.0;

	c->isfixed = (c->maxw && c->maxh && c->maxw == c->minw && c->maxh == c->minh);
	c->hintsvalid = 1;
}

void update_window_type(client_t *c)
{
	Atom state = get_atom_prop(c, net_atom[NetWMState]);
	Atom wtype = get_atom_prop(c, net_atom[NetWMWindowType]);

	if (state == net_atom[NetWMFullscreen])
		set_full_screen(c, 1);

	if (wtype == net_atom[NetWMWindowTypeDialog])
		c->isfloating = 1;
}

client_t *win_to_client(Window w)
{
	client_t *c;
	monitor_t *m;

	for (m = mons; m; m = m->next)
		for (c = m->clients; c; c = c->next)
			if (c->win == w)
				return c;
				
	return NULL;
}
