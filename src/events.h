#ifndef EVENTS_H
#define EVENTS_H

void button_press(XEvent *event);
void client_message(XEvent *event);
void configure_notify(XEvent *event);
void configure_request(XEvent *event);
void destroy_notify(XEvent *event);
void enter_notify(XEvent *event);
void expose(XEvent *event);
void focus_in(XEvent *event);
void key_press(XEvent *event);
void mapping_notify(XEvent *event);
void map_request(XEvent *event);
void motion_notify(XEvent *event);
void property_notify(XEvent *event);
void resize_request(XEvent *event);
void unmap_notify(XEvent *event);

#endif