#include "dwm.h"
#include "bar.h"
#include "helpers.h"
#include "layout.h"
#include "settings.h"
#include "window.h"

static void fibonacci(monitor_t *mon, int s);

void centered_master(monitor_t *mon)
{
	unsigned int i, n, h, mw, mx, my, oty, ety, tw;
	client_t *c;

	/* count number of clients in the selected monitor */
	for (n = 0, c = next_tiled(mon->clients); c; c = next_tiled(c->next), n++);
	if (n == 0)
		return;

	/* initialize areas */
	mw = mon->ww;
	mx = 0;
	my = 0;
	tw = mw;

	if (n > mon->nmaster) {
		/* go mfact box in the center if more than nmaster clients */
		mw = mon->nmaster ? mon->ww * mon->mfact : 0;
		tw = mon->ww - mw;

		if (n - mon->nmaster > 1) {
			/* only one client */
			mx = (mon->ww - mw) / 2;
			tw = (mon->ww - mw) / 2;
		}
	}

	oty = 0;
	ety = 0;
	for (i = 0, c = next_tiled(mon->clients); c; c = next_tiled(c->next), i++)
	if (i < mon->nmaster) {
		/* nmaster clients are stacked vertically, in the center
		 * of the screen */
		h = (mon->wh - my) / (MIN(n, mon->nmaster) - i);
		resize(c, mon->wx + mx, mon->wy + my, mw - (2*c->bw),
		       h - (2*c->bw), 0);
		my += HEIGHT(c);
	} else {
		/* stack clients are stacked vertically */
		if ((i - mon->nmaster) % 2 ) {
			h = (mon->wh - ety) / ( (1 + n - i) / 2);
			resize(c, mon->wx, mon->wy + ety, tw - (2*c->bw),
			       h - (2*c->bw), 0);
			ety += HEIGHT(c);
		} else {
			h = (mon->wh - oty) / ((1 + n - i) / 2);
			resize(c, mon->wx + mx + mw, mon->wy + oty,
			       tw - (2*c->bw), h - (2*c->bw), 0);
			oty += HEIGHT(c);
		}
	}
}

void centered_floating_master(monitor_t *mon)
{
	unsigned int i, n, w, mh, mw, mx, mxo, my, myo, tx;
	client_t *c;

	/* count number of clients in the selected monitor */
	for (n = 0, c = next_tiled(mon->clients); c; c = next_tiled(c->next), n++);
	if (n == 0)
		return;

	/* initialize nmaster area */
	if (n > mon->nmaster) {
		/* go mfact box in the center if more than nmaster clients */
		if (mon->ww > mon->wh) {
			mw = mon->nmaster ? mon->ww * mon->mfact : 0;
			mh = mon->nmaster ? mon->wh * 0.9 : 0;
		} else {
			mh = mon->nmaster ? mon->wh * mon->mfact : 0;
			mw = mon->nmaster ? mon->ww * 0.9 : 0;
		}
		mx = mxo = (mon->ww - mw) / 2;
		my = myo = (mon->wh - mh) / 2;
	} else {
		/* go fullscreen if all clients are in the master area */
		mh = mon->wh;
		mw = mon->ww;
		mx = mxo = 0;
		my = myo = 0;
	}

	for(i = tx = 0, c = next_tiled(mon->clients); c; c = next_tiled(c->next), i++)
	if (i < mon->nmaster) {
		/* nmaster clients are stacked horizontally, in the center
		 * of the screen */
		w = (mw + mxo - mx) / (MIN(n, mon->nmaster) - i);
		resize(c, mon->wx + mx, mon->wy + my, w - (2*c->bw),
		       mh - (2*c->bw), 0);
		mx += WIDTH(c);
	} else {
		/* stack clients are stacked horizontally */
		w = (mon->ww - tx) / (n - i);
		resize(c, mon->wx + tx, mon->wy, w - (2*c->bw),
		       mon->wh - (2*c->bw), 0);
		tx += WIDTH(c);
	}
}

void dwindle(monitor_t *mon) 
{
	fibonacci(mon, 1);
}

void spiral(monitor_t *mon) 
{
	fibonacci(mon, 0);
}

void fibonacci(monitor_t *mon, int s) 
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
	unsigned int n;
	client_t *nbc;

	/* grab number of clients on the selected monitor */
	for (n = 0, nbc = next_tiled(selmon->clients); nbc; nbc = next_tiled(nbc->next), n++);
	if (n == 0)
		return;

	if (selmon->showbar) {
		for(last_layout = (layout_t *)layouts; last_layout != selmon->lt[selmon->sellt]; last_layout++);
		set_layout(&((arg_t) { .v = &layouts[2] }));
	} else {
		set_layout(&((arg_t) { .v = last_layout }));
	}

	toggle_bar(arg);
}

void grid(monitor_t *mon)
{
	unsigned int n, cols, rows, cn, rn, i, cx, cy, cw, ch;
	client_t *c;

	for(n = 0, c = next_tiled(mon->clients); c; c = next_tiled(c->next))
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
	cw = cols ? mon->ww / cols : mon->ww;
	cn = 0; /* current column number */
	rn = 0; /* current row number */

	for(i = 0, c = next_tiled(mon->clients); c; i++, c = next_tiled(c->next)) 
	{
		if(i / rows + 1 > cols - n%cols)
			rows = n / cols + 1;

		ch = rows ? mon->wh / rows : mon->wh;
		cx = mon->wx + cn*cw;
		cy = mon->wy + rn*ch;
		resize(c, cx, cy, cw - 2 * c->bw, ch - 2 * c->bw, False);
		
		rn++;
		if(rn >= rows) 
		{
			rn = 0;
			cn++;
		}
	}
}

void monocle(monitor_t *mon)
{
	unsigned int n = 0;
	client_t *c;

	for (c = mon->clients; c; c = c->next)
		if (IS_VISIBLE(c))
			n++;

	if (n > 0) /* override layout symbol */
		snprintf(mon->ltsymbol, sizeof mon->ltsymbol, "[%d]", n);

	for (c = next_tiled(mon->clients); c; c = next_tiled(c->next))
		resize(c, mon->wx, mon->wy, mon->ww - 2 * c->bw, mon->wh - 2 * c->bw, 0);
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

void tile(monitor_t *mon)
{
	unsigned int i, n, h, r, g = 0, mw, my, ty;
	client_t *c;

	for (n = 0, c = next_tiled(mon->clients); c; c = next_tiled(c->next), n++);

	if (n == 0)
		return;

	if (n > mon->nmaster)
		mw = mon->nmaster ? (mon->ww - (g = gap_px)) * mon->mfact : 0;
	else
		mw = mon->ww;

	for (i = my = ty = 0, c = next_tiled(mon->clients); c; c = next_tiled(c->next), i++) {
		if (i < mon->nmaster) {
			r = MIN(n, mon->nmaster) - i;
			h = (mon->wh - my - gap_px * (r -1)) / r;
			resize(c, mon->wx, mon->wy + my, mw - (2*c->bw) + (n > 1 ? gap_px : 0), h - (2*c->bw), 0);
			
			if (my + HEIGHT(c) < mon->wh)
				my += HEIGHT(c) + gap_px;
		} else {
			r = n - i;
			h = (mon->wh - ty - gap_px * (r - 1)) / r;
			resize(c, mon->wx + mw + g, mon->wy + ty, mon->ww - mw - g - (2*c->bw), h - (2*c->bw), False);
			
			if (ty + HEIGHT(c) < mon->wh)
				ty += HEIGHT(c) + gap_px;
		}
	}
}
