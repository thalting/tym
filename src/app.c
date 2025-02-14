/**
 * app.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "app.h"

App* app = NULL;


int on_local_options(GApplication* gapp, GVariantDict* values, void* user_data);
int on_command_line(GApplication* app, GApplicationCommandLine* cli, void* user_data);

void app_init()
{
  df();
  app = g_new0(App, 1);
  app->meta = meta_init();
  app->ipc = ipc_init();
}

void app_close()
{
  df();
  for (GList* li = app->contexts; li != NULL; li = li->next) {
    Context* c = (Context*)li->data;
    context_close(c);
  }
  g_application_quit(app->gapp);
  g_object_unref(app->gapp);
  meta_close(app->meta);
  ipc_close(app->ipc);
  g_free(app);
}

static char* _get_dest_path_from_option(Option* option) {
  char* path = NULL;
  char* dest = option_get_str(option, "dest");
  if (dest) {
    path = g_strdup_printf(TYM_OBJECT_PATH_FMT_STR, dest);
  } else {
    char** env = g_get_environ();
    const char* dest_str = g_environ_getenv(env, "TYM_ID");
    if (!dest_str) {
      return NULL;
    }
    path = g_strdup_printf(TYM_OBJECT_PATH_FMT_STR, dest_str);
  }
  return path;
}

int app_start(int argc, char** argv)
{
  df();
  g_assert(!app->gapp);

  GError* error = NULL;
  if (error) {
    g_error("%s", error->message);
    g_error_free(error);
    return 1;
  }


  GOptionEntry* entries = meta_get_option_entries(app->meta);
  Option* option = option_init(entries);

  if (!option_parse(option, argc, argv)) {
    return 1;
  }

  if (option_get_bool(option, "version")) {
    g_print("version %s\n", PACKAGE_VERSION);
    return 0;
  }

  GApplicationFlags flags = G_APPLICATION_HANDLES_COMMAND_LINE | G_APPLICATION_SEND_ENVIRONMENT;
  char* app_id = TYM_APP_ID;
  if (option_get_bool(option, "isolated")) {
    flags |= G_APPLICATION_NON_UNIQUE;
    app_id = TYM_APP_ID_ISOLATED;
  }

  app->gapp = G_APPLICATION(gtk_application_new(app_id, flags));
  g_application_register(app->gapp, NULL, &error);

  char* signal_name = option_get_str(option, "signal");
  char* method_name = option_get_str(option, "call");
  if (signal_name || method_name) {
    GDBusConnection* conn = g_application_get_dbus_connection(app->gapp);
    char* path = _get_dest_path_from_option(option);
    if (!path) {
      g_warning("--dest is not provided and $TYM_ID is not set.");
      return 1;
    }

    char* param = option_get_str(option, "param");

    GVariant* params = param
      ? g_variant_new("(s)", param)
      : g_variant_new("()");

    /* process signal */
    if (signal_name) {
      g_dbus_connection_emit_signal(conn, NULL, path, TYM_APP_ID, signal_name, params, &error);
      g_print("Sent signal:%s to path:%s interface:%s\n", signal_name, path, TYM_APP_ID);
      g_free(signal_name);
      g_free(path);
      if (error) {
        g_error("%s", error->message);
        g_error_free(error);
      }
      return 0;
    }

    /* process method call */
    GVariant* result = g_dbus_connection_call_sync(
        conn,        // conn
        TYM_APP_ID,  // bus_name
        path,        // object_path
        TYM_APP_ID,  // interface_name
        method_name, // method_name
        params,      // parameters
        NULL,        // reply_type
        G_DBUS_CALL_FLAGS_NONE, // flags
        1000,        // timeout
        NULL,        // cancellable
        &error
    );
    g_print("Call method:%s on path:%s interface:%s\n", method_name, path, TYM_APP_ID);
    g_free(method_name);
    if (error) {
      g_warning("%s", error->message);
      g_error_free(error);
      return 1;
    }
    dd("result type:%s", g_variant_get_type_string(result));
    char* msg = g_variant_print(result, true);
    g_print("%s\n", msg);
    g_free(msg);
    return 0;
  }

  /* g_application_add_main_option_entries(app->gapp, entries); */

  g_signal_connect(app->gapp, "handle-local-options", G_CALLBACK(on_local_options), option);
  g_signal_connect(app->gapp, "command-line", G_CALLBACK(on_command_line), NULL);
  return g_application_run(app->gapp, argc, argv);
  return 0;
}

static int _contexts_sort_func(const void* a, const void* b)
{
  return ((Context*)a)->id - ((Context*)b)->id;
}

Context* app_spawn_context(Option* option)
{
  df();
  unsigned index = 0;
  int ordered_id = option_get_int(option, "id");
  if (ordered_id) {
    for (GList* li = app->contexts; li != NULL; li = li->next) {
      Context* c = (Context*)li->data;
      if (c->id == ordered_id) {
        context_log_warn(c, true, "id=%d has been already acquired.", ordered_id);
        return NULL;
      }
    }
    index = ordered_id;
  } else {
    for (GList* li = app->contexts; li != NULL; li = li->next) {
      Context* c = (Context*)li->data;
      /* scanning from 0 and if find first ctx that is not continus from 0, the index is new index. */
      if (c->id != index) {
        break;
      }
      index += 1;
    }
  }

  Context* context = context_init(index, option);
  app->contexts = g_list_insert_sorted(app->contexts, context, _contexts_sort_func);
  g_application_hold(app->gapp);

  context_log_message(context, false, "Started.");
  return context;
}

void app_quit_context(Context* context)
{
  df();
  g_application_release(app->gapp);
  GDBusConnection* conn = g_application_get_dbus_connection(app->gapp);
  g_dbus_connection_unregister_object(conn, context->registration_id);
  context_log_message(context, false, "Quit.");
  app->contexts = g_list_remove(app->contexts, context);
  context_close(context);
}

static void on_vte_drag_data_received(
  VteTerminal* vte,
  GdkDragContext* drag_context,
  int x,
  int y,
  GtkSelectionData* data,
  unsigned int info,
  unsigned int time,
  void* user_data)
{
  Context* context = (Context*)user_data;
  if (!data || gtk_selection_data_get_format(data) != 8) {
    return;
  }

  gchar** uris = g_uri_list_extract_uris(gtk_selection_data_get_data(data));
  if (!uris) {
    return;
  }

  GRegex* regex = g_regex_new("'", 0, 0, NULL);
  for (gchar** p = uris; *p; ++p) {
    gchar* file_path = g_filename_from_uri(*p, NULL, NULL);
    if (file_path) {
      bool result;
      if (!(hook_perform_drag(context->hook, context->lua, file_path, &result) && result)) {
        gchar* path_escaped = g_regex_replace(regex, file_path, -1, 0, "'\\\\''", 0, NULL);
        gchar* path_wrapped = g_strdup_printf("'%s' ", path_escaped);
        vte_terminal_feed_child(vte, path_wrapped, strlen(path_wrapped));
        g_free(path_escaped);
        g_free(path_wrapped);
      }
      g_free(file_path);
    }
  }
  g_regex_unref(regex);
}

static bool on_vte_key_press(GtkWidget* widget, GdkEventKey* event, void* user_data)
{
  Context* context = (Context*)user_data;

  unsigned mod = event->state & gtk_accelerator_get_default_mod_mask();
  unsigned key = gdk_keyval_to_lower(event->keyval);

  if (context_perform_keymap(context, key, mod)) {
    return true;
  }
  return false;
}

static bool on_vte_mouse_scroll(GtkWidget* widget, GdkEventScroll* e, void* user_data)
{
  Context* context = (Context*)user_data;
  bool result = false;
  if (hook_perform_scroll(context->hook, context->lua, e->delta_x, e->delta_y, e->x, e->y, &result) && result) {
    return true;
  }
  return false;
}

static void on_vte_child_exited(VteTerminal* vte, int status, void* user_data)
{
  df();
  Context* context = (Context*)user_data;
  gtk_window_close(context->layout.window);
  app_quit_context(context);
}

static void on_vte_title_changed(VteTerminal* vte, void* user_data)
{
  df();
  Context* context = (Context*)user_data;
  GtkWindow* window = context->layout.window;
  bool result = false;
  const char* title = vte_terminal_get_window_title(context->layout.vte);
  if (hook_perform_title(context->hook, context->lua, title, &result) && result) {
    return;
  }
  if (title) {
    gtk_window_set_title(window, title);
  }
}

static void on_vte_bell(VteTerminal* vte, void* user_data)
{
  df();
  Context* context = (Context*)user_data;
  bool result = false;
  if (hook_perform_bell(context->hook, context->lua, &result) && result) {
    return;
  }
  GtkWindow* window = context->layout.window;
  if (!gtk_window_is_active(window)) {
    gtk_window_set_urgency_hint(window, true);
  }
}

static bool on_vte_click(VteTerminal* vte, GdkEventButton* event, void* user_data)
{
  df();
  Context* context = (Context*)user_data;
  char* uri = NULL;
  if (context->layout.uri_tag >= 0) {
    uri = vte_terminal_match_check_event(vte, (GdkEvent*)event, NULL);
  }
  bool result = false;
  if (hook_perform_clicked(context->hook, context->lua, event->button, uri, &result) && result) {
    return true;
  }
  if (uri) {
    context_launch_uri(context, uri);
    return true;
  }
  return false;
}

static void on_vte_selection_changed(GtkWidget* widget, void* user_data)
{
  df();
  Context* context = (Context*)user_data;
  if (!vte_terminal_get_has_selection(context->layout.vte)) {
    hook_perform_unselected(context->hook, context->lua);
    return;
  }
  GtkClipboard* cb = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
  char* text = gtk_clipboard_wait_for_text(cb);
  hook_perform_selected(context->hook, context->lua, text);
}


static gboolean on_window_close(GtkWidget* widget, cairo_t* cr, void* user_data)
{
  df();
  // close context in child-exited handler
  return true;
}

static bool on_window_focus_in(GtkWindow* window, GdkEvent* event, void* user_data)
{
  Context* context = (Context*)user_data;
  gtk_window_set_urgency_hint(window, false);
  hook_perform_activated(context->hook, context->lua);
  return false;
}

static bool on_window_focus_out(GtkWindow* window, GdkEvent* event, void* user_data)
{
  Context* context = (Context*)user_data;
  hook_perform_deactivated(context->hook, context->lua);
  return false;
}

static gboolean on_window_draw(GtkWidget* widget, cairo_t* cr, void* user_data)
{
  Context* context = (Context*)user_data;
  const char* value = context_get_str(context, "color_window_background");
  if (is_none(value)) {
    return false;
  }
  GdkRGBA color = {};
  if (gdk_rgba_parse(&color, value)) {
    if (context->layout.alpha_supported) {
      cairo_set_source_rgba(cr, color.red, color.green, color.blue, color.alpha);
    } else {
      cairo_set_source_rgb(cr, color.red, color.green, color.blue);
    }
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
  }
  return false;
}

void on_dbus_signal(
  GDBusConnection* conn,
  const char* sender_name,
  const char* object_path,
  const char* interface_name,
  const char* signal_name,
  GVariant* params,
  void* user_data)
{
  Context* context = (Context*)user_data;
  dd("DBus signal received");
  dd("\tcontext id: %d", context->id);
  dd("\tsender_name: %s", sender_name);
  dd("\tobject_path: %s", object_path);
  dd("\tinterface_name: %s", interface_name);
  dd("\tsignal_name: %s", signal_name);

  if (ipc_signal_perform(app->ipc, context, signal_name, params)) {
    context_log_message(context, false, "Signal received:`%s` object_path:`%s`", signal_name, object_path);
    return;
  }

  context_log_warn(context, true, "Unsupported signal: `%s`", signal_name);
}

void on_dbus_call_method(
    GDBusConnection* conn,
    const gchar* sender_name,
    const gchar* object_path,
    const gchar* interface_name,
    const gchar* method_name,
    GVariant* params,
    GDBusMethodInvocation* invocation,
    gpointer user_data)
{
  Context* context = (Context*)user_data;
  dd("DBus method call");
  dd("\tcontext id: %d", context->id);
  dd("\tsender_name: %s", sender_name);
  dd("\tobject_path: %s", object_path);
  dd("\tinterface_name: %s", interface_name);
  dd("\tmethod_name: %s", method_name);

  if (ipc_method_perform(app->ipc, context, method_name, params, invocation)) {
    context_log_message(context, false, "Method call:`%s` object_path:`%s`", method_name, object_path);
    g_dbus_connection_flush(conn, NULL, NULL, NULL);
    return;
  }

  context_log_warn(context, true, "Unsupported method call:`%s`", method_name);
  GError* error = g_error_new(
      g_quark_from_static_string("TymInvalidMethodCall"),
      TYM_ERROR_INVALID_METHOD_CALL,
      "Unsupported method call: %s",
      method_name);

  g_dbus_method_invocation_return_gerror(invocation, error);
  g_dbus_connection_flush(conn, NULL, NULL, NULL);
}

int on_local_options(GApplication* gapp, GVariantDict* values, void* user_data)
{
  df();
  return -1;
}

static bool _subscribe_dbus(Context* context)
{
  df();
  GError* error = NULL;

  const char* app_id = g_application_get_application_id(app->gapp);
  GDBusConnection* conn = g_application_get_dbus_connection(app->gapp);

  g_dbus_connection_signal_subscribe(
    conn,
    NULL,        // sender
    app_id,      // interface_name
    NULL,        // member
    context->object_path, // object_path
    NULL,        // arg0
    G_DBUS_SIGNAL_FLAGS_NONE,
    on_dbus_signal,
    context,
    NULL         // user data free func
  );

  GDBusInterfaceVTable vtable = {
    on_dbus_call_method,
    NULL,
    NULL,
  };

  static const char introspection_xml[] =
    "<node>"
    "  <interface name='" TYM_APP_ID "'>"
    "    <method name='echo'>"
    "      <arg type='s' direction='in'/>"
    "      <arg type='s' direction='out'/>"
    "    </method>"
    "    <method name='get_ids'>"
    "      <arg type='ai' direction='out'/>"
    "    </method>"
    "    <method name='eval'>"
    "      <arg type='s' direction='in'/>"
    "      <arg type='s' direction='out'/>"
    "    </method>"
    "    <method name='exec'>"
    "      <arg type='s' direction='in'/>"
    "    </method>"
    "    <method name='exec_file'>"
    "      <arg type='s' direction='in'/>"
    "    </method>"
    "  </interface>"
    "</node>";

  GDBusNodeInfo* introspection_data = g_dbus_node_info_new_for_xml(introspection_xml, &error);
  if (error) {
    g_error("%s", error->message);
    g_error_free(error);
    app_quit_context(context);
    return false;
  }

  context_log_message(context, false, "DBus: object_path='%s' interface_name:'%s'", context->object_path, app_id);
  context->registration_id = g_dbus_connection_register_object(
      conn,
      context->object_path,
      introspection_data->interfaces[0], // interface_info,
      &vtable, // vtable
      context, // user_data,
      NULL,    // user_data_free_func,
      &error   // error
  );
  if (context->registration_id <= 0) {
    context_log_warn(context, true, "Could not subscribe DBus with path:%s", context->object_path);
  }

  return true;
}

#ifdef TYM_USE_VTE_SPAWN_ASYNC
static void on_vte_spawn(VteTerminal* vte, GPid pid, GError* error, void* user_data)
{
  Context* context = (Context*)user_data;
  context->initialized = true;
  if (error) {
    g_warning("vte-spawn error: %s", error->message);
    /* g_error_free(error); */
    gtk_window_close(context->layout.window);
    app_quit_context(context);
    /* dd("%d", gtk_application_new); */
    return;
  }
}
#endif

int on_command_line(GApplication* gapp, GApplicationCommandLine* cli, void* user_data)
{
  df();
  GError* error = NULL;

  int argc = -1;
  char** argv = g_application_command_line_get_arguments(cli, &argc);

  Option* option = option_init(meta_get_option_entries(app->meta));
  if (!option_parse(option, argc, argv)){
    return 1;
  };

  Context* context = app_spawn_context(option);
  if (!context) {
    return 1;
  }

  context_load_device(context);
  context_load_lua_context(context);

  context_build_layout(context);
  context_restore_default(context);
  context_load_theme(context);
  context_load_config(context);
  context_override_by_option(context);

  VteTerminal* vte = context->layout.vte;
  GtkWindow* window = context->layout.window;

  GtkTargetEntry drop_types[] = {
    {"text/uri-list", 0, 0}
  };
  gtk_drag_dest_set(GTK_WIDGET(vte), GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_DROP, drop_types, G_N_ELEMENTS(drop_types), GDK_ACTION_COPY);

  context_signal_connect(context, vte, "drag-data-received", G_CALLBACK(on_vte_drag_data_received));
  context_signal_connect(context, vte, "key-press-event", G_CALLBACK(on_vte_key_press));
  context_signal_connect(context, vte, "scroll-event", G_CALLBACK(on_vte_mouse_scroll));
  context_signal_connect(context, vte, "child-exited", G_CALLBACK(on_vte_child_exited));
  context_signal_connect(context, vte, "window-title-changed", G_CALLBACK(on_vte_title_changed));
  context_signal_connect(context, vte, "bell", G_CALLBACK(on_vte_bell));
  context_signal_connect(context, vte, "button-press-event", G_CALLBACK(on_vte_click));
  context_signal_connect(context, vte, "selection-changed", G_CALLBACK(on_vte_selection_changed));
  context_signal_connect(context, window, "destroy", G_CALLBACK(on_window_close));
  context_signal_connect(context, window, "focus-in-event", G_CALLBACK(on_window_focus_in));
  context_signal_connect(context, window, "focus-out-event", G_CALLBACK(on_window_focus_out));
  context_signal_connect(context, window, "draw", G_CALLBACK(on_window_draw));

  if (app->is_isolated) {
    g_message("This process is isolated so never listen to D-Bus signal/method call.");
  } else {
    _subscribe_dbus(context);
  }

  const char* shell_line = context_get_str(context, "shell");

  char** shell_argv = NULL;
  g_shell_parse_argv(shell_line, NULL, &shell_argv, &error);
  if (error) {
    g_warning("Parse error: %s", error->message);
    g_error_free(error);
    app_quit_context(context);
    return 0;
  }

  const char* const* env = g_application_command_line_get_environ(cli);
  char** shell_env = g_new0(char*, g_strv_length((char**)env) + 1);
  int i = 0;
  while (env[i]) {
    shell_env[i] = g_strdup(env[i]);
    i += 1;
  }
  shell_env = g_environ_setenv(shell_env, "TERM", context_get_str(context, "term"), true);
  char* id_str = g_strdup_printf("%i", context->id);
  shell_env = g_environ_setenv(shell_env, "TYM_ID", id_str, true);
  g_free(id_str);

#ifdef TYM_USE_VTE_SPAWN_ASYNC
  vte_terminal_spawn_async(
    vte,                 // terminal
    VTE_PTY_DEFAULT,     // pty flag
    NULL,                // working directory
    shell_argv,          // argv
    shell_env,           // envv
    G_SPAWN_SEARCH_PATH, // spawn_flags
    NULL,                // child_setup
    NULL,                // child_setup_data
    NULL,                // child_setup_data_destroy
    5000,                // timeout
    NULL,                // cancel callback
    on_vte_spawn,        // callback
    context              // user_data
  );
#else
  GPid child_pid;
  vte_terminal_spawn_sync(
    vte,
    VTE_PTY_DEFAULT,
    NULL,
    shell_argv,
    shell_env,
    G_SPAWN_SEARCH_PATH,
    NULL,
    NULL,
    &child_pid,
    NULL,
    &error
  );

  if (error) {
    g_strfreev(shell_env);
    g_strfreev(shell_argv);
    g_error("%s", error->message);
    g_error_free(error);
    app_quit_context(context);
    return;
  }
#endif

  g_strfreev(shell_env);
  g_strfreev(shell_argv);
  gtk_widget_grab_focus(GTK_WIDGET(vte));
  gtk_widget_show_all(GTK_WIDGET(context->layout.window));
  return 0;
}
