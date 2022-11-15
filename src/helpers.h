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
#define SYSTEM_TRAY_REQUEST_DOCK    0

/* XEMBED messages */
#define XEMBED_EMBEDDED_NOTIFY      0
#define XEMBED_WINDOW_ACTIVATE      1
#define XEMBED_FOCUS_IN             4
#define XEMBED_MODALITY_ON          10
#define XEMBED_MAPPED               (1 << 0)
#define XEMBED_WINDOW_ACTIVATE      1
#define XEMBED_WINDOW_DEACTIVATE    2
#define VERSION_MAJOR               0
#define VERSION_MINOR               0
#define XEMBED_EMBEDDED_VERSION (VERSION_MAJOR << 16) | VERSION_MINOR

#endif