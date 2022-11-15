#include "dwm.h"
#include "bar.h"
#include "helpers.h"
#include "layout.h"
#include "settings.h"
#include "window.h"

static void fibonacci(monitor_t *mon, int s);

void dwindle(monitor_t *mon) 
{
	fibonacci(mon, 1);
}

void spiral(monitor_t *mon) 
{
	fibonacci(mon, 0);
}

static void fibonacci(monitor_t *mon, int s) 
{
	unsigned int i, n, nx, ny, nw, nh;
	client_t *c;

	for(n = 0, c = next_tiled(mon->clients); c; c = next_tiled(c->next), n++);
	if(n == 0)
		return;
	
	nx = mon->wx;
	ny = 0;
	nw = mon->ww;
	nh = mon->wh;
	
	for(i = 0, c = next_tiled(mon->clients); c; c = next_tiled(c->next)) {
		if((i % 2 && nh / 2 > 2 * c->bw)
		   || (!(i % 2) && nw / 2 > 2 * c->bw)) {
			if(i < n - 1) {
				if(i % 2)
					nh /= 2;
				else
					nw /= 2;
				if((i % 4) == 2 && !s)
					nx += nw;
				else if((i % 4) == 3 && !s)
					ny += nh;
			}
			if((i % 4) == 0) {
				if(s)
					ny += nh;
				else
					ny -= nh;
			}
			else if((i % 4) == 1)
				nx += nw;
			else if((i % 4) == 2)
				ny += nh;
			else if((i % 4) == 3) {
				if(s)
					nx += nw;
				else
					nx -= nw;
			}
			if(i == 0)
			{
				if(n != 1)
					nw = mon->ww * mon->mfact;
				ny = mon->wy;
			}
			else if(i == 1)
				nw = mon->ww - nw;
			i++;
		}
		resize(c, nx, ny, nw - 2 * c->bw, nh - 2 * c->bw, False);
	}
}

void fullscreen(const arg_t *arg)
{
	static layout_t *last_layout;

	if (selmon->showbar) {
		for(last_layout = (layout_t *)layouts; last_layout != selmon->lt[selmon->sellt]; last_layout++);
		set_layout(&((arg_t) { .v = &layouts[2] }));
	} else {
		set_layout(&((arg_t) { .v = last_layout }));
	}
	toggle_bar(arg);
}

void grid(monitor_t *m)
{
	unsigned int n, cols, rows, cn, rn, i, cx, cy, cw, ch;
	client_t *c;

	for(n = 0, c = next_tiled(m->clients); c; c = next_tiled(c->next))
		n++;
	if(n == 0)
		return;

	/* grid dimensions */
	for(cols = 0; cols <= n/2; cols++)
		if(cols * cols >= n)
			break;
	if(n == 5) /* set layout against the general calculation: not 1:2:2, but 2:3 */
		cols = 2;
	rows = n/cols;

	/* window geometry */
	cw = cols ? m->ww / cols : m->ww;
	cn = 0; /* current column number */
	rn = 0; /* current row number */
	for(i = 0, c = next_tiled(m->clients); c; i++, c = next_tiled(c->next)) 
	{
		if(i / rows + 1 > cols - n%cols)
			rows = n / cols + 1;
		ch = rows ? m->wh / rows : m->wh;
		cx = m->wx + cn*cw;
		cy = m->wy + rn*ch;
		resize(c, cx, cy, cw - 2 * c->bw, ch - 2 * c->bw, False);
		rn++;
		if(rn >= rows) 
		{
			rn = 0;
			cn++;
		}
	}
}

void monocle(monitor_t *m)
{
	unsigned int n = 0;
	client_t *c;

	for (c = m->clients; c; c = c->next)
		if (IS_VISIBLE(c))
			n++;
	if (n > 0) /* override layout symbol */
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%d]", n);
	for (c = next_tiled(m->clients); c; c = next_tiled(c->next))
		resize(c, m->wx, m->wy, m->ww - 2 * c->bw, m->wh - 2 * c->bw, 0);
}

void set_layout(const arg_t *arg)
{
	if (!arg || !arg->v || arg->v != selmon->lt[selmon->sellt])
		selmon->sellt ^= 1;
	if (arg && arg->v)
		selmon->lt[selmon->sellt] = (layout_t *)arg->v;
	strncpy(selmon->ltsymbol, selmon->lt[selmon->sellt]->symbol, sizeof selmon->ltsymbol);
	if (selmon->sel)
		arrange(selmon);
	else
		draw_bar(selmon);
}

void tile(monitor_t *m)
{
	unsigned int i, n, h, r, g = 0, mw, my, ty;
	client_t *c;

	for (n = 0, c = next_tiled(m->clients); c; c = next_tiled(c->next), n++);
	if (n == 0)
		return;

	if (n > m->nmaster)
		mw = m->nmaster ? (m->ww - (g = gap_px)) * m->mfact : 0;
	else
		mw = m->ww;
	for (i = my = ty = 0, c = next_tiled(m->clients); c; c = next_tiled(c->next), i++)
		if (i < m->nmaster) {
			r = MIN(n, m->nmaster) - i;
			h = (m->wh - my - gap_px * (r -1)) / r;
			resize(c, m->wx, m->wy + my, mw - (2*c->bw) + (n > 1 ? gap_px : 0), h - (2*c->bw), 0);
			if (my + HEIGHT(c) < m->wh)
				my += HEIGHT(c) + gap_px;
		} else {
			r = n - i;
			h = (m->wh - ty - gap_px * (r - 1)) / r;
			resize(c, m->wx + mw + g, m->wy + ty, m->ww - mw - g - (2*c->bw), h - (2*c->bw), False);
			if (ty + HEIGHT(c) < m->wh)
				ty += HEIGHT(c) + gap_px;
		}
}
