#include <stdbool.h>
#include <gtk/gtk.h>

GtkStatusIcon *status_icon;

void flogf(FILE* stream, const char *fmt1, const char* fmt2, ...) {
  char *fmt = malloc(strlen(fmt1) + strlen(fmt2) + 1);
  strcpy(fmt, fmt1);
  strcat(fmt, fmt2);
  va_list args;
  va_start(args, fmt2);
  vfprintf(stderr, fmt, args); 
  va_end(args);
  free(fmt);
}

#define LOG_INFO(fmt, ...) flogf(stdout, "[*] %s:%d ", fmt, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(fmt, ...) flogf(stderr, "[!] %s:%d ", fmt, __FILE__, __LINE__, __VA_ARGS__)

const char* pamixer_run(const char* args) {
  const char *bin = "pamixer ";
  char *cmd = malloc(strlen(bin) + strlen(args) + 1);
  strcpy(cmd, bin);
  strcat(cmd, args);
  FILE *f = popen(cmd, "r");
  if (f == NULL) {
    LOG_ERROR("popen failed\n", NULL);
    goto defer;
  }
  char buf[128] = "";
  while(fgets(buf, sizeof(buf), f) != NULL) {
  }
  if (pclose(f) != 0) {
    LOG_ERROR("%s failed\n", cmd);
    goto defer;
  }
defer:
  free(cmd);
  const char *result = buf;
  return result;
}

bool get_mute() {
  const char *result = pamixer_run("--get-mute");
  return strcmp(result, "true\n") == 0;
}

void set_mute(bool mute) {
  if (mute) {
    pamixer_run("--mute");
  } else  {
    pamixer_run("--unmute");
  } 
}

int get_volume() {
  const char* result = pamixer_run("--get-volume");
  return atoi(result);
}

void set_volume(int value) {
  char args[128];
  sprintf(args, "--set-volume %d", value);
  pamixer_run(args);
}

gboolean set_status_icon_icon(gpointer user_data) {
  bool mute = get_mute();
  if (mute) {
    gtk_status_icon_set_from_icon_name(status_icon, "audio-volume-muted");
    return true;
  }
  int volume = get_volume();
  if (volume <= 30) {
    gtk_status_icon_set_from_icon_name(status_icon, "audio-volume-low");
  } else if (volume <= 70) {
    gtk_status_icon_set_from_icon_name(status_icon, "audio-volume-medium");
  } else {
    gtk_status_icon_set_from_icon_name(status_icon, "audio-volume-high");
  }
  return true;
}

gchar* scale_format_value(GtkWidget *scale, gdouble value, gpointer user_data) {
  // hide value
  return g_strdup_printf("");
}

void scale_on_volume_changed(GtkWidget *scale, gpointer user_data) {
  int value = (int) gtk_range_get_value(GTK_RANGE(scale));
  set_volume(value);
}

void switch_on_mute_changed(GtkWidget *_switch, gboolean state, gpointer user_data) {
  gtk_switch_set_state(GTK_SWITCH(_switch), state);
  set_mute(state);
}

void destroy_menu(GtkWidget *window, GdkEvent *event, gpointer user_data) {
  gdouble x = event->button.x;
  gdouble y = event->button.y;
  gint w, h;
  gtk_window_get_size(GTK_WINDOW(window), &w, &h);
  if (x < 0 || y < 0 || x > w || y > h) {
    gdk_pointer_ungrab(event->button.time);
    gtk_widget_destroy(window);
  }
}

void make_menu(GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer user_data){
  GtkWidget *labelVolume = gtk_label_new("Volume:");
  gtk_widget_set_halign(labelVolume, GTK_ALIGN_START);
  GtkWidget *scaleVolume = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
  gtk_scale_set_value_pos(GTK_SCALE(scaleVolume), GTK_POS_RIGHT);
  gtk_range_set_value(GTK_RANGE(scaleVolume), get_volume());
  g_signal_connect(G_OBJECT(scaleVolume), "format-value", G_CALLBACK(scale_format_value), NULL);
  g_signal_connect(G_OBJECT(scaleVolume), "value-changed", G_CALLBACK(scale_on_volume_changed), NULL);
  GtkWidget *switchMute = gtk_switch_new();
  gtk_switch_set_state(GTK_SWITCH(switchMute), get_mute());
  gtk_widget_set_halign(switchMute, GTK_ALIGN_START);
  gtk_widget_set_valign(switchMute, GTK_ALIGN_CENTER);
  g_signal_connect(G_OBJECT(switchMute), "state-set", G_CALLBACK(switch_on_mute_changed), NULL);
  GtkWidget *hbox = gtk_hbox_new(false, 0);
  gtk_box_pack_start(GTK_BOX(hbox), scaleVolume, true, true, 0);
  gtk_box_pack_end(GTK_BOX(hbox), switchMute, false, false, 0);
  GtkWidget *vbox = gtk_vbox_new(false, 0);
  gtk_container_add(GTK_CONTAINER(vbox), labelVolume);
  gtk_container_add(GTK_CONTAINER(vbox), hbox);
  GtkWidget *window = gtk_window_new(GTK_WINDOW_POPUP);
  gtk_window_set_decorated(GTK_WINDOW(window), false);
  gtk_window_set_resizable(GTK_WINDOW(window), false);
  gtk_window_set_default_size(GTK_WINDOW(window), 200, -1);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);
  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
  gtk_widget_show_all(window);
  gdk_pointer_grab(gtk_widget_get_window(window), true, GDK_BUTTON_PRESS_MASK, NULL, NULL, activate_time);
  g_signal_connect(G_OBJECT(window), "button-press-event", G_CALLBACK(destroy_menu), NULL);
}

void make_status_icon(void){
  status_icon = gtk_status_icon_new();
  g_signal_connect(G_OBJECT(status_icon), "activate", G_CALLBACK(make_menu), NULL);
  g_signal_connect(G_OBJECT(status_icon), "popup-menu", G_CALLBACK(make_menu), NULL);
  gtk_status_icon_set_visible(status_icon, true);
}

int main(int argc, char **argv) {
  gtk_init(&argc, &argv);
  make_status_icon();
  gdk_threads_add_timeout(1000, set_status_icon_icon, NULL);
  gtk_main();
  return 0;
}
