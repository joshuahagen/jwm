#include <X11/Xatom.h>
#include "dwm.h"
#include "bar.h"
#include "events.h"
#include "helpers.h"
#include "keyboard.h"
#include "monitor.h"
#include "settings.h"
#include "window.h"

void button_press(XEvent *event)
{
	unsigned int i, x, click;
	arg_t arg = {0};
	client_t *c;
	monitor_t *m;
	XButtonPressedEvent *ev = &event->xbutton;

	click = ClkRootWin;
	/* focus monitor if necessary */
	if ((m = win_to_mon(ev->window)) && m != selmon) {
		unfocus(selmon->sel, 1);
		selmon = m;
		focus(NULL);
	}

	if (ev->window == selmon->barwin) {
		i = x = 0;
		do 
			x += TEXTW(tags[i]);
		while (ev->x >= x && ++i < tag_len);
		if (i < tag_len) {
			click = ClkTagBar;
			arg.ui = 1 << i;
		} else if (ev->x < x + TEXTW(selmon->ltsymbol))
			click = ClkLtSymbol;
		else if (ev->x > selmon->ww - (int)TEXTW(stext))
			click = ClkStatusText;
		else
			click = ClkWinTitle;
	} else if ((c = win_to_client(ev->window))) {
		focus(c);
		restack(selmon);
		XAllowEvents(dpy, ReplayPointer, CurrentTime);
		click = ClkClientWin;
	}

	for (i = 0; i < LENGTH(buttons); i++)
		if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
		&& CLEAN_MASK(buttons[i].mask) == CLEAN_MASK(ev->state))
			buttons[i].func(click == ClkTagBar && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg);
}

void client_message(XEvent *event)
{
	XClientMessageEvent *cme = &event->xclient;
	client_t *c = win_to_client(cme->window);

	if (!c)
		return;

	if (cme->message_type == net_atom[NetWMState]) {
		if (cme->data.l[1] == net_atom[NetWMFullscreen]
		|| cme->data.l[2] == net_atom[NetWMFullscreen])
			set_full_screen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD    */
				|| (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ && !c->isfullscreen)));
	} else if (cme->message_type == net_atom[NetActiveWindow]) {
		if (c != selmon->sel && !c->isurgent)
			set_urgent(c, 1);
	}
}

void configure_notify(XEvent *event)
{
	monitor_t *m;
	client_t *c;
	XConfigureEvent *ev = &event->xconfigure;
	int dirty;

	/* TODO: updategeom handling sucks, needs to be simplified */
	if (ev->window == root) {
		dirty = (sw != ev->width || sh != ev->height);
		sw = ev->width;
		sh = ev->height;

		if (update_geom() || dirty) {
			drw_resize(drw, sw, bh);
			update_bars();

			for (m = mons; m; m = m->next) {
				for (c = m->clients; c; c = c->next)
					if (c->isfullscreen)
						resize_client(c, m->mx, m->my, m->mw, m->mh);

				resize_bar_win(m);
			}

			focus(NULL);
			arrange(NULL);
		}
	}
}

void configure_request(XEvent *event)
{
	client_t *c;
	monitor_t *m;
	XConfigureRequestEvent *ev = &event->xconfigurerequest;
	XWindowChanges wc;

	if ((c = win_to_client(ev->window))) {
		if (ev->value_mask & CWBorderWidth)
			c->bw = ev->border_width;
		else if (c->isfloating || !selmon->lt[selmon->sellt]->arrange) {
			m = c->mon;
			if (ev->value_mask & CWX) {
				c->oldx = c->x;
				c->x = m->mx + ev->x;
			}

			if (ev->value_mask & CWY) {
				c->oldy = c->y;
				c->y = m->my + ev->y;
			}

			if (ev->value_mask & CWWidth) {
				c->oldw = c->w;
				c->w = ev->width;
			}

			if (ev->value_mask & CWHeight) {
				c->oldh = c->h;
				c->h = ev->height;
			}

			if ((c->x + c->w) > m->mx + m->mw && c->isfloating)
				c->x = m->mx + (m->mw / 2 - WIDTH(c) / 2); /* center in x direction */

			if ((c->y + c->h) > m->my + m->mh && c->isfloating)
				c->y = m->my + (m->mh / 2 - HEIGHT(c) / 2); /* center in y direction */
			
			if ((ev->value_mask & (CWX|CWY)) && !(ev->value_mask & (CWWidth|CWHeight)))
				configure(c);
			
			if (IS_VISIBLE(c))
				XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
		} else
			configure(c);
	} else {
		wc.x = ev->x;
		wc.y = ev->y;
		wc.width = ev->width;
		wc.height = ev->height;
		wc.border_width = ev->border_width;
		wc.sibling = ev->above;
		wc.stack_mode = ev->detail;
		XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
	}

	XSync(dpy, False);
}

void destroy_notify(XEvent *event)
{
	client_t *c;
	XDestroyWindowEvent *ev = &event->xdestroywindow;

	if ((c = win_to_client(ev->window)))
		unmanage(c, 1);
}

void enter_notify(XEvent *event)
{
	client_t *c;
	monitor_t *m;
	XCrossingEvent *ev = &event->xcrossing;

	if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root)
		return;

	c = win_to_client(ev->window);
	m = c ? c->mon : win_to_mon(ev->window);
	if (m != selmon) {
		unfocus(selmon->sel, 1);
		selmon = m;
	} else if (!c || c == selmon->sel)
		return;

	focus(c);
}

void expose(XEvent *event)
{
	monitor_t *m;
	XExposeEvent *ev = &event->xexpose;

	if (ev->count == 0 && (m = win_to_mon(ev->window))) {
		draw_bar(m);
	}
}

/* there are some broken focus acquiring clients needing extra handling */
void focus_in(XEvent *event)
{
	XFocusChangeEvent *ev = &event->xfocus;

	if (selmon->sel && ev->window != selmon->sel->win)
		set_focus(selmon->sel);
}

void key_press(XEvent *event)
{
	unsigned int i;
	KeySym keysym;
	XKeyEvent *ev;

	ev = &event->xkey;
	keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
	for (i = 0; i < LENGTH(keys); i++)
		if (keysym == keys[i].keysym && CLEAN_MASK(keys[i].mod) == CLEAN_MASK(ev->state) && keys[i].func)
			keys[i].func(&(keys[i].arg));
}

void mapping_notify(XEvent *event)
{
	XMappingEvent *ev = &event->xmapping;

	XRefreshKeyboardMapping(ev);
	if (ev->request == MappingKeyboard)
		grab_keys();
}

void map_request(XEvent *event)
{
	static XWindowAttributes wa;
	XMapRequestEvent *ev = &event->xmaprequest;

	if (!XGetWindowAttributes(dpy, ev->window, &wa) || wa.override_redirect)
		return;

	if (!win_to_client(ev->window))
		manage(ev->window, &wa);
}

void motion_notify(XEvent *event)
{
	static monitor_t *mon = NULL;
	monitor_t *m;
	XMotionEvent *ev = &event->xmotion;

	if (ev->window != root)
		return;

	if ((m = rect_to_mon(ev->x_root, ev->y_root, 1, 1)) != mon && mon) {
		unfocus(selmon->sel, 1);
		selmon = m;
		focus(NULL);
	}

	mon = m;
}

void property_notify(XEvent *event)
{
	client_t *c;
	Window trans;
	XPropertyEvent *ev = &event->xproperty;

	if ((ev->window == root) && (ev->atom == XA_WM_NAME))
		update_status();
	else if (ev->state == PropertyDelete)
		return; /* ignore */
	else if ((c = win_to_client(ev->window))) {
		switch(ev->atom) {
			default: break;
			case XA_WM_TRANSIENT_FOR:
				if (!c->isfloating && (XGetTransientForHint(dpy, c->win, &trans)) &&
					(c->isfloating = (win_to_client(trans)) != NULL))
					arrange(c->mon);
				break;
			case XA_WM_NORMAL_HINTS:
				c->hintsvalid = 0;
				break;
			case XA_WM_HINTS:
				update_wm_hints(c);
				draw_bars();
				break;
		}

		if (ev->atom == XA_WM_NAME || ev->atom == net_atom[NetWMName]) {
			update_title(c);
			if (c == c->mon->sel)
				draw_bar(c->mon);
		}
		else if (ev->atom == net_atom[NetWMIcon]) {
			update_icon(c);
			if (c == c->mon->sel)
				draw_bar(c->mon);
		}

		if (ev->atom == net_atom[NetWMWindowType])
			update_window_type(c);
	}
}

void unmap_notify(XEvent *event)
{
	client_t *c;
	XUnmapEvent *ev = &event->xunmap;

	if ((c = win_to_client(ev->window))) {
		if (ev->send_event)
			set_client_state(c, WithdrawnState);
		else
			unmanage(c, 0);
	}
}
