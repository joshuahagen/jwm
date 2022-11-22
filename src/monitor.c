#include "dwm.h"
#include "helpers.h"
#include "monitor.h"
#include "settings.h"
#include "window.h"

void arrange_mon(monitor_t *m)
{
	strncpy(m->ltsymbol, m->lt[m->sellt]->symbol, sizeof m->ltsymbol);
	if (m->lt[m->sellt]->arrange)
		m->lt[m->sellt]->arrange(m);
}

void cleanup_mon(monitor_t *mon)
{
	monitor_t *m;

	if (mon == mons)
		mons = mons->next;
	else {
		for (m = mons; m && m->next != mon; m = m->next);
		m->next = mon->next;
	}

	XUnmapWindow(dpy, mon->barwin);
	XDestroyWindow(dpy, mon->barwin);
	free(mon);
}

monitor_t *create_mon(void)
{
	monitor_t *m;

	m = ecalloc(1, sizeof(monitor_t));
	m->tagset[0] = m->tagset[1] = 1;
	m->mfact = mfact;
	m->nmaster = nmaster;
	m->showbar = showbar;
	m->topbar = topbar;
	m->lt[0] = &layouts[0];
	m->lt[1] = &layouts[1 % LENGTH(layouts)];
	strncpy(m->ltsymbol, layouts[0].symbol, sizeof m->ltsymbol);

	return m;
}

monitor_t *dir_to_mon(int dir)
{
	monitor_t *m = NULL;

	if (dir > 0) {
		if (!(m = selmon->next))
			m = mons;
	} else if (selmon == mons)
		for (m = mons; m->next; m = m->next);
	else
		for (m = mons; m->next != selmon; m = m->next);

	return m;
}

void focus_mon(const arg_t *arg)
{
	monitor_t *m;

	if (!mons->next)
		return;
	if ((m = dir_to_mon(arg->i)) == selmon)
		return;

	unfocus(selmon->sel, 0);
	selmon = m;
	focus(NULL);
}

monitor_t *rect_to_mon(int x, int y, int w, int h)
{
	monitor_t *m, *r = selmon;
	int a, area = 0;

	for (m = mons; m; m = m->next)
		if ((a = INTERSECT(x, y, w, h, m)) > area) {
			area = a;
			r = m;
		}

	return r;
}

void send_mon(client_t *c, monitor_t *m)
{
	if (c->mon == m)
		return;

	unfocus(c, 1);
	detach(c);
	detach_stack(c);
	c->mon = m;
	c->tags = m->tagset[m->seltags]; /* assign tags of target monitor_t */
	attach(c);
	attach_stack(c);
	focus(NULL);
	arrange(NULL);
}

monitor_t *sys_tray_to_mon(monitor_t *m) 
{
	monitor_t *t;
	int i, n;
	if(!sys_tray_pinning) {
		if(!m)
			return selmon;

		return m == selmon ? m : NULL;
	}

	for(n = 1, t = mons; t && t->next; n++, t = t->next) ;
	for(i = 1, t = mons; t && t->next && i < sys_tray_pinning; i++, t = t->next) ;
	
	if(sys_tray_pinning_fail_first && n < sys_tray_pinning)
		return mons;
	
	return t;
}

void tag_mon(const arg_t *arg)
{
	if (!selmon->sel || !mons->next)
		return;

	send_mon(selmon->sel, dir_to_mon(arg->i));
}

monitor_t *win_to_mon(Window w)
{
	int x, y;
	client_t *c;
	monitor_t *m;

	if (w == root && get_root_ptr(&x, &y))
		return rect_to_mon(x, y, 1, 1);

	for (m = mons; m; m = m->next)
		if (w == m->barwin)
			return m;
	
	if ((c = win_to_client(w)))
		return c->mon;
	
	return selmon;
}