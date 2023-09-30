#include <gtk/gtk.h>
#include <alsa/asoundlib.h>

struct Sound {
  snd_mixer_t *mixer;
  snd_mixer_elem_t *mixer_elem;
  snd_mixer_selem_id_t *sid;
  long min, max;
} sound;

gint get_volume() {
  long vol;
  snd_mixer_selem_get_playback_volume(sound.mixer_elem, 0, &vol);
  long value = vol * 100 / sound.max;
  return value;
}

void set_volume(gint value) {
  snd_mixer_selem_set_playback_volume_all(sound.mixer_elem, value * sound.max / 100);
}

void set_status_icon_icon(GtkStatusIcon *status_icon, gint volume) {
  if (volume == 0) {
    gtk_status_icon_set_from_icon_name(status_icon, "audio-volume-muted");
  } else if (volume <= 33) {
    gtk_status_icon_set_from_icon_name(status_icon, "audio-volume-low");
  } else if (volume <= 66) {
    gtk_status_icon_set_from_icon_name(status_icon, "audio-volume-medium");
  } else if (volume <= 100) {
    gtk_status_icon_set_from_icon_name(status_icon, "audio-volume-high");
  }
}

gchar* format_value(GtkWidget *scale, gdouble value, gpointer user_data) {
  // hide value
  return g_strdup_printf("");
}

void on_volume_changed(GtkWidget *scale, gpointer user_data) {
  gint value = (gint) gtk_range_get_value(GTK_RANGE(scale));
  set_volume(value);
  set_status_icon_icon(GTK_STATUS_ICON(user_data), get_volume());
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
  g_signal_connect(G_OBJECT(scaleVolume), "format-value", G_CALLBACK(format_value), NULL);
  g_signal_connect(G_OBJECT(scaleVolume), "value-changed", G_CALLBACK(on_volume_changed), status_icon);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(vbox), labelVolume);
  gtk_container_add(GTK_CONTAINER(vbox), scaleVolume);
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

void make_status_icon(void){
  GtkStatusIcon *status_icon = gtk_status_icon_new();
  g_signal_connect(G_OBJECT(status_icon), "activate", G_CALLBACK(make_menu), NULL);
  g_signal_connect(G_OBJECT(status_icon), "popup-menu", G_CALLBACK(make_menu), NULL);
  set_status_icon_icon(status_icon, get_volume());
  gtk_status_icon_set_visible(status_icon, TRUE);
}

void init_sound(void) {
  snd_mixer_t *mixer;
  snd_mixer_open(&mixer, 0);
  snd_mixer_attach(mixer, "default");
  snd_mixer_selem_register(mixer, NULL, NULL);
  snd_mixer_load(mixer);
  snd_mixer_selem_id_t *sid;
  snd_mixer_selem_id_alloca(&sid);
  snd_mixer_selem_id_set_index(sid, 0);
  snd_mixer_selem_id_set_name(sid, "Master");
  snd_mixer_elem_t* mixer_elem = snd_mixer_find_selem(mixer, sid);
  long min, max;
  snd_mixer_selem_get_playback_volume_range(mixer_elem, &min, &max);
  sound.mixer = mixer;
  sound.sid = sid;
  sound.mixer_elem = mixer_elem;
  sound.min = min;
  sound.max = max;
}

void deinit_sound(void) {
  snd_mixer_close(sound.mixer);
}

int main(int argc, char **argv) {
  gtk_init(&argc, &argv);
  init_sound();
  make_status_icon();
  gtk_main();
  deinit_sound();
  return 0;
}
