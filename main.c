#include <gtk/gtk.h>

gint get_volume() {
  FILE *f = popen("pamixer --get-volume", "r");
  if (f == NULL) {
    fprintf(stderr, "fail to open shell\n");
    return 0;
  }
  char buf[64];
  while (fgets(buf, sizeof(buf), f) != 0) {
    /*...*/
  }
  int exit_status = pclose(f);
  if (exit_status != 0) {
    fprintf(stderr, "is pamixer installed?\n");
    return 0;
  }
  return atoi(buf);
}

void set_volume(gint value) {
  char str[128];
  sprintf(str, "pamixer --set-volume %d", value);
  if (system(str) == 0) {
    fprintf(stdout, "volume changed to %d\n", value);
  } else {
    fprintf(stderr, "is pamixer installed?\n");
  }
}

void on_volume_changed(GtkWidget *scale, GdkEvent *event, gpointer user_data) {
  gint value = (gint) gtk_range_get_value(GTK_RANGE(scale));
  set_volume(value);
}

void on_brightness_changed(GtkWidget *scale, GdkEvent *event, gpointer user_data) {
  gint value = (gint) gtk_range_get_value(GTK_RANGE(scale));
  fprintf(stdout, "brightness changed to %d\n", value);
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
  gint volume = get_volume();
  GtkWidget *labelVolume = gtk_label_new("Volume:");
  gtk_widget_set_halign(labelVolume, GTK_ALIGN_START);
  GtkWidget *scaleVolume = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
  gtk_scale_set_value_pos(GTK_SCALE(scaleVolume), GTK_POS_RIGHT);
  gtk_range_set_value(GTK_RANGE(scaleVolume), volume);
  g_signal_connect(G_OBJECT(scaleVolume), "value-changed", G_CALLBACK(on_volume_changed), NULL);
  GtkWidget *labelBrightness = gtk_label_new("Brightness:");
  gtk_widget_set_halign(labelBrightness, GTK_ALIGN_START);
  GtkWidget *scaleBrightness = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
  gtk_scale_set_value_pos(GTK_SCALE(scaleBrightness), GTK_POS_RIGHT);
  g_signal_connect(G_OBJECT(scaleBrightness), "value-changed", G_CALLBACK(on_brightness_changed), NULL);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(vbox), labelVolume);
  gtk_container_add(GTK_CONTAINER(vbox), scaleVolume);
  gtk_container_add(GTK_CONTAINER(vbox), labelBrightness);
  gtk_container_add(GTK_CONTAINER(vbox), scaleBrightness);
  GtkWidget *window = gtk_window_new(GTK_WINDOW_POPUP);
  gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
  gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
  gtk_window_set_default_size(GTK_WINDOW(window), 200, -1);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);
  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
  gtk_widget_show_all(window);
  gdk_pointer_grab(gtk_widget_get_window(window), TRUE, GDK_BUTTON_PRESS_MASK, NULL, NULL, activate_time);
  g_signal_connect(G_OBJECT(window), "button-press-event", G_CALLBACK(destroy_menu), NULL);
}

static void make_tray_icon(void){
	GtkStatusIcon *tray_icon = gtk_status_icon_new();
	g_signal_connect(G_OBJECT(tray_icon), "activate", G_CALLBACK(make_menu), NULL);
	g_signal_connect(G_OBJECT(tray_icon), "popup-menu", G_CALLBACK(make_menu), NULL);
	gtk_status_icon_set_from_icon_name(tray_icon, GTK_STOCK_PREFERENCES);
	gtk_status_icon_set_tooltip_text(tray_icon, "Example Tray Icon");
	gtk_status_icon_set_visible(tray_icon, TRUE);
}

int main(int argc, char **argv) {
	gtk_init(&argc, &argv);
	make_tray_icon();
	gtk_main();
	return 0;
}
