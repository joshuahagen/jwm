#include <stdint.h>
#include <X11/Xatom.h>
#include "dwm.h"
#include "bar.h"
#include "helpers.h"
#include "monitor.h"
#include "settings.h"
#include "window.h"

int sp, vp;
char stext[1024];
int tag_len = 0;

void draw_bar(monitor_t *m)
{
	int x, w, tw = 0, stw = 0;
	int boxs = drw->fonts->h / 9;
	int boxw = drw->fonts->h / 6 + 2;
	unsigned int i, occ = 0, urg = 0;
	client_t *c;

	if (!m->showbar)
		return;

	if(show_sys_tray && m == sys_tray_to_mon(m) && !sys_tray_on_left)
		stw = get_sys_tray_width();

	/* draw status first so it can be overdrawn by tags later */
	if (m == selmon && m->mw >= 2560) { /* status is only drawn on monitor wide enough to display full statusbar */
		tw = m->ww - draw_status_bar(m, bh, stext);
	}
	
	resize_bar_win(m);
	for (c = m->clients; c; c = c->next) {
		occ |= c->tags;
		if (c->isurgent)
			urg |= c->tags;
	}

	x = 0;
	for (i = 0; i < LENGTH(tags); i++) {
		/* ensure we only count the actual number of tags */
		if (tags[i] == NULL) {
			tag_len = i;
			break;
		}

		w = TEXTW(tags[i]);
		drw_setscheme(drw, scheme[m->tagset[m->seltags] & 1 << i ? SchemeSel : SchemeNorm]);
		drw_text(drw, x, 0, w, bh, lrpad / 2, tags[i], urg & 1 << i);
		
		if (occ & 1 << i)
			drw_rect(drw, x + boxs, boxs, boxw, boxw,
				m == selmon && selmon->sel && selmon->sel->tags & 1 << i,
				urg & 1 << i);

		x += w;
	}

	w = TEXTW(m->ltsymbol);
	drw_setscheme(drw, scheme[SchemeNorm]);
	x = drw_text(drw, x, 0, w, bh, lrpad / 2, m->ltsymbol, 0);

	if ((w = m->ww - tw - stw - x) > bh) {
		if (m->sel) {
			drw_setscheme(drw, scheme[m == selmon ? SchemeSelAlt : SchemeNorm]);

			if (!center_title)
				drw_text(drw, x, 0, w - 2 * sp, bh, lrpad / 2 + (m->sel->icon ? m->sel->icw + icon_spacing : 0), m->sel->name, 0);
			else {
				if (TEXTW(m->sel->name) > w) /* title is bigger than the width of the title rectangle, don't center */
					drw_text(drw, x, 0, w - 2 * sp, bh, lrpad / 2, m->sel->name, 0);
				else /* center window title */
					drw_text(drw, x, 0, w - 2 * sp, bh, (w - TEXTW(m->sel->name)) / 2, m->sel->name, 0);
			}

			if (m->sel->icon) {
				if (!center_title)
					drw_pic(drw, x + lrpad / 2, (bh - m->sel->ich) / 2, m->sel->icw, m->sel->ich, m->sel->icon);
				else
					/* only draw icon if it has room */
					if (TEXTW(m->sel->name) < w)
						drw_pic(drw, x + (w - TEXTW(m->sel->name)) / 2 - (m->sel->icw + 3), 
								(bh - m->sel->ich) / 2, m->sel->icw, m->sel->ich, m->sel->icon);

				if (m->sel->isfloating)
					drw_rect(drw, x + boxs, boxs, boxw, boxw, m->sel->isfixed, 0);
			}
		} else {
			drw_setscheme(drw, scheme[SchemeNorm]);
			drw_rect(drw, x, 0, w - 2 * sp, bh, 1, 1);
		}
	}

	drw_map(drw, m->barwin, 0, 0, m->ww - stw, bh);
}

void draw_bars(void)
{
	monitor_t *m;

	for (m = mons; m; m = m->next)
		draw_bar(m);
}

int draw_status_bar(monitor_t *m, int bh, char* stext) 
{
	int ret, i, w, x, len;
	short isCode = 0;
	char *text;
	char *p;

	len = strlen(stext) + 1 ;
	if (!(text = (char*) malloc(sizeof(char)*len)))
		die("malloc");

	p = text;
	memcpy(text, stext, len);

	/* compute width of the status text */
	w = 0;
	i = -1;
	while (text[++i]) {
		if (text[i] == '^') {
			if (!isCode) {
				isCode = 1;
				text[i] = '\0';
				w += TEXTW(text) - lrpad;
				text[i] = '^';
				if (text[++i] == 'f')
					w += atoi(text + ++i);
			} else {
				isCode = 0;
				text = text + i + 1;
				i = -1;
			}
		}
	}

	if (!isCode)
		w += TEXTW(text) - lrpad;
	else
		isCode = 0;

	text = p;
	w += 2; /* 1px padding on both sides */
	ret = m->ww - w;
	x = m->ww - w - get_sys_tray_width();

	drw_setscheme(drw, scheme[scm_len]);
	drw->scheme[ColFg] = scheme[SchemeNorm][ColFg];
	drw->scheme[ColBg] = scheme[SchemeNorm][ColBg];
	drw_rect(drw, x, 0, w, bh, 1, 1);
	x++;

	/* process status text */
	i = -1;
	while (text[++i]) {
		if (text[i] == '^' && !isCode) {
			isCode = 1;

			text[i] = '\0';
			w = TEXTW(text) - lrpad;
			drw_text(drw, x - 2 * sp, 0, w, bh, 0, text, 0);

			x += w;

			/* process code */
			while (text[++i] != '^') {
				if (text[i] == 'c') {
					char buf[8];
					memcpy(buf, (char*)text+i+1, 7);
					buf[7] = '\0';
					drw_clr_create(drw, &drw->scheme[ColFg], buf);
					i += 7;
				} else if (text[i] == 'b') {
					char buf[8];
					memcpy(buf, (char*)text+i+1, 7);
					buf[7] = '\0';
					drw_clr_create(drw, &drw->scheme[ColBg], buf);
					i += 7;
				} else if (text[i] == 'd') {
					drw->scheme[ColFg] = scheme[SchemeNorm][ColFg];
					drw->scheme[ColBg] = scheme[SchemeNorm][ColBg];
				} else if (text[i] == 'r') {
					int rx = atoi(text + ++i);
					while (text[++i] != ',');
					int ry = atoi(text + ++i);
					while (text[++i] != ',');
					int rw = atoi(text + ++i);
					while (text[++i] != ',');
					int rh = atoi(text + ++i);

					drw_rect(drw, rx + x, ry, rw, rh, 1, 0);
				} else if (text[i] == 'f') {
					x += atoi(text + ++i);
				}
			}

			text = text + i + 1;
			i=-1;
			isCode = 0;
		}
	}

	if (!isCode) {
		w = TEXTW(text) - lrpad;
		drw_text(drw, x - 2 * sp, 0, w, bh, 0, text, 0);
	}

	drw_setscheme(drw, scheme[SchemeNorm]);
	free(p);

	return ret;
}

void free_icon(client_t *c)
{
	if (c->icon) {
		XRenderFreePicture(dpy, c->icon);
		c->icon = None;
	}
}

Picture get_icon_prop(Window win, unsigned int *picw, unsigned int *pich)
{
	int format;
	unsigned long n, extra, *p = NULL;
	Atom real;

	if (XGetWindowProperty(dpy, win, net_atom[NetWMIcon], 0L, LONG_MAX, False, AnyPropertyType, 
						   &real, &format, &n, &extra, (unsigned char **)&p) != Success)
		return None; 

	if (n == 0 || format != 32) { 
		XFree(p); 
		return None; 
	}

	unsigned long *bstp = NULL;
	uint32_t w, h, sz;
	{
		unsigned long *i; const unsigned long *end = p + n;
		uint32_t bstd = UINT32_MAX, d, m;
		for (i = p; i < end - 1; i += sz) {
			if ((w = *i++) >= 16384 || (h = *i++) >= 16384) { 
				XFree(p); 
				return None; 
			}

			if ((sz = w * h) > end - i) break;
			
			if ((m = w > h ? w : h) >= icon_size && (d = m - icon_size) < bstd) { 
				bstd = d; bstp = i; 
			}
		}

		if (!bstp) {
			for (i = p; i < end - 1; i += sz) {
				if ((w = *i++) >= 16384 || (h = *i++) >= 16384) { 
					XFree(p); 
					return None; 
				}

				if ((sz = w * h) > end - i) break;

				if ((d = icon_size - (w > h ? w : h)) < bstd) { bstd = d; bstp = i; }
			}
		}

		if (!bstp) { 
			XFree(p); 
			return None; 
		}
	}

	if ((w = *(bstp - 2)) == 0 || (h = *(bstp - 1)) == 0) { 
		XFree(p); 
		return None; 
	}

	uint32_t icw, ich;
	if (w <= h) {
		ich = icon_size; icw = w * icon_size / h;
		if (icw == 0) icw = 1;
	}
	else {
		icw = icon_size; ich = h * icon_size / w;
		if (ich == 0) ich = 1;
	}

	*picw = icw; *pich = ich;

	uint32_t i, *bstp32 = (uint32_t *)bstp;
	for (sz = w * h, i = 0; i < sz; ++i) bstp32[i] = pre_alpha(bstp[i]);

	Picture ret = drw_picture_create_resized(drw, (char *)bstp, w, h, icw, ich);
	XFree(p);

	return ret;
}

unsigned int get_sys_tray_width()
{
	unsigned int w = 0;
	client_t *i;

	if(show_sys_tray)
		for(i = sys_tray->icons; i; w += i->w + sys_tray_spacing, i = i->next);

	return w ? w + sys_tray_spacing : 1;
}

void remove_sys_tray_icon(client_t *i)
{
	client_t **ii;

	if (!show_sys_tray || !i)
		return;

	for (ii = &sys_tray->icons; *ii && *ii != i; ii = &(*ii)->next);

	if (ii)
		*ii = i->next;

	free(i);
}

void resize_bar_win(monitor_t *m) 
{
	unsigned int w = m->ww;
	if (show_sys_tray && m == sys_tray_to_mon(m) && !sys_tray_on_left)
		w -= get_sys_tray_width();

	XMoveResizeWindow(dpy, m->barwin, m->wx + sp, m->by + vp, w - 2 * sp, bh);
}

void toggle_bar(const arg_t *arg)
{
	selmon->showbar = !selmon->showbar;
	update_bar_pos(selmon);
	resize_bar_win(selmon);
	if (show_sys_tray) {
		XWindowChanges wc;
		if (!selmon->showbar)
			wc.y = -bh;
		else if (selmon->showbar) {
			wc.y = 0;
			if (!selmon->topbar)
				wc.y = selmon->mh - bh;
		}

		XConfigureWindow(dpy, sys_tray->win, CWY, &wc);
	}

	arrange(selmon);
}

void update_bars(void)
{
	unsigned int w;
	monitor_t *m;
	
	XSetWindowAttributes wa = {
		.override_redirect = True,
		.background_pixmap = ParentRelative,
		.event_mask = ButtonPressMask|ExposureMask
	};

	XClassHint ch = {"dwm", "dwm"};
	for (m = mons; m; m = m->next) {
		if (m->barwin)
			continue;

		w = m->ww;
		if (show_sys_tray && m == sys_tray_to_mon(m))
			w -= get_sys_tray_width();

		m->barwin = XCreateWindow(dpy, root, m->wx + sp, m->by + vp, w - 2 * sp, bh, 0, DefaultDepth(dpy, screen),
				CopyFromParent, DefaultVisual(dpy, screen),
				CWOverrideRedirect|CWBackPixmap|CWEventMask, &wa);
		XDefineCursor(dpy, m->barwin, cursor[CurNormal]->cursor);

		if (show_sys_tray && m == sys_tray_to_mon(m))
			XMapRaised(dpy, sys_tray->win);

		XMapRaised(dpy, m->barwin);
		XSetClassHint(dpy, m->barwin, &ch);
	}
}

void update_bar_pos(monitor_t *m)
{
	m->wy = m->my;
	m->wh = m->mh;
	if (m->showbar) {
		m->wh = m->wh - vert_pad - bh;
		m->by = m->topbar ? m->wy : m->wy + m->wh + vert_pad;
		m->wy = m->topbar ? m->wy + bh + vp : m->wy;
	} else
		m->by = -bh - vp;
}

void update_icon(client_t *c)
{
	free_icon(c);
	c->icon = get_icon_prop(c->win, &c->icw, &c->ich);
}

void update_status(void)
{
	if (!get_text_prop(root, XA_WM_NAME, stext, sizeof(stext)))
		strcpy(stext, "dwm-"VERSION);

	draw_bar(selmon);
	update_sys_tray();
}

void update_sys_tray(void)
{
	XSetWindowAttributes wa;
	XWindowChanges wc;
	client_t *i;
	monitor_t *m = sys_tray_to_mon(NULL);
	unsigned int x = m->mx + m->mw - sp;
	unsigned int y = m->by + vp;
	unsigned int sw = TEXTW(stext) - lrpad + sys_tray_spacing;
	unsigned int w = 1;

	if (!show_sys_tray)
		return;

	if (sys_tray_on_left)
		x -= sw + lrpad / 2;

	if (!sys_tray) {
		/* init sys_tray */
		if (!(sys_tray = (sys_tray_t *)calloc(1, sizeof(sys_tray))))
			die("fatal: could not malloc() %u bytes\n", sizeof(sys_tray));

		sys_tray->win = XCreateSimpleWindow(dpy, root, x, y, w, bh, 0, 0, scheme[SchemeSel][ColBg].pixel);
		wa.event_mask        = ButtonPressMask | ExposureMask;
		wa.override_redirect = True;
		wa.background_pixel  = scheme[SchemeNorm][ColBg].pixel;
		XSelectInput(dpy, sys_tray->win, SubstructureNotifyMask);
		XChangeProperty(dpy, sys_tray->win, net_atom[NetSystemTrayOrientation], XA_CARDINAL, 32,
				PropModeReplace, (unsigned char *)&net_atom[NetSystemTrayOrientationHorz], 1);
		XChangeWindowAttributes(dpy, sys_tray->win, CWEventMask|CWOverrideRedirect|CWBackPixel, &wa);
		XMapRaised(dpy, sys_tray->win);
		XSetSelectionOwner(dpy, net_atom[NetSystemTray], sys_tray->win, CurrentTime);
		
		if (XGetSelectionOwner(dpy, net_atom[NetSystemTray]) == sys_tray->win) {
			send_event(root, xatom[Manager], StructureNotifyMask, CurrentTime, net_atom[NetSystemTray], sys_tray->win, 0, 0);
			XSync(dpy, False);
		}
		else {
			fprintf(stderr, "dwm: unable to obtain system tray.\n");
			free(sys_tray);
			sys_tray = NULL;
			return;
		}
	}

	for (w = 0, i = sys_tray->icons; i; i = i->next) {
		/* make sure the background color stays the same */
		wa.background_pixel  = scheme[SchemeNorm][ColBg].pixel;
		XChangeWindowAttributes(dpy, i->win, CWBackPixel, &wa);
		XMapRaised(dpy, i->win);
		w += sys_tray_spacing;
		i->x = w;
		XMoveResizeWindow(dpy, i->win, i->x, 0, i->w, i->h);
		w += i->w;

		if (i->mon != m)
			i->mon = m;
	}

	w = w ? w + sys_tray_spacing : 1;
	x -= w;
	XMoveResizeWindow(dpy, sys_tray->win, x, y, w, bh);
	wc.x = x; wc.y = y; wc.width = w; wc.height = bh;
	wc.stack_mode = Above; wc.sibling = m->barwin;
	XConfigureWindow(dpy, sys_tray->win, CWX|CWY|CWWidth|CWHeight|CWSibling|CWStackMode, &wc);
	XMapWindow(dpy, sys_tray->win);
	XMapSubwindows(dpy, sys_tray->win);
	/* redraw background */
	XSetForeground(dpy, drw->gc, scheme[SchemeNorm][ColBg].pixel);
	XFillRectangle(dpy, sys_tray->win, drw->gc, 0, 0, w, bh);
	XSync(dpy, False);
}

void update_sys_tray_icon_geom(client_t *i, int w, int h)
{
	if (i) {
		i->h = bh;
		if (w == h)
			i->w = bh;
		else if (h == bh)
			i->w = w;
		else
			i->w = (int) ((float)bh * ((float)w / (float)h));

		apply_size_hints(i, &(i->x), &(i->y), &(i->w), &(i->h), False);
		/* force icons into the sys_tray dimensions if they don't want to */
		if (i->h > bh) {
			if (i->w == i->h)
				i->w = bh;
			else
				i->w = (int) ((float)bh * ((float)i->w / (float)i->h));

			i->h = bh;
		}
	}
}

void update_sys_tray_icon_state(client_t *i, XPropertyEvent *ev)
{
	long flags;
	int code = 0;

	if (!show_sys_tray || !i || ev->atom != xatom[XembedInfo] ||
			!(flags = get_atom_prop(i, xatom[XembedInfo])))
		return;

	if (flags & XEMBED_MAPPED && !i->tags) {
		i->tags = 1;
		code = XEMBED_WINDOW_ACTIVATE;
		XMapRaised(dpy, i->win);
		set_client_state(i, NormalState);
	}
	else if (!(flags & XEMBED_MAPPED) && i->tags) {
		i->tags = 0;
		code = XEMBED_WINDOW_DEACTIVATE;
		XUnmapWindow(dpy, i->win);
		set_client_state(i, WithdrawnState);
	}
	else
		return;

	send_event(i->win, xatom[Xembed], StructureNotifyMask, CurrentTime, code, 0,
			sys_tray->win, XEMBED_EMBEDDED_VERSION);
}

void update_title(client_t *c)
{
	if (!get_text_prop(c->win, net_atom[NetWMName], c->name, sizeof c->name))
		get_text_prop(c->win, XA_WM_NAME, c->name, sizeof c->name);

	if (c->name[0] == '\0') /* hack to mark broken clients */
		strcpy(c->name, broken);
}

client_t *win_to_sys_tray_icon(Window w) 
{
	client_t *i = NULL;

	if (!show_sys_tray || !w)
		return i;

	for (i = sys_tray->icons; i && i->win != w; i = i->next);

	return i;
}
