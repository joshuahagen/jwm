#ifndef HELPERS_H
#define HELPERS_H

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
#define TAG_MASK                 ((1 << tag_len) - 1)

#endif