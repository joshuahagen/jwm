/* See LICENSE file for copyright and license details. */

/* Drawable abstraction */
drw_t *drw_create(Display *dpy, int screen, Window win, unsigned int w, unsigned int h, Visual *visual, unsigned int depth, Colormap cmap);
void drw_resize(drw_t *drw, unsigned int w, unsigned int h);
void drw_free(drw_t *drw);

/* Fnt abstraction */
fnt_t *drw_fontset_create(drw_t* drw, const char *fonts[], size_t fontcount);
void drw_fontset_free(fnt_t* set);
unsigned int drw_fontset_getwidth(drw_t *drw, const char *text);
unsigned int drw_fontset_getwidth_clamp(drw_t *drw, const char *text, unsigned int n);
void drw_font_getexts(fnt_t *font, const char *text, unsigned int len, unsigned int *w, unsigned int *h);

/* Colorscheme abstraction */
void drw_clr_create(drw_t *drw, clr_t *dest, const char *clrname, unsigned int alpha);
clr_t *drw_scm_create(drw_t *drw, const char *clrnames[], const unsigned int alphas[], size_t clrcount);

/* Cursor abstraction */
cur_t *drw_cur_create(drw_t *drw, int shape);
void drw_cur_free(drw_t *drw, cur_t *cursor);

/* Drawing context manipulation */
void drw_setfontset(drw_t *drw, fnt_t *set);
void drw_setscheme(drw_t *drw, clr_t *scm);

/* Picture drawing functions */
Picture drw_picture_create_resized(drw_t *drw, char *src, unsigned int src_w, unsigned int src_h, unsigned int dst_w, unsigned int dst_h);

/* Drawing functions */
void drw_rect(drw_t *drw, int x, int y, unsigned int w, unsigned int h, int filled, int invert);
int drw_text(drw_t *drw, int x, int y, unsigned int w, unsigned int h, unsigned int lpad, const char *text, int invert);
void drw_pic(drw_t *drw, int x, int y, unsigned int w, unsigned int h, Picture pic);

/* Map functions */
void drw_map(drw_t *drw, Window win, int x, int y, unsigned int w, unsigned int h);
